#include "headers/am328pEm.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <optional>
#include <stdint.h>

char ram[2048];
char regs[32];

#define CARRY 0
#define INTERRUPT 1
bool flags[2];

char comparisonRegister;

std::vector<opcode> srcCode;

std::vector<int> args;

std::vector<std::pair<std::string, int>> labels;

int stackptr = 2047;

int currentOp = 0;

int argIdx = 0;

void printRam(const int loc) {
	printf("loc %d: %d\n", loc, ram[loc]);
}

void setupVm() {
	comparisonRegister = 0;
	memset(ram, 0, sizeof(ram));
	memset(regs, 0, sizeof(regs));
	memset(flags, 0, sizeof(flags));
	comparisonRegister = 0;
}

void addOpcode(opcode op, std::optional<int> arg1, std::optional<int> arg2) {

}

void addFlag() {

}

void pushArgsBackToFlag(const std::pair<std::string, int>& flag) {

}

void runVm() {

}

void mov(const int r1, const int r2) {
	regs[r1] = regs[r2];
}

void sts(const int loc, const int r2) {
	ram[loc] = regs[r2];
}

void lds(const int reg, const int loc) {
	regs[reg] = ram[loc];
}

void mul(const int reg1, const int reg2) {
	const uint16_t result = regs[reg1] * regs[reg2];

	regs[0] = result & 0xFF;
	regs[1] = (result & (0xFF00)) >> 8;
}

void add(const int reg1, const int reg2) {
	if (regs[reg1] + regs[reg2] > 127 || regs[reg1] + regs[reg2] < -127) {
		flags[CARRY] = 1;
	}
	else {
		flags[CARRY] = 0;
	}
	regs[reg1] += regs[reg2];
}

void adc(const int reg1, const int reg2) {
	const int r2 = regs[reg2] + flags[CARRY];

	if (regs[reg1] + r2 > 127 || regs[reg1] + r2 < -127) {
		flags[CARRY] = 1;
	}
	else {
		flags[CARRY] = 0;
	}
	regs[reg1] += r2;
}

void clr(const int reg1) {
	regs[reg1] = 0;
}

void cp(const int reg1, const int reg2) {
	comparisonRegister = (regs[reg1] > regs[reg2]) ? 1 : (regs[reg1] < regs[reg2] ? -1 : 0);
}

void cpi(const int reg1, const int val) {
	comparisonRegister = (regs[reg1] > val) ? 1 : (regs[reg1] < val ? -1 : 0);
}

void brgt(const std::string& label) {
	int loc = -1;
	for (auto it = labels.begin(); it != labels.end(); it++) {
		loc = (it->first.compare(label)) ? loc : it->second;
	}

	currentOp = loc;
}

void push(const int reg) {
	ram[stackptr] = regs[reg];
	stackptr--;
}

void pop(const int reg) {
	regs[reg] = ram[stackptr];
	stackptr++;
}