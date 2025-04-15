// c_style_compiler_avr.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "fileToString.h"
#include "tokenizer.h"
#include "parser.h"

int main()
{
    std::string src = readFileIntoString("src.txt");
    setSrc(src);
    printf("set source string\n\n");
    tokenizeSrc();
    setIterator();
    printf("tokenized\n\n");
    printf("tokens:     \n");

    for (; !isIteratorEnd(); advanceIterator()) {
        if (getTypeIterator() != -1) {
            printf("%s: %s\n", getTypeStringIterator(), getStringIterator());
        }
    }

    printf("\n\n");

    setTokenSrc(returnTokenList());

    //parseExpression2(0, 100);

    syntaxNode* n = parseTokens();

    printNode(n);

    printf("\n");

}