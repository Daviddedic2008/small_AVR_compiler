// avrCompilerCmake.cpp : Defines the entry point for the application.
//

#include "avrCompilerCmake.h"
#include "src/headers/compiler.h"
#include "src/headers/variableManager.h"

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

    printf("\n");

    incrementScope();
    storedVar v = storedVar(2, "v1");

    
    addVariable(v);
    writeImmediateToVariable(v, 10);
    multiplyVariableImmediate(v, 2);
    decrementScope();
}