#include "headers/tokenizer.h"
#include "headers/parser.h"
#include "headers/opcodes.h"
#include "headers/variableManager.h"

#include "boardType.h"

const char tempRegs[] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool takenTempRegs[] = { true, false, false , false, false, false, false , false , false, false, false , false , false, false, false , false, false };

#define DELETE_REG 0

unsigned char currentScope = 0;

std::vector<storedVar> allVariables;

std::vector<storedVar> currentStack;

void addVariable(const char* name, const char size) {
	storedVar ret;
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
	int totalSize = 0;
	for (auto v = currentStack.begin(); v != currentStack.end(); v++) {
		if (v->scopeValue > currentScope) {
			totalSize += v->size;
			currentStack.erase(v);
			continue;
		}
		break;
	}

	for (int i = 0; i < totalSize; i++) {
		pop(DELETE_REG);
	}
}

int findVariableInStack(const storedVar& v) {
	int ret = -1;

	int idx = 0;
	for (storedVar tmp : currentStack) {
		ret = (tmp == v) ? idx : ret;
		idx++;
	}

	return ret;
}

void readStackIntoRegisters(storedVar& v) {
	int sr = 1;

	const int stackIdx = findVariableInStack(v);

	int szBefore = 0;

	for (int i = 0; i < stackIdx; i++) {
		szBefore += currentStack[i].size;
	}

	for(int cc = 0; sr < 17 && cc < v.size; cc = takenTempRegs[sr+cc] ? 0 : (cc + 1), sr += takenTempRegs[sr + cc] * (cc+1)) {}

	v.startRegister = sr;

	for (int i = 0; i < v.size; i++) {
		takenTempRegs[i + sr] = true;
		lds(i + sr, RAMEND - szBefore - i);
	}
}

void freeStackFromRegisters(const storedVar& v) {
	for (int r = v.startRegister; r < v.startRegister + v.size; r++) {
		takenTempRegs[r] = false;
	}
}