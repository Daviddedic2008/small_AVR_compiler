#pragma once

#include "parser.h"

extern syntaxNode programParentNode;

void setParentNode(const syntaxNode& n);

void convertExpression(syntaxNode* node); // tmp test