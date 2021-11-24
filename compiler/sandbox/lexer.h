#pragma once
#include <iostream>
#include <string>
#include <map>

enum Token {
	tok_eof = -1,
	tok_def = -2,
	tok_extern = -3,
	tok_identifier = -4,
	tok_number = -5,
};

static std::string g_identifier_str;
static double g_num_val;

static int GetToken() {
	static int s_last_char = ' ';

	while (std::isspace(s_last_char)) {
		s_last_char = std::getchar();
	}

	if (std::isalpha(s_last_char)) {
		g_identifier_str = s_last_char;
		while (std::isalnum((s_last_char = std::getchar()))) {
			g_identifier_str += s_last_char;
		}

		if (g_identifier_str == "def") {
			return tok_def;
		}
		if (g_identifier_str == "extern") {
			return tok_extern;
		}
		return tok_identifier;
	}

	if (std::isdigit(s_last_char) || s_last_char == '.') {
		std::string NumStr;
		do {
			NumStr += s_last_char;
			s_last_char = std::getchar();
		} while (isdigit(s_last_char) || s_last_char == '.');
		g_num_val = std::strtod(NumStr.c_str(), nullptr);
		return tok_number;
	}

	if (s_last_char == '#') {
		do {
			s_last_char = std::getchar();
		} while (s_last_char != EOF && s_last_char != '\n' && s_last_char != '\r');

		if (s_last_char != EOF) {
			return GetToken();
		}
	}

	if (s_last_char == EOF) {
		return tok_eof;
	}
	int c = s_last_char;
	s_last_char = std::getchar();
	return c;
}

void PrintToken(int t) {
	if (t == tok_eof) {
		std::cout << "End of file" << std::endl;
	} else if (t == tok_def) {
		std::cout << "def token" << std::endl;
	} else if (t == tok_extern) {
		std::cout << "extern token" << std::endl;
	} else if (t == tok_identifier) {
		std::cout << "identifier token: " << g_identifier_str << std::endl;
	} else if (t == tok_number) {
		std::cout << "number token: " << g_num_val << std::endl;
	} else {
		std::cout << "Unrecognised token" << std::endl;
	}
}

static std::map<char, int> OperatorPrecedence = {
		{'<', 10},
		{'+', 20},
		{'-', 20},
		{'*', 40},
};