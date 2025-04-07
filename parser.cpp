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
	delete[] tokensRandomAccessArray;

	tokensRandomAccessArray = new token_str[tokenSrc.size()];
	int i = 0;
	for (auto it = tokenSrc.begin(); it != tokenSrc.end(); ++it, ++i) {
		if (it->t.type == -1) { continue; }
		tokensRandomAccessArray[i] = *it;
	}
	tokenSrcIterator = s.begin();
}

token_str eatToken() {
	const token_str ret = *tokenSrcIterator;
	tokenSrcIterator++;

	return ret;
}

token_str eatTokenArr() {
	const token_str ret = tokensRandomAccessArray[currentIndexArr];
	currentIndexArr++;
	return ret;
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

syntaxNode* parseExpressionNode(const int startIndex, const int endIndex) {
	const int oldPos = currentIndexArr;
	setArrayPosition(startIndex);
	operatorNode* ret = new operatorNode();

	token_str curToken = tokensRandomAccessArray[startIndex];

	token_str nextToken = tokensRandomAccessArray[startIndex+1];

	switch (nextToken.t.type) {

		case END_LINE_TOKEN:
			delete ret;
			switch (curToken.t.type) {
				literalNode* ret2;
				identifierNode* ret3;
				syntaxNode* ret4;

			case CONSTANT_TOKEN:
				ret2 = new literalNode(std::stoi(curToken.str));
				return ret2;
			case NAME_TOKEN:
				ret3 = new identifierNode(curToken);
				return ret3;
			default:
				
				ret4 = new syntaxNode(nodeType::uninitialized);
				return ret4;
			}
			break;

		case OPERATION_TOKEN:
			delete ret;
			ret = new operatorNode(nextToken.t);
			
			*ret->getValue1() = (curToken.t.type == NAME_TOKEN) ? (syntaxNode)identifierNode(curToken) : (syntaxNode)literalNode(std::stoi(curToken.str));
			token_str tokenTmp = tokensRandomAccessArray[startIndex + 2];
			if (tokenTmp.t.type == END_LINE_TOKEN) {
				if (tokenTmp.t.subtype > 12) {
					// inc or dec
					*ret->getValue2() = literalNode(0);
				}
				break;
			}
			*ret->getValue2() = *parseExpressionNode(startIndex + 2, endIndex);
			break;
	}
	return ret;
}

syntaxNode* parseExpression(const int startIndex, const int endIndex) {
	// assume startIndex is on variable/literal
	setArrayPosition(startIndex + 1);

	token_str varToken = eatTokenArr();

	token_str opToken = spitTokenArr();

	operatorNode* ret = new operatorNode(opToken.t);

	*ret->getValue1() = identifierNode(varToken);

	*ret->getValue2() = *parseExpressionNode(startIndex+2, endIndex);

	return ret;
}

void parseIfNode(keywordNode* node) {
	// assume current token index is on the if itself

}