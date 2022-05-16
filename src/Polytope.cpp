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

std::optional<LoopDependencies> PolytopePass::RunAnalysis(Loop& L, LoopStandardAnalysisResults& AR) {
	IVList = {};
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

	auto dependencies = GetArrayAccessesIfAffine();

	if (!HasInvariantBounds()) {
		dbgs() << "Not affine\n";
		return {};
	} else if (!dependencies) {
		dbgs() << "No dependencies\n";
		return {};
	}
	dbgs() << "Affine\n";
	return dependencies;
}

std::optional<LoopDependencies> PolytopePass::GetArrayAccessesIfAffine() {
	std::vector<std::vector<std::vector<int>>> reads;
	std::vector<std::vector<std::vector<int>>> writes;
	for (auto& instr: *(innerLoop->getHeader())) {
		/* Extract array access index functions for all array read/writes */
		if (isa<StoreInst>(instr) || isa<LoadInst>(instr)) {
			bool isWrite = isa<StoreInst>(instr);
			std::vector<int> T(IVList.size() + 1, 0);
			auto I = instr.getOperand(isWrite ? 1 : 0);
			if (isa<GetElementPtrInst>(I)) {
				auto GEPInstr = dyn_cast<GetElementPtrInst>(I);
				auto size = GEPInstr->getNumOperands();
				auto v1 = GetValueIfAffine(GEPInstr->getOperand(size-2));
				auto v2 = GetValueIfAffine(GEPInstr->getOperand(size-1));
				if (v1 && v2) {
					if (isWrite) {
						writes.push_back({v1.value(), v2.value()});
					} else {
						reads.push_back({v1.value(), v2.value()});
					}
				}
			} else {
				return {};
			}
		}
	}

	if (reads.empty() || writes.empty()) {
		return {};
	}

	return LoopDependencies(writes, reads);
}

LoopDependencies
PolytopePass::TransformAssignment(const LoopDependencies& assignment, const std::vector<std::vector<int>>& transform) {
	std::vector<std::vector<std::vector<int>>> writeVectors;
	std::vector<std::vector<std::vector<int>>> readVectors;

	std::vector<std::vector<int>> transform1;
	for (auto& row: transform) {
		auto v = row;
		v.push_back(0);
		transform1.push_back(v);
	}
	transform1.emplace_back(transform1.at(0).size(), 0);
	transform1.back().back() = 1;

	for (auto& write: assignment.writes) {
		std::vector<std::vector<int>> v;
		v.reserve(write.size());
		for (auto& writeIndex: write) {
			v.push_back(IntegerSolver::LinearTransform(transform1, writeIndex));
		}
		writeVectors.push_back(v);
	}

	/* Can this be done with a single matrix multiplication? */
	for (auto& readVector: assignment.reads) {
		std::vector<std::vector<int>> v;
		v.reserve(readVector.size());
		for (auto& readIndex: readVector) {
			v.push_back(IntegerSolver::LinearTransform(transform1, readIndex));
		}
		readVectors.push_back(v);
	}

	return {writeVectors, readVectors};
}

std::optional<std::vector<std::vector<int>>>
PolytopePass::ComputeAffineTransformationInner(const LoopDependencies& assignment,
											   const std::vector<std::vector<int>>& genA,
											   const std::vector<std::vector<int>>& genB,
											   const std::vector<std::vector<int>>& transform,
											   int depth) {
	auto transformedAssignment = TransformAssignment(assignment, transform);
	if (!transformedAssignment.HasLoopCarrierDependencies()) {
		auto preservesDependencies = true;
		std::vector<std::vector<int>> depVectors;
		for (auto write : assignment.writes) {
			for (auto read : assignment.reads) {
				std::vector<int> vec = {write[0].back() - read[0].back(),
										write[1].back() - read[1].back()};
				auto transformedVec = IntegerSolver::LinearTransform(transform, vec);
				for (int i = 0; i < vec.size(); i++) {
					if ((vec[i]<0) != (transformedVec[i]<0)) {
						preservesDependencies = false;
					}
				}
				if (!preservesDependencies) {
					break;
				}
			}
		}

		if (preservesDependencies) {
			return transform;
		}
	}
	if (depth == 0) {
		return {};
	}
	depth--;

	auto transform1 = ComputeAffineTransformationInner(assignment, genA, genB, IntegerSolver::Multiply(genA, transform),
													   depth);
	if (transform1) {
		return transform1;
	}

	auto transform2 = ComputeAffineTransformationInner(assignment, genA, genB, IntegerSolver::Multiply(genB, transform),
													   depth);
	return transform2;
}

