#include "parser.h"

using namespace llvm;

// Contains core LLVM data structures like type/constant value tables.
static LLVMContext ctx;
// Helper object used to build LLVM instructions.
static IRBuilder<> builder(ctx);
// Contains functions and global variables - the top-level structure used by LLVM IR to hold code.
static std::unique_ptr<Module> module;
// Keeps track of which values are in the current scope & their LLVM representation.
static std::map<std::string, Value*> named_values;

Value* LogErrorV(const char* s) {
	std::cerr << s << std::endl;
	return nullptr;
}

Value* NumberExprAST::CodeGen() {
	return ConstantFP::get(ctx, APFloat(val));
}

Value* VariableExprAST::CodeGen() {
	Value* v = named_values[name];
	if (!v) {
		LogErrorV("Unknown variable name");
	}
	return v;
}

Value* BinaryExprAST::CodeGen() {
	Value* l = left->CodeGen();
	Value* r = right->CodeGen();

	if (!l || !r) {
		return nullptr;
	}

	switch (op) {
		case '+':
			return builder.CreateFAdd(l, r, "addtmp");
		case '-':
			return builder.CreateFSub(l, r, "subtmp");
		case '*':
			return builder.CreateFMul(l, r, "multmp");
		case '<':
			l = builder.CreateFCmpULT(l, r, "cmptmp");
			return builder.CreateUIToFP(l, Type::getDoubleTy(ctx), "booltmp");
		default:
			return LogErrorV("Unrecognised binary operator");
	}
}

Value* CallExprAST::CodeGen() {
	Function* calleeF = module->getFunction(callee);
	if (!calleeF) {
		return LogErrorV("Reference to unknown function");
	}
	if (calleeF->arg_size() != args.size()) {
		return LogErrorV("Incorrect number of arguments");
	}

	std::vector<Value*> argsV;
	for (auto& arg: args) {
		argsV.push_back(arg->CodeGen());
		if (!argsV.back()) {
			return nullptr;
		}
	}
	return builder.CreateCall(calleeF, argsV, "calltmp");
}

Function* PrototypeAST::CodeGen() {
	// Create a list of double types matching the arity of the function
	std::vector<Type*> doubles(args.size(), Type::getDoubleTy(ctx));

	// Construct the function type (which always returns a double)
	FunctionType* ft = FunctionType::get(Type::getDoubleTy(ctx), doubles, false);

	// Create the function prototype itself
	Function* f = Function::Create(ft, Function::ExternalLinkage, name, module.get());

	// Set the names for each function parameter
	unsigned idx = 0;
	for (auto& arg: f->args()) {
		arg.setName(args[idx++]);
	}
	return f;
}

Function* FunctionAST::CodeGen() {
	Function* func = module->getFunction(prototype->GetName());

	if (!func) {
		func = prototype->CodeGen();

		if (!func) {
			return nullptr;
		}
	}

	if (!func->empty()) {
		return (Function*)LogErrorV("Redefinition of function");
	}

	// All functions only contain a single block at the moment. This will change when control flow is added.
	BasicBlock* block = BasicBlock::Create(ctx, "entry", func);
	builder.SetInsertPoint(block);

	named_values.clear();
	for (auto& arg : func->args()) {
		named_values[arg.getName()] = &arg;
	}

	if (Value* return_val = body->CodeGen()) {
		builder.CreateRet(return_val);
//		verifyFunction(*func);
		return func;
	}
	func->eraseFromParent();
	return nullptr;
}