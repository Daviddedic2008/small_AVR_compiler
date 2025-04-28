#include "headers/tokenizer.h"
#include "headers/parser.h"
#include "headers/opcodes.h"
#include "headers/variableManager.h"

#include "SETUP/boardType.h"

// prologue/epilogue stuff

const char tempRegs[] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool takenTempRegs[] = { true, false, false , false, false, false, false , false , false, false, false , false , false, false, false , false, false };

#define DELETE_REG 0

unsigned char currentScope = 0;

std::vector<storedVar> allVariables;

std::vector<storedVar> currentStack;

void addVariable(const char* name, const char size) {
	storedVar add = storedVar(size, name);
	currentStack.push_back(add);
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

	sr = 0;
	bool foundSpot = false;
	for (int cc = 0; sr+cc < 17; cc = takenTempRegs[sr + cc] ? 0 : (cc + 1), sr = takenTempRegs[sr + cc] ? (sr + cc + 1) : sr) {
		if (cc == v.size) {
			foundSpot = true;
			break;
		}
	}

	if (foundSpot = false) {
		throw std::exception("no no no");
	}

	for (int i = 0; i < v.size; i++) {
		takenTempRegs[i + sr] = true;
		lds(i + sr, RAMEND - szBefore - i);
	}
}

void freeStackFromRegisters(storedVar& v) {
	for (int r = v.startRegister; r < v.startRegister + v.size; r++) {
		takenTempRegs[r] = false;
	}
	v.startRegister = -1;
}

void saveVariableFromRegisters(storedVar& v) {
	const int stackIdx = findVariableInStack(v);

	int szBefore = 0;

	for (int i = 0; i < stackIdx; i++) {
		szBefore += currentStack[i].size;
	}

	for (int i = 0; i < v.size; i++) {
		sts(RAMEND - szBefore - i, v.startRegister + i);
	}

	freeStackFromRegisters(v);
}