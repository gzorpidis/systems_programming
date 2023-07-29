#pragma once

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

namespace parsing {
    // A number of function designated to be used when parsing the input
    void removeDoubleQuotes(std::string& str);
    bool isDoubleQuoted(std::string& str);
    bool splitOnSymbol(std::vector<std::string> &words, int i, char c);
    
    std::vector<std::string> tokenize(const std::vector<std::string>& input);
    std::vector<std::string> parseString(std::string input);
    std::vector<std::string> splitOnAppend(std::vector<std::string>& vec);
    
    // Debug function
    void print(std::vector<std::string> vec);
}