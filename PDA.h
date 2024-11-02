#ifndef PDA_H
#define PDA_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include "CFG.h"  // Include the CFG class

class PDA {
private:
    std::string startState;
    std::string startStack;
    std::set<std::string> states;
    std::set<char> alphabet;
    std::set<char> stackAlphabet;
    std::vector<std::tuple<std::string, char, std::string, std::string, std::vector<std::string>>> transitions;

public:
    PDA(const std::string& filename);
    CFG toCFG() const;  // Method to convert PDA to CFG
};

#endif // PDA_H
