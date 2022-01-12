#ifndef LLVM_TRANSFORMS_POLYLOOP_H
#define LLVM_TRANSFORMS_POLYLOOP_H

#include <optional>
#include <utility>
#include "ArrayAssignment.h"
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
		bool IsAffineLoop(Loop& L, LoopStandardAnalysisResults& AR);
		bool RunAnalysis(Loop& L, LoopStandardAnalysisResults& AR);
		int GetConstantInt(Value* V);
		static void PrintValue(Value* V);
		PreservedAnalyses run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U);

	private:
		std::vector<IVInfo> IVList;
		Loop* innerLoop;
		Loop* outerLoop;
		std::optional<std::vector<int>>
		GetAffineValue(Value* V, const std::vector<int>& Transform, const std::vector<Value*>& Visited, int scale);
		ArrayAssignment ExtractArrayAccesses(Loop& L, LoopStandardAnalysisResults& AR);
		std::optional<std::vector<std::vector<int>>> ComputeAffineTransformation(const ArrayAssignment& assignment);
	};

} // namespace llvm

#endif // LLVM_TRANSFORMS_POLYLOOP_H