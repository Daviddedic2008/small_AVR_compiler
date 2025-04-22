#pragma once


void ldi(const char reg, const char val);

void mov(const char reg1, const char reg2);

void push(const char reg);

void pop(const char reg);

void sts(const unsigned int loc, const char reg);

void lds(const char reg, const unsigned int loc);