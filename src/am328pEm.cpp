#include "headers/am328pEm.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <optional>
#include <stdint.h>

char ram[2048];
char regs[32];

bool flags[2];

char comparisonRegister;

std::vector<opcode> opcodes;

std::vector<std::pair<std::string, int>> labels;

int stackptr = 2047;

int currentOp = 0;

int argIdx = 0;

void printRam(const int loc) {
	printf("loc %d: %d\n", loc, ram[loc]);
}

void printReg(const int reg) {
	printf("reg %d : %d", reg, regs[reg]);
}

void printCmp() {
	printf("cmp : %d\n", comparisonRegister);
}

void setupVm() {
	currentOp = 0;
}

void addOpcode(opcodeType op, std::optional<int> arg1, std::optional<int> arg2, std::optional<std::string> str) {
	opcodes.push_back(opcode(op, arg1, arg2, str));
	currentOp++;
}

void addLabel(const std::string& lbl) {
	labels.push_back({ lbl, currentOp });
}

void mov(const int r1, const int r2) {
	regs[r1] = regs[r2];
}

void ldi(const int r1, const int val) {
	regs[r1] = val;
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
		flags[carry] = 1;
	}
	else {
		flags[carry] = 0;
	}
	regs[reg1] += regs[reg2];
}

void adc(const int reg1, const int reg2) {
	const int r2 = regs[reg2] + flags[carry];

	if (regs[reg1] + r2 > 127 || regs[reg1] + r2 < -127) {
		flags[carry] = 1;
	}
	else {
		flags[carry] = 0;
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

	if (comparisonRegister > 0) {
		int loc = -1;
		for (auto it = labels.begin(); it != labels.end(); it++) {
			loc = (it->first.compare(label)) ? loc : it->second;
		}

		currentOp = loc-1;
	}
}

void push(const int reg) {
	ram[stackptr] = regs[reg];
	stackptr--;
}

void pop(const int reg) {
	regs[reg] = ram[stackptr];
	stackptr++;
}

void runVm() {
	currentOp = 0;
	memset(ram, 0, sizeof(ram));
	memset(regs, 0, sizeof(regs));
	memset(flags, 0, sizeof(flags));
	comparisonRegister = 0;

	for (; currentOp < opcodes.size(); currentOp++) {
		auto op = &opcodes[currentOp];

		switch (op->type) {
		case opcodeType::ldi:
			ldi(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::mov:
			mov(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::sts:
			sts(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::lds:
			lds(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::add:
			add(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::adc:
			adc(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::mul:
			mul(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::clr:
			clr(op->arg1.value());
			break;
		case opcodeType::cpi:
			cpi(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::cp:
			cp(op->arg1.value(), op->arg2.value());
			break;
		case opcodeType::brgt:
			brgt(op->stringArg.value());
			break;
		}

	}
}