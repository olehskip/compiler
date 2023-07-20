#include "lexical_analyzer.hpp"

#include <iostream>
#include <stack>
#include <exception>

#define eps '\0'

LexicalAnalyzer::LexicalAnalyzer() {}

enum class SubregexType {
    GROUP, // (...)
    UNION,  // [...]
    STAR,
    PLUS,
    SIMPLE,
    ERROR
};

struct Subregex {
    LexicalVertice *begin = nullptr, *end = nullptr;
    SubregexType subregexType;
};

bool isRegularChar(char ch)
{
    return std::isdigit(ch) || std::isalpha(ch) || ch == '.';
}

static void addTransition(LexicalVertice *from, LexicalVertice *to, char transChar)
{
    from->transitions.push_back({to, transChar});
}

// returns subregex type based on first char of the subregex
static SubregexType getSubregexTypeByChar(char ch)
{
    if (isRegularChar(ch)) {
        return SubregexType::SIMPLE;
    }
    switch (ch) {
        case '(':
            return SubregexType::GROUP;
        case '[':
            return SubregexType::UNION;
        case '*':
            return SubregexType::STAR;
        case '+':
            return SubregexType::PLUS;
        default:
            return SubregexType::ERROR;
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

static Subregex processSubregex(std::string_view &ruleTail)
{
    const char currChar = ruleTail[0];
    ruleTail = ruleTail.substr(1);
    const auto subregexType = getSubregexTypeByChar(currChar);
    switch (subregexType) {
        case SubregexType::SIMPLE: {
            auto currSubregexBegin = new LexicalVertice(), newSubregexEnd = new LexicalVertice();
            addTransition(currSubregexBegin, newSubregexEnd, currChar);
            return Subregex{currSubregexBegin, newSubregexEnd, SubregexType::SIMPLE};
        } case SubregexType::GROUP: {
            auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
            auto lastChildSubregexEnd = currSubregexBegin;
            for (;;) {
                if(ruleTail.empty()) {
                    return {nullptr, nullptr, SubregexType::ERROR};
                }
                else if(ruleTail.at(0) == getSubregexTypeEndChar(SubregexType::GROUP)) {
                    ruleTail = ruleTail.substr(1);
                    break;
                }

                auto childSubregex = processSubregex(ruleTail);
                if (childSubregex.subregexType == SubregexType::ERROR) {
                    return {nullptr, nullptr, SubregexType::ERROR};
                }
                else {
                    addTransition(lastChildSubregexEnd, childSubregex.begin, eps);
                    lastChildSubregexEnd = childSubregex.end;
                }
            }

            addTransition(lastChildSubregexEnd, currSubregexEnd, eps);
            return Subregex{currSubregexBegin, currSubregexEnd, SubregexType::GROUP};
        }
        case SubregexType::UNION: {
            auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
            for (;;) {
                if(ruleTail.empty()) {
                    return {nullptr, nullptr, SubregexType::ERROR};
                }
                else if(ruleTail.at(0) == getSubregexTypeEndChar(SubregexType::UNION)) {
                    ruleTail = ruleTail.substr(1);
                    break;
                }

                auto childSubregex = processSubregex(ruleTail);
                if (childSubregex.subregexType == SubregexType::ERROR) {
                    return {nullptr, nullptr, SubregexType::ERROR};
                }
                else {
                    addTransition(currSubregexBegin, childSubregex.begin, eps);
                    addTransition(childSubregex.end, currSubregexEnd, eps);
                }
            }

            return Subregex{currSubregexBegin, currSubregexEnd, SubregexType::UNION};
        }
        case SubregexType::STAR: {
            auto currSubregexBegin = new LexicalVertice(),
                 currSubregexBeforeEnd = new LexicalVertice(),
                 currSubregexEnd = new LexicalVertice();
            auto childSubregex = processSubregex(ruleTail);
            addTransition(currSubregexBegin, currSubregexEnd, eps);
            addTransition(currSubregexBegin, childSubregex.begin, eps);
            addTransition(currSubregexBeforeEnd, currSubregexBegin, eps);
            addTransition(childSubregex.end, currSubregexBeforeEnd, eps);
            return Subregex{currSubregexBegin, currSubregexEnd, SubregexType::STAR};
        }
        case SubregexType::PLUS: {
            auto currSubregexBegin = new LexicalVertice(),
                 currSubregexBeforeEnd = new LexicalVertice(),
                 currSubregexEnd = new LexicalVertice();
            auto childSubregex = processSubregex(ruleTail);
            addTransition(currSubregexBegin, childSubregex.begin, eps);
            addTransition(currSubregexBeforeEnd, currSubregexBegin, eps);
            addTransition(childSubregex.end, currSubregexBeforeEnd, eps);
            return Subregex{currSubregexBegin, currSubregexEnd, SubregexType::STAR};
        }
        case SubregexType::ERROR:
        default: {
            return Subregex{nullptr, nullptr, SubregexType::ERROR};
        }
    }
}

void LexicalAnalyzer::addRule(std::string rule, Token tokenToReturn)
{
    rule = '(' + rule + ')';
    std::string_view ruleView = rule;
    Subregex regex = processSubregex(ruleView);
    assert(regex.subregexType != SubregexType::ERROR);
    regex.end->isAccepting = true;
    firstVertices.push_back({regex.begin, tokenToReturn});
}

std::pair<size_t, bool> matchMaxRule(std::string_view str, LexicalVertice *vertice)
{
    bool isMatched = vertice->isAccepting;
    size_t mx = 0;
    char currChar = str[0];
    for (auto &trans : vertice->transitions) {
        if (trans.second == '\0' || (str.size() != 0 && trans.second == currChar)) {
            const auto res = matchMaxRule(str.substr(trans.second == '\0' ? 0 : 1), trans.first);
            if (res.second) {
                mx = std::max(mx, (trans.second == '\0' ? 0 : 1) + res.first);
                isMatched = true;
            }
        }
    }

    return {isMatched ? mx : 0, isMatched};
}

std::vector<Token> LexicalAnalyzer::parse(std::string toParse)
{
    std::vector<Token> tokens;
    std::string_view view = toParse;
    size_t maxRuleMatched = 0;
    do {
        maxRuleMatched = 0;
        Token currentToken = Token::ERROR;
        for (size_t ruleIndex = 0; ruleIndex < firstVertices.size(); ++ruleIndex) {
            const size_t curr = matchMaxRule(view, firstVertices[ruleIndex].first).first;
            if (curr > maxRuleMatched) {
                currentToken = firstVertices[ruleIndex].second;
                maxRuleMatched = curr;
            }
        }
        tokens.push_back(currentToken);
        view = view.substr(maxRuleMatched);
    } while (view.size() > 0 && maxRuleMatched > 0);

    return tokens;
}
