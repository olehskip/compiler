#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include "symbols.hpp"

class LexicalAnalyzer
{
public:
    virtual ~LexicalAnalyzer() = 0;

    virtual void addRule(std::string rule, TerminalSymbol tokenToReturn) = 0;
    virtual TerminalSymbolsAst parse(std::string toParse) = 0;
};

#endif // LEXICAL_ANALYZER_HPP

