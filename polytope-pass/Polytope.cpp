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

	PreservedAnalyses PolytopePass::run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U) {
//			bool Changed = runOnLoop(L, AR);
//			if (true) {
			std::cout << "Inserted" << std::endl;
//				return PreservedAnalyses::none();
//			}
		return PreservedAnalyses::all();
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