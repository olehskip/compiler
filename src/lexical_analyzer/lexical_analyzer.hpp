#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include "symbols.hpp"

class LexicalAnalyzer;
struct LexicalVertice;

struct Transition
{
    LexicalVertice *dstVertice;
    struct EPS
    {
    };
    struct ANY
    {
    };
    using Symbol = std::variant<EPS, ANY, char>;
    Symbol symbol;
};

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
    virtual void addRules(std::vector<std::string> rules, TerminalSymbol tokenToReturn) = 0;

    inline static const std::string allLetters =
        "[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ]";
    inline static const std::string allDigits = "[0123456789]";
    inline static const std::string allLettersAndDigits = "[" + allLetters + allDigits + "]";

protected:
    friend LexicalAnalyzer;
    std::vector<std::pair<LexicalVertice *, TerminalSymbol>> firstVertices;
};

class LexicalAnalyzer
{
public:
    LexicalAnalyzer(std::shared_ptr<LexicalAnalyzerConstructor> constructor_);

    TerminalSymbolsSt parse(std::string toParse);

private:
    const std::shared_ptr<LexicalAnalyzerConstructor> constructor;
};

#endif // LEXICAL_ANALYZER_HPP
