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
								  scale * ValueToInt(MulInstr->getOperand(0)));
		} else if (isa<Constant>(MulInstr->getOperand(1))) {
			return GetAffineValue(MulInstr->getOperand(0), Transform, Visited,
								  scale * ValueToInt(MulInstr->getOperand(1)));
		}
		return {};
	}
	if (isa<ShlOperator>(V)) {
		auto* ShlInstr = dyn_cast<ShlOperator>(V);
		if (isa<Constant>(ShlInstr->getOperand(1))) {
			auto Multiplier = 1 << ValueToInt(ShlInstr->getOperand(1));
			return GetAffineValue(ShlInstr->getOperand(0), Transform, Visited, scale * Multiplier);
		}
		return {};
	}
	/* Exclusively for testing XOR */
	if (isa<BinaryOperator>(V)) {
		auto* BinInstr = dyn_cast<BinaryOperator>(V);
		/* The instruction %1 = xor k -1 simplifies to %1 = -k - 1 */
		if (BinInstr->getOpcode() == Instruction::Xor && ValueToInt(BinInstr->getOperand(1)) == -1) {
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
	if (!RunAnalysis(L, AR)) {
		return PreservedAnalyses::all();
	}

	auto assignment = ExtractArrayAccesses(L, AR);
	auto transformation = ComputeAffineTransformation(assignment);
	auto T = transformation.value();
//	std::vector<std::vector<int>> T = {{4, 2},
//									   {1, 3}};
	auto H = IntegerSolver::HermiteNormal(T);

	auto outerBounds = outerLoop->getBounds(AR.SE).getValue();
	auto innerBounds = innerLoop->getBounds(AR.SE).getValue();

	auto Int32Ty = outerLoop->getInductionVariable(AR.SE)->getType();
	auto FloatTy = Type::getFloatTy(outerLoop->getHeader()->getContext());

	/* Extract redundant IV instructions */
	auto oldOuterIV = outerLoop->getInductionVariable(AR.SE);
	auto oldInnerIV = innerLoop->getInductionVariable(AR.SE);
	auto oldOuterComparison = outerLoop->getLatchCmpInst();
	auto oldInnerComparison = innerLoop->getLatchCmpInst();
	/* Retrieve increment instructions by analysing comparison */
	auto oldOuterIncrement = cast<Instruction>(oldOuterComparison->getOperand(0));
	auto oldInnerIncrement = cast<Instruction>(oldInnerComparison->getOperand(0));
	auto oldOuterBranch = FindInstr(Instruction::Br, outerLoop->getLoopLatch()).value();
	auto oldInnerBranch = FindInstr(Instruction::Br, innerLoop->getLoopLatch()).value();

	/* Update outer loop header */
	IRBuilder builder(outerLoop->getHeader()->getContext());
	builder.SetInsertPoint(outerLoop->getHeader()->getFirstNonPHI());
	auto outerLowerBound = builder.CreateAdd(
			builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getInitialIVValue()),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getInitialIVValue()), "p.lower");
	auto outerIV = builder.CreatePHI(Int32Ty, 2, "p");
