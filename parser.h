#pragma once
#include <vector>
#include "tokenizer.h"

enum class nodeType {
	uninitialized,

	opNode,
	identifierNode,
	literalNode,
	compareNode,
	keywordNode,

	bodyStatement,
	comparisonStatement,
	iteratorStatement,
	preconditionStatement
};

class syntaxNode {
public:

	virtual ~syntaxNode() = default;


	nodeType type;

	syntaxNode() {}

	syntaxNode(nodeType t) : type(t) {}

	std::vector<syntaxNode> childNodes;

	void addToLeft(syntaxNode n) {
		childNodes.insert(childNodes.begin(), n);
	}

	void addToRight(syntaxNode n) {
		childNodes.push_back(n);
	}

	void insertNode(syntaxNode n, int index) {
		childNodes.insert(childNodes.begin()+index, n);
	}
	
	// implement depth algo
	// scans first layer of nodes(ifs, assignment ops, fors, whiles, etc)
	// lists down token locations of the start of each op's body
	// recursively does this until each node is reduced down to primitives(ops and immediate/variable values)
	// each next pass inits each node inside the subNodes of the previously returned layer
	// should result in a properly ordered tree
};

class literalNode : public syntaxNode {
public:

	int value;

	literalNode(int v) : syntaxNode(nodeType::literalNode), value(v){}

	bool canParseFurther() {
		return false;
	}
};

class identifierNode : public syntaxNode {
public:

	token_str identifier;

	identifierNode(token_str s) : syntaxNode(nodeType::identifierNode), identifier(s) {}

	bool canParseFurther() {
		return true;
	}
};

class operatorNode : public syntaxNode {
public:
	
	token operatorToken;

	operatorNode(){}

	operatorNode(token op) : syntaxNode(nodeType::opNode), operatorToken(op) {
		for (int i = 0; i < 2; i++) { addToRight(syntaxNode()); }
	}

	bool canParseFurther() {
		for (int i = 0; i < 2; i++) {
			if (childNodes[i].type != nodeType::literalNode && childNodes[i].type != nodeType::identifierNode) {
				return true;
			}
		}
		return false;
	}

	syntaxNode* getValue1() {
		return &(childNodes[0]);
	}

	syntaxNode* getValue2() {
		return &(childNodes[1]);
	}
};

class keywordNode : public syntaxNode {
public:

	token keywordToken;

	keywordNode(token kw) : syntaxNode(nodeType::keywordNode), keywordToken(kw) {
		switch (keywordToken.subtype) {
		case IDENTIFIER_FOR:
			childNodes.push_back(syntaxNode(nodeType::preconditionStatement));
			childNodes.push_back(syntaxNode(nodeType::comparisonStatement));
			childNodes.push_back(syntaxNode(nodeType::iteratorStatement));
			childNodes.push_back(syntaxNode(nodeType::bodyStatement));
		default:
			childNodes.push_back(syntaxNode(nodeType::comparisonStatement));
			childNodes.push_back(syntaxNode(nodeType::bodyStatement));
		}
	}

	bool canParseFurther() {
		return true;
	}

public:
	syntaxNode* getCondition() {
		switch (keywordToken.subtype) {
			case IDENTIFIER_FOR:
				return &(childNodes[1]);
			default:
				return &(childNodes[0]);
		}
	}

	syntaxNode* getIterator() {
		switch (keywordToken.subtype) {
		case IDENTIFIER_FOR:
			return &(childNodes[2]);
		default:
			return nullptr;
		}
	}

	syntaxNode* getPrecondition() {
		switch (keywordToken.subtype) {
		case IDENTIFIER_FOR:
			return &(childNodes[0]);
		default:
			return nullptr;
		}
	}

	syntaxNode* getBody() {
		switch (keywordToken.subtype) {
		case IDENTIFIER_FOR:
			return &(childNodes[3]);
		default:
			return &(childNodes[1]);
		}
	}
};

void setTokenSrc(std::list<token_str> s);

syntaxNode* parseExpression(const int startIndex, const int endIndex);