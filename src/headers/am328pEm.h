#pragma once

#include <optional>
#include <string>

extern char ram[2048];
extern char regs[32];

extern bool flags[2];

extern char comparisonRegister;

enum class opcodeType {
	ldi,
	sts,
	lds,
	mov,
	add,
	adc,
	sub,
	subc,
	mul,
	brge,
	brlt,
	breq,
	brne,
	clr,
	cpi,
	cp,
	pop,
	push,
	rjmp,
	jmp,
};

enum flags {
	carry,
	interrupt,
};

struct opcode {
	opcodeType type;
	std::optional<int> arg1, arg2;

	std::optional<std::string> stringArg;

	opcode(opcodeType t, std::optional<int> a1 = std::nullopt, std::optional<int> a2 = std::nullopt, std::optional<std::string> sarg = std::nullopt) {
		arg1 = a1; arg2 = a2; type = t; stringArg = sarg;
	}

	opcode(){}
};

void setupVm();

void printRam(const int loc);

void printReg(const int reg);

void printCmp();

void setupVm();

void addOpcode(opcodeType op, std::optional<int> arg1 = std::nullopt, std::optional<int> arg2 = std::nullopt, std::optional<std::string> str = std::nullopt);

void addLabel(const std::string& lbl);

void runVm();