#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

#include "lexer.h"

class ExprAST {
public:
	virtual ~ExprAST() {}
	virtual void Print(int indent = 0) const = 0;
};

class NumberExprAST : public ExprAST {
private:
	double val;
public:
	explicit NumberExprAST(double val) : val(val) {}
	void Print(int indent = 0) const override {
		auto s = std::string(indent * 2, ' ') + "| ";
		std::cout << s << "NUM " << val << std::endl;
	}
};

class VariableExprAST : public ExprAST {
private:
	std::string name;
public:
	explicit VariableExprAST(std::string name) : name(std::move(name)) {}
	void Print(int indent = 0) const override {
		auto s = std::string(indent * 2, ' ') + "| ";
		std::cout << s << "VAR " << name << std::endl;
	}
};

class BinaryExprAST : public ExprAST {
private:
	char op;
	std::unique_ptr<ExprAST> left, right;
public:
	BinaryExprAST(char op, std::unique_ptr<ExprAST> left, std::unique_ptr<ExprAST> right)
			: op(op), left(std::move(left)), right(std::move(right)) {}

	void Print(int indent = 0) const override {
		auto s = std::string(indent * 2, ' ') + "| ";
		std::cout << s << "OP " << op << std::endl;
		left->Print(indent + 1);
		right->Print(indent + 1);
	}
};

class CallExprAST : public ExprAST {
private:
	std::string callee;
	std::vector<std::unique_ptr<ExprAST>> args;
public:
	CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
			: callee(std::move(callee)), args(std::move(args)) {}

	void Print(int indent = 0) const override {
		auto s = std::string(indent * 2, ' ') + "| ";
		std::cout << s << "CALL " << callee << std::endl;
		for (auto const& arg: args) {
			arg->Print(indent + 1);
		}
	}
};

class PrototypeAST : ExprAST {
private:
	std::string name;
	std::vector<std::string> args;
public:
	PrototypeAST(std::string name, std::vector<std::string> args)
			: name(std::move(name)), args(std::move(args)) {};

	void Print(int indent = 0) const override {
		auto s = std::string(indent * 2, ' ') + "| ";
		std::cout << s << "NAME " << name << std::endl;
		std::string arg_list = std::accumulate(std::begin(args), std::end(args), std::string(),
									 [](const std::string& ss, const std::string& s) {
													return ss.empty()? s : ss + ", " + s;
												});
		std::cout << s << "ARGS " << arg_list << std::endl;
	}
};

class FunctionAST : ExprAST {
private:
	std::unique_ptr<PrototypeAST> prototype;
	std::unique_ptr<ExprAST> body;
public:
	FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::unique_ptr<ExprAST> body)
			: prototype(std::move(prototype)), body(std::move(body)) {}

	void Print(int indent = 0) const override {
		auto s = std::string(indent * 2, ' ') + "| ";
		std::cout << s << "FUNC  " << std::endl;
		prototype->Print(indent + 1);
		body->Print(indent + 1);
	}
};


static int g_token;

static int GetNextToken() {
	g_token = GetToken();
//	PrintToken(g_token);
	return g_token;
}

static std::unique_ptr<ExprAST> ParseExpr();

static std::unique_ptr<ExprAST> ParseNumberExpr() {
	auto result = std::make_unique<NumberExprAST>(g_num_val);
	GetNextToken();
	return std::move(result);
}

static std::unique_ptr<ExprAST> ParseParenExpr() {
	GetNextToken();
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

static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
	std::string name = g_identifier_str;
	GetNextToken();
	if (g_token == '(') {
		GetNextToken();
		std::vector<std::unique_ptr<ExprAST>> args;
		if (g_token != ')') {
			while (true) {
				if (auto arg = ParseExpr()) {
					args.push_back(std::move(arg));
				} else {
					return nullptr;
				}
				if (g_token == ')') {
					GetNextToken();
					return std::make_unique<CallExprAST>(name, std::move(args));
				}
				if (g_token != ',') {
					std::cerr << "Error: arguments must be comma-separated list." << std::endl;
				}
				GetNextToken();
			}
		}
	}
	return std::make_unique<VariableExprAST>(name);
}

static std::unique_ptr<ExprAST> ParsePrimary() {
	switch (g_token) {
		default:
			std::cerr << "Error: unexpected token: " << g_token << std::endl;
		case tok_identifier:
			return ParseIdentifierExpr();
		case tok_number:
			return ParseNumberExpr();
		case '(':
			return ParseParenExpr();
	}
}

static int GetTokenPrecedence() {
	if (!isascii(g_token)) {
		return -1;
	}

	int precedence = OperatorPrecedence[g_token];
	if (precedence <= 0) {
		return -1;
	}
	return precedence;
}

static std::unique_ptr<ExprAST> ParseBinaryRight(int min_precedence, std::unique_ptr<ExprAST> left) {
	while (true) {
		int precedence = GetTokenPrecedence();

		if (precedence < min_precedence) {
			return left;
		}

		int binary_op = g_token;
		GetNextToken();

		auto right = ParsePrimary();
		if (!right) {
			return nullptr;
		}

		int next_precedence = GetTokenPrecedence();
		if (precedence < next_precedence) {
			right = ParseBinaryRight(precedence + 1, std::move(right));
			if (!right) {
				return nullptr;
			}std::cout << "Parsed function definition" << std::endl;
		}
		left = std::make_unique<BinaryExprAST>(binary_op, std::move(left), std::move(right));
	}
}

static std::unique_ptr<PrototypeAST> ParsePrototype() {
	if (g_token != tok_identifier) {
		std::cerr << "Expected function name in prototype" << std::endl;
		return nullptr;
	}

	std::string name = g_identifier_str;
	GetNextToken();

	if (g_token != '(') {
		std::cerr << "Expected '(' after function name" << std::endl;
		return nullptr;
	}

	std::vector<std::string> args;
	while (GetNextToken()) {
		args.push_back(g_identifier_str);
		GetNextToken();

		if (g_token == ')') {
			GetNextToken();
			break;
		}

		if (g_token != ',') {
			std::cerr << "Expected comma-separated list in function prototype" << std::endl;
			return nullptr;
		}
	}
	return std::make_unique<PrototypeAST>(name, std::move(args));
}

static std::unique_ptr<FunctionAST> ParseDefinition() {
	GetNextToken();std::cout << "Parsed function definition" << std::endl;
	auto prototype = ParsePrototype();
	if (!prototype) {
		return nullptr;
	}
	auto body = ParseExpr();
	if (!body) {
		return nullptr;
	}
	return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

static std::unique_ptr<PrototypeAST> ParseExtern() {
	GetNextToken();
	return ParsePrototype();
}

static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
	if (auto e = ParseExpr()) {
		auto prototype = std::make_unique<PrototypeAST>("", std::vector<std::string>());
		return std::make_unique<FunctionAST>(std::move(prototype), std::move(e));
	}
	return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpr() {
	auto left = ParsePrimary();
	if (!left) {
		return nullptr;
	}

	return ParseBinaryRight(0, std::move(left));
}

static void HandleDefinition() {
	if (auto e = ParseDefinition()) {
		e->Print();
	} else {
		std::cout << "Error handling definition." << std::endl;
		GetNextToken();
	}
}

static void HandleExtern() {
	if (auto e = ParseExtern()) {
		e->Print();
	} else {
		std::cout << "Error handling extern." << std::endl;
		GetNextToken();
	}
}

static void HandleTopLevelExpr() {
	if (auto e = ParseTopLevelExpr()) {
		e->Print();
	} else {
		std::cout << "Error parsing top-level expression." << std::endl;
		GetNextToken();
	}
}

static void PrintAST() {

}

static void driver() {
	std::cout << "Parsing File" << std::endl;
	while (true) {
		switch (g_token) {
			case tok_eof:
				return;
			case ';':
				GetNextToken();
				break;
			case tok_def:
				HandleDefinition();
				break;
			case tok_extern:
				HandleExtern();
				break;
			default:
				HandleTopLevelExpr();
				break;
		}
	}
}