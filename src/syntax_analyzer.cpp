#include "syntax_analyzer.hpp"
#include "log.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stack>

Item::Item(Rule rule, size_t tPos, Symbol tLookaheadSymbol)
    : Rule(std::move(rule)), pos(tPos), lookaheadSymbol(tLookaheadSymbol)
{
    ASSERT(pos <= rule.rhs.size());
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
    ASSERT(decisionTable.find(lookaheadSymbol) == decisionTable.end());
    decisionTable[lookaheadSymbol] = decision;
}

State::SharedPtr State::getGotoState(Symbol lookaheadSymbol)
{
    const auto it = gotoTable.find(lookaheadSymbol);
    if (it == gotoTable.end()) {
        return nullptr;
    }

    return it->second;
}

void State::addGotoState(Symbol lookaheadSymbol, State::SharedPtr state)
{
    ASSERT(gotoTable.find(lookaheadSymbol) == gotoTable.end());
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

void SyntaxAnalyzer::addRules(NonTerminalSymbol lhs, std::vector<Symbols> rhses)
{
    for (auto &rhs : rhses) {
        addRule(lhs, rhs);
    }
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

NonTerminalSymbolSt::SharedPtr SyntaxAnalyzer::parse(TerminalSymbolsSt symbols)
{
    std::stack<std::pair<State::SharedPtr, SymbolSt::SharedPtr>> statesStack;
    statesStack.push({startState, nullptr});
    size_t currSymbolPos = 0;

    while (true) {
        assert(statesStack.size() > 0);
        assert(currSymbolPos < symbols.size());
        State::SharedPtr &currState = statesStack.top().first;
        auto decisionOpt = currState->getDecision(symbols[currSymbolPos]->symbolType);
        if (!decisionOpt) {
            std::cerr << "Error during parsing. Can't find what to do. currSymbolPos = "
                      << currSymbolPos << "\n";
            return nullptr;
        }
        auto decision = *decisionOpt;

        if (auto reduceDecision = tryConvertDecision<ReduceDecision>(decision)) {
            assert(statesStack.size() > reduceDecision->rhs.size());
            SymbolsSt symbolsChildren;
            for (size_t i = 0; i < reduceDecision->rhs.size(); ++i) {
                assert(statesStack.top().second);
                symbolsChildren.push_back(statesStack.top().second);
                statesStack.pop();
            }

            std::reverse(symbolsChildren.begin(), symbolsChildren.end());
            NonTerminalSymbolSt::SharedPtr newSymbolAst =
                std::make_shared<NonTerminalSymbolSt>(reduceDecision->lhs, symbolsChildren);
            // for (auto children: symbolsChildren) {
            //     children->parent = newSymbolAst;
            // }
            if (reduceDecision->lhs == startSymbol) {
                assert(statesStack.size() == 1);
                assert(currSymbolPos == symbols.size() - 1);
                return newSymbolAst;
            }
            assert(statesStack.size() > 0);
            auto nextState = statesStack.top().first->getGotoState(reduceDecision->lhs);
            statesStack.push({nextState, newSymbolAst});
        } else if (auto shiftDecision = tryConvertDecision<ShiftDecision>(decision)) {
            TerminalSymbolSt::SharedPtr newSymbolAst = std::make_shared<TerminalSymbolSt>(
                symbols[currSymbolPos]->symbolType, symbols[currSymbolPos]->text);
            statesStack.push({shiftDecision->state, newSymbolAst});
            currSymbolPos++;
        } else {
            // this should never happen if we process all decision types
            std::cerr << "Error during parsing. A decision was not proccessed. currSymbolPos = "
                      << currSymbolPos << "\n";
            return nullptr;
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
        if (Symbol(rule.lhs) != symbol) {
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

    if (symbol == Symbol(startSymbol)) {
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
    ItemsSet resSet;
    std::stack<Item> toCheck;
    for (const auto &item : itemsSet) {
        toCheck.push(item);
    }

    while (!toCheck.empty()) {
        const auto item = toCheck.top();
        toCheck.pop();
        if (item.pos == item.rhs.size()) {
            continue;
        }
        ASSERT(item.pos < item.rhs.size());
        const auto &itemCurrSymbol = item.rhs[item.pos];

        Symbols itemTail;
        itemTail.insert(itemTail.end(), std::next(item.rhs.begin(), item.pos + 1), item.rhs.end());
        itemTail.push_back(item.lookaheadSymbol);
        const auto lookaheadSymbols = first(itemTail);

        for (const auto &rule : allRulesSet) {
            if (Symbol(rule.lhs) != itemCurrSymbol) {
                continue;
            }

            for (const auto &lookaheadSymbol : lookaheadSymbols) {
                const auto newItem = Item(rule, 0, lookaheadSymbol);
                const bool wasInserted = resSet.insert(newItem).second;
                if (wasInserted) {
                    toCheck.push(newItem);
                }
            }
        }
    }
    for (const auto &item : itemsSet) {
        resSet.insert(item);
    }

    return resSet;
}

ItemsSet SyntaxAnalyzer::gotoItems(const ItemsSet &itemsSet, Symbol symbol)
{
    ItemsSet resSet;

    for (const auto &item : itemsSet) {
        if (item.pos != item.rhs.size()) {
            const auto itemCurrSymbol = item.rhs[item.pos];
            if (itemCurrSymbol == symbol) {
                resSet.insert(Item(item, item.pos + 1));
            }
        }
    }

    return closure(resSet);
}

static void reportConflict(const Decision &decision, Symbol lookaheadSym)
{
    LOG_FATAL << "Conflict: can't add decision with type = " << variantTypeToString(decision)
              << ", because decision with type = " << variantTypeToString(decision)
              << "; lookaheadSymbol = " << getSymbolName(lookaheadSym);
}

void SyntaxAnalyzer::fillStateTables(const State::SharedPtr state)
{
    // add reductions
    for (auto &item : state->itemsSet) {
        if (item.pos == item.rhs.size()) {
            if (const auto &existingDecision = state->getDecision(item.lookaheadSymbol)) {
                reportConflict(*existingDecision, item.lookaheadSymbol);
            }

            state->addDecision(item.lookaheadSymbol, ReduceDecision{item.lhs, item.rhs});
        }
    }

    // add shifts
    for (auto symbol : allSymbols) {
        ItemsSet destStateItems = gotoItems(state->itemsSet, symbol);
        if (destStateItems.empty()) {
            continue;
        }
        if (const auto &existingDecision = state->getDecision(symbol)) {
            reportConflict(*existingDecision, symbol);
        }
        auto destState = std::make_shared<State>(destStateItems);
        /*
         * Consider the case with left recursion:
         * E->EA; E->A;
         * At some point we will have a state:
         * E->E*A|(smth), E->A*|(smth)
         * The algorithm will try to add a shift decision, so the state becomes:
         * E->E*A|(smth), E->A*|(smth), E->*EA|(smth), E->*A|(smth)
         * At some point the algorithm will try to add a decision for symbol A, so the next state:
         * E->EA*|(smth), E->E*A|(smth), E->A*|(smth)
         * And here the analyzer will do the closure operation for E and create a similar state to
         * the previous one(when eventually creting this state again)
         * hence creating states infinitely.
         *
         * While the example may not be 100% accurate, it proves that the algorithm will create
         * infinite number of states, so let's check whether we already have the same state in the
         * analyzer. This can be optimized, but it works ok for now.
         */
        auto existingStateIt =
            std::find_if(allStates.begin(), allStates.end(), [&destState](const auto &toCmp) {
                return toCmp->itemsSet == destState->itemsSet;
            });
        const bool doesStateExist = existingStateIt != allStates.end();
        if (doesStateExist) {
            destState = *existingStateIt;
        }

        state->addDecision(symbol, ShiftDecision{destState});
        state->addGotoState(symbol, destState);
        if (!doesStateExist) {
            allStates.emplace_back(destState);
            fillStateTables(destState);
        }
    }
}

void prettySt(SymbolSt::SharedPtr stNode, std::stringstream &stream)
{
    const auto id = std::to_string((unsigned long long)stNode.get());
    if (auto nonTerminalSymbolSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(stNode)) {
        const auto symbolName = getSymbolName(nonTerminalSymbolSt->symbolType);
        if (nonTerminalSymbolSt->symbolType == NonTerminalSymbol::PROGRAM) {
            stream << "digraph G {\n";
        } else {
            stream << '"' << symbolName << " " << id << '"' << "\n";
        }
        for (auto child : nonTerminalSymbolSt->children) {
            stream << "\t" << '"' << symbolName << " " << id << '"' << " -> ";
            prettySt(child, stream);
        }
        if (nonTerminalSymbolSt->symbolType == NonTerminalSymbol::PROGRAM) {
            stream << "}\n";
        }
    } else if (auto terminalSymbolSt = std::dynamic_pointer_cast<TerminalSymbolSt>(stNode)) {
        const auto symbolName = getSymbolName(terminalSymbolSt->symbolType);
        const auto text =
            terminalSymbolSt->symbolType == TerminalSymbol::STRING
                ? "\\\"" + terminalSymbolSt->text.substr(1, terminalSymbolSt->text.size() - 2) +
                      "\\\""
                : terminalSymbolSt->text;
        stream << "\t" << '"' << symbolName << " " << text << " " << id << "\"\n";
    } else {
        SHOULD_NOT_HAPPEN;
    }
}
