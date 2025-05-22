#include "headers/tokenizer.h"
#include "headers/parser.h"
#include "headers/opcodes.h"
#include "headers/variableManager.h"

#include "SETUP/boardType.h"

// prologue/epilogue stuff

const char tempRegs[] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool takenTempRegs[] = { true, true, true, false, false, false, false , false , false, false, false , false , false, false, false , false, false };

#define DELETE_REG 0

#define DUMP_REG 16

#define TEMP_REG 17

#define RESULT_REG 18

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

storedVar findVariableFromName(const char* name) {
	for (const storedVar v : currentStack) {
		if (strcmp(name, v.name) == 0) {
			return v;
		}
	}

	return storedVar();
}

int findDistanceFromStackStart(const storedVar& v) {
	int ret = 0;

	for (storedVar tmp : currentStack) {
		if (tmp == v) {
			return ret;
		}
		ret += tmp.size;
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

int referenceVariable(const storedVar& v) {

	int idx = 0;
	for (storedVar tmp : currentStack) {
		if (tmp == v) {
			break;
		}
		idx += tmp.size;
	}

	return idx;
}

void writeValueToVariable(const storedVar& v, const unsigned int value) {
	const int dist = findDistanceFromStackStart(v);
	
	for (int b = 0; b < v.size; b++) {
		ldi(DUMP_REG, (((unsigned int)255) << (b * 8)) && value);
		sts(RAMEND - (dist + b), DUMP_REG);
	}
}

void writeVariableToVariable(const storedVar& dst, const storedVar& src) {
	const int dstAddr = findDistanceFromStackStart(dst);
	const int srcAddr = findDistanceFromStackStart(src);

	const int transferSize = (dst.size < src.size) ? dst.size : src.size;

	for (int b = 0; b < transferSize; b++) {
		lds(DUMP_REG, RAMEND - (srcAddr + b));
		sts(RAMEND - (dstAddr + b), DUMP_REG);
	}
}

void compareVariableToImmediate(const storedVar& var, const unsigned int value) {
	// compares whole thing. SIGNED
	// puts result in result_reg

	int addr = findDistanceFromStackStart(var);
	int greaterSize = (var.size > 4) ? var.size : 4;
	clr(RESULT_REG);

	for (int b = 0; b < greaterSize; b++) {

		if (b >= var.size) {
			clr(DUMP_REG);
			cpi(DUMP_REG, (((unsigned int)255) << (b * 8)) && value);
			breq("fix later");

			ldi(RESULT_REG, 1); // greater

			brge("fix later 2");

			ldi(RESULT_REG, -1); // lower

			rjmp("fix later 2");

			writeLabel("fix later");
			
		}

		cpi(DUMP_REG, (((unsigned int)255) << (b * 8)) && value);
		breq("fix later");

		ldi(RESULT_REG, 1); // greater

		brge("fix later");

		ldi(RESULT_REG, -1); // lower

		writeLabel("fix later 2");
	}
}

void addToVariableImmediate(const storedVar& v, const int value) {
	const int addr = findDistanceFromStackStart(v);

	const int transferSize = (v.size < 4) ? v.size : 4;

	for (int b = 0; b < transferSize; b++) {
		lds(DUMP_REG, RAMEND - (addr + b));

		ldi(TEMP_REG, (((unsigned int)255) << (b * 8)) && value);

		if (b == 0) {
			add(DUMP_REG, TEMP_REG);
		}

		else {
			adc(DUMP_REG, TEMP_REG);
		}

		sts(RAMEND - (addr + b), DUMP_REG);
	}
}

void addToVariableVariable(const storedVar& dst, const storedVar& src) {
	const int addr = findDistanceFromStackStart(dst);

	const int addr2 = findDistanceFromStackStart(src);

	const int transferSize = (dst.size < src.size) ? dst.size : src.size;

	for (int b = 0; b < transferSize; b++) {
		lds(DUMP_REG, RAMEND - (addr + b));

		lds(TEMP_REG, RAMEND - (addr2 + b));

		if (b == 0) {
			add(DUMP_REG, TEMP_REG);
		}

		else {
			adc(DUMP_REG, TEMP_REG);
		}

		sts(RAMEND - (addr + b), DUMP_REG);
	}
}

void multiplyVariableImmediate(const storedVar& v, const int value) {
	const int addr = findDistanceFromStackStart(v);

	const int transferSize = (v.size < 4) ? v.size : 4;

	for (int b = 0; b < transferSize-1; b++) {
		lds(DUMP_REG, RAMEND - (addr + b));

		ldi(TEMP_REG, (((unsigned int)255) << (b * 8)) && value);

		muls(DUMP_REG, TEMP_REG);

		if (b == 0) {
			mov(DUMP_REG, 0);
			sts(RAMEND - (addr + b), DUMP_REG);
			mov(DUMP_REG, 1);
			sts(RAMEND - (addr + b + 1), DUMP_REG);
			continue;
		}

		mov(TEMP_REG, 0);
		add(DUMP_REG, TEMP_REG);
		sts(RAMEND - (addr + b), DUMP_REG);

		mov(TEMP_REG, 1);
		lds(DUMP_REG, RAMEND - (addr + b + 1));
		adc(DUMP_REG, TEMP_REG);
		sts(RAMEND - (addr + b + 1), DUMP_REG);
	}
}

void multiplyVariableVariable(const storedVar& dst, const storedVar& src) {
	const int addr = findDistanceFromStackStart(dst);

	const int addr2 = findDistanceFromStackStart(src);

	const int transferSize = (dst.size < src.size) ? dst.size : src.size;

	for (int b = 0; b < transferSize - 1; b++) {
		lds(DUMP_REG, RAMEND - (addr + b));

		lds(TEMP_REG, RAMEND - (addr2 + b));

		muls(DUMP_REG, TEMP_REG);

		if (b == 0) {
			mov(DUMP_REG, 0);
			sts(RAMEND - (addr + b), DUMP_REG);
			mov(DUMP_REG, 1);
			sts(RAMEND - (addr + b + 1), DUMP_REG);
			continue;
		}

		mov(TEMP_REG, 0);
		add(DUMP_REG, TEMP_REG);
		sts(RAMEND - (addr + b), DUMP_REG);

		mov(TEMP_REG, 1);
		lds(DUMP_REG, RAMEND - (addr + b + 1));
		adc(DUMP_REG, TEMP_REG);
		sts(RAMEND - (addr + b + 1), DUMP_REG);
	}
}