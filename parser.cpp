#include "parser.h"
#include "tokenizer.h"
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

std::list<token_str> tokenSrc;
token_str* tokensRandomAccessArray = nullptr;
token currentNodeStructure[2];

std::list<token_str>::iterator tokenSrcIterator;
int currentIndex, currentIndexArr;

const char opValues[] = { 0, 0, 0, 0, 1, 1, 2, 2 };

void setTokenSrc(std::list<token_str> s) {
    tokenSrc = s;
    currentIndex = 0;
    currentIndexArr = 0;
    tokenSrcIterator = s.begin();
    delete[] tokensRandomAccessArray;

    tokensRandomAccessArray = new token_str[tokenSrc.size()];
    int i = 0;
    for (auto it = tokenSrc.begin(); it != tokenSrc.end(); ++it, ++i) {
        if (it->t.type == -1) {
            continue;
        }
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
    return nodeType::uninitialized;
}

syntaxNode* parseExpressionNode(const int startIndex, const int endIndex) {
    const int oldPos = currentIndexArr;
    setArrayPosition(startIndex);
    operatorNode* ret = new operatorNode();

    token_str curToken = tokensRandomAccessArray[startIndex];
    token_str nextToken = tokensRandomAccessArray[startIndex + 1];

    switch (nextToken.t.type) {
    case END_LINE_TOKEN:
        delete ret;
        switch (curToken.t.type) {
        case CONSTANT_TOKEN: {
            literalNode* ret2 = new literalNode(std::stoi(curToken.str));
            return ret2;
        }
        case NAME_TOKEN: {
            identifierNode* ret3 = new identifierNode(curToken);
            return ret3;
        }
        default: {
            syntaxNode* ret4 = new syntaxNode(nodeType::uninitialized);
            return ret4;
        }
        }
        break;

    case OPERATION_TOKEN:
        delete ret;
        ret = new operatorNode(nextToken.t);

        ret->childNodes[0] = (curToken.t.type == NAME_TOKEN)
            ? static_cast<syntaxNode*>(new identifierNode(curToken))
            : static_cast<syntaxNode*>(new literalNode(std::stoi(curToken.str)));

        token_str tokenTmp = tokensRandomAccessArray[startIndex + 2];
        if (tokenTmp.t.type == END_LINE_TOKEN) {
            if (tokenTmp.t.subtype > 12) {
                // inc or dec
                ret->childNodes[1] = new literalNode(0);
            }
            break;
        }

        ret->childNodes[1] = parseExpressionNode(startIndex + 2, endIndex);
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
    ret->childNodes[0] = new identifierNode(varToken);
    ret->childNodes[1] = parseExpressionNode(startIndex + 2, endIndex);

    return ret;
}

int findNextSignificantTokenIndexRight(const int startIndex, int index, const int endIndex) {
    index += 1;
    token_str tmp = tokensRandomAccessArray[index];
    while (tmp.t.type == CLAMP_TOKEN) {
        if (tmp.t.type == END_LINE_TOKEN || index == startIndex || index == endIndex - 1) {
            return -1;
        }
        index++;
        tmp = tokensRandomAccessArray[index];
    }
    //printf("%s\n", tmp.str.c_str());
    return index;
}

int findNextSignificantTokenIndexLeft(const int startIndex, int index, const int endIndex) {
    index -= 1;
    token_str tmp = tokensRandomAccessArray[index];
    while (tmp.t.type == CLAMP_TOKEN) {
        if (tmp.t.type == END_LINE_TOKEN || index == startIndex || index == endIndex - 1) {
            return -1;
        }
        index--;
        tmp = tokensRandomAccessArray[index];
    }
    return index;
}

syntaxNode* getVarNodeFromToken(const token_str& t) {
    switch (t.t.type) {
    case CONSTANT_TOKEN:
        return new literalNode(std::stoi(t.str));
    case NAME_TOKEN:
        return new identifierNode(t);
    default:
        return new syntaxNode(nodeType::uninitialized);
    }
}

syntaxNode* parseExpression2(const int startIndex, const int endIndex) {
    std::unordered_map<int, int> operatorPrecedence;

    setArrayPosition(startIndex);

    // first: index of op. second: precedence value (greater means higher precedence)
    token t = eatTokenArr().t;

    int currentPrecedenceAddon = 0;

    for (int i = startIndex; i < endIndex; i++, t = eatTokenArr().t) {
        switch (t.type) {
            int val;
        case CLAMP_TOKEN:
            currentPrecedenceAddon += (t.subtype == CLAMP_PARANTHESES_L) * 5;
            currentPrecedenceAddon -= (t.subtype == CLAMP_PARANTHESES_R) * 5;
            break;
        case OPERATION_TOKEN:
            val = opValues[t.subtype];
            operatorPrecedence[i] = val + currentPrecedenceAddon;
            break;
        default:
            ;
        }

        if (t.type == END_LINE_TOKEN) {
            break;
        }
    }

    std::vector<std::pair<int, int>> vecOp(operatorPrecedence.begin(), operatorPrecedence.end());

    std::sort(vecOp.begin(), vecOp.end(), [](const auto a, const auto b) {
        return (a.second == b.second) ? (a.first < b.first) : (a.second > b.second);
        });

    int* takenTokens = new int[endIndex - startIndex];
    memset(takenTokens, -1, sizeof(int) * (endIndex - startIndex));

    std::vector<syntaxNode*> nodesOfOps(vecOp.size(), nullptr);
    int i2 = 0;

    for (auto it = vecOp.begin(); it != vecOp.end(); it++, i2++) {
        operatorNode* tmp = new operatorNode(tokensRandomAccessArray[it->first].t);

        const int leftTokenId = findNextSignificantTokenIndexLeft(startIndex, it->first, endIndex);
        const int rightTokenId = findNextSignificantTokenIndexRight(startIndex, it->first, endIndex);

        printf("%d\n", rightTokenId);
        
        int nextNodePlace = -1;
        int dir = 0;
        if (i2 < vecOp.size() - 1) {
            nextNodePlace = vecOp[i2 + 1].first;
            dir = nextNodePlace - it->first;
        }

        if (takenTokens[leftTokenId - startIndex] != -1) {
            const int takenBy = takenTokens[leftTokenId - startIndex];
            tmp->childNodes[0] = nodesOfOps[takenBy];
            for (int iterator = leftTokenId; takenTokens[iterator - startIndex] == takenBy && iterator >= startIndex; iterator--) {
                takenTokens[iterator - startIndex] = i2;
            }
        }
        else {
            tmp->childNodes[0] = getVarNodeFromToken(tokensRandomAccessArray[leftTokenId]);
            takenTokens[leftTokenId - startIndex] = i2;
        }

        if (takenTokens[rightTokenId - startIndex] != -1) {
            const int takenBy = takenTokens[rightTokenId - startIndex];
            tmp->childNodes[1] = nodesOfOps[takenBy];
            for (int iterator = rightTokenId; takenTokens[iterator - startIndex] == takenBy && iterator < endIndex; iterator++) {
                takenTokens[iterator - startIndex] = i2;
            }
        }
        else {
            tmp->childNodes[1] = getVarNodeFromToken(tokensRandomAccessArray[rightTokenId]);
            takenTokens[rightTokenId - startIndex] = i2;
        }

        nodesOfOps[i2] = tmp;
    }

    delete[] takenTokens;

    return nodesOfOps[vecOp.size() - 1];
}

void printNode(const syntaxNode* node) {

    printf("[ ");

    if (auto literal = dynamic_cast<const literalNode*>(node)) {
        printf(" literal %d ", literal->value);
    }
    else if (auto identifier = dynamic_cast<const identifierNode*>(node)) {
        printf(" identifier %s ", identifier->identifier.str.c_str());
    }
    else if (auto opNode = dynamic_cast<const operatorNode*>(node)) {
        printf(" op %d ", opNode->operatorToken.subtype);
        // print child nodes
        for (const auto child : opNode->childNodes) {
            printNode(child);
        }
    }

    printf("]");
}