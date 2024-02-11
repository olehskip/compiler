#include "lexical_analyzer.hpp"

#include <exception>
#include <iostream>
#include <stack>

#define EPS '\0'

LexicalAnalyzer::LexicalAnalyzer() {}

enum class SubregexType
{
    GROUP, // (...)
    UNION, // [...]
    SIMPLE,
    ERROR
};

enum class QuantifierType
{
    ASTERISK,
    PLUS,
    ERROR
};

struct Subregex
{
    LexicalVertice *begin = nullptr, *end = nullptr;
    SubregexType subregexType;
};

bool isRegularChar(char ch)
{
    return std::isdigit(ch) || std::isalpha(ch) || ch == '.' || ch == ';';
}

static void addTransition(LexicalVertice *from, LexicalVertice *to, char transChar)
{
    from->transitions.push_back(Transition{to, transChar});
}

// returns subregex type based on first char of the subregex
static SubregexType getSubregexType(char ch)
{
    if (isRegularChar(ch)) {
        return SubregexType::SIMPLE;
    }
    switch (ch) {
        case '(':
            return SubregexType::GROUP;
        case '[':
            return SubregexType::UNION;

        default:
            return SubregexType::ERROR;
    }
}

static QuantifierType getQuantifierType(char ch)
{
    switch (ch) {
        case '*':
            return QuantifierType::ASTERISK;
        case '+':
            return QuantifierType::PLUS;
        default:
            return QuantifierType::ERROR;
    }
}

constexpr static char getSubregexTypeEndChar(SubregexType subregexType)
{
    switch (subregexType) {
        case SubregexType::GROUP:
            return ')';
        case SubregexType::UNION:
            return ']';
        default:
            throw std::invalid_argument("subgregexType is invalid");
    }
}

static Subregex processSubregex(std::string_view &ruleTail);

static void processAsteriskQuantifier(std::string_view &ruleTail, Subregex &subregex)
{
    ruleTail = ruleTail.substr(1);
    auto newBegin = new LexicalVertice(), newEnd = new LexicalVertice();
    addTransition(newBegin, subregex.begin, EPS);
    addTransition(newBegin, newEnd, EPS);
    addTransition(subregex.end, subregex.begin, EPS);
    addTransition(subregex.end, newEnd, EPS);
    subregex.begin = newBegin;
    subregex.end = newEnd;
}

static void processPlusQuantifier(std::string_view &ruleTail, Subregex &subregex)
{
    ruleTail = ruleTail.substr(1);
    auto newBegin = new LexicalVertice(), newEnd = new LexicalVertice();
    addTransition(newBegin, subregex.begin, EPS);
    addTransition(subregex.end, subregex.begin, EPS);
    addTransition(subregex.end, newEnd, EPS);
    subregex.begin = newBegin;
    subregex.end = newEnd;
}

static void processPossibleQuantifier(std::string_view &ruleTail, Subregex &subregex)
{
    if (ruleTail.empty()) {
        return;
    }
    const auto quantifierType = getQuantifierType(ruleTail.at(0));
    switch (quantifierType) {
        case QuantifierType::ASTERISK:
            processAsteriskQuantifier(ruleTail, subregex);
            break;
        case QuantifierType::PLUS:
            processPlusQuantifier(ruleTail, subregex);
            break;
        default:
            break;
    }
}

static Subregex processSimpleSubregex(std::string_view &ruleTail)
{
    const char currChar = ruleTail[0];
    ruleTail = ruleTail.substr(1);
    auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
    addTransition(currSubregexBegin, currSubregexEnd, currChar);
    Subregex currSubregex{currSubregexBegin, currSubregexEnd, SubregexType::SIMPLE};
    processPossibleQuantifier(ruleTail, currSubregex);
    return currSubregex;
}

static Subregex processGroupSubregex(std::string_view &ruleTail)
{
    ruleTail = ruleTail.substr(1);
    auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
    auto lastChildSubregexEnd = currSubregexBegin;
    for (;;) {
        if (ruleTail.empty()) {
            return {nullptr, nullptr, SubregexType::ERROR};
        } else if (ruleTail.at(0) == getSubregexTypeEndChar(SubregexType::GROUP)) {
            ruleTail = ruleTail.substr(1);
            break;
        }

        auto childSubregex = processSubregex(ruleTail);
        if (childSubregex.subregexType == SubregexType::ERROR) {
            return {nullptr, nullptr, SubregexType::ERROR};
        } else {
            addTransition(lastChildSubregexEnd, childSubregex.begin, EPS);
            lastChildSubregexEnd = childSubregex.end;
        }
    }

    addTransition(lastChildSubregexEnd, currSubregexEnd, EPS);
    Subregex currSubregex{currSubregexBegin, currSubregexEnd, SubregexType::GROUP};
    processPossibleQuantifier(ruleTail, currSubregex);
    return currSubregex;
}

static Subregex processUnionSubregex(std::string_view &ruleTail)
{
    ruleTail = ruleTail.substr(1);
    auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
    for (;;) {
        if (ruleTail.empty()) {
            return {nullptr, nullptr, SubregexType::ERROR};
        } else if (ruleTail.at(0) == getSubregexTypeEndChar(SubregexType::UNION)) {
            ruleTail = ruleTail.substr(1);
            break;
        }

        auto childSubregex = processSubregex(ruleTail);
        if (childSubregex.subregexType == SubregexType::ERROR) {
            return {nullptr, nullptr, SubregexType::ERROR};
        } else {
            addTransition(currSubregexBegin, childSubregex.begin, EPS);
            addTransition(childSubregex.end, currSubregexEnd, EPS);
        }
    }

    Subregex currSubregex{currSubregexBegin, currSubregexEnd, SubregexType::UNION};
    processPossibleQuantifier(ruleTail, currSubregex);
    return currSubregex;
}

static Subregex processSubregex(std::string_view &ruleTail)
{
    if (ruleTail.empty()) {
        return Subregex{nullptr, nullptr, SubregexType::ERROR};
    }
    const char currChar = ruleTail[0];
    const auto subregexType = getSubregexType(currChar);
    switch (subregexType) {
        case SubregexType::SIMPLE:
            return processSimpleSubregex(ruleTail);
        case SubregexType::GROUP:
            return processGroupSubregex(ruleTail);
        case SubregexType::UNION:
            return processUnionSubregex(ruleTail);
        case SubregexType::ERROR:
        default:
            return Subregex{nullptr, nullptr, SubregexType::ERROR};
    }
}

void LexicalAnalyzer::addRule(std::string rule, TerminalSymbol tokenToReturn)
{
    rule = '(' + rule + ')';
    std::string_view ruleView = rule;
    Subregex regex = processSubregex(ruleView);
    assert(regex.subregexType != SubregexType::ERROR);
    regex.end->isAccepting = true;
    firstVertices.push_back({regex.begin, tokenToReturn});
}

static std::pair<size_t, bool> matchMaxRule(std::string_view str, LexicalVertice *vertice)
{
    bool isMatched = vertice->isAccepting;
    size_t mx = 0;
    for (auto &trans : vertice->transitions) {
        if (trans.second == EPS ||
            (str.size() > 0 && (trans.second == '.' || trans.second == str[0]))) {
            const auto res = matchMaxRule(str.substr(trans.second != EPS), trans.first);
            if (res.second) {
                mx = std::max(mx, (trans.second != EPS) + res.first);
                isMatched = true;
            }
        }
    }

    return {isMatched ? mx : 0, isMatched};
}

TerminalSymbolsAst LexicalAnalyzer::parse(std::string toParse)
{
    TerminalSymbolsAst tokens;
    std::string_view view = toParse;
    size_t maxRuleMatched = 0;
    do {
        maxRuleMatched = 0;
        TerminalSymbol currentToken = TerminalSymbol::ERROR;
        for (size_t ruleIndex = 0; ruleIndex < firstVertices.size(); ++ruleIndex) {
            const size_t curr = matchMaxRule(view, firstVertices[ruleIndex].first).first;
            if (curr > maxRuleMatched) {
                currentToken = firstVertices[ruleIndex].second;
                maxRuleMatched = curr;
            }
        }
        tokens.push_back(std::make_shared<TerminalSymbolAst>(
            currentToken, std::string(view.substr(0, maxRuleMatched))));
        view = view.substr(maxRuleMatched);
    } while (view.size() > 0 && maxRuleMatched > 0);

    return tokens;
}
