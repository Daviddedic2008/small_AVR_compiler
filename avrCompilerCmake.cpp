// avrCompilerCmake.cpp : Defines the entry point for the application.
//

#include "avrCompilerCmake.h"
#include "src/headers/compiler.h"
#include "src/headers/variableManager.h"
#include "src/headers/am328pEm.h"

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

    printf("\n");

    setupVm();

    addOpcode(opcodeType::ldi, 16, 5);

    addOpcode(opcodeType::ldi, 17, 10);

    addOpcode(opcodeType::cp, 16, 17);

    addOpcode(opcodeType::brgt, nullopt, nullopt, "a");

    addOpcode(opcodeType::ldi, 16, 1);

    addLabel("a");

    runVm();

    printReg(16);

    //printCmp();
    
}