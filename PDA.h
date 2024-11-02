#ifndef PDA_H
#define PDA_H

#include "CFG.h"
#include <string>
#include <map>
#include <vector>

class PDA {
private:
    std::string startState;
    std::string startStack;
    std::set<std::string> states;
    std::set<char> alphabet;
    std::set<std::string> stackAlphabet;
    std::vector<std::tuple<std::string, std::string, std::string, std::string, std::vector<std::string>>> transitions;

    void loadFromFile(const std::string &filename);

public:
    PDA(const std::string &filename);
    CFG toCFG();
};

#endif // PDA_H
