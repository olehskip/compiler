#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include "symbols.hpp"

class LexicalAnalyzer;
struct LexicalVertice;
using Transition = std::pair<LexicalVertice *, char>;

enum class MetaRuleSymbol
{
    PARENTHESIS_OPEN,
    PARENTHESIS_CLOSE,
    BRACKET_OPEN,
    BRACKET_CLOSE,
    DOT,
    ASTERIX,
    PLUS
};

class LexicalAnalyzerConstructor
{
public:
    virtual ~LexicalAnalyzerConstructor() = 0;

    virtual void addRule(std::string rule, TerminalSymbol tokenToReturn) = 0;

protected:
    friend LexicalAnalyzer;
    std::vector<std::pair<LexicalVertice *, TerminalSymbol>> firstVertices;
};

class LexicalAnalyzer
{
public:
    LexicalAnalyzer(std::shared_ptr<LexicalAnalyzerConstructor> constructor_);

    TerminalSymbolsAst parse(std::string toParse);

private:
    const std::shared_ptr<LexicalAnalyzerConstructor> constructor;
};

#endif // LEXICAL_ANALYZER_HPP