std::optional<std::vector<std::vector<int>>>
PolytopePass::ComputeAffineTransformation(const LoopDependencies& assignment) {
	if (!assignment.HasLoopCarrierDependencies()) {
		return {};
	}

	unsigned dim = IVList.size();
	auto T = IntegerSolver::GetInitialTransform(dim);
	auto generators = IntegerSolver::GetGenerators(dim);
	return ComputeAffineTransformationInner(assignment, generators.first, generators.second,
											T, 5);
}

PreservedAnalyses PolytopePass::run(Loop& L, LoopAnalysisManager& AM, LoopStandardAnalysisResults& AR, LPMUpdater& U) {
	auto assignment = RunAnalysis(L, AR);
	if (!assignment) {
		return PreservedAnalyses::all();
	}

	auto transformation = ComputeAffineTransformation(*assignment);
	if (!transformation) {
		dbgs() << "No transformation found\n";
		return PreservedAnalyses::all();
	}
	auto T = transformation.value();
	auto H = IntegerSolver::HermiteNormal(T);
	auto det = IntegerSolver::Det(T);

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

	auto LL = std::make_tuple<>(&outerBounds.getInitialIVValue(), &innerBounds.getInitialIVValue());
	auto LR = std::make_tuple<>(&outerBounds.getFinalIVValue(), &innerBounds.getInitialIVValue());
	auto UL = std::make_tuple<>(&outerBounds.getInitialIVValue(), &innerBounds.getFinalIVValue());
	auto UR = std::make_tuple<>(&outerBounds.getFinalIVValue(), &innerBounds.getFinalIVValue());

	std::tuple<Value*, Value*> outerLBPoint;
	std::tuple<Value*, Value*> outerUBPoint;
	std::tuple<Value*, Value*> innerLBPoint;
	std::tuple<Value*, Value*> innerUBPoint;
	if (T[0][0] > 0 && T[0][1] > 0) {
		outerLBPoint = LL;
		outerUBPoint = UR;
		if (det > 0) {
			innerLBPoint = LR;
			innerUBPoint = UL;
		} else {
			innerLBPoint = UL;
			innerUBPoint = LR;
		}
	} else if (T[0][0] <= 0 && T[0][1] > 0) {
		outerLBPoint = LR;
		outerUBPoint = UL;
		if (det > 0) {
			innerLBPoint = UR;
			innerUBPoint = LL;
		} else {
			innerLBPoint = LL;
			innerUBPoint = UR;
		}
	} else if (T[0][0] > 0 && T[0][1] <= 0) {
		outerLBPoint = UL;
		outerUBPoint = LR;
		if (det > 0) {
			innerLBPoint = LL;
			innerUBPoint = UR;
		} else {
			innerLBPoint = UR;
			innerUBPoint = LL;
		}
	} else {
		outerLBPoint = UR;
		outerUBPoint = LL;
		if (det > 0) {
			innerLBPoint = UL;
			innerUBPoint = LR;
		} else {
			innerLBPoint = LR;
			innerUBPoint = UL;
		}
	}

	/* Update outer loop header */
	IRBuilder builder(outerLoop->getHeader()->getContext());
	builder.SetInsertPoint(outerLoop->getHeader()->getFirstNonPHI());
	auto outerLowerBound = builder.CreateAdd(
			builder.CreateMul(IntToValue(T[0][0]), std::get<0>(outerLBPoint)),
			builder.CreateMul(IntToValue(T[0][1]), std::get<1>(outerLBPoint)), "p.lower");
	auto outerIV = builder.CreatePHI(Int32Ty, 2, "p");

	/* Update outer loop latch */
	builder.SetInsertPoint(outerLoop->getLoopLatch()->getTerminator());
	auto outerIncrement = builder.CreateAdd(outerIV, IntToValue(H[0][0]), "p.inc");
	outerIV->addIncoming(outerLowerBound, outerLoop->getLoopPreheader());
	outerIV->addIncoming(outerIncrement, outerLoop->getLoopLatch());
	auto outerUpperBound = builder.CreateAdd(
			builder.CreateMul(IntToValue(T[0][0]), std::get<0>(outerUBPoint)),
			builder.CreateMul(IntToValue(T[0][1]), std::get<1>(outerUBPoint)), "p.upper");
	auto outerComp = builder.CreateCmp(CmpInst::ICMP_SLE, outerIncrement, outerUpperBound);
	auto outerBranch = builder.CreateCondBr(outerComp, outerLoop->getHeader(), outerLoop->getExitBlock());

	/* Determine values for inner loop bounds */
	builder.SetInsertPoint(innerLoop->getLoopPreheader()->getTerminator());

	auto l1 = builder.CreateSub(
			builder.CreateSub(outerIV, builder.CreateMul(IntToValue(T[0][0]), std::get<0>(innerLBPoint))),
			builder.CreateMul(IntToValue(T[0][1]), std::get<1>(innerLBPoint)), "l1");

	// TODO: Change the check for zero to remove the zero division entirely
	auto l1Ceil = builder.CreateAdd(
		builder.CreateCall(maxFunc, {
			(T[0][0] == 0) ? IntToValue(INT_MIN) : builder.CreateAdd(
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
			builder.CreateMul(IntToValue(T[1][0]), std::get<0>(innerLBPoint)),
			builder.CreateMul(IntToValue(T[1][1]), std::get<1>(innerLBPoint))
		),
		"l1.ceil"
	);

	auto l3 = builder.CreateSub(
			builder.CreateSub(outerIV, builder.CreateMul(IntToValue(T[0][0]), std::get<0>(innerUBPoint))),
			builder.CreateMul(IntToValue(T[0][1]), std::get<1>(innerUBPoint)), "l3");

	/* Upper bound for inner loop */
	// TODO: Change the check for zero to remove the zero division entirely
	auto innerUpperBound = builder.CreateAdd(
			builder.CreateCall(minFunc, {
				(T[0][0] == 0) ? IntToValue(MAX_INT) :builder.CreateAdd(
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
				builder.CreateMul(IntToValue(T[1][0]), std::get<0>(innerUBPoint)),
				builder.CreateMul(IntToValue(T[1][1]), std::get<1>(innerUBPoint))
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

	addStringMetadataToLoop(innerLoop, "llvm.loop.parallel_accesses");
	addStringMetadataToLoop(innerLoop, "llvm.mem.parallel_loop_access");
	addStringMetadataToLoop(innerLoop, "llvm.loop.vectorize.enable");

	auto boo = innerLoop->isAnnotatedParallel();

	dbgs() << "================================\n";
	dbgs() << "Performed polytope optimisation\n";
	PrintTransform(T);
	dbgs() << "================================\n";

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

void PolytopePass::PrintTransform(const std::vector<std::vector<int>>& T) {
	dbgs() << "Selected transform:\n";
	for (auto row : T) {
		dbgs() << "(";
		for (auto col= row.begin(); col != row.end(); col++) {
			if (col != row.begin()) {
				dbgs() << ", ";
			}
			dbgs() << *col;
		}
		dbgs() << ")\n";
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