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

bool PolytopePass::IsPerfectNest(Loop& L, LoopInfo& LI, ScalarEvolution& SE) {
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
PolytopePass::GetAffineValue(Value* V, const std::vector<int>& Transform, const std::vector<Value*>& Visited = {},
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
		unsigned index = it - IVList.begin();
		NewTransform[index] = scale;
		return NewTransform;
	}
	if (isa<AddOperator>(V)) {
		auto* AddInstr = dyn_cast<AddOperator>(V);
		auto Fst = GetAffineValue(AddInstr->getOperand(0), Transform, Visited, scale);
		auto Snd = GetAffineValue(AddInstr->getOperand(1), Transform, Visited, scale);
		if (Fst && Snd) {
			std::transform(Fst->begin(), Fst->end(), Snd->begin(), Fst->begin(), std::plus<>());
			return Fst;
		}
	}
	if (isa<SubOperator>(V)) {
		auto* SubInstr = dyn_cast<SubOperator>(V);
		auto Fst = GetAffineValue(SubInstr->getOperand(0), Transform, Visited, scale);
		auto Snd = GetAffineValue(SubInstr->getOperand(1), Transform, Visited, scale);
		if (Fst && Snd) {
			std::transform(Fst->begin(), Fst->end(), Snd->begin(), Fst->begin(), std::minus<>());
			return Fst;
		}
	}
	if (isa<MulOperator>(V)) {
		auto* MulInstr = dyn_cast<MulOperator>(V);
		if (isa<Constant>(MulInstr->getOperand(0))) {
			return GetAffineValue(MulInstr->getOperand(1), Transform, Visited,
								  scale * GetConstantInt(MulInstr->getOperand(0)));
		} else if (isa<Constant>(MulInstr->getOperand(1))) {
			return GetAffineValue(MulInstr->getOperand(0), Transform, Visited,
								  scale * GetConstantInt(MulInstr->getOperand(1)));
		}
		return {};
	}
	if (isa<ShlOperator>(V)) {
		auto* ShlInstr = dyn_cast<ShlOperator>(V);
		if (isa<Constant>(ShlInstr->getOperand(1))) {
			auto Multiplier = 1 << GetConstantInt(ShlInstr->getOperand(1));
			return GetAffineValue(ShlInstr->getOperand(0), Transform, Visited, scale * Multiplier);
		}
		return {};
	}
	/* Exclusively for testing XOR */
	if (isa<BinaryOperator>(V)) {
		auto* BinInstr = dyn_cast<BinaryOperator>(V);
		/* The instruction %1 = xor k -1 simplifies to %1 = -k - 1 */
		if (BinInstr->getOpcode() == Instruction::Xor && GetConstantInt(BinInstr->getOperand(1)) == -1) {
			auto res = GetAffineValue(BinInstr->getOperand(0), Transform, Visited, -scale);
			if (res) {
				res.value().back() -= 1;
				return res;
			}
		}
		return {};
	}
	if (isa<CallInst>(V)) {
		auto* FuncInstr = dyn_cast<CallInst>(V);
		if (FuncInstr->getCalledFunction()->getName() == "llvm.smax.i32") {
			auto Fst = GetAffineValue(FuncInstr->getArgOperand(0), Transform, Visited, scale);
			auto Snd = GetAffineValue(FuncInstr->getArgOperand(1), Transform, Visited, scale);
			if (Fst && Snd) {
				std::transform(Fst->begin(), Fst->end(), Snd->begin(), Fst->begin(),
							   [](int m, int n) { return m > n ? m : n; });
				return Fst;
			}
		} else {
			return {};
		}
	}
	if (isa<CastInst>(V)) {
		auto* CastInstr = dyn_cast<CastInst>(V);
		PrintValue(CastInstr->getOperand(0));
		return GetAffineValue(CastInstr->getOperand(0), Transform, Visited, scale);
	}

	/* Code may be redundant */
//	if (isa<PHINode>(V)) {
//		std::optional<std::vector<int>> NewTransform;
//		auto* PhiInstr = dyn_cast<PHINode>(V);
//		std::vector<Value*> NewVisited;
//		NewVisited.insert(NewVisited.begin(), Visited.begin(), Visited.end());
//		NewVisited.push_back(PhiInstr);
//		for (auto& Op: PhiInstr->operands()) {
//			NewTransform = GetAffineValue(Op.get(), Transform, NewVisited);
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
bool PolytopePass::IsAffineLoop(Loop& L, LoopStandardAnalysisResults& AR) {
	std::vector<int> InitTransform(IVList.size() + 1, 0);
	std::vector<int> FinalTransform(IVList.size() + 1, 0);
	for (auto& IV: IVList) {
		auto res1 = GetAffineValue(IV.init, InitTransform);
		if (!res1) {
			return false;
		}
		auto res2 = GetAffineValue(IV.final, FinalTransform);
		if (!res2) {
			return false;
		}
	}

	return true;
}

bool PolytopePass::RunAnalysis(Loop& L, LoopStandardAnalysisResults& AR) {
	if (!IsPerfectNest(L, AR.LI, AR.SE)) {
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
	outerLoop = &L;
	innerLoop = IL;

	OuterIV.IV = OuterIV.loop->getInductionVariable(AR.SE);
	InnerIV.IV = InnerIV.loop->getInductionVariable(AR.SE);

	auto InnerBounds = InnerIV.loop->getBounds(AR.SE);
	auto OuterBounds = OuterIV.loop->getBounds(AR.SE);

	if (!InnerBounds.hasValue() || !OuterBounds.hasValue()) {
		return false;
	}

	InnerIV.init = &InnerBounds->getInitialIVValue();
	OuterIV.init = &OuterBounds->getInitialIVValue();
	InnerIV.final = &InnerBounds->getFinalIVValue();
	OuterIV.final = &OuterBounds->getFinalIVValue();

	IVList.push_back(OuterIV);
	IVList.push_back(InnerIV);


	if (!IsAffineLoop(L, AR)) {
		dbgs() << "Not affine\n";
		return false;
	} else {
		dbgs() << "Affine\n";
	}
	return true;
}

ArrayAssignment PolytopePass::ExtractArrayAccesses(Loop& L, LoopStandardAnalysisResults& AR) {
	std::vector<std::vector<std::vector<int>>> readVectors;
	/* In the future, analysis pass will ensure there is precisely one write, and perform this extraction */
	std::vector<std::vector<int>> writeVector;
	for (auto& Instr: *(innerLoop->getHeader())) {
		/* Extract array access index functions for all array read/writes */
		if (isa<StoreInst>(Instr) || isa<LoadInst>(Instr)) {
			bool isWrite = isa<StoreInst>(Instr);
			std::vector<int> T(IVList.size() + 1, 0);
			auto I = Instr.getOperand(isWrite ? 1 : 0);
			if (isa<GetElementPtrInst>(I)) {
				auto GEPInstr = dyn_cast<GetElementPtrInst>(I);
				auto v1 = GetAffineValue(GEPInstr->getOperand(2), T);
				auto v2 = GetAffineValue(GEPInstr->getOperand(3), T);
				if (v1 && v2) {
					if (isWrite) {
						writeVector.push_back(v1.value());
						writeVector.push_back(v2.value());
					} else {
						readVectors.push_back({v1.value(), v2.value()});
					}
				}
			}
		}
	}
	return {writeVector, readVectors};
}

std::optional<std::vector<std::vector<int>>>
PolytopePass::ComputeAffineTransformation(const ArrayAssignment& assignment) {
	if (!assignment.HasLoopCarrierDependencies()) {
		return {};
	}



	return {};
}

PreservedAnalyses PolytopePass::run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U) {
	if (!RunAnalysis(L, AR)) {
		return PreservedAnalyses::all();
	}

	auto assignment = ExtractArrayAccesses(L, AR);
	auto transformation = ComputeAffineTransformation(assignment);

	return PreservedAnalyses::all();
}

int PolytopePass::GetConstantInt(Value* V) {
	auto k = dyn_cast<ConstantInt>(V);
	return k->getSExtValue();
}

void PolytopePass::PrintValue(Value* V) {
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