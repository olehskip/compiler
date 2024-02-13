#include "lexical_analyzer.hpp"

#define EPS '\0'

struct LexicalVertice
{
    std::vector<Transition> transitions;
    bool isAccepting = false;
};

LexicalAnalyzerConstructor::~LexicalAnalyzerConstructor() {}

LexicalAnalyzer::LexicalAnalyzer(std::shared_ptr<LexicalAnalyzerConstructor> constructor_)
    : constructor(constructor_)
{
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
        for (size_t ruleIndex = 0; ruleIndex < constructor->firstVertices.size(); ++ruleIndex) {
            const size_t curr =
                matchMaxRule(view, constructor->firstVertices[ruleIndex].first).first;
            if (curr > maxRuleMatched) {
                currentToken = constructor->firstVertices[ruleIndex].second;
                maxRuleMatched = curr;
            }
        }
        tokens.push_back(std::make_shared<TerminalSymbolAst>(
            currentToken, std::string(view.substr(0, maxRuleMatched))));
        view = view.substr(maxRuleMatched);
    } while (view.size() > 0 && maxRuleMatched > 0);

    return tokens;
}
