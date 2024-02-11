#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include <memory>
#include <unordered_map>
#include <variant>

#include "symbols.hpp"

struct Rule
{
public:
    auto operator<=>(const Rule &) const = default;

    NonTerminalSymbol lhs;
    Symbols rhs;
};

using RulesSet = std::set<Rule>;

class Item : public Rule
{
public:
    Item(Rule rule, size_t tPos, Symbol tLookaheadSymbol);
    Item(Item item, size_t tPos);

    auto operator<=>(const Item &) const = default;

    const size_t pos;
    const Symbol lookaheadSymbol;
};

using Items = std::vector<Item>;
using ItemsSet = std::set<Item>;

struct ReduceDecision;
struct ShiftDecision;
using Decision = std::variant<ReduceDecision, ShiftDecision>;

class State
{
public:
    using SharedPtr = std::shared_ptr<State>;

    State(ItemsSet tItemsSet);
    std::optional<Decision> getDecision(Symbol lookaheadSymbol);
    void addDecision(Symbol lookaheadSymbol, Decision decision);

    SharedPtr getGotoState(Symbol lookaheadSymbol);
    void addGotoState(Symbol lookaheadSymbol, SharedPtr state);

    ItemsSet itemsSet;

private:
    std::unordered_map<Symbol, Decision> decisionTable;
    std::unordered_map<Symbol, SharedPtr> gotoTable;
};

struct ReduceDecision
{
    NonTerminalSymbol lhs;
    Symbols rhs;
};

struct ShiftDecision
{
    State::SharedPtr state;
};

template <class T>
std::optional<T> tryConvertDecision(const Decision &decision)
{
    const T *ret = std::get_if<T>(&decision);
    return ret ? std::make_optional(*ret) : std::nullopt;
}

class SyntaxAnalyzer
{
public:
    SyntaxAnalyzer(NonTerminalSymbol tStartSymbol, TerminalSymbol tEndSymbol);

    void addRule(NonTerminalSymbol lhs, Symbols rhsSymbols);

    void start();
    NonTerminalSymbolAst::SharedPtr parse(TerminalSymbolsAst symbols);

private:
    SymbolsSet first(Symbol symbol);
    SymbolsSet first(Symbols symbols);
    SymbolsSet follow(Symbol symbol);
    ItemsSet closure(const ItemsSet &items);
    ItemsSet gotoItems(const ItemsSet &itemSet, Symbol symbol);

    void fillStateTables(const State::SharedPtr state);

    RulesSet allRulesSet;
    ItemsSet allItemsSet;
    State::SharedPtr startState;
    std::vector<State::SharedPtr> allStates;

    NonTerminalSymbol startSymbol;
    Symbol endSymbol;
    std::set<Symbol> allSymbols;

    std::unordered_map<Symbol, SymbolsSet> mFollowCache, mFirstCache;
};

#endif // SYNTAX_ANALYZER_HPP

