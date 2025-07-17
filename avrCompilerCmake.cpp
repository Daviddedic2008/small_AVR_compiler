// avrCompilerCmake.cpp : Defines the entry point for the application.
//

#include "avrCompilerCmake.h"
#include "src/headers/compiler.h"
#include "src/headers/variableManager.h"
#include "src/headers/am328pEm.h"
#include "src/headers/assemblyGenerator.h"

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

    syntaxNode* n = parseTokens();

    convertExpression(n->childNodes[0]);

    printf("\n");

    //setupVm();

    //runVm();

    //printReg(16);

    //printCmp();
    
}