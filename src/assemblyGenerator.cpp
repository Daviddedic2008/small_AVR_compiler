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