#include <iostream>
#include <fstream>
#include <sstream>
#include "fileToString.h"

std::string readFileIntoString(const std::string& filePath) {
    std::ifstream fileStream(filePath);

    if (!fileStream) {
        throw std::runtime_error("cant open " + filePath);
    }

    std::ostringstream stringStream;
    stringStream << fileStream.rdbuf();
    return stringStream.str();
}