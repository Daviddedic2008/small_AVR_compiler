#include <iostream>
#include <sstream>
#include <string>
#include "tokenizer.h"

std::string src;
int charId = 0;

#define NUM_OP_CHARS 7
char OP_CHARS[] = { '=', '-', '*', '/', '+', '<', '>'};

#define NUM_COMPARISONS 5
const char* COMPARE_OPS[NUM_COMPARISONS] = { "<", ">", "==", "<=", ">=" };

#define NUM_SINGLE_CHARS 7
char SINGLE_CHARS[] = { '(', ')', '[', ']', '{', '}', ';' };

std::list<token_str> tokens;
std::list<token_str>::iterator iterator;

std::list<token_str> returnTokenList() {
	return tokens;
}

int charInOps(const char c) {
	for (int i = 0; i < NUM_OP_CHARS; i++) {
		if (OP_CHARS[i] == c) {
			return i;
		}
	}

	return -1;
}

int charInSingleChars(const char c) {
	for (int i = 0; i < NUM_SINGLE_CHARS; i++) {
		if (SINGLE_CHARS[i] == c) {
			return i;
		}
	}

	return -1;
}

void setSrc(std::string s) {
	src = s.c_str();
}

void setIterator() {
	iterator = tokens.begin();
}

void advanceIterator() {
	iterator++;
}

bool isIteratorEnd() {
	return iterator == tokens.end();
}

const char* getStringIterator() {
	return iterator->str.c_str();
}

const char* getTypeStringIterator() {
	return iterator->t.typeToString();
}

char getTypeIterator() {
	return iterator->t.type;
}

bool token::checkNextTokenCompatibility(token t) {
	switch (t.type) {
		case(OPERATION_TOKEN):
			return t.type == CONSTANT_TOKEN || t.type == NAME_TOKEN;
		case(CONSTANT_TOKEN):
			return t.type == END_LINE_TOKEN;
		case(NAME_TOKEN):
			return (subtype == NAME_VAR) ? (t.type == END_LINE_TOKEN || t.type == OPERATION_TOKEN) : (t.type == CLAMP_TOKEN);
		case(TYPE_TOKEN):
			return (t.type == NAME_TOKEN);
		case(IDENTIFIER_TOKEN):
			return (t.type == CLAMP_TOKEN);
		case(CLAMP_TOKEN):
			return true;
		case(END_LINE_TOKEN):
			return true;
		default:
			return false;
	}
}

inline bool strcmpd(std::string s, const char* c) {
	return strcmp(s.c_str(), c) == 0;
}

