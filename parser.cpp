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

const char opValues[] = {
    1, // += (lowest precedence)
    1, // -=
    1, // *=
    1, // = 
    2, // +
    2, // -
    3, // *
    3, // /
    2, // ==
    2, // >
    2, // <
    2, // >=
    2, // <=
    4, // ++
    4  // --
};

const char* operatorStrings[] = {
    "+=",  // OP_PLUS_EQUALS
    "-=",  // OP_MINUS_EQUALS
    "*=",  // OP_MUL_EQUALS
    "=",   // OP_EQUALS
    "+",   // OP_PLUS
    "-",   // OP_MINUS
    "*",   // OP_MUL
    "/",   // OP_DIV
    "==",  // OP_EQUALS_EQUALS
    ">",   // OP_GREATER
    "<",   // OP_LESS
    ">=",  // OP_GREATER_EQUAL
    "<=",  // OP_LESS_EQUAL
    "++",  // OP_INCREMENT
    "--"   // OP_DECREMENT
};

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

syntaxNode* parseExpression(const int startIndex, const int endIndex) {
    std::unordered_map<int, int> operatorPrecedence;

    setArrayPosition(startIndex);

    // first: index of op. second: precedence value (greater means higher precedence)
    token_str t = eatTokenArr();

    int allocSz = -1;

    if (t.t.type == TYPE_TOKEN) {
        allocSz = (t.t.subtype == TYPE_INT) ? 4 : 1;
    }

    int currentPrecedenceAddon = 0;

    int currentSigToken = 0;

    std::vector<token_str> sigTokens;

    for (int i = startIndex; i < endIndex; i++, t = eatTokenArr()) {
        switch (t.t.type) {
            int val;
        case CLAMP_TOKEN:

            currentPrecedenceAddon += (t.t.subtype == CLAMP_PARANTHESES_L) * 5;
            currentPrecedenceAddon -= (t.t.subtype == CLAMP_PARANTHESES_R) * 5;
            break;
        case TYPE_TOKEN:
            break;

        case OPERATION_TOKEN:
            val = opValues[t.t.subtype];
            operatorPrecedence[currentSigToken] = val + currentPrecedenceAddon;
            sigTokens.push_back(t);
            currentSigToken++;
            break;
        default:
            sigTokens.push_back(t);
            currentSigToken++;
            ;
        }

        if (t.t.type == END_LINE_TOKEN || (t.t.type == CLAMP_TOKEN && t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_L)) {
            break;
        }

    }

    std::vector<std::pair<int, int>> vecOp(operatorPrecedence.begin(), operatorPrecedence.end());

    std::sort(vecOp.begin(), vecOp.end(), [](const auto a, const auto b) {
        return (a.second == b.second) ? (a.first < b.first) : (a.second > b.second);
        });


    
    int* takenTokens = new int[sigTokens.size()];
    memset(takenTokens, -1, sizeof(int) * sigTokens.size());

    std::vector<syntaxNode*> nodesOfOps(vecOp.size(), nullptr);
    int i2 = 0;

    for (auto it = vecOp.begin(); it != vecOp.end(); it++, i2++) {
        operatorNode* tmp = new operatorNode(sigTokens[it->first].t);
        
        if (tmp->operatorToken.subtype == OP_EQUALS) {
            tmp->allocSize = allocSz;
        }

        const int leftTokenId = it->first - 1;
        const int rightTokenId = it->first + 1;
        
        int nextNodePlace = -1;
        int dir = 0;
        if (i2 < vecOp.size() - 1) {
            nextNodePlace = vecOp[i2 + 1].first;
            dir = nextNodePlace - it->first;
        }

        if (takenTokens[leftTokenId] != -1) {
            const int takenBy = takenTokens[leftTokenId];
            tmp->childNodes[0] = nodesOfOps[takenBy];
            for (int iterator = 0; iterator < sigTokens.size(); iterator++) {
                if (takenTokens[iterator] == takenBy) {
                    takenTokens[iterator] = i2;
                }
            }
        }
        else {
            tmp->childNodes[0] = getVarNodeFromToken(sigTokens[leftTokenId]);
            takenTokens[leftTokenId] = i2;
        }

        if (takenTokens[rightTokenId] != -1) {
            const int takenBy = takenTokens[rightTokenId];
            tmp->childNodes[1] = nodesOfOps[takenBy];
            for (int iterator = 0; iterator < sigTokens.size(); iterator++) {
                if (takenTokens[iterator] == takenBy) {
                    takenTokens[iterator] = i2;
                }
            }
        }
        else {
            tmp->childNodes[1] = getVarNodeFromToken(sigTokens[rightTokenId]);
            takenTokens[rightTokenId] = i2;
        }

        nodesOfOps[i2] = tmp;
    }

    delete[] takenTokens;

    return nodesOfOps[vecOp.size() - 1];
}

syntaxNode* parseChunk(int startBody, const int endBody) {
    syntaxNode* ret = new syntaxNode(nodeType::bodyStatement);

    std::vector<int> locationsOfEndlines;

    locationsOfEndlines.push_back(startBody - 1);

    int br = 0;

    for (token_str t = tokensRandomAccessArray[startBody]; startBody < endBody; startBody++, t = tokensRandomAccessArray[startBody]) {

        if ((t.t.type == END_LINE_TOKEN && br == 0) || (t.t.type == CLAMP_TOKEN && t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_R)) {
            locationsOfEndlines.push_back(startBody);
        }

        br += (t.t.type == CLAMP_TOKEN) * ((t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_L) * -1 + (t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_R));
    }
    locationsOfEndlines.push_back(startBody + 1);

    std::vector<nodeType> typesOfExpr;

    for (int i = 0; i < locationsOfEndlines.size() - 1 + (locationsOfEndlines.size() == 1); i++) {
        switch (tokensRandomAccessArray[locationsOfEndlines[i] + 1].t.type) {
        case NAME_TOKEN:
            typesOfExpr.push_back(nodeType::opNode);
            ret->childNodes.push_back(parseExpression(locationsOfEndlines[i] + 1, locationsOfEndlines[i + 1]));
            break;
        case TYPE_TOKEN:
            typesOfExpr.push_back(nodeType::opNode);
            ret->childNodes.push_back(parseExpression(locationsOfEndlines[i] + 1, locationsOfEndlines[i + 1]));
            break;
        case IDENTIFIER_TOKEN:
            typesOfExpr.push_back(nodeType::keywordNode);

            ret->childNodes.push_back(parseIf(locationsOfEndlines[i] + 1, locationsOfEndlines[i + 1] - 1));
            break;
        }
    }

    return ret;
}

syntaxNode* parseIf(const int startIndex, const int endIndex) {

    token_str currentToken = tokensRandomAccessArray[startIndex];

    keywordNode* ret = new keywordNode(currentToken.t);

    const int startExpression = startIndex + 1;
    
    int endExpression = startExpression;

    for(token t = tokensRandomAccessArray[startExpression].t; t.type != CLAMP_TOKEN && t.subtype != CLAMP_SQUIGGLY_BRACKETS_L; endExpression++, t = tokensRandomAccessArray[endExpression].t){}

    syntaxNode* tmpOp = parseExpression(startExpression, endExpression);

    ret->childNodes[0] = tmpOp;

    int startBody = endExpression+1;

    int endBody = startBody+1;

    int br = 0;

    for (token_str t = tokensRandomAccessArray[startBody]; t.t.type != CLAMP_TOKEN || t.t.subtype != CLAMP_SQUIGGLY_BRACKETS_R || br != 0; endBody++, t = tokensRandomAccessArray[endBody]) {

        br += (t.t.type == CLAMP_TOKEN) * ((t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_L) * -1 + (t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_R));
    }
    
    ret->childNodes[1] = parseChunk(startBody, endBody);

    return ret;
}

syntaxNode* parseFor(const int startIndex, const int endIndex) {
    token_str currentToken = tokensRandomAccessArray[startIndex];

    keywordNode* ret = new keywordNode(currentToken.t);

    const int startExpression = startIndex + 1;

    int endExpression = startExpression;

    for (token t = tokensRandomAccessArray[startExpression].t; t.type != CLAMP_TOKEN && t.subtype != CLAMP_SQUIGGLY_BRACKETS_L; endExpression++, t = tokensRandomAccessArray[endExpression].t) {}

    syntaxNode* tmpOp = parseExpression(startExpression, endExpression);

    ret->childNodes[0] = tmpOp;

    int startBody = endExpression + 1;

    int endBody = startBody + 1;

    int br = 0;

    for (token_str t = tokensRandomAccessArray[startBody]; t.t.type != CLAMP_TOKEN || t.t.subtype != CLAMP_SQUIGGLY_BRACKETS_R || br != 0; endBody++, t = tokensRandomAccessArray[endBody]) {

        br += (t.t.type == CLAMP_TOKEN) * ((t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_L) * -1 + (t.t.subtype == CLAMP_SQUIGGLY_BRACKETS_R));
    }

    ret->childNodes[1] = parseChunk(startBody, endBody);

    return ret;
}

void printNode(const syntaxNode* node) {

    printf("[ ");

    if (node->type == nodeType::literalNode) {
        auto literal = dynamic_cast<const literalNode*>(node);
        printf(" %d ", literal->value);
    }
    else if (node->type == nodeType::identifierNode) {
        auto identifier = dynamic_cast<const identifierNode*>(node);
        printf(" %s ", identifier->identifier.str.c_str());
    }
    else if (node->type == nodeType::opNode) {
        auto opNode = dynamic_cast<const operatorNode*>(node);
        printf(" %s ", operatorStrings[opNode->operatorToken.subtype]);
        if (opNode->allocSize != -1) {
            printf(" alloc %d bytes ", opNode->allocSize);
        }
        // print child nodes
        for (const auto child : opNode->childNodes) {
            printNode(child);
        }
        
    }

    else if (node->type == nodeType::keywordNode) {
        auto keyNode = dynamic_cast<const keywordNode*>(node);
        printf(" %s ", "if");
        for (const auto child : keyNode->childNodes) {
            if (child->type != nodeType::uninitialized) {
                printNode(child);
            }
        }
    }

    else if (node->type == nodeType::bodyStatement) {
        printf("\n");
        printf(" body ");
        for (const auto child : node->childNodes) {
            if (child->type != nodeType::uninitialized) {
                printNode(child);
            }
        }
    }

    printf("]");
    printf("\n");
}

syntaxNode* parseTokens() {
    syntaxNode* ret = parseChunk(0, tokenSrc.size()-1);
    return ret;
}