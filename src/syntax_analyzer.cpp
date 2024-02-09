#include "syntax_analyzer.hpp"

#include <cassert>
#include <iostream>
#include <set>
#include <stack>

#define EPS Symbol(NonTerminalSymbol::EPS) // TODO: remove it

Item::Item(Rule rule, size_t tPos, Symbol tLookaheadSymbol)
    : Rule(std::move(rule)), pos(tPos), lookaheadSymbol(tLookaheadSymbol)
{
    assert(pos <= rule.rhs.size());
}

Item::Item(Item item, size_t tPos) : pos(tPos), lookaheadSymbol(item.lookaheadSymbol)
{
    lhs = item.lhs;
    rhs = item.rhs;
}

State::State(ItemsSet tItemsSet) : itemsSet(tItemsSet) {}

std::optional<Decision> State::getDecision(Symbol lookaheadSymbol)
{
    const auto it = decisionTable.find(lookaheadSymbol);
    if (it == decisionTable.end()) {
        return std::nullopt;
    }

    return it->second;
}

void State::addDecision(Symbol lookaheadSymbol, Decision decision)
{
    assert(decisionTable.find(lookaheadSymbol) == decisionTable.end());
    decisionTable[lookaheadSymbol] = decision;
}

StateShared State::getGotoState(Symbol lookaheadSymbol)
{
    const auto it = gotoTable.find(lookaheadSymbol);
    if (it == gotoTable.end()) {
        return nullptr;
    }

    return it->second;
}

void State::addGotoState(Symbol lookaheadSymbol, StateShared state)
{
    assert(gotoTable.find(lookaheadSymbol) == gotoTable.end());
    gotoTable[lookaheadSymbol] = state;
}

SyntaxAnalyzer::SyntaxAnalyzer(NonTerminalSymbol tStartSymbol, TerminalSymbol tEndSymbol)
    : startSymbol(tStartSymbol), endSymbol(tEndSymbol)
{
    //
}

void SyntaxAnalyzer::addRule(NonTerminalSymbol lhs, Symbols rhs)
{
    allSymbols.merge(SymbolsSet(rhs.begin(), rhs.end()));
    allRulesSet.insert(Rule{lhs, rhs});
}

void SyntaxAnalyzer::start()
{
    ItemsSet startItemsSet;
    for (const auto &rule : allRulesSet) {
        if (rule.lhs == startSymbol) {
            startItemsSet.insert(Item(rule, 0, endSymbol));
        }
    }
    startItemsSet = closure(startItemsSet);
    allItemsSet = startItemsSet;

    startState = std::make_shared<State>(startItemsSet);
    allStates.emplace_back(startState);
    fillStateTables(startState);
}

void SyntaxAnalyzer::parse(Symbols symbols)
{
    std::stack<StateShared> statesStack;
    statesStack.push(startState);
    size_t currSymbolPos = 0;

    while (true) {
        assert(statesStack.size() > 0);
        assert(currSymbolPos < symbols.size());
        StateShared &currState = statesStack.top();
        auto decisionOpt = currState->getDecision(symbols[currSymbolPos]);
        if (!decisionOpt) {
            std::cerr << "Error during parsing. Can't find what to do. currSymbolPos = "
                      << currSymbolPos << "\n";
            abort();
        }
        auto decision = *decisionOpt;

        if (auto reduceDecision = tryConvertDecision<ReduceDecision>(decision)) {
            assert(statesStack.size() > reduceDecision->rhs.size());
            for (size_t i = 0; i < reduceDecision->rhs.size(); ++i) {
                statesStack.pop();
            }

            StateShared nextState = statesStack.top()->getGotoState(reduceDecision->lhs);
            statesStack.push(nextState);
        } else if (auto shiftDecision = tryConvertDecision<ShiftDecision>(decision)) {
            currSymbolPos++;
            statesStack.push(shiftDecision->state);
        } else if (auto acceptDecision = tryConvertDecision<AcceptDecision>(decision)) {
            if (currSymbolPos + 1 == symbols.size()) {
                std::cout << "Parsed\n";
            } else {
                std::cerr << "Error during parsing. Found accept, but didn't read all symbols. "
                             "currSymbolPos = "
                          << currSymbolPos << "\n";
            }
            break;
        } else {
            // this should never happen if we process all decision types
            std::cerr << "Error during parsing. A decision was not proccessed. currSymbolPos = "
                      << currSymbolPos << "\n";
            abort();
        }
    }
}

SymbolsSet SyntaxAnalyzer::first(Symbols symbols)
{
    SymbolsSet res;

    for (auto symbol : symbols) {
        auto currSymbolFirstSet = first(symbol);
        res.merge(currSymbolFirstSet);
        if (currSymbolFirstSet.find(EPS) == currSymbolFirstSet.end()) {
            break;
        }
    }

    return res;
}

SymbolsSet SyntaxAnalyzer::first(Symbol symbol)
{
    if (isTerminal(symbol)) {
        return {symbol};
    }

    if (auto symbolsIt = mFirstCache.find(symbol); symbolsIt != mFirstCache.end()) {
        return symbolsIt->second;
    }

    mFirstCache[symbol] = {}; // to prevent infinite recursion

    SymbolsSet res;

    for (const auto &rule : allRulesSet) {
        if (rule.lhs != symbol) {
            continue;
        }
        for (auto rhsSymbol : rule.rhs) {
            auto currSymbolFirstSet = first(rhsSymbol);
            res.merge(currSymbolFirstSet);
            if (currSymbolFirstSet.find(EPS) == currSymbolFirstSet.end()) {
                break;
            }
        }
    }

    mFirstCache[symbol] = res;

    return res;
}

SymbolsSet SyntaxAnalyzer::follow(Symbol symbol)
{
    if (auto symbolsIt = mFollowCache.find(symbol); symbolsIt != mFollowCache.end()) {
        return symbolsIt->second;
    }

    mFollowCache[symbol] = {}; // to prevent infinite recursion
    SymbolsSet res;

    if (symbol == startSymbol) {
        res = {endSymbol};
    } else {
        for (const auto &rule : allRulesSet) {
            for (size_t i = 0; i < rule.rhs.size(); ++i) {
                if (rule.rhs[i] != symbol) {
                    continue;
                }

                if (i == rule.rhs.size() - 1) {
                    res.merge(follow(rule.lhs));
                } else {
                    auto currFirst = first(rule.rhs[i + 1]);
                    res.merge(currFirst);
                    if (currFirst.find(EPS) != currFirst.end()) {
                        res.merge(follow(rule.lhs));
                    } else {
                        break;
                    }
                }
            }
        }
    }

    res.erase(EPS);
    mFollowCache[symbol] = res;
    return res;
}

ItemsSet SyntaxAnalyzer::closure(const ItemsSet &itemsSet)
{
    ItemsSet resSet = {itemsSet.begin(), itemsSet.end()};

    for (const auto &item : resSet) {
        if (item.pos >= item.rhs.size()) {
            continue;
        }
        assert(item.pos <= item.rhs.size());
        const auto &itemCurrSymbol = item.rhs[item.pos];

        Symbols itemTail;
        itemTail.insert(itemTail.end(), std::next(item.rhs.begin(), item.pos + 1), item.rhs.end());
        itemTail.push_back(item.lookaheadSymbol);
        const auto lookaheadSymbols = first(itemTail);

        for (const auto &rule : allRulesSet) {
            if (rule.lhs != itemCurrSymbol) {
                continue;
            }

            for (const auto &lookaheadSymbol : lookaheadSymbols) {
                resSet.insert(Item(rule, 0, lookaheadSymbol));
            }
        }
    }

    return resSet;
}

ItemsSet SyntaxAnalyzer::gotoItems(const ItemsSet &itemsSet, Symbol symbol)
{
    ItemsSet resSet;

    for (const auto &item : itemsSet) {
        const auto itemCurrSymbol = item.rhs[item.pos];
        if (item.pos != item.rhs.size() && itemCurrSymbol == symbol) {
            resSet.insert(Item(item, item.pos + 1));
        }
    }

    return closure(resSet);
}

void SyntaxAnalyzer::fillStateTables(const StateShared state)
{
    std::cout << "fill\n";
    // add reductions
    for (auto &item : state->itemsSet) {
        if (item.pos == item.rhs.size()) {
            if (state->getDecision(item.lookaheadSymbol)) {
                std::cerr << "Conflict. Can't add reduction.\n";
                abort();
            }

            if (item.lhs == startSymbol) {
                // TODO: add check whether lookaheadSymbol is endSymbol
                state->addDecision(item.lookaheadSymbol, AcceptDecision());
            } else {
                state->addDecision(item.lookaheadSymbol, ReduceDecision{item.lhs, item.rhs});
            }
        }
    }

    // add shifts
    for (auto symbol : allSymbols) {
        ItemsSet destStateItems = gotoItems({state->itemsSet}, symbol);
        if (destStateItems.empty()) {
            continue;
        }
        if (state->getDecision(symbol)) {
            std::cerr << "Conflict. Can't add shift.\n";
            abort();
        }
        auto destState = std::make_shared<State>(destStateItems);
        state->addDecision(symbol, ShiftDecision{destState});
        state->addGotoState(symbol, destState);
        allStates.emplace_back(destState);
        fillStateTables(destState);
    }
}
