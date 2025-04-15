#pragma once
#include <string>
#include <list>


#define OPERATION_TOKEN 0
#define OP_PLUS_EQUALS 0
#define OP_MINUS_EQUALS 1
#define OP_MUL_EQUALS 2
#define OP_EQUALS 3
#define OP_PLUS 4
#define OP_MINUS 5
#define OP_MUL 6
#define OP_DIV 7
#define OP_EQUALS_EQUALS 8
#define OP_GREATER 9
#define OP_LESS 10
#define OP_GREATER_EQUAL 11
#define OP_LESS_EQUAL 12
#define OP_INCREMENT 13
#define OP_DECREMENT 14

#define NAME_TOKEN 1
#define NAME_VAR 0
#define NAME_FUNC 1

#define CLAMP_TOKEN 2
#define CLAMP_PARANTHESES_R 0
#define CLAMP_PARANTHESES_L 1
#define CLAMP_BRACKETS_R 2
#define CLAMP_BRACKETS_L 3
#define CLAMP_SQUIGGLY_BRACKETS_R 4
#define CLAMP_SQUIGGLY_BRACKETS_L 5

#define CONSTANT_TOKEN 3
#define CONST_INT 0
#define CONST_CHAR 1

#define TYPE_TOKEN 4
#define TYPE_CHAR 0
#define TYPE_INT 1

#define IDENTIFIER_TOKEN 5
#define IDENTIFIER_IF 0 
#define IDENTIFIER_FOR 1
#define IDENTIFIER_WHILE 2

#define END_LINE_TOKEN 6

#define COMPARISON_TOKEN 7
#define COMPARISON_GR 0
#define COMPARISON_LW 1
#define COMPARISON_GE 2
#define COMPARISON_LE 3
#define COMPARISON_EQ 4

struct token {
	char type, subtype;
	token(char Type, char sub_type) : type(Type) , subtype(sub_type){}

	token(){}

	bool checkNextTokenCompatibility(token t);

	const char* typeToString();

	static token getTokenType(std::string str);
};

struct token_str {
	// token w/ str val of said token
	token t;
	std::string str;

	token_str(token T, std::string s) : t(T), str(s){}

	token_str(){}
};

std::list<token_str> returnTokenList();

void setSrc(std::string s);

void tokenizeSrc();

void advanceIterator();

bool isIteratorEnd();

const char* getStringIterator();

const char* getTypeStringIterator();

char getTypeIterator();

void setIterator();