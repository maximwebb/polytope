#include "Polytope.h"
#include <iostream>
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
std::optional<std::vector<int>>
PolytopePass::isAffineValue(Value* V, const std::vector<int>& Transform, const std::vector<Value*>& Visited = {},
							int scale = 1) {
	/* Test for recursively defined variable in IR */
//	if (std::find(Visited.begin(), Visited.end(), V) != Visited.end()) {
//		return true;
//	}
	if (isa<Constant>(V)) {
		std::vector<int> NewTransform;
		NewTransform.insert(NewTransform.begin(), Transform.begin(), Transform.end());
		auto k = dyn_cast<ConstantInt>(V);
		NewTransform[NewTransform.size() - 1] = k->getSExtValue() * scale;
		return NewTransform;
	}
	/* Test if the value is an induction variable */
	auto it = std::find_if(IVList.begin(), IVList.end(), [V](IVInfo& info) { return info.IV == V; });
	if (it != IVList.end()) {
		std::vector<int> NewTransform;
		NewTransform.insert(NewTransform.begin(), Transform.begin(), Transform.end());
		int index = it - IVList.begin();
		NewTransform[index] = scale;
		return NewTransform;
	}
	if (isa<AddOperator>(V)) {
		auto* AddInstr = dyn_cast<AddOperator>(V);
		auto Fst = isAffineValue(AddInstr->getOperand(0), Transform, Visited, scale);
		auto Snd = isAffineValue(AddInstr->getOperand(1), Transform, Visited, scale);
		if (Fst && Snd) {
			std::transform(Fst->begin(), Fst->end(), Snd->begin(), Fst->begin(), std::plus<>());
			return Fst;
		}
	}
	if (isa<SubOperator>(V)) {
		auto* SubInstr = dyn_cast<SubOperator>(V);
		auto Fst = isAffineValue(SubInstr->getOperand(0), Transform, Visited, scale);
		auto Snd = isAffineValue(SubInstr->getOperand(1), Transform, Visited, scale);
		if (Fst && Snd) {
			std::transform(Fst->begin(), Fst->end(), Snd->begin(), Fst->begin(), std::minus<>());
			return Fst;
		}
	}
	if (isa<MulOperator>(V)) {
		auto* MulInstr = dyn_cast<MulOperator>(V);
		if (isa<Constant>(MulInstr->getOperand(0))) {
			return isAffineValue(MulInstr->getOperand(1), Transform, Visited,
								 scale * getConstantInt(MulInstr->getOperand(0)));
		} else if (isa<Constant>(MulInstr->getOperand(1))) {
			return isAffineValue(MulInstr->getOperand(0), Transform, Visited,
								 scale * getConstantInt(MulInstr->getOperand(1)));
		}
		return {};
	}
	if (isa<ShlOperator>(V)) {
		auto* ShlInstr = dyn_cast<ShlOperator>(V);
		if (isa<Constant>(ShlInstr->getOperand(1))) {
			auto Multiplier = 1 << getConstantInt(ShlInstr->getOperand(1));
			return isAffineValue(ShlInstr->getOperand(0), Transform, Visited, scale * Multiplier);
		}
		return {};
	}
	if (isa<CallInst>(V)) {
		auto* FuncInstr = dyn_cast<CallInst>(V);
		if (FuncInstr->getCalledFunction()->getName() == "llvm.smax.i32") {
			auto Fst = isAffineValue(FuncInstr->getArgOperand(0), Transform, Visited, scale);
			auto Snd = isAffineValue(FuncInstr->getArgOperand(1), Transform, Visited, scale);
			if (Fst && Snd) {
				std::transform(Fst->begin(), Fst->end(), Snd->begin(), Fst->begin(),
							   [](int m, int n) { return m > n ? m : n; });
				return Fst;
			}
		} else {
			return {};
		}
	}
	/* Code may be redundant */
//	if (isa<PHINode>(V)) {
//		std::optional<std::vector<int>> NewTransform;
//		auto* PhiInstr = dyn_cast<PHINode>(V);
//		std::vector<Value*> NewVisited;
//		NewVisited.insert(NewVisited.begin(), Visited.begin(), Visited.end());
//		NewVisited.push_back(PhiInstr);
//		for (auto& Op: PhiInstr->operands()) {
//			NewTransform = isAffineValue(Op.get(), Transform, NewVisited);
//			if (!NewTransform) {
//				return {};
//			}
//		}
//		dbgs() << "PHI Node\n";
//		return NewTransform;
//	}
	return {};
}

/* Verifies that induction variables and array accesses are affine functions */
/* This will initially only check for a subset of affine functions, eg. constant bounds */
bool PolytopePass::isAffineLoop(Loop& L, LoopStandardAnalysisResults& AR) {
	std::vector<int> InitTransform(IVList.size() + 1, 0);
	std::vector<int> FinalTransform(IVList.size() + 1, 0);
	for (auto& IV: IVList) {
		auto res1 = isAffineValue(IV.init, InitTransform);
		if (!res1) {
			return false;
		}
		auto res2 = isAffineValue(IV.final, FinalTransform);
		if (!res2) {
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
	InnerIV.final = &InnerBounds->getFinalIVValue();
	InnerIV.tripCount = BinaryOperator::CreateSub(&InnerBounds->getFinalIVValue(),
												  &InnerBounds->getInitialIVValue(),
												  "inner.tripcount", InnerIV.loop->getLoopPreheader()->getTerminator());
	OuterIV.init = &OuterBounds->getInitialIVValue();
	OuterIV.final = &OuterBounds->getFinalIVValue();
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

int PolytopePass::getConstantInt(Value* V) {
	auto k = dyn_cast<ConstantInt>(V);
	return k->getSExtValue();
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