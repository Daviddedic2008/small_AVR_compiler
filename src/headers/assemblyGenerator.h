#pragma once

#include "parser.h"

extern syntaxNode programParentNode;

void setParentNode(const syntaxNode& n);

void convertExpression(syntaxNode* node); // tmp test

void asmOp(syntaxNode** node); // test

void assembleOpTree(syntaxNode** startNode);

void ifAsm(syntaxNode** startNode);

void forAsm(syntaxNode** startNode);

void whileAsm(syntaxNode** startNode);