#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <string>
#include <vector>

#include "tokens.hpp"

class LexicalVertice;

typedef std::pair<LexicalVertice *, char> Transition;

class LexicalVertice {
  public:
    // friend class LexicalAnalyzer;
    // private:
    std::vector<Transition> transitions;
    bool isAccepting = false;
};

class LexicalAnalyzer {
  public:
    LexicalAnalyzer();

    void addRule(std::string rule, Token tokenToReturn);

    std::vector<Token> parse(std::string toParse);

// private:
    std::vector<std::pair<LexicalVertice *, Token>> firstVertices;
};

#endif // LEXICAL_ANALYZER_H
