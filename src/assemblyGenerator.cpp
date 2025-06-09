#include "headers/assemblyGenerator.h"
#include "headers/variableManager.h"
#include <optional>

syntaxNode programParentNode;

void setParentNode(const syntaxNode& n) {
	programParentNode = n;
}

std::pair<std::optional<syntaxNode&>, std::optional<syntaxNode&>> findDeepestNode(operatorNode& node) { // cant do const
	int maxDepth = -1;

	int curDepth = 0;

	std::pair<std::optional<syntaxNode&>, std::optional<syntaxNode&>> ret;

	for (auto childNode : node.childNodes) {
		if (childNode->type == nodeType::keywordNode || childNode->type == nodeType::identifierNode) {
			maxDepth = curDepth;
			ret.first = *childNode;
			ret.second = node;
		}

		else if (childNode->type == nodeType::opNode) {
			ret = findDeepestNode(dynamic_cast<operatorNode&>(*childNode));
		}
	}

	return ret;
}

struct modifiableValue {
	storedVar v;
	std::optional<int> value;
	bool imm;

	modifiableValue(const storedVar& v) : v(v), value(std::nullopt), imm(false){}
	modifiableValue(const int val) : v(storedVar()), value(val), imm(true){}

	modifiableValue(){}

	void* returnValue() {
		return (value == std::nullopt) ? (void*)&v : (void*)&value; // implicit casting wont work in ternary op w 2 diff pointers????? huh
	}
};

void convertOp(modifiableValue& v1, modifiableValue& v2, token op) {
	bool pushResult = true;

	const int lowerSz = (v1.imm * 4 + !v1.imm * v1.v.size) < (v2.imm * 4 + !v2.imm * v2.v.size) ? (v1.imm * 4 + !v1.imm * v1.v.size) : ;
	pushCompilerVar()

	switch (op.subtype) {
	case OP_PLUS:

		if (v2.imm) {
			addToVariableImmediate(v1.v, v2.value.value());
			break;
		}
		addToVariableVariable(v1.v, v2.v);
		break;
	case OP_MUL:

		if (v2.imm) {
			multiplyVariableImmediate(v1.v, v2.value.value());
			break;
		}
		multiplyVariableVariable(v1.v, v2.v);
		break;
	}
}

void convertExpression(operatorNode& node) {
	std::pair<std::optional<syntaxNode&>, std::optional<syntaxNode&>> childParentLowest = findDeepestNode(node);

	if (childParentLowest.first.value().type == nodeType::opNode) {
		convertExpression(dynamic_cast<operatorNode&>(childParentLowest.first.value()));
	}

	const modifiableValue v1 = childParentLowest.first.value().type == nodeType::identifierNode ? modifiableValue(findVariableFromName(dynamic_cast<identifierNode&>(childParentLowest.first.value()).identifier.str.c_str())) : modifiableValue(dynamic_cast<literalNode&>(childParentLowest.first.value()).value);

	modifiableValue v2;
	bool v2f = false;


	if (childParentLowest.second.value().childNodes[1]->type == nodeType::opNode) {
		convertExpression(dynamic_cast<operatorNode&>(*childParentLowest.second.value().childNodes[1]));
	}

	if (node.operatorToken.subtype != OP_INCREMENT && node.operatorToken.subtype != OP_DECREMENT && node.operatorToken.subtype != OP_DEREFERENCE && node.operatorToken.subtype != OP_REFERENCE) {
		v2f = true;
		v2 = childParentLowest.second.value().childNodes[1]->type == nodeType::identifierNode ? modifiableValue(findVariableFromName(dynamic_cast<identifierNode&>(*childParentLowest.second.value().childNodes[1]).identifier.str.c_str())) : modifiableValue(dynamic_cast<literalNode&>(*childParentLowest.second.value().childNodes[1]).value);
	}
}

void convertTreeToAsm() {
	;
}