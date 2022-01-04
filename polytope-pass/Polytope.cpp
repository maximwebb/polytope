#include <iostream>
#include "Polytope.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

using namespace llvm;

bool PolytopePass::isPerfectNest(Loop& L, LoopInfo& LI, ScalarEvolution& SE) {
	// Will have to be changed for handing cases like for (int j = i; ...)
	PHINode* IV = L.getInductionVariable(SE);
	if (!IV) {
		return false;
	}
	if (L.getSubLoops().size() != 1) {
		return false;
	}

	auto* IL = L.getSubLoops().at(0);

	/* Truncate max nesting at depth 2 - will be changed later */
	if (!IL->getSubLoops().empty()) {
		return false;
	}
	if (IL->getExitBlock() == L.getLoopLatch() || IL->getExitBlock()->getNextNode() == L.getLoopLatch()) {
		// Second condition to test for any loop preheaders
		if (L.getHeader()->getNextNode() == IL->getHeader() ||
			L.getHeader()->getNextNode() == IL->getLoopPreheader()) {
			return true;
		}
		dbgs() << "Preceded by other statements...\n\n";
		return false;
	}
	dbgs() << "Succeeded by other statements...\n\n";
	return false;
}

/* Recursively test if a value is an affine function of induction variables */
bool PolytopePass::isAffineValue(Value* V, const std::vector<Value*>& Visited = {}) {
	/* Test for recursively defined variable in IR */
	if (std::find(Visited.begin(), Visited.end(), V) != Visited.end()) {
		return true;
	}
	if (isa<Constant>(V)) {
		return true;
	}
	/* LLVM requires that variables must dominate their uses, so for (int i = i; ...) will never occur */
	if (std::find_if(IVList.begin(), IVList.end(), [V](IVInfo& info){return info.IV == V;}) != IVList.end()) {
		return true;
	}
	if (isa<AddOperator>(V)) {
		auto* AddInstr = dyn_cast<AddOperator>(V);
		return isAffineValue(AddInstr->getOperand(0), Visited) && isAffineValue(AddInstr->getOperand(1), Visited);
	}
	if (isa<SubOperator>(V)) {
		auto* SubInstr = dyn_cast<SubOperator>(V);
		return isAffineValue(SubInstr->getOperand(0), Visited) && isAffineValue(SubInstr->getOperand(1), Visited);
	}
	if (isa<MulOperator>(V)) {
		auto* MulInstr = dyn_cast<MulOperator>(V);
		if (isa<Constant>(MulInstr->getOperand(0))) {
			return isAffineValue(MulInstr->getOperand(1), Visited);
		} else if (isa<Constant>(MulInstr->getOperand(1))) {
			return isAffineValue(MulInstr->getOperand(0), Visited);
		}
		return false;
	}
	if (isa<ShlOperator>(V)) {
		auto* ShlInstr = dyn_cast<ShlOperator>(V);
		if (isa<Constant>(ShlInstr->getOperand(1))) {
			return isAffineValue(ShlInstr->getOperand(0), Visited);
		}
		return false;
	}
	if (isa<CallInst>(V)) {
		auto* FuncInstr = dyn_cast<CallInst>(V);
		if (FuncInstr->getCalledFunction()->getName() == "llvm.smax.i32") {
			return isAffineValue(FuncInstr->getArgOperand(0), Visited) && isAffineValue(FuncInstr->getArgOperand(1), Visited);
		} else {
			return false;
		}
	}
	if (isa<PHINode>(V)) {
		auto* PhiInstr = dyn_cast<PHINode>(V);
		std::vector<Value*> NewVisited;
		NewVisited.insert(NewVisited.begin(), Visited.begin(), Visited.end());
		NewVisited.push_back(PhiInstr);
		for (auto& Op : PhiInstr->operands()) {
			if (!isAffineValue(Op.get(), NewVisited)) {
				return false;
			}
		}
		return true;
	}
	return false;
}

/* Verifies that induction variables and array accesses are affine functions */
/* This will initially only check for a subset of affine functions, eg. constant bounds */
bool PolytopePass::isAffineLoop(Loop& L, LoopStandardAnalysisResults& AR) {
	for (auto& IV : IVList) {
		if (!isAffineValue(IV.init)) {
			return false;
		}
		if (!isAffineValue(IV.tripCount)) {
			return false;
		}
	}
	return true;
}

bool PolytopePass::runAnalysis(Loop& L, LoopStandardAnalysisResults& AR) {
	if (!isPerfectNest(L, AR.LI, AR.SE)) {
		return false;
	}
	IVInfo OuterIV;
	IVInfo InnerIV;

	OuterIV.loop = &L;
	auto* IL = L.getSubLoops().at(0);
	if (IL == nullptr) {
		return false;
	}
	InnerIV.loop = IL;

	OuterIV.IV = OuterIV.loop->getInductionVariable(AR.SE);
	InnerIV.IV = InnerIV.loop->getInductionVariable(AR.SE);

	auto InnerBounds = InnerIV.loop->getBounds(AR.SE);
	auto OuterBounds = OuterIV.loop->getBounds(AR.SE);

	if (!InnerBounds.hasValue() || !OuterBounds.hasValue()) {
		return false;
	}

	InnerIV.init = &InnerBounds->getInitialIVValue();
	InnerIV.tripCount = BinaryOperator::CreateSub(&InnerBounds->getFinalIVValue(),
											  &InnerBounds->getInitialIVValue(),
											  "inner.tripcount", InnerIV.loop->getLoopPreheader()->getTerminator());
	OuterIV.init = &OuterBounds->getInitialIVValue();
	OuterIV.tripCount = BinaryOperator::CreateSub(&OuterBounds->getFinalIVValue(),
											  &OuterBounds->getInitialIVValue(),
											  "outer.tripcount", InnerIV.loop->getLoopPreheader()->getTerminator());

	IVList.push_back(OuterIV);
	IVList.push_back(InnerIV);
	if (!isAffineLoop(L, AR)) {
		dbgs() << "Not affine\n";
		return false;
	} else {
		dbgs() << "Affine\n";
	}
	return true;
}


PreservedAnalyses PolytopePass::run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U) {
	if (runAnalysis(L, AR)) {
		std::cout << "Inserted" << std::endl;
	}
	return PreservedAnalyses::all();
}

void PolytopePass::printValue(Value* V) {
	if (V->getName().empty()) {
		auto k = cast<ConstantInt>(V);
		dbgs() << k->getSExtValue() << "\n";
	} else {
		dbgs() << V->getName() << "\n";
	}
}

llvm::PassPluginLibraryInfo getPolyLoopPluginInfo() {
	return {LLVM_PLUGIN_API_VERSION, "PolyLoop", LLVM_VERSION_STRING,
			[](PassBuilder& PB) {
				PB.registerPipelineParsingCallback(
						[](StringRef Name, LoopPassManager& LPM,
						   ArrayRef<PassBuilder::PipelineElement>) {
							if (Name == "polytope") {
								LPM.addPass(PolytopePass());
								return true;
							}
							return false;
						});
			}};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
	return getPolyLoopPluginInfo();
}