#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include <memory>
#include <unordered_map>

#include "symbols.hpp"

struct Rule
{
public:
    auto operator<=>(const Rule &) const = default;

    Symbol lhs;
    Symbols rhs;
};

using RulesSet = std::set<Rule>;

class Item: public Rule
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

enum class DecisionType
{
    ERROR,
    REDUCE,
    SHIFT,
    ACCEPT
};

class State;
using StateShared = std::shared_ptr<State>;

class Decision
{
public:
    // Decision(DecisionType tDecisionType, State *tState) = default;
    // Decision &operator=(const Decision &anotherDecision) = default;
    DecisionType decisionType;
    StateShared state;
    Symbol lhs = Symbol(NonTerminalSymbol::EPS); // TODO: remove it
    Symbols rhs = {}; // TODO: remove it
};

class State
{
public:
    State(ItemsSet tItemsSet);
    Decision getDecision(Symbol lookaheadSymbol);
    void addDecision(Symbol lookaheadSymbol, Decision decision);

    StateShared getGotoState(Symbol lookaheadSymbol);
    void addGotoState(Symbol lookaheadSymbol, StateShared state);

    ItemsSet itemsSet;

private:
    std::unordered_map<Symbol, Decision> decisionTable;
    std::unordered_map<Symbol, StateShared> gotoTable;
};


class SyntaxAnalyzer
{
public:
    SyntaxAnalyzer(NonTerminalSymbol tStartSymbol, TerminalSymbol tEndSymbol);

    void addRule(NonTerminalSymbol lhs, Symbols rhsSymbols);

    void start();
    void parse(Symbols symbols);

private:
    SymbolsSet first(Symbol symbol);
    SymbolsSet first(Symbols symbols);
    SymbolsSet follow(Symbol symbol);
    ItemsSet closure(const ItemsSet &items);
    ItemsSet gotoItems(const ItemsSet &itemSet, Symbol symbol);

    void fillStateTables(const StateShared state);

    RulesSet allRulesSet;
    ItemsSet allItemsSet;
    StateShared startState;
    std::vector<StateShared> allStates;

    Symbol startSymbol;
    Symbol endSymbol;
    std::set<Symbol> allSymbols;

    std::unordered_map<Symbol, SymbolsSet> mFollowCache, mFirstCache;
};

#endif // SYNTAX_ANALYZER_HPP
