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
#include "llvm/IR/LLVMContext.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

using namespace llvm;

bool PolytopePass::IsPerfectNest(Loop& L, LoopInfo& LI, ScalarEvolution& SE) {
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
std::optional<std::vector<int>> PolytopePass::GetValueIfAffine(Value* V) {
	if (isa<Constant>(V)) {
		std::vector<int> res(IVList.size() + 1, 0);
		auto k = dyn_cast<ConstantInt>(V);
		res[IVList.size()] = k->getSExtValue();
		return res;
	}
	/* Test if the value is an induction variable */
	auto it = std::find_if(IVList.begin(), IVList.end(), [V](IVInfo& info) { return info.IV == V; });
	if (it != IVList.end()) {
		std::vector<int> res(IVList.size() + 1, 0);
		unsigned index = it - IVList.begin();
		res[index] = 1;
		return res;
	}
	if (isa<AddOperator>(V)) {
		auto* addInstr = dyn_cast<AddOperator>(V);
		auto fst = GetValueIfAffine(addInstr->getOperand(0));
		auto snd = GetValueIfAffine(addInstr->getOperand(1));
		if (fst && snd) {
			std::transform(fst->begin(), fst->end(), snd->begin(), fst->begin(), std::plus<>());
			return fst;
		}
	}
	if (isa<SubOperator>(V)) {
		auto* subInstr = dyn_cast<SubOperator>(V);
		auto fst = GetValueIfAffine(subInstr->getOperand(0));
		auto snd = GetValueIfAffine(subInstr->getOperand(1));
		if (fst && snd) {
			std::transform(fst->begin(), fst->end(), snd->begin(), fst->begin(), std::minus<>());
			return fst;
		}
	}
	if (isa<MulOperator>(V)) {
		auto* mulInstr = dyn_cast<MulOperator>(V);
		std::optional<std::vector<int>> res = {};
		/* Ensure at least one of the two branches is a constant */
		if (isa<Constant>(mulInstr->getOperand(0))) {
			int scale = ValueToInt(mulInstr->getOperand(0));
			res = GetValueIfAffine(mulInstr->getOperand(1));
			std::transform(res->begin(), res->end(), res->begin(), [scale](int& x){ return x * scale; });
		} else if (isa<Constant>(mulInstr->getOperand(1))) {
			int scale = ValueToInt(mulInstr->getOperand(1));
			res = GetValueIfAffine(mulInstr->getOperand(0));
			std::transform(res->begin(), res->end(), res->begin(), [scale](int& x){ return x * scale; });
		}
		return res;
	}
	if (isa<ShlOperator>(V)) {
		auto* shlInstr = dyn_cast<ShlOperator>(V);
		if (isa<Constant>(shlInstr->getOperand(1))) {
			int scale = 1 << ValueToInt(shlInstr->getOperand(1));
			auto res = GetValueIfAffine(shlInstr->getOperand(0));
			std::transform(res->begin(), res->end(), res->begin(), [scale](int& x){ return x * scale; });
			return res;
		}
		return {};
	}
	/* Exclusively for testing XOR */
	if (isa<BinaryOperator>(V)) {
		auto* binInstr = dyn_cast<BinaryOperator>(V);
		/* The instruction %1 = xor k -1 simplifies to %1 = -k - 1 */
		if (binInstr->getOpcode() == Instruction::Xor && ValueToInt(binInstr->getOperand(1)) == -1) {
			auto res = GetValueIfAffine(binInstr->getOperand(0));
			std::transform(res->begin(), res->end(), res->begin(), std::negate<>());
			if (res) {
				res.value().back() -= 1;
				return res;
			}
		}
		return {};
	}
	if (isa<CallInst>(V)) {
		auto* funcInstr = dyn_cast<CallInst>(V);
		if (funcInstr->getCalledFunction()->getName() == "llvm.smax.i32") {
			auto fst = GetValueIfAffine(funcInstr->getArgOperand(0));
			auto snd = GetValueIfAffine(funcInstr->getArgOperand(1));
			if (fst && snd) {
				std::transform(fst->begin(), fst->end(), snd->begin(), fst->begin(),
							   [](int m, int n) { return m > n ? m : n; });
				return fst;
			}
		} else {
			return {};
		}
	}
	if (isa<CastInst>(V)) {
		auto* castInstr = dyn_cast<CastInst>(V);
		return GetValueIfAffine(castInstr->getOperand(0));
	}
	return {};
}

/* Verifies that induction variables and array accesses are affine functions */
bool PolytopePass::HasInvariantBounds() {
	for (auto& IV: IVList) {
		if (!(outerLoop->isLoopInvariant(IV.init) && outerLoop->isLoopInvariant(IV.final))) {
			return false;
		}
	}
	return true;
}

std::optional<ArrayAssignment> PolytopePass::RunAnalysis(Loop& L, LoopStandardAnalysisResults& AR) {
	if (!IsPerfectNest(L, AR.LI, AR.SE)) {
		return {};
	}
	IVInfo OuterIV;
	IVInfo InnerIV;

	OuterIV.loop = &L;
	auto* IL = L.getSubLoops().at(0);
	if (IL == nullptr) {
		return {};
	}
	InnerIV.loop = IL;
	outerLoop = &L;
	innerLoop = IL;

	OuterIV.IV = OuterIV.loop->getInductionVariable(AR.SE);
	InnerIV.IV = InnerIV.loop->getInductionVariable(AR.SE);

	auto InnerBounds = InnerIV.loop->getBounds(AR.SE);
	auto OuterBounds = OuterIV.loop->getBounds(AR.SE);

	if (!InnerBounds.hasValue() || !OuterBounds.hasValue()) {
		return {};
	}

	InnerIV.init = &InnerBounds->getInitialIVValue();
	OuterIV.init = &OuterBounds->getInitialIVValue();
	InnerIV.final = &InnerBounds->getFinalIVValue();
	OuterIV.final = &OuterBounds->getFinalIVValue();

	IVList.push_back(OuterIV);
	IVList.push_back(InnerIV);

	auto assignment = GetArrayAccessesIfAffine();

	if (!HasInvariantBounds() || !assignment || assignment->writeAccess.empty()) {
		dbgs() << "Not affine\n";
		return {};
	}
	dbgs() << "Affine\n";
	return assignment;
}

std::optional<ArrayAssignment> PolytopePass::GetArrayAccessesIfAffine() {
	std::vector<std::vector<std::vector<int>>> readVectors;
	/* In the future, analysis pass will ensure there is precisely one write, and perform this extraction */
	std::vector<std::vector<int>> writeVector;
	for (auto& instr: *(innerLoop->getHeader())) {
		/* Extract array access index functions for all array read/writes */
		if (isa<StoreInst>(instr) || isa<LoadInst>(instr)) {
			bool isWrite = isa<StoreInst>(instr);
			std::vector<int> T(IVList.size() + 1, 0);
			auto I = instr.getOperand(isWrite ? 1 : 0);
			if (isa<GetElementPtrInst>(I)) {
				auto GEPInstr = dyn_cast<GetElementPtrInst>(I);
				auto v1 = GetValueIfAffine(GEPInstr->getOperand(2));
				auto v2 = GetValueIfAffine(GEPInstr->getOperand(3));
				if (v1 && v2) {
					if (isWrite) {
						writeVector.push_back(v1.value());
						writeVector.push_back(v2.value());
					} else {
						readVectors.push_back({v1.value(), v2.value()});
					}
				}
			} else {
				return {};
			}
		}
	}
	return ArrayAssignment(writeVector, readVectors);
}

ArrayAssignment
PolytopePass::TransformAssignment(const ArrayAssignment& assignment, const std::vector<std::vector<int>>& transform) {
	std::vector<std::vector<int>> writeVector;
	std::vector<std::vector<std::vector<int>>> readVectors;

	std::vector<std::vector<int>> transform1;
	for (auto& row: transform) {
		auto v = row;
		v.push_back(0);
		transform1.push_back(v);
	}
	transform1.emplace_back(transform1.at(0).size(), 0);
	transform1.back().back() = 1;

	for (auto& writeIndex: assignment.writeAccess) {
		writeVector.push_back(IntegerSolver::LinearTransform(transform1, writeIndex));
	}

	/* Can this be done with a single matrix multiplication? */
	for (auto& readVector: assignment.readAccesses) {
		std::vector<std::vector<int>> v;
		v.reserve(readVector.size());
		for (auto& readIndex: readVector) {
			v.push_back(IntegerSolver::LinearTransform(transform1, readIndex));
		}
		readVectors.push_back(v);
	}

	return {writeVector, readVectors};
}

std::optional<std::vector<std::vector<int>>>
PolytopePass::ComputeAffineTransformationInner(const ArrayAssignment& assignment,
											   const std::vector<std::vector<int>>& genA,
											   const std::vector<std::vector<int>>& genB,
											   const std::vector<std::vector<int>>& transform,
											   int depth) {
	auto transformedAssignment = TransformAssignment(assignment, transform);
	if (!transformedAssignment.HasLoopCarrierDependencies()) {
		return transform;
	}
	if (depth == 0) {
		return {};
	}
	depth--;

	auto transform1 = ComputeAffineTransformationInner(assignment, genA, genB, IntegerSolver::Multiply(genA, transform),
													   depth);
	auto transform2 = ComputeAffineTransformationInner(assignment, genA, genB, IntegerSolver::Multiply(genB, transform),
													   depth);

	return transform1 ? transform1 : transform2;
}

std::optional<std::vector<std::vector<int>>>
PolytopePass::ComputeAffineTransformation(const ArrayAssignment& assignment) {
	if (!assignment.HasLoopCarrierDependencies()) {
		return {};
	}

	unsigned dim = IVList.size();
	auto generators = IntegerSolver::GetGenerators(dim);
	return ComputeAffineTransformationInner(assignment, generators.first, generators.second,
											IntegerSolver::IdentityMatrix(dim), 5);
}


PreservedAnalyses PolytopePass::run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U) {
	auto assignment = RunAnalysis(L, AR);
	if (!assignment) {
		return PreservedAnalyses::all();
	}

	auto transformation = ComputeAffineTransformation(*assignment);
//	auto T = transformation.value();
	std::vector<std::vector<int>> T = {{1, 1},
									   {0, 2}};
	auto H = IntegerSolver::HermiteNormal(T);

	auto outerBounds = outerLoop->getBounds(AR.SE).getValue();
	auto innerBounds = innerLoop->getBounds(AR.SE).getValue();

	Module* M = L.getHeader()->getModule();
	auto Int32Ty = outerLoop->getInductionVariable(AR.SE)->getType();
	auto FloatTy = Type::getFloatTy(outerLoop->getHeader()->getContext());
	std::vector<Type*> IntTypes(2, Int32Ty);
	FunctionType* FTMin = FunctionType::get(Int32Ty, IntTypes, false);
	FunctionType* FTMax = FunctionType::get(Int32Ty, IntTypes, false);
	Function* minFunc = Function::Create(FTMin, Function::ExternalLinkage, "llvm.smin.i32", M);
	Function* maxFunc = Function::Create(FTMax, Function::ExternalLinkage, "llvm.smax.i32", M);

	/* Extract redundant IV instructions */
	auto oldOuterIV = outerLoop->getInductionVariable(AR.SE);
	auto oldInnerIV = innerLoop->getInductionVariable(AR.SE);
	auto oldOuterComparison = outerLoop->getLatchCmpInst();
	auto oldInnerComparison = innerLoop->getLatchCmpInst();
	/* Retrieve increment instructions by analysing comparison */
	auto oldOuterIncrement = cast<Instruction>(oldOuterIV->getIncomingValueForBlock(outerLoop->getLoopLatch()));
	auto oldInnerIncrement = cast<Instruction>(oldInnerIV->getIncomingValueForBlock(innerLoop->getLoopLatch()));
	auto oldOuterBranch = FindInstr(Instruction::Br, outerLoop->getLoopLatch()).value();
	auto oldInnerBranch = FindInstr(Instruction::Br, innerLoop->getLoopLatch()).value();

	/* Update outer loop header */
	IRBuilder builder(outerLoop->getHeader()->getContext());
	builder.SetInsertPoint(outerLoop->getHeader()->getFirstNonPHI());
	auto outerLowerBound = builder.CreateAdd(
			builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getInitialIVValue()),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getInitialIVValue()), "p.lower");
	auto outerIV = builder.CreatePHI(Int32Ty, 2, "p");

	/* Update outer loop latch */
	builder.SetInsertPoint(outerLoop->getLoopLatch()->getTerminator());
	auto outerIncrement = builder.CreateAdd(outerIV, IntToValue(H[0][0]), "p.inc");
	outerIV->addIncoming(outerLowerBound, outerLoop->getLoopPreheader());
	outerIV->addIncoming(outerIncrement, outerLoop->getLoopLatch());
	auto outerUpperBound = builder.CreateAdd(
			builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getFinalIVValue()),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getFinalIVValue()), "p.upper");
	auto outerComp = builder.CreateCmp(CmpInst::ICMP_SLE, outerIncrement, outerUpperBound);
	auto outerBranch = builder.CreateCondBr(outerComp, outerLoop->getHeader(), outerLoop->getExitBlock());

	/* Determine values for inner loop bounds */
	builder.SetInsertPoint(innerLoop->getLoopPreheader()->getTerminator());
	auto l1 = builder.CreateSub(
			builder.CreateSub(outerIV, builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getFinalIVValue())),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getInitialIVValue()), "l1");

	auto l1Ceil = builder.CreateAdd(
		builder.CreateCall(maxFunc, {
			builder.CreateAdd(
				builder.CreateMul(
					IntToValue(T[1][0]),
					builder.CreateSDiv(l1, IntToValue(T[0][0]))
				),
				builder.CreateCall(minFunc, {
					builder.CreateSRem(l1, IntToValue(T[0][0])),
					IntToValue(1)
				})
			),
			(T[0][1] == 0) ? IntToValue(INT_MIN) : builder.CreateAdd(
				builder.CreateMul(
					IntToValue(T[1][1]),
					builder.CreateSDiv(l1, IntToValue(T[0][1]))
				),
				builder.CreateCall(minFunc, {
					builder.CreateSRem(l1, IntToValue(T[0][1])),
					IntToValue(1)
				})
			)
		}),
		builder.CreateAdd(
			builder.CreateMul(IntToValue(T[1][0]), &outerBounds.getFinalIVValue()),
			builder.CreateMul(IntToValue(T[1][1]), &innerBounds.getInitialIVValue())
		),
		"l1.ceil"
	);

	auto l3 = builder.CreateSub(
			builder.CreateSub(outerIV, builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getInitialIVValue())),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getFinalIVValue()), "l3");

	/* Upper bound for inner loop */
	auto innerUpperBound = builder.CreateAdd(
			builder.CreateCall(minFunc, {
				builder.CreateAdd(
					builder.CreateMul(
						IntToValue(T[1][0]),
						builder.CreateSDiv(l3, IntToValue(T[0][0]))
					),
					builder.CreateCall(minFunc, {
						builder.CreateSRem(l3, IntToValue(T[0][0])),
						IntToValue(1)
					})
				),
				(T[0][1] == 0) ? IntToValue(MAX_INT) : builder.CreateAdd(
					builder.CreateMul(
						IntToValue(T[1][1]),
						builder.CreateSDiv(l3, IntToValue(T[0][1]))
					),
					builder.CreateCall(minFunc, {
						builder.CreateSRem(l3, IntToValue(T[0][1])),
						IntToValue(1)
					})
				)
			}),
			builder.CreateAdd(
				builder.CreateMul(IntToValue(T[1][0]), &outerBounds.getInitialIVValue()),
				builder.CreateMul(IntToValue(T[1][1]), &innerBounds.getFinalIVValue())
			),
		"q.upper"
	);

	auto offset = builder.CreateSRem(
		builder.CreateSub(
			builder.CreateMul(
				IntToValue(H[1][0]),
				builder.CreateSDiv(outerIV, IntToValue(H[0][0]))
			),
			l1Ceil
		),
		IntToValue(H[1][1]),
		"offset"
	);

	/* Update inner loop header */
	builder.SetInsertPoint(innerLoop->getLoopPreheader()->getTerminator());
	auto innerLowerBound = builder.CreateAdd(l1Ceil, offset, "q.lower");
	builder.SetInsertPoint(innerLoop->getHeader()->getFirstNonPHI());
	auto innerIV = builder.CreatePHI(Int32Ty, 2, "q");
	auto iNew = builder.CreateSDiv(
				builder.CreateSub(
					builder.CreateMul(
						IntToValue(T[1][1]),
						outerIV
					),
					builder.CreateMul(
						IntToValue(T[0][1]),
						innerIV
					)
				),
				IntToValue(T[0][0] * T[1][1] - T[0][1] * T[1][0]),
				"i.new"
		   );
	auto jNew = builder.CreateSDiv(
					builder.CreateSub(
						builder.CreateMul(
							IntToValue(T[0][0]),
							innerIV
						),
						builder.CreateMul(
							IntToValue(T[1][0]),
							outerIV
						)
					),
				   	IntToValue(T[0][0] * T[1][1] - T[0][1] * T[1][0]),
				   	"j.new"
			   );

	/* Update inner loop latch */
	builder.SetInsertPoint(innerLoop->getLoopLatch()->getTerminator());
	auto innerIncrement = builder.CreateAdd(innerIV, IntToValue(H[1][1]), "q.inc");
	innerIV->addIncoming(innerLowerBound, innerLoop->getLoopPreheader());
	innerIV->addIncoming(innerIncrement, innerLoop->getLoopLatch());

	auto innerComp = builder.CreateCmp(CmpInst::ICMP_SLE, innerIncrement, innerUpperBound);
	auto innerBranch = builder.CreateCondBr(innerComp, innerLoop->getHeader(), innerLoop->getExitBlock());


	/* Clean up old induction variables */
	oldOuterIV->replaceAllUsesWith(iNew);
	oldInnerIV->replaceAllUsesWith(jNew);

	oldOuterIV->eraseFromParent();
	oldOuterIncrement->eraseFromParent();
	oldOuterBranch->eraseFromParent();
	oldOuterComparison->eraseFromParent();

	oldInnerIV->eraseFromParent();
	oldInnerIncrement->eraseFromParent();
	oldInnerBranch->eraseFromParent();
	oldInnerComparison->eraseFromParent();

	dbgs() << "Performed polytope optimisation\n";

	return PreservedAnalyses::none();
}

std::optional<Instruction*> PolytopePass::FindInstr(unsigned int opCode, BasicBlock* basicBlock) {
	auto instr = std::find_if(basicBlock->begin(), basicBlock->end(),
							  [opCode](Instruction& I) { return I.getOpcode() == opCode; });
	if (instr != std::end(*basicBlock)) {
		// Pointer hack to get correct type
		return &*instr;
	}
	return {};
}

int PolytopePass::ValueToInt(Value* V) {
	auto k = dyn_cast<ConstantInt>(V);
	return k->getSExtValue();
}

Value* PolytopePass::IntToValue(int n) {
	return ConstantInt::get(IntegerType::getInt32Ty(outerLoop->getHeader()->getContext()), n);
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