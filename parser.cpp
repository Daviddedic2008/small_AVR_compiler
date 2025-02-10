#include "parser.h"
#include "tokenizer.h"
#include <cstdlib>
#include <string>

std::list<token_str> tokenSrc;
token_str* tokensRandomAccessArray;
token currentNodeStructure[2];

std::list<token_str>::iterator tokenSrcIterator;
int currentIndex, currentIndexArr;

void setTokenSrc(std::list<token_str> s) {
	tokenSrc = s;
	currentIndex = 0;
	currentIndexArr = 0;
	tokenSrcIterator = s.begin();
	tokensRandomAccessArray = (token_str*)malloc(sizeof(token_str) * tokenSrc.size());
	for (int i = 0; tokenSrcIterator != tokenSrc.end(); tokenSrcIterator++, i++) {
		tokensRandomAccessArray[i] = *tokenSrcIterator;
	}
	tokenSrcIterator = s.begin();
}

void eatToken() {
	tokenSrcIterator++;
	currentIndex++;
}

token_str spitToken() {
	const token_str ret = *tokenSrcIterator;
	tokenSrcIterator++;
	currentIndex++;
	return ret;
}

void setArrayPosition(int pos) {
	currentIndexArr = pos;
}

void incrementArray(int increment) {
	currentIndexArr += increment;
}

token_str spitTokenArr() {
	return tokensRandomAccessArray[currentIndexArr];
}

token_str getArrayPosition(int pos) {
	return pos < tokenSrc.size() ? tokensRandomAccessArray[pos] : token_str(token(-1, -1), "");
}

nodeType getTypeOfNextNode(int startIndex, int endIndex) {
	int pPos = currentIndexArr;
	for (int i = startIndex; i < endIndex; i++, incrementArray(1)) {
		token_str s = spitTokenArr();
		switch (s.t.type) {
			case IDENTIFIER_TOKEN:
				return nodeType::keywordNode;
			case OPERATION_TOKEN:
				return nodeType::opNode;
			case CONSTANT_TOKEN:
				return nodeType::literalNode;
			default:
				return nodeType::uninitialized;
		}
	}
	setArrayPosition(pPos);
}

syntaxNode* returnExpressionNode(int startIndex, int endIndex) {
	// current index is ON FIRST VARIABLE/LITERAL
	int pPos = currentIndexArr;
	setArrayPosition(startIndex);
	operatorNode* op = (operatorNode*)malloc(sizeof(operatorNode));
	*op = operatorNode(token(-1, -1));
	*op->getValue1() = syntaxNode(nodeType::uninitialized);
	*op->getValue2() = syntaxNode(nodeType::uninitialized);
	for (int i = startIndex; i < endIndex; i++, incrementArray(1)) {
		token_str s = spitTokenArr();
		token_str nextToken = getArrayPosition(i + 1);
		switch (s.t.type) {
			case NAME_TOKEN:
				if (op->getValue1()->type == nodeType::uninitialized) {
					*op->getValue1() = identifierNode(s);
				}
				else {
					if (nextToken.t.type != OPERATION_TOKEN) {
						if (op->operatorToken.type == -1) { goto returnNullOpNode; }
						*op->getValue2() = identifierNode(s);
						goto returnOperationNode;
					}
					else {
						*op->getValue2() = *returnExpressionNode(i, endIndex);
					}
				}
			case CONSTANT_TOKEN:
				if (op->getValue1()->type == nodeType::uninitialized) {
					*op->getValue1() = literalNode(std::stoi(s.str));
				}
				else {
					if (nextToken.t.type != OPERATION_TOKEN) {
						if (op->operatorToken.type == -1) { goto returnNullOpNode; }
						*op->getValue2() = literalNode(std::stoi(s.str));
						
					}
					else {
						*op->getValue2() = *returnExpressionNode(i, endIndex);
						goto returnOperationNode;
					}
				}
			case OPERATION_TOKEN:
				op->operatorToken = s.t;
			default:
				return nullptr;
		}
	}
	returnOperationNode:
	setArrayPosition(pPos);
	return op;

	returnNullOpNode:
	setArrayPosition(pPos);
	return nullptr;
}

void parseIfNode(keywordNode* node) {
	// assume current token index is on the if itself

}