#include "lexical_analyzer.hpp"

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
        const bool isEPS = std::holds_alternative<Transition::EPS>(trans.symbol);
        const bool isANY = std::holds_alternative<Transition::ANY>(trans.symbol);
        auto doesCharMatch = [&](Transition::Symbol transSymbol, char toMatch) {
            const char *charSymbol = std::get_if<char>(&transSymbol);
            return charSymbol && *charSymbol == toMatch;
        };
        if (isEPS || (str.size() > 0 && (doesCharMatch(trans.symbol, str[0]) || isANY))) {
            const auto res = matchMaxRule(str.substr(!isEPS), trans.dstVertice);
            if (res.second) {
                mx = std::max(mx, !isEPS + res.first);
                isMatched = true;
            }
        }
    }

    return {isMatched ? mx : 0, isMatched};
}

TerminalSymbolsSt LexicalAnalyzer::parse(std::string toParse)
{
    TerminalSymbolsSt tokens;
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
        tokens.push_back(std::make_shared<TerminalSymbolSt>(
            currentToken, std::string(view.substr(0, maxRuleMatched))));
        view = view.substr(maxRuleMatched);
    } while (view.size() > 0 && maxRuleMatched > 0);

    return tokens;
}
