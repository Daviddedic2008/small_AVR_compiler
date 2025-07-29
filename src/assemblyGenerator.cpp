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
		if (!(dynamic_cast<literalNode*>(node.childNodes[0]) && dynamic_cast<literalNode*>(node.childNodes[1]))) {
		}
		return dynamic_cast<literalNode*>(node.childNodes[0])->value + dynamic_cast<literalNode*>(node.childNodes[1])->value;
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
		delete *ovrNode;
		*ovrNode = new identifierNode(token_str(token(NAME_TOKEN, NAME_VAR), name));
	}
	else if (node->operatorToken.subtype == OP_PLUS) {
		if (node->childNodes[0]->type == nodeType::literalNode) {
			if (node->childNodes[1]->type == nodeType::literalNode) {
				int tmp = evalNode(*node);
				delete *ovrNode;
				*ovrNode = new literalNode(tmp);
				return;
			}
			loadValueIntoRegisters(dynamic_cast<literalNode*>(node->childNodes[0])->value, 16);
			loadVarIntoRegisters(findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[1])->identifier.str), 20);
			const storedVar sv = findVariableFromName(dynamic_cast<identifierNode*>(node->childNodes[1])->identifier.str);

			int startreg, sz, secondreg, wsize;
			if (4 < sv.size) {
				startreg = 20;
				secondreg = 16;
				wsize = sv.size;
				sz = 4;
			}
			else {
				startreg = 16;
				secondreg = 20;
				wsize = 4;
				sz = sv.size;
			}

			add(startreg, secondreg);
			for (int o = 1; o < sz; o++) {
				adc(startreg + o, secondreg + o);
			}

			pushCompilerVarRegs(startreg, wsize);
		}
		
		delete *ovrNode;
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