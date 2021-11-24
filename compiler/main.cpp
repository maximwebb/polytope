#include <llvm/Support/raw_ostream.h>
#include "parser.h"
#include "generator.h"

void Parse() {
	GetNextToken();
	module = std::make_unique<Module>("m", ctx);
	driver();
	std::cout << "\e[1m=============" << std::endl;
	std::cout << "GENERATED IR:" << std::endl;
	std::cout << "=============\e[0m" << std::endl;

	module->print(outs().changeColor(raw_ostream::CYAN), nullptr);
}

int main() {
	Parse();
	return 0;
}