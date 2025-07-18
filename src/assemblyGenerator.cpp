#include "headers/assemblyGenerator.h"
#include "headers/variableManager.h"
#include "headers/parser.h"
#include "headers/tokenizer.h"
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

void asmOp(const operatorNode& node) {
	// assumes opNode has child nodes that are both either literal or variable
	switch (node.operatorToken.subtype) {
	case OP_EQUALS:
		switch (node.childNodes[1]->type) {
		case nodeType::literalNode:
			writeImmediateToVariable(findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[0])->identifier.str.c_str()), dynamic_cast<literalNode*>(node.childNodes[1])->value);
			break;
		case nodeType::identifierNode:
			writeVariableToVariable(findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[0])->identifier.str.c_str()), findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[1])->identifier.str.c_str()));
			break;
		}
		break;
	case OP_PLUS:
		pushCompilerVar(getGreaterChildSize(&node));
		switch (node.childNodes[0]->type) {
		case nodeType::literalNode:
			writeImmediateToVariable(getLastCompilerVar(), dynamic_cast<literalNode*>(node.childNodes[0])->value);
			break;

		case nodeType::identifierNode:
			writeVariableToVariable(getLastCompilerVar(), findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[0])->identifier.str.c_str()));
			break;
		}

		if (node.childNodes[1]->type == nodeType::literalNode) {
			addToVariableImmediate(getLastCompilerVar(), dynamic_cast<literalNode*>(node.childNodes[1])->value);
			break;
		}

		addToVariableVariable(getLastCompilerVar(), findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[1])->identifier.str.c_str()));
		break;
	case OP_MUL:
		pushCompilerVar(getGreaterChildSize(&node));
		switch (node.childNodes[0]->type) {
		case nodeType::literalNode:
			writeImmediateToVariable(getLastCompilerVar(), dynamic_cast<literalNode*>(node.childNodes[0])->value);
			break;

		case nodeType::identifierNode:
			writeVariableToVariable(getLastCompilerVar(), findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[0])->identifier.str.c_str()));
			break;
		}
		if (node.childNodes[1]->type == nodeType::literalNode) {
			multiplyVariableImmediate(getLastCompilerVar(), dynamic_cast<literalNode*>(node.childNodes[1])->value);
			break;
		}

		multiplyVariableVariable(getLastCompilerVar(), findVariableFromName(dynamic_cast<identifierNode*>(node.childNodes[1])->identifier.str.c_str()));
		break;
	}
}