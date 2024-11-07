#include "PDA.h"
#include "CFG.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

PDA::PDA(const std::string &filename) {
    loadFromFile(filename);
}

void PDA::loadFromFile(const std::string &filename) {
    std::ifstream input(filename);
    if (!input) {
        std::cerr << "Could not open file " << filename << std::endl;
        exit(1);
    }

    json j;
    input >> j;

    // Load PDA properties
    startState = j["StartState"];
    startStack = j["StartStack"];

    for (const auto &state : j["States"]) {
        states.insert(state.get<std::string>());
    }

    for (const auto &symbol : j["Alphabet"]) {
        alphabet.insert(symbol.get<std::string>()[0]);
    }

    for (const auto &stackSym : j["StackAlphabet"]) {
        stackAlphabet.insert(stackSym.get<std::string>());
    }

    for (const auto &transition : j["Transitions"]) {
        std::string from = transition["from"];
        std::string input = transition["input"];
        std::string stacktop = transition["stacktop"];
        std::string to = transition["to"];
        std::vector<std::string> replacement;
        for (const auto &rep : transition["replacement"]) {
            replacement.push_back(rep.get<std::string>());
        }
        transitions.emplace_back(from, input, stacktop, to, replacement);
    }
}

std::map<std::string, std::vector<std::string>> PDA::getCFGProductions() {
    std::map<std::string, std::vector<std::string>> productions;

    // Start productions
    for (const auto &state : states) {
        productions["S"].push_back("[" + startState + "," + startStack + "," + state + "]");
    }

    // Process each transition in the PDA
    for (const auto &transition : transitions) {
        std::string fromState = std::get<0>(transition);
        std::string inputSymbol = std::get<1>(transition);
        std::string stackTop = std::get<2>(transition);
        std::string toState = std::get<3>(transition);
        std::vector<std::string> replacement = std::get<4>(transition);

        if (replacement.empty()) {
            // Case 1: Empty replacement
            std::string head = "[" + fromState + "," + stackTop + "," + toState + "]";
            // If inputSymbol is not empty, use it as production; otherwise, add ε (empty production)
            if (!inputSymbol.empty()) {
                productions[head].push_back(inputSymbol);
            } else {
                productions[head].push_back(""); // epsilon production
            }
        } else if (replacement.size() == 1) {
            // Case 2: Single symbol replacement
            for (const auto &intermediateState : states) {
                std::string head = "[" + fromState + "," + stackTop + "," + intermediateState + "]";
                std::string body = inputSymbol + " [" + toState + "," + replacement[0] + "," + intermediateState + "]";
                productions[head].push_back(body);
            }
        } else if (replacement.size() == 2) {
            // Case 3: Double symbol replacement
            for (const auto &intermediateState1 : states) {
                for (const auto &intermediateState2 : states) {
                    std::string head = "[" + fromState + "," + stackTop + "," + intermediateState1 + "]";
                    std::string body = inputSymbol + " [" + toState + "," + replacement[0] + "," + intermediateState2 + "] [" + intermediateState2 + "," + replacement[1] + "," + intermediateState1 + "]";
                    productions[head].push_back(body);
                }
            }
        }
    }

    return productions;
}




CFG PDA::toCFG() {
    CFG cfg;
    // Define non-terminals in the format [state1, stack_symbol, state2]
    for (const auto& state1 : states) {
        for (const auto& stackSymbol : stackAlphabet) {
            for (const auto& state2 : states) {
                std::string nonTerminal = "[" + state1 + "," + stackSymbol + "," + state2 + "]";
                cfg.nonTerminals.insert(nonTerminal);
            }
        }
    }

    // Add the start symbol 'S' to non-terminals
    cfg.nonTerminals.insert("S");

    // Add PDA's input alphabet symbols as terminals in CFG
    for (const auto& symbol : alphabet) {
        cfg.terminals.insert(symbol);
    }

    // Add production rules
    cfg.productionRules = getCFGProductions();

    // Set start symbol
    cfg.startSymbol = "S";

    return cfg;
}

