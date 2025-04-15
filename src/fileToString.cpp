#include <iostream>
#include <fstream>
#include <sstream>
#include "headers/fileToString.h"

std::ofstream outPath;

void setOutFile(const std::string& filePath) {
    outPath = std::ofstream(filePath);
}

void writeLine(const std::string& ln) {
    outPath << ln << std::endl;
}

std::string readFileIntoString(const std::string& filePath) {
    std::ifstream fileStream(filePath);

    if (!fileStream) {
        throw std::runtime_error("cant open " + filePath);
    }

    std::ostringstream stringStream;
    stringStream << fileStream.rdbuf();
    return stringStream.str();
}