#pragma once

extern char ram[2048];
extern char regs[32];

extern bool flags[2];

extern char comparisonRegister;

enum opcode {
	ldi,
	sts,
	mov,
	add,
	adc
};

enum flags {
	carry,
	interrupt
};

void setupVm();