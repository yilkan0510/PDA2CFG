#include "PDA.h"
#include "CFG.h"
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

PDA::PDA(const string& filename) {
    ifstream input(filename);
    if (!input) {
        cerr << "Unable to open file " << filename << endl;
        exit(1);
    }
    json j;
    input >> j;

    // Laad PDA details
    startState = j["StartState"];
    startStack = j["StartStack"];
    for (const auto& state : j["States"]) {
        states.insert(state.get<string>());
    }
    for (const auto& symbol : j["Alphabet"]) {
        alphabet.insert(symbol.get<string>()[0]);
    }
    for (const auto& symbol : j["StackAlphabet"]) {
        stackAlphabet.insert(symbol.get<string>()[0]);
    }
    for (const auto& trans : j["Transitions"]) {
        string from = trans["from"];
        char input = trans["input"].get<string>()[0];
        string stackTop = trans["stacktop"];
        string to = trans["to"];
        vector<string> replacement = trans["replacement"];
        transitions.emplace_back(from, input, stackTop, to, replacement);
    }
}

CFG PDA::toCFG() const {
    CFG cfg;
    cfg.setStartSymbol("S");

    // Voeg producties toe voor elke combinatie van status en stacksymbool
    for (const auto& from : states) {
        for (const auto& stackSym : stackAlphabet) {
            for (const auto& to : states) {
                string variable = "[" + from + "," + string(1, stackSym) + "," + to + "]";
                cfg.addNonTerminal(variable);
            }
        }
    }

    // Start productie
    cfg.addProduction("S", "[" + startState + "," + startStack + "," + startState + "]");

    // Voeg producties toe gebaseerd op PDA-transities
    for (const auto& [from, input, stackTop, to, replacement] : transitions) {
        string head = "[" + from + "," + stackTop + "," + to + "]";


        if (replacement.empty()) {
            // Geen stackvervanging, enkele terminal productie
            cfg.addProduction(head, string(1, input));
        } else if (replacement.size() == 1) {
            // Vervanging van één stacksymbool
            string body = string(1, input) + " [" + to + "," + replacement[0] + "," + to + "]";
            cfg.addProduction(head, body);
        } else if (replacement.size() == 2) {
            // Vervanging van twee stack symbolen
            string body1 = "[" + from + "," + replacement[0] + "," + to + "]";
            string body2 = "[" + to + "," + replacement[1] + "," + to + "]";
            cfg.addProduction(head, body1 + " " + body2);
        }
    }

    return cfg;
}
