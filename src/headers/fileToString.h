#pragma once
#include <string>

std::string readFileIntoString(const std::string& filePath);

void setOutFile(const std::string& filePath);

void writeLine(const std::string& ln);