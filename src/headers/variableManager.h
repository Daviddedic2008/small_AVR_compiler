#pragma once

#include <vector>
#include <cstring>
#include <string>

extern unsigned char currentScope;

struct storedVar {
	char size; // 1 or 4 bytes (for now)
	char startRegister; // -1 if not in regs
	unsigned char scopeValue;
	std::string name;
	bool compilerGenerated; // tmp variables created by compiler are flagged to prevent naming issues

	storedVar() {}

	storedVar(const char sz, std::string name, const unsigned char scopeValue = currentScope, const bool compilerGenerated = false) :
		size(sz), name(name), scopeValue(scopeValue), compilerGenerated(compilerGenerated), startRegister(-1) {
	}

	inline bool operator==(const storedVar& v) const {
		return size == size && name == v.name && compilerGenerated == v.compilerGenerated;
	}
};

int referenceVariable(const storedVar& v);

storedVar addVariable(std::string name, const char size);

storedVar addVariable(const storedVar& add);

void pushVariable(storedVar& v);

void decrementScope();

void incrementScope();

int findVariableInStack(const storedVar& v);

storedVar findVariableFromName(std::string name);

void readStackIntoRegisters(storedVar& v);

void freeStackFromRegisters(storedVar& v);

void saveVariableFromRegisters(storedVar& v);

void writeImmediateToVariable(const storedVar& v, const int value);

void writeVariableToVariable(const storedVar& dst, const storedVar& src);

void addToVariableImmediate(const storedVar& v, const int value);

void addToVariableVariable(const storedVar& dst, const storedVar& src);

void multiplyVariableImmediate(const storedVar& v, const int value);

void multiplyVariableVariablePushCompiler(const storedVar& dst, const storedVar& src);

void pushCompilerVar(const int sz);

void multiplyVariableVariable(const storedVar& dst, const storedVar& src);

storedVar& getLastCompilerVar();

storedVar& getSecondToLastCompilerVar();

storedVar popCompilerVar();

void pushCompilerVarImmediate(const int value);

void pushCompilerVarRegs(const char regstart, const unsigned char size);

void loadVarIntoRegisters(const storedVar& var, const char startReg);

void loadValueIntoRegisters(const int value, const char startReg);

storedVar addVariableImmediate(std::string name, const int value);

storedVar addVariableVar(std::string name, const storedVar& var);