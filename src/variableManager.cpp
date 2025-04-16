#include "headers/tokenizer.h"
#include "headers/parser.h"
#include "headers/opcodes.h"
#include <vector>

const char tempRegs[] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool takenTempRegs[] = { false, false, false , false, false, false, false , false , false, false, false , false , false, false, false , false, false };

struct storedVar {
	char size; // 1 or 4 bytes (for now)
	char startRegister; // -1 if not in regs
	unsigned char scopeValue;
	const char* name;
};

std::vector<storedVar> allVariables;

std::vector<storedVar> currentStack;

unsigned char currentScope = 0;

void addVariable(const char* name, const char size) {
	storedVar ret;
	ret.name = name;
	ret.size = size;
	ret.scopeValue = currentScope;
}

void pushVariable(storedVar& v) {
	currentStack.push_back(v);
	if (v.startRegister != -1) {
		for (char r = v.startRegister; r < v.startRegister + v.size; r++) {
			takenTempRegs[r] = false;
			push(r);
		}
	}
	v.startRegister = -1;
}

void incrementScope() {
	currentScope++;
}

void decrementScope() {
	currentScope--;
	for (storedVar v : currentStack) {
		if (v.scopeValue > currentScope) {

		}
	}
}