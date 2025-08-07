#include "headers/assemblyGenerator.h"
#include "headers/variableManager.h"
#include "headers/parser.h"
#include "headers/tokenizer.h"
#include "headers/opcodes.h"
#include <optional>

syntaxNode programParentNode;

void setParentNode(const syntaxNode& n) {
	programParentNode = n;
}

char nodeSize(const syntaxNode* n) {
	if (n->type == nodeType::literalNode) {
		return 4;
	}

	else {
		return dynamic_cast<const identifierNode*>(n)->allocSize;
	}
}

char getGreaterChildSize(const syntaxNode* parent) {
	const char s1 = nodeSize(parent->childNodes[0]); const char s2 = nodeSize(parent->childNodes[1]);

	return s1 > s2 ? s1 : s2;
}

int evalNode(const operatorNode& node) {
	switch (node.operatorToken.subtype) {
	case OP_PLUS:
		return dynamic_cast<literalNode*>(node.childNodes[0])->value + dynamic_cast<literalNode*>(node.childNodes[1])->value;
	case OP_MINUS:
		return dynamic_cast<literalNode*>(node.childNodes[0])->value - dynamic_cast<literalNode*>(node.childNodes[1])->value;
	case OP_MUL:
		return dynamic_cast<literalNode*>(node.childNodes[0])->value * dynamic_cast<literalNode*>(node.childNodes[1])->value;
	default:
		return 0;
	}
}

void skipIfSmaller(const int r1, const int s1, const int r2, const int s2, const int off) {
	const char* tmplbl = "tempLabel"; // fix later
	const char* endlbl = "endLabel"; // fix later
	const int lessSz = (s1 < s2) ? s1 : s2;
	for (int o1 = 0; o1 < lessSz; o1++) {
		cp(r1 + lessSz - o1, r2 + lessSz - o1);
		brlt(tmplbl);
		brne(endlbl);
	}

	writeLabel(endlbl); // saves a lil cycles
}

void subFromFirst(const int startRegisterDividend, const int sizeDividend, const int startRegisterDivisor, const int sizeDivisor) {
	sub(startRegisterDividend, startRegisterDivisor);
	for (int o = 1; o < sizeDivisor; o++) {
		subc(startRegisterDividend + o, startRegisterDivisor + o);
	}
}

void softwareDiv32(const int startRegisterDividend, const int startRegisterDivisor) {
	// dividend must  always be before divisor(dividend starts at r16)
	// 32 x 32 bit div subroutine
	// will add 32 x 16 and 16 x 16 later lazy rn
	const int rem1 = 10;
	const int rem2 = 11;
	const int rem3 = 12;
	const int rem4 = 14;

	const int dividend1 = startRegisterDividend;
	const int dividend2 = startRegisterDividend + 1;
	const int dividend3 = startRegisterDividend + 2;
	const int dividend4 = startRegisterDividend + 3;

	const int divisor1 = startRegisterDivisor;
	const int divisor2 = startRegisterDivisor + 1;
	const int divisor3 = startRegisterDivisor + 2;
	const int divisor4 = startRegisterDivisor + 3;

	const int result1 = dividend1 + 4 + 4;
	const int result2 = result1 + 1;
	const int result3 = result2 + 1;
	const int result4 = result3 + 1;

	const int count1 = 32;

	clr(rem1);
	clr(rem2);
	clr(rem3);
	sub(rem4, rem4);
	ldi(count1, 33);

	writeLabel("d32u_1");
	// loop to shift ts dividend into remainder one bit at a time
	rol(dividend1);
	rol(dividend2);
	rol(dividend3);
	rol(dividend4);
	dec(count1);
	brne("d32u_2"); // if counter is 0, all bits pushed
	rjmp("end");

	writeLabel("d32u_2");
	// rotate the carry bit from dividend into remainder regs
	rol(rem1);
	rol(rem2);
	rol(rem3);
	rol(rem4);
	// subtract divisor from remainder regs
	sub(rem1, divisor1);
	subc(rem2, divisor2);
	subc(rem3, divisor3);
	subc(rem4, divisor4);
	brcc("d32u_3");
	// if carry flag is set, meaning divisor more than remainder
	// add back divisor, and clear carry so we dont accidentally set a bit in the 
	add(rem1, divisor1);
	adc(rem2, divisor2);
	adc(rem3, divisor3);
	adc(rem4, divisor4);
	clc();
	rjmp("skp");

	writeLabel("d32u_3");
	sec();
	writeLabel("skp");
	// rotate carry set earlier into result regs
	rol(result1);
	rol(result2);
	rol(result3);
	rol(result4);
	rjmp("d32u_1");
}

void asmOp(syntaxNode** ovrNode) {
	operatorNode* node = dynamic_cast<operatorNode*>(*ovrNode);
	// assumes opNode has child nodes that are both either literal or variable

	if (node->operatorToken.subtype == OP_EQUALS) {
		std::string name = dynamic_cast<identifierNode*>(node->childNodes[0])->identifier.str;
		storedVar v = findVariableFromName(name);
		switch (node->childNodes[1]->type) {
		case nodeType::literalNode:
			if (v.size == -1) {
				addVariableImmediate(name, dynamic_cast<literalNode*>(node->childNodes[1])->value);
				break;
			}
			writeImmediateToVariable(v, dynamic_cast<literalNode*>(node->childNodes[1])->value);
			break;
		case nodeType::identifierNode:
			if (v.size == -1) {
				addVariableVar(name, findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[1])->identifier.str));
				break;
			}
			writeVariableToVariable(v, findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[1])->identifier.str));
			break;
		}
		delete* ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), name));
		return;
	}

	int startRegHigherSize, lowerSize, startRegLowerSize, higherSize;

	if (node->childNodes[0]->type == nodeType::literalNode) {
		if (node->childNodes[1]->type == nodeType::literalNode) {
			int tmp = evalNode(*node);
			delete* ovrNode;
			*ovrNode = new literalNode(tmp);
			return;
		}
		loadValueIntoRegisters(dynamic_cast<literalNode*>(node->childNodes[0])->value, 16);
		const storedVar sv = findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[1])->identifier.str);
		loadVarIntoRegisters(sv, 20);
		if (sv.size < 4) {
			lowerSize = sv.size;
			higherSize = 4;
			startRegHigherSize = 16;
			startRegLowerSize = 20;
		}
		else {
			lowerSize = 4;
			higherSize = sv.size;
			startRegHigherSize = 20;
			startRegLowerSize = 16;
		}
	}
	else {
		const storedVar sv = findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[0])->identifier.str);
		loadVarIntoRegisters(sv, 16);
		const int secondReg = 16 + sv.size;
		if (node->childNodes[1]->type == nodeType::literalNode) {
			loadValueIntoRegisters(secondReg, dynamic_cast<literalNode*>(node->childNodes[1])->value);
			lowerSize = 4 < sv.size ? 4 : sv.size;
			higherSize = 4 < sv.size ? sv.size : 4;
			startRegHigherSize = 4 < sv.size ? 16 : secondReg;
			startRegLowerSize = startRegHigherSize == 16 ? secondReg : 16;
		}
		else {
			const storedVar sv2 = findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[1])->identifier.str);
			loadVarIntoRegisters(sv2, secondReg);
			lowerSize = sv2.size < sv.size ? sv2.size : sv.size;
			higherSize = sv2.size < sv.size ? sv.size : sv2.size;
			startRegHigherSize = sv2.size < sv.size ? 16 : secondReg;
			startRegLowerSize = startRegHigherSize == 16 ? secondReg : 16;
		}
	}
	
	if (node->operatorToken.subtype == OP_PLUS) {
		add(startRegHigherSize, startRegLowerSize);
		for (int o = 1; o < lowerSize; o++) {
			adc(startRegHigherSize + o, startRegLowerSize + o);
		}

		pushCompilerVarRegs(startRegHigherSize, higherSize);
		
		delete *ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), getLastCompilerVar().name));
	}

	else if (node->operatorToken.subtype == OP_MINUS) {

		sub(startRegHigherSize, startRegLowerSize);
		for (int o = 1; o < lowerSize; o++) {
			subc(startRegHigherSize + o, startRegLowerSize + o);
		}

		pushCompilerVarRegs(startRegHigherSize, higherSize);

		delete* ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), getLastCompilerVar().name));
	}

	else if (node->operatorToken.subtype == OP_MUL) {
		int resbyte = 16 + higherSize + lowerSize;
		int bytesUsed = 0;

		for (int r = resbyte; r < resbyte + higherSize; r++) {
			clr(r);
		}
		clc();
		for (int r1 = startRegLowerSize; r1 < startRegLowerSize + lowerSize; r1++) {
			for (int r2 = startRegHigherSize; r2 < startRegHigherSize + higherSize; r2++) {
				if (bytesUsed == higherSize) {
					goto endFor;
				}
				muls(r1, r2);
				if (bytesUsed == 0) {
					movw(resbyte, 0);
					bytesUsed++;
					continue;
				}
				bytesUsed++;
				adc(resbyte + bytesUsed, 0);
				adc(resbyte + bytesUsed + 1, 1);
			}
		}
		endFor:;

		pushCompilerVarRegs(resbyte, higherSize);

		delete* ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), getLastCompilerVar().name));
	}

	else if (node->operatorToken.subtype == OP_DIV) {
		if (higherSize == 4 && higherSize == lowerSize) {
			softwareDiv32(16, 20);
			pushCompilerVarRegs(24, 4);
			// only div32 for now
		}
	}

	else if (node->operatorToken.subtype == OP_REFERENCE) {
		// push ptr
		pushCompilerVarImmediate(2047 - findDistanceFromStackStart(findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[0])->identifier.str)), 2);
		delete* ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), getLastCompilerVar().name));
	}

	else if (node->operatorToken.subtype == OP_DEREFERENCE) {
		// uses size of variable to signify the data type being pointed to
		storedVar tmp = findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[0])->identifier.str);
		tmp.size = 2; // ensure pointer reads as 2 byte type
		loadVarIntoRegisters(tmp, 26);
		ld(16, "X");
		for (int o = 1; o < findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[0])->identifier.str).size; o++) {
			ldd(16+o, (std::string("X") + std::to_string(o)).c_str()); // load direct with displacement
		}
		pushCompilerVarRegs(16, findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[0])->identifier.str).size);
		delete* ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), getLastCompilerVar().name));
	}

	else if (node->operatorToken.subtype == OP_EQUALS_EQUALS) {
		clr(10);
		for (int lo = startRegLowerSize, hi = startRegHigherSize; lo < startRegLowerSize + lowerSize; lo++, hi++) {
			sub(lo, hi);
		}
		for (int hi = startRegHigherSize + lowerSize - 1; hi < startRegHigherSize + higherSize; hi++) {
			add(startRegLowerSize, hi);
		}
		for (int hi = startRegHigherSize; hi < startRegHigherSize + higherSize; hi++) {
			OR(startRegLowerSize, hi);
		}
		cpi(startRegLowerSize, 0);
		brne("skip");
		inc(10);
		writeLabel("skip");
		pushCompilerVarRegs(10, 1); // push result wohoo
	}
	int s1, r2, s2;
	if (startRegLowerSize == 16) {
		s1 = lowerSize;
		r2 = 16 + s1;
		s2 = higherSize;
	}
	if (startRegLowerSize == 16) {
		s1 = higherSize;
		r2 = 16 + s1;
		s2 = lowerSize;
	}
	else if (node->operatorToken.subtype == OP_GREATER_EQUAL) {
		cp(16, 16 + s1);
		for (int sr1 = 17, sr2 = 16 + s1 + 1; sr1 < 16 + s1; sr1++, r2++) {
			cpc(sr1, sr2);
		}
		clr(10);
		for (int rs = startRegHigherSize + lowerSize; rs < startRegHigherSize + higherSize; rs++) {
			if (startRegHigherSize == 16) {
				cpc(rs, 10);
				continue;
			}
			cpc(10, rs);
		}
		brlt("skip");
		inc(10);
		writeLabel("skip");
		pushCompilerVarRegs(10, 1);
	}
	else if (node->operatorToken.subtype == OP_LESS) {
		cp(16, 16 + s1);
		for (int sr1 = 17, sr2 = 16 + s1 + 1; sr1 < 16 + s1; sr1++, r2++) {
			cpc(sr1, sr2);
		}
		clr(10);
		for (int rs = startRegHigherSize + lowerSize; rs < startRegHigherSize + higherSize; rs++) {
			if (startRegHigherSize == 16) {
				cpc(rs, 10);
				continue;
			}
			cpc(10, rs);
		}
		brge("skip");
		inc(10);
		writeLabel("skip");
		pushCompilerVarRegs(10, 1);
	}
	else if (node->operatorToken.subtype == OP_LESS_EQUAL) {
		cp(16, 16 + s1);
		for (int sr1 = 17, sr2 = 16 + s1 + 1; sr1 < 16 + s1; sr1++, r2++) {
			cpc(sr1, sr2);
		}
		clr(10);
		for (int rs = startRegHigherSize + lowerSize; rs < startRegHigherSize + higherSize; rs++) {
			if (startRegHigherSize == 16) {
				cpc(rs, 10);
				continue;
			}
			cpc(10, rs);
		}
		brlt("skip");
		breq("skip");
		rjmp("continue");
		writeLabel("skip");
		pushCompilerVarRegs(10, 1);
		writeLabel("continue");
	}
	else if (node->operatorToken.subtype == OP_GREATER) {
		cp(16, 16 + s1);
		for (int sr1 = 17, sr2 = 16 + s1 + 1; sr1 < 16 + s1; sr1++, r2++) {
			cpc(sr1, sr2);
		}
		clr(10);
		for (int rs = startRegHigherSize + lowerSize; rs < startRegHigherSize + higherSize; rs++) {
			if (startRegHigherSize == 16) {
				cpc(rs, 10);
				continue;
			}
			cpc(10, rs);
		}
		brge("skip");
		brne("skip");
		rjmp("continue");
		writeLabel("skip");
		pushCompilerVarRegs(10, 1);
		writeLabel("continue");
	}
}

void assembleOpTree(syntaxNode** startNode) {
	// should be enough to work?.....
	if ((*startNode)->childNodes[0]->type == nodeType::opNode) {
		assembleOpTree(&(*startNode)->childNodes[0]);
	}
	if ((*startNode)->childNodes[1]->type == nodeType::opNode) {
		assembleOpTree(&(*startNode)->childNodes[1]);
	}
	asmOp(startNode);
}