#ifndef LLVM_TRANSFORMS_POLYLOOP_H
#define LLVM_TRANSFORMS_POLYLOOP_H

#include <optional>
#include <utility>
#include "LoopDependencies.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"


struct IVInfo {
	llvm::PHINode* IV = nullptr;
	llvm::Value* init = nullptr;
	llvm::Value* final = nullptr;
	llvm::Loop* loop = nullptr;
};

namespace llvm {
	class PolytopePass : public PassInfoMixin<PolytopePass> {
	public:
		bool IsPerfectNest(Loop& L, LoopInfo& LI, ScalarEvolution& SE);
		bool HasInvariantBounds();
		std::optional<LoopDependencies> RunAnalysis(Loop& L, LoopStandardAnalysisResults& AR);
		std::optional<Instruction*> FindInstr(unsigned int opCode, BasicBlock* basicBlock);
		int ValueToInt(Value* V);
		Value* IntToValue(int n);
		static void PrintValue(Value* V);
		PreservedAnalyses run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U);

	private:
		static llvm::LLVMContext ctx;
		std::vector<IVInfo> IVList;
		Loop* innerLoop;
		Loop* outerLoop;
		std::optional<std::vector<int>> GetValueIfAffine(Value* V);
		std::optional<LoopDependencies> GetArrayAccessesIfAffine();
		std::optional<std::vector<std::vector<int>>> ComputeAffineTransformation(const LoopDependencies& assignment);
		std::optional<std::vector<std::vector<int>>> ComputeAffineTransformationInner(const LoopDependencies& assignment,
																					  const std::vector<std::vector<int>>& genA,
																					  const std::vector<std::vector<int>>& genB,
																					  const std::vector<std::vector<int>>& transform,
																					  int depth);

		LoopDependencies
		TransformAssignment(const LoopDependencies& assignment, const std::vector<std::vector<int>>& transform);
		void PrintTransform(const std::vector<std::vector<int>>& T);
	};

} // namespace llvm

#endif // LLVM_TRANSFORMS_POLYLOOP_H