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

	modifiableValue(const storedVar& v) : v(v), value(std::nullopt){}
	modifiableValue(const int val) : v(storedVar()), value(val){}

	void* returnValue() {
		return (value == std::nullopt) ? (void*)&v : (void*)&value; // implicit casting wont work in ternary op w 2 diff pointers????? huh
	}
};

void convertExpression(operatorNode& node) {
	std::pair<std::optional<syntaxNode&>, std::optional<syntaxNode&>> childParentLowest = findDeepestNode(node);

	const modifiableValue v1 = childParentLowest.first.value().type == nodeType:: findVariableFromName(dynamic_cast<identifierNode&>(childParentLowest.first.value()).identifier.str.c_str());

	storedVar v2;

	if (node.operatorToken.subtype != OP_INCREMENT && node.operatorToken.subtype != OP_DECREMENT) {
		v2 = findVariableFromName(dynamic_cast<identifierNode&>(*childParentLowest.second.value().childNodes[1]).identifier.str.c_str());

	}
}

void convertTreeToAsm() {
	;
}