// avrCompilerCmake.cpp : Defines the entry point for the application.
//

#include "avrCompilerCmake.h"
#include "src/compiler.h"

using namespace std;

int main()
{
    std::string src = readFileIntoString("C:\\Users\\david\\OneDrive\\Documents\\avrCompilerCmake\\src.txt");
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