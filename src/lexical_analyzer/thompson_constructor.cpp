#include "thompson_constructor.hpp"
#include "log.hpp"

#include <exception>
#include <stack>

struct LexicalVertice
{
    std::vector<Transition> transitions;
    bool isAccepting = false;
};

ThompsonConstructor::ThompsonConstructor() {}
ThompsonConstructor::~ThompsonConstructor() {}

enum class SubregexType
{
    GROUP, // (...)
    UNION, // [...]
    SIMPLE
};

struct RuleSymbol
{
    std::variant<char, MetaRuleSymbol, std::monostate> symbol;
    size_t width;
};

struct Subregex
{
    LexicalVertice *begin = nullptr, *end = nullptr;
    SubregexType subregexType; // is stored to know whether the finishing symbol matches the type
};

using MaybeSubregex = std::optional<Subregex>;

static inline void addTransition(LexicalVertice *from, LexicalVertice *to,
                                 Transition::Symbol transSymbol)
{
    from->transitions.push_back(Transition{to, transSymbol});
}

static bool isAllowedChar(char c)
{
    static std::string allowedChars = std::string("_;'\"# \n");
    return std::isalnum(c) || allowedChars.find(c) != std::string::npos;
}

static RuleSymbol getNextRuleSymbol(std::string_view &rule_view)
{
    RuleSymbol ret = {std::monostate(), 0};
    if (rule_view.size() == 0) {
        ret = {std::monostate(), 0};
    } else if (rule_view[0] == '\\') {
        if (rule_view.size() > 1) {
            ret = {rule_view[1], 2};
        }
    } else if (isAllowedChar(rule_view[0])) { // TODO: redo it
        ret = {rule_view[0], 1};
    } else {
        switch (rule_view[0]) {
            case '(': {
                ret = {MetaRuleSymbol::PARENTHESIS_OPEN, 1};
                break;
            }
            case ')': {
                ret = {MetaRuleSymbol::PARENTHESIS_CLOSE, 1};
                break;
            }
            case '[': {
                ret = {MetaRuleSymbol::BRACKET_OPEN, 1};
                break;
            }
            case ']': {
                ret = {MetaRuleSymbol::BRACKET_CLOSE, 1};
                break;
            }
            case '.': {
                ret = {MetaRuleSymbol::DOT, 1};
                break;
            }
            case '*': {
                ret = {MetaRuleSymbol::ASTERIX, 1};
                break;
            }
            case '+': {
                ret = {MetaRuleSymbol::PLUS, 1};
                break;
            }
        }
    }
    return ret;
}

static bool isRuleSymbolValid(RuleSymbol ruleSymbol)
{
    return !std::holds_alternative<std::monostate>(ruleSymbol.symbol);
}

static inline bool doesSubregexLastSymbolMatch(RuleSymbol lastSymbol, SubregexType subregexType)
{
    if (const MetaRuleSymbol *metaRuleSymbol = std::get_if<MetaRuleSymbol>(&lastSymbol.symbol)) {
        switch (*metaRuleSymbol) {
            case MetaRuleSymbol::PARENTHESIS_CLOSE: {
                return subregexType == SubregexType::GROUP;
            }
            case MetaRuleSymbol::BRACKET_CLOSE: {
                return subregexType == SubregexType::UNION;
            }
            default: {
                break;
            }
        }
    }
    return false;
}

static std::optional<Subregex> processSubregex(std::string_view &ruleTail);

static void processAsteriskQuantifier(std::string_view &ruleTail, Subregex &subregex)
{
    ruleTail = ruleTail.substr(1);
    auto newBegin = new LexicalVertice(), newEnd = new LexicalVertice();
    addTransition(newBegin, subregex.begin, Transition::EPS());
    addTransition(newBegin, newEnd, Transition::EPS());
    addTransition(subregex.end, subregex.begin, Transition::EPS());
    addTransition(subregex.end, newEnd, Transition::EPS());
    subregex.begin = newBegin;
    subregex.end = newEnd;
}

static void processPlusQuantifier(std::string_view &ruleTail, Subregex &subregex)
{
    ruleTail = ruleTail.substr(1);
    auto newBegin = new LexicalVertice(), newEnd = new LexicalVertice();
    addTransition(newBegin, subregex.begin, Transition::EPS());
    addTransition(subregex.end, subregex.begin, Transition::EPS());
    addTransition(subregex.end, newEnd, Transition::EPS());
    subregex.begin = newBegin;
    subregex.end = newEnd;
}

static void processPossibleQuantifier(std::string_view &ruleTail, Subregex &subregex)
{
    const auto currRuleSymbol = getNextRuleSymbol(ruleTail);
    if (const MetaRuleSymbol *metaRuleSymbol =
            std::get_if<MetaRuleSymbol>(&currRuleSymbol.symbol)) {
        switch (*metaRuleSymbol) {
            case MetaRuleSymbol::ASTERIX: {
                processAsteriskQuantifier(ruleTail, subregex);
                break;
            }
            case MetaRuleSymbol::PLUS: {
                processPlusQuantifier(ruleTail, subregex);
                break;
            }
            default:
                break;
        }
    }
}

static MaybeSubregex processSimpleSubregex(std::string_view &ruleTail)
{
    const auto currRuleSymbol = getNextRuleSymbol(ruleTail);
    ruleTail = ruleTail.substr(currRuleSymbol.width);
    auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
    addTransition(currSubregexBegin, currSubregexEnd, [&]() -> Transition::Symbol {
        if (const char *charSymbol = std::get_if<char>(&currRuleSymbol.symbol)) {
            return *charSymbol;
        } else if (const MetaRuleSymbol *metaSymbol =
                       std::get_if<MetaRuleSymbol>(&currRuleSymbol.symbol);
                   *metaSymbol == MetaRuleSymbol::DOT) {
            return Transition::ANY();
        }
        SHOULD_NOT_HAPPEN;
        return {};
    }());
    Subregex currSubregex{currSubregexBegin, currSubregexEnd, SubregexType::SIMPLE};
    processPossibleQuantifier(ruleTail, currSubregex);
    return currSubregex;
}

static MaybeSubregex processGroupSubregex(std::string_view &ruleTail)
{
    ruleTail = ruleTail.substr(1);
    auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
    auto lastChildSubregexEnd = currSubregexBegin;
    for (;;) {
        const auto currRuleSymbol = getNextRuleSymbol(ruleTail);
        if (!isRuleSymbolValid(currRuleSymbol)) {
            return std::nullopt;
        } else if (doesSubregexLastSymbolMatch(currRuleSymbol, SubregexType::GROUP)) {
            ruleTail = ruleTail.substr(1);
            break;
        }

        auto childSubregex = processSubregex(ruleTail);
        if (childSubregex) {
            addTransition(lastChildSubregexEnd, childSubregex->begin, Transition::EPS());
            lastChildSubregexEnd = childSubregex->end;
        } else {
            return std::nullopt;
        }
    }

    addTransition(lastChildSubregexEnd, currSubregexEnd, Transition::EPS());
    Subregex currSubregex{currSubregexBegin, currSubregexEnd, SubregexType::GROUP};
    processPossibleQuantifier(ruleTail, currSubregex);
    return currSubregex;
}

static MaybeSubregex processUnionSubregex(std::string_view &ruleTail)
{
    ruleTail = ruleTail.substr(1);
    auto currSubregexBegin = new LexicalVertice(), currSubregexEnd = new LexicalVertice();
    for (;;) {
        const auto currRuleSymbol = getNextRuleSymbol(ruleTail);
        if (!isRuleSymbolValid(currRuleSymbol)) {
            return std::nullopt;
        } else if (doesSubregexLastSymbolMatch(currRuleSymbol, SubregexType::UNION)) {
            ruleTail = ruleTail.substr(1);
            break;
        }

        auto childSubregex = processSubregex(ruleTail);
        if (childSubregex) {
            addTransition(currSubregexBegin, childSubregex->begin, Transition::EPS());
            addTransition(childSubregex->end, currSubregexEnd, Transition::EPS());
        } else {
            return std::nullopt;
        }
    }

    Subregex currSubregex{currSubregexBegin, currSubregexEnd, SubregexType::UNION};
    processPossibleQuantifier(ruleTail, currSubregex);
    return currSubregex;
}

static MaybeSubregex processSubregex(std::string_view &ruleTail)
{
    const auto currRuleSymbol = getNextRuleSymbol(ruleTail);
    if (std::holds_alternative<char>(currRuleSymbol.symbol)) {
        return processSimpleSubregex(ruleTail);
    } else if (const MetaRuleSymbol *metaRuleSymbol =
                   std::get_if<MetaRuleSymbol>(&currRuleSymbol.symbol)) {
        switch (*metaRuleSymbol) {
            case MetaRuleSymbol::PARENTHESIS_OPEN: {
                return processGroupSubregex(ruleTail);
            }
            case MetaRuleSymbol::BRACKET_OPEN: {
                return processUnionSubregex(ruleTail);
            }
            case MetaRuleSymbol::DOT: {
                return processSimpleSubregex(ruleTail);
            }
            default: {
                break;
            }
        }
    }
    return std::nullopt;
}

void ThompsonConstructor::addRule(std::string rule, TerminalSymbol tokenToReturn)
{
    rule = '(' + rule + ')';
    std::string_view ruleView = rule;
    MaybeSubregex maybeRegex = processSubregex(ruleView);
    assert(maybeRegex);
    maybeRegex->end->isAccepting = true;
    firstVertices.push_back({maybeRegex->begin, tokenToReturn});
}

void ThompsonConstructor::addRules(std::vector<std::string> rules, TerminalSymbol tokenToReturn)
{
    for (auto &rule : rules) {
        addRule(rule, tokenToReturn);
    }
}
