#pragma once


void ldi(const char reg, const char val);

void mov(const char reg1, const char reg2);

void push(const char reg);

void pop(const char reg);

void sts(const unsigned int loc, const char reg);

void lds(const char reg, const unsigned int loc);

void rjmp(const char* label);

void jmp(const char* label);

void writeLabel(const char* label);

void brne(const char* label);

void breq(const char* label);

void brlt(const char* label);

void brge(const char* label);

void cpi(const char reg1, const char val);

void cp(const char reg1, const char reg2);

void clr(const char reg);

void add(const char reg1, const char reg2);

void adc(const char reg1, const char reg2);

void sub(const char reg1, const char reg2);

void subc(const char reg1, const char reg2);

void muls(const char reg1, const char reg2);

void movw(const char reg1, const char reg2);

void clc();

void ror(const char reg);

void lsr(const char reg);

void rol(const char reg);

void lsl(const char reg);

void inc(const char reg);

void dec(const char reg);

void brcc(const char* lbl);

void sec();