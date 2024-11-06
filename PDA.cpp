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

CFG PDA::toCFG() {
    CFG cfg;

    // Define start symbol
    cfg.startSymbol = "S";
    cfg.nonTerminals.insert("S");

    // Define terminals (same as alphabet)
    cfg.terminals = alphabet;

    // Generate non-terminals of the form [p,X,q] for each state and stack symbol combination
    for (const auto &p : states) {
        for (const auto &q : states) {
            for (const auto &X : stackAlphabet) {
                std::string variable = "[" + p + "," + X + "," + q + "]";
                cfg.nonTerminals.insert(variable);
            }
        }
    }

    // Add production rules for the start symbol
    for (const auto &q : states) {
        std::string startVar = "[" + startState + "," + startStack + "," + q + "]";
        cfg.productionRules["S"].push_back(startVar);
    }

    // Explicitly handle each transition
    for (const auto &transition : transitions) {
        std::string from = std::get<0>(transition);
        std::string input = std::get<1>(transition);
        std::string stacktop = std::get<2>(transition);
        std::string to = std::get<3>(transition);
        const std::vector<std::string> &replacement = std::get<4>(transition);

        std::string varFromTo = "[" + from + "," + stacktop + "," + to + "]";

        if (replacement.empty()) {
            // Direct epsilon transition
            if (input.empty()) {
                cfg.productionRules[varFromTo].push_back("");
            } else {
                cfg.productionRules[varFromTo].push_back(input);
            }
        }
        else if (replacement.size() == 1) {
            // Single replacement with intermediate states
            for (const auto &r : states) {
                std::string repVar = "[" + to + "," + replacement[0] + "," + r + "]";
                if (input.empty()) {
                    cfg.productionRules[varFromTo].push_back(repVar);
                } else {
                    cfg.productionRules[varFromTo].push_back(input + " " + repVar);
                }
            }
        }
        else if (replacement.size() == 2) {
            // Double replacement with specific intermediate states
            for (const auto &r : states) {
                for (const auto &s : states) {
                    std::string repVar1 = "[" + to + "," + replacement[0] + "," + r + "]";
                    std::string repVar2 = "[" + r + "," + replacement[1] + "," + s + "]";
                    if (input.empty()) {
                        cfg.productionRules[varFromTo].push_back(repVar1 + " " + repVar2);
                    } else {
                        cfg.productionRules[varFromTo].push_back(input + " " + repVar1 + " " + repVar2);
                    }
                }
            }
        }
    }

    // Sort and deduplicate production rules
    for (auto &rule : cfg.productionRules) {
        std::sort(rule.second.begin(), rule.second.end());
        rule.second.erase(std::unique(rule.second.begin(), rule.second.end()), rule.second.end());
    }

    return cfg;
}

