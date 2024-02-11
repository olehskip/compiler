#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include "symbols.hpp"
#include <string>
#include <vector>

class LexicalVertice;

typedef std::pair<LexicalVertice *, char> Transition;

class LexicalVertice
{
public:
    // friend class LexicalAnalyzer;
    // private:
    std::vector<Transition> transitions;
    bool isAccepting = false;
};

class LexicalAnalyzer
{
public:
    LexicalAnalyzer();

    void addRule(std::string rule, TerminalSymbol tokenToReturn);

    TerminalSymbolsAst parse(std::string toParse);

    // private:
    std::vector<std::pair<LexicalVertice *, TerminalSymbol>> firstVertices;
};

#endif // LEXICAL_ANALYZER_HPP