//	auto iNew = builder.CreateAdd(outerIV, IntToValue(9), "i.new");

	/* Update outer loop latch */
	builder.SetInsertPoint(outerLoop->getLoopLatch()->getTerminator());
	auto outerIncrement = builder.CreateAdd(outerIV, IntToValue(H[0][0]), "p.inc");
	outerIV->addIncoming(outerLowerBound, outerLoop->getLoopPreheader());
	outerIV->addIncoming(outerIncrement, outerLoop->getLoopLatch());
	auto outerUpperBound = builder.CreateAdd(
			builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getFinalIVValue()),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getFinalIVValue()), "p.upper");
	auto outerComp = builder.CreateCmp(CmpInst::ICMP_NE, outerIncrement, outerUpperBound);
	auto outerBranch = builder.CreateCondBr(outerComp, outerLoop->getHeader(), outerLoop->getExitBlock());

	/* Determine values for inner loop bounds */
	builder.SetInsertPoint(outerLoop->getHeader()->getTerminator());
	auto l1 = builder.CreateSub(
			builder.CreateSub(outerIV, builder.CreateMul(IntToValue(T[0][0]), &outerBounds.getFinalIVValue())),
			builder.CreateMul(IntToValue(T[0][1]), &innerBounds.getInitialIVValue()), "l1");

	auto l1Ceil = builder.CreateAdd(
		builder.CreateCast(
			Instruction::FPToSI,
			builder.CreateMaximum(
				builder.CreateCast(
					Instruction::SIToFP,
					builder.CreateAdd(
						builder.CreateMul(
							IntToValue(T[1][0]),
							builder.CreateSDiv(l1, IntToValue(T[0][0]))
						),
						builder.CreateCast(
							Instruction::FPToSI,
							builder.CreateMinimum(
								builder.CreateCast(Instruction::SIToFP, builder.CreateSRem(l1, IntToValue(T[0][0])), FloatTy),
								builder.CreateCast(Instruction::SIToFP, IntToValue(1), FloatTy)
							),
							Int32Ty
						)
					),
					FloatTy
				),
				builder.CreateCast(
					Instruction::SIToFP,
					builder.CreateAdd(
						builder.CreateMul(
							IntToValue(T[1][1]),
							builder.CreateSDiv(l1, IntToValue(T[0][1]))
						),
						builder.CreateCast(
							Instruction::FPToSI,
							builder.CreateMinimum(
								builder.CreateCast(Instruction::SIToFP, builder.CreateSRem(l1, IntToValue(T[0][1])), FloatTy),
								builder.CreateCast(Instruction::SIToFP, IntToValue(1), FloatTy)
							),
							Int32Ty
						)
					),
					FloatTy
				)
			),
			Int32Ty
		),
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
			builder.CreateCast(
				Instruction::FPToSI,
				builder.CreateMinimum(
					builder.CreateCast(
						Instruction::SIToFP,
						builder.CreateAdd(
							builder.CreateMul(
								IntToValue(T[1][0]),
								builder.CreateSDiv(l3, IntToValue(T[0][0]))
							),
							builder.CreateCast(
								Instruction::FPToSI,
								builder.CreateMinimum(
									builder.CreateCast(Instruction::SIToFP, builder.CreateSRem(l3, IntToValue(T[0][0])), FloatTy),
									builder.CreateCast(Instruction::SIToFP, IntToValue(1), FloatTy)
								),
								Int32Ty
							)
						),
						FloatTy
					),
					builder.CreateCast(
						Instruction::SIToFP,
						builder.CreateAdd(
							builder.CreateMul(
								IntToValue(T[1][1]),
								builder.CreateSDiv(l3, IntToValue(T[0][1]))
							),
							builder.CreateCast(
								Instruction::FPToSI,
								builder.CreateMinimum(
									builder.CreateCast(Instruction::SIToFP, builder.CreateSRem(l3, IntToValue(T[0][1])), FloatTy),
									builder.CreateCast(Instruction::SIToFP, IntToValue(1), FloatTy)
								),
								Int32Ty
							)
						),
						FloatTy
					)
				),
		   	Int32Ty
		   ),
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
	builder.SetInsertPoint(outerLoop->getHeader()->getTerminator());
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
	innerIV->addIncoming(innerLowerBound, outerLoop->getHeader());
	innerIV->addIncoming(innerIncrement, innerLoop->getLoopLatch());

	auto innerComp = builder.CreateCmp(CmpInst::ICMP_NE, innerIncrement, innerUpperBound);
	auto innerBranch = builder.CreateCondBr(innerComp, innerLoop->getHeader(), innerLoop->getExitBlock());


	/* Clean up old induction variables */
	oldOuterIV->replaceAllUsesWith(iNew);
	oldInnerIV->replaceAllUsesWith(jNew);
	oldOuterIncrement->eraseFromParent();
	oldInnerIncrement->eraseFromParent();
	oldOuterComparison->eraseFromParent();
	oldInnerComparison->eraseFromParent();
	oldOuterBranch->eraseFromParent();
	oldInnerBranch->eraseFromParent();

	return PreservedAnalyses::all();
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