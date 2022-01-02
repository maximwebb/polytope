#ifndef LLVM_TRANSFORMS_POLYLOOP_H
#define LLVM_TRANSFORMS_POLYLOOP_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {

class PolytopePass : public PassInfoMixin<PolytopePass> {
public:
  PreservedAnalyses run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U);

  bool isPerfectNest(Loop& L, LoopInfo& LI, ScalarEvolution& SE);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_POLYLOOP_H