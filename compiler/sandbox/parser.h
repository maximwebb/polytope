#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lexer.h"

class ExprAST {
public:
	virtual ~ExprAST() {}
};

class NumberExprAST : public ExprAST {
private:
	double val;
public:
	explicit NumberExprAST(double val) : val(val) {}
};

class VariableExprAST : public ExprAST {
private:
	std::string name;
public:
	explicit VariableExprAST(std::string name) : name(std::move(name)) {}
};

class BinaryExprAST : public ExprAST {
private:
	char op;
	std::unique_ptr<ExprAST> left, right;
public:
	BinaryExprAST(char op, std::unique_ptr<ExprAST> left, std::unique_ptr<ExprAST> right)
			: op(op), left(std::move(left)), right(std::move(right)) {}
};

class CallExprAST : public ExprAST {
private:
	std::string callee;
	std::vector<std::unique_ptr<ExprAST>> args;
public:
	CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
			: callee(std::move(callee)), args(std::move(args)) {}
};

class PrototypeAST {
private:
	std::string name;
	std::vector<std::string> args;
public:
	PrototypeAST(std::string name, std::vector<std::string> args)
			: name(std::move(name)), args(std::move(args)) {};
};

class FunctionAST {
private:
	std::unique_ptr<PrototypeAST> prototype;
	std::unique_ptr<ExprAST> body;
public:
	FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::unique_ptr<ExprAST> body)
			: prototype(std::move(prototype)), body(std::move(body)) {}
};


static int g_token;

static int GetNextToken() {
	return g_token = GetToken();
}

static std::unique<ExprAST> ParseExpr() {
	return 0;
}

static std::unique_ptr<ExprAST> ParseNumberExpr() {
	auto result = std::make_unique<NumberExprAST>(g_num_val);
	GetNextToken();
	return std::move(result);
}

static std::unique_ptr<ExprAST> ParseParenExpr() {
	getNextToken();
	auto result = ParseExpr();
	if (!result) {
		return nullptr;
	}
	if (g_token != ')') {
		std::cerr << "Expected ')' token" << std::endl;
	}
	GetNextToken();
	return result;
}

