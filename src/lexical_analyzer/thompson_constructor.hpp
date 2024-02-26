#ifndef THOMPSON_ANALYZER_HPP
#define THOMPSON_ANALYZER_HPP

#include "lexical_analyzer.hpp"

class ThompsonConstructor : public LexicalAnalyzerConstructor
{
public:
    ThompsonConstructor();
    ~ThompsonConstructor() override;

    void addRule(std::string rule, TerminalSymbol tokenToReturn) override;
    void addRules(std::vector<std::string> rules, TerminalSymbol tokenToReturn) override;
};

#endif // THOMPSON_ANALYZER_HPP