token token::getTokenType(std::string str) {
	if (str.at(0) == '\'') {
		return token(CONSTANT_TOKEN, CONST_CHAR);
	}
	for (int i = 0; i < str.size(); i++) {
		if (!isdigit(str.at(i))) {
			goto skpdigit;
		}
	}
	return token(CONSTANT_TOKEN, CONST_INT);
skpdigit:
	
	if (strcmpd(str, "+")) {
		return token(OPERATION_TOKEN, OP_PLUS);
	}
	if (strcmpd(str, "-")) {
		return token(OPERATION_TOKEN, OP_MINUS);
	}
	if (strcmpd(str, "*")) {
		return token(OPERATION_TOKEN, OP_MUL);
	}
	if (strcmpd(str, "=")) {
		return token(OPERATION_TOKEN, OP_EQUALS);
	}
	if (strcmpd(str, "+=")) {
		return token(OPERATION_TOKEN, OP_PLUS_EQUALS);
	}
	if (strcmpd(str, "++")) {
		return token(OPERATION_TOKEN, OP_INCREMENT);
	}
	if (strcmpd(str, "--")) {
		return token(OPERATION_TOKEN, OP_DECREMENT);
	}
	if (strcmpd(str, "-=")) {
		return token(OPERATION_TOKEN, OP_MINUS_EQUALS);
	}
	if (strcmpd(str, "*=")) {
		return token(OPERATION_TOKEN, OP_MUL_EQUALS);
	}
	if (strcmpd(str, ">")) {
		return token(COMPARISON_TOKEN, COMPARISON_GR);
	}
	if (strcmpd(str, ">=")) {
		return token(COMPARISON_TOKEN, COMPARISON_GE);
	}
	if (strcmpd(str, "<")) {
		return token(COMPARISON_TOKEN, COMPARISON_LW);
	}
	if (strcmpd(str, "<=")) {
		return token(COMPARISON_TOKEN, COMPARISON_LE);
	}
	if (strcmpd(str, "==")) {
		return token(COMPARISON_TOKEN, COMPARISON_EQ);
	}
	if (strcmpd(str, "(")) {
		return token(CLAMP_TOKEN, CLAMP_PARANTHESES_L);
	}
	if (strcmpd(str, ")")) {
		return token(CLAMP_TOKEN, CLAMP_PARANTHESES_R);
	}
	if (strcmpd(str, "[")) {
		return token(CLAMP_TOKEN, CLAMP_BRACKETS_L);
	}
	if (strcmpd(str, "]")) {
		return token(CLAMP_TOKEN, CLAMP_BRACKETS_R);
	}
	if (strcmpd(str, "{")) {
		return token(CLAMP_TOKEN, CLAMP_SQUIGGLY_BRACKETS_L);
	}
	if (strcmpd(str, "}")) {
		return token(CLAMP_TOKEN, CLAMP_SQUIGGLY_BRACKETS_R);
	}
	if (strcmpd(str, "char")) {
		return token(TYPE_TOKEN, TYPE_CHAR);
	}
	if (strcmpd(str, "int")) {
		return token(TYPE_TOKEN, TYPE_INT);
	}
	if (strcmpd(str, ";")) {
		return token(END_LINE_TOKEN, 0);
	}
	if (strcmpd(str, "if")) {
		return token(IDENTIFIER_TOKEN, IDENTIFIER_IF);
	}
	if (strcmpd(str, "or")) {
		return token(IDENTIFIER_TOKEN, IDENTIFIER_OR);
	}
	if (strcmpd(str, "for")) {
		return token(IDENTIFIER_TOKEN, IDENTIFIER_FOR);
	}
	return token(NAME_TOKEN, -1);
}
const char* token::typeToString() {
	switch (type) {
		case(OPERATION_TOKEN):
			return "operation token";
		case(NAME_TOKEN):
			return "name token";
		case(IDENTIFIER_TOKEN):
			return "identifier";
		case(CONSTANT_TOKEN):
			return "constant token";
		case(TYPE_TOKEN):
			return "type token";
		case(END_LINE_TOKEN):
			return "end line token";
		case(CLAMP_TOKEN):
			return "clamp token";
		case(COMPARISON_TOKEN):
			return "comparison token";
		default:
			return "";
	}
}

bool charEndToken(char c) {
	return c == '{' || c == '}' || c == '[' || c == ']' || c == '(' || c == ')' || c == '+' || c == '-' || c == ' ' || c == '=' || c == '*' || c == '&' || c == '\n' || c == '~' || c == ';' || c == '<' || c == '>';
}

char peekAtNextChar() {
	return (charId < src.size() - 1) ? src.at(charId + 1) : '~';
}

char peekAtCurrentChar() {
	return src.at(charId);
}

char advanceChar() {
	if (charId < src.size()) {
		charId++;
		return src.at(charId-1);
	}
	return '~';
}

token_str advanceToken() {
	std::string retString;

	if (charId == src.size()) {
		return token_str(token(-1, -1), "~");
	}
	
	int inOps = charInOps(peekAtCurrentChar());
	int inOpsNext = charInOps(peekAtNextChar());
	if (inOps != -1) {
		if (inOpsNext == 0 || (inOps != 2 && inOps != 3 && inOps == inOpsNext)) {
			retString += advanceChar();
			retString += advanceChar();
			return token_str(token::getTokenType(retString), retString);
		}
		retString += advanceChar();
		return token_str(token::getTokenType(retString), retString);
	}

	if (charInSingleChars(peekAtCurrentChar()) != -1) {
		retString += advanceChar();
		return token_str(token::getTokenType(retString), retString);
	}

	if (isdigit(peekAtCurrentChar())) {
		while (true) {
			retString += advanceChar();
			if (!isdigit(peekAtCurrentChar())) {
				return token_str(token(CONSTANT_TOKEN, CONST_INT), retString);
			}
		}
	}

	if (peekAtCurrentChar() == ' ' || peekAtCurrentChar() == '\n' || peekAtCurrentChar() == '\t') {
		advanceChar();
		return token_str(token(-1, -1), "");
	}

	retString += advanceChar();
	while (!charEndToken(peekAtCurrentChar())) {
		retString += advanceChar();
	}

	return token_str(token::getTokenType(retString), retString);
}

void tokenizeSrc() {
	while (true) {
		token_str s = advanceToken();
		if (s.str == "~") {
			return;
		}
		tokens.push_back(s);
	}
}