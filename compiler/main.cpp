#include "lexer.h"

int main() {
	int t;
	while ((t = GetToken()) != tok_eof) {
		PrintToken(t);
	}
	return 0;
}