#ifndef LLVM_TRANSFORMS_POLYLOOP_H
#define LLVM_TRANSFORMS_POLYLOOP_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

struct IVInfo {
	llvm::PHINode* IV = nullptr;
	llvm::Value* init = nullptr;
	llvm::Value* tripCount = nullptr;
	llvm::Loop* loop = nullptr;
};

namespace llvm {
	class PolytopePass : public PassInfoMixin<PolytopePass> {
	public:
		bool isPerfectNest(Loop& L, LoopInfo& LI, ScalarEvolution& SE);
		bool isAffineValue(Value* V, const std::vector<Value*>& Visited);
		bool isAffineLoop(Loop& L, LoopStandardAnalysisResults& AR);
		bool runAnalysis(Loop& L, LoopStandardAnalysisResults& AR);
		static void printValue(Value* V);
		PreservedAnalyses run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U);

	private:
		std::vector<IVInfo> IVList;
	};

} // namespace llvm

#endif // LLVM_TRANSFORMS_POLYLOOP_H