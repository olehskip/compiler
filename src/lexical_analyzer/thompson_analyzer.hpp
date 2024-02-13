#ifndef THOMPSON_ANALYZER_HPP
#define THOMPSON_ANALYZER_HPP

#include "lexical_analyzer.hpp"

struct LexicalVertice;
using Transition = std::pair<LexicalVertice *, char>;

class ThompsonAnalyzer : public LexicalAnalyzer
{
public:
    ThompsonAnalyzer();
    ~ThompsonAnalyzer() override;

    void addRule(std::string rule, TerminalSymbol tokenToReturn) override;
    TerminalSymbolsAst parse(std::string toParse) override;

private:
    std::vector<std::pair<LexicalVertice *, TerminalSymbol>> firstVertices;
};

#endif // THOMPSON_ANALYZER_HPP
