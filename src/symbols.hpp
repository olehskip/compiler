#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <memory>
#include <set>
#include <string>
#include <variant>

#include "magic_enum.hpp"

enum class TerminalSymbol
{
    ERROR,
    COMMENT,
    SEMICOLON,
    BLANK, // space or tab
    NEWLINE,
    PLUS_OP,
    MINUS_OP,
    MULT_OP,
    DIV_OP,
    ASSIGN_OP,
    OPEN_BRACKET,
    CLOSED_BRACKET,
    NUM_LIT,
    STR_LIT,
    ID,

    FINISH,
};

enum class NonTerminalSymbol
{
    PROGRAM,
    STMT,
    EXPR,
    EPS, // TODO: remove it
    START,
    A,
    B
};

using Symbol = std::variant<TerminalSymbol, NonTerminalSymbol>;
using Symbols = std::vector<Symbol>;
using SymbolsSet = std::set<Symbol>;

inline bool isTerminal(Symbol symbol)
{
    bool res;
    std::visit(
        [&res](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            res = std::is_same_v<T, TerminalSymbol>;
        },
        symbol);
    return res;
}

inline bool isNonTermional(Symbol symbol)
{
    return !isTerminal(symbol);
}

inline std::string getSymbolName(Symbol symbol)
{
    std::string res;
    std::visit([&res](auto &&arg) { res = std::string(magic_enum::enum_name(arg)); }, symbol);
    return res;
}

struct SymbolAst
{
    using SharedPtr = std::shared_ptr<SymbolAst>;
};

using SymbolsAst = std::vector<SymbolAst::SharedPtr>;

class TerminalSymbolAst : public SymbolAst
{
public:
    TerminalSymbolAst(TerminalSymbol symbolType_, std::string text_)
        : symbolType(symbolType_), text(std::move(text_))
    {
    }

    const TerminalSymbol symbolType;
    const std::string text;

    using SharedPtr = std::shared_ptr<TerminalSymbolAst>;
};

using TerminalSymbolsAst = std::vector<TerminalSymbolAst::SharedPtr>;

class NonTerminalSymbolAst : public SymbolAst
{
public:
    NonTerminalSymbolAst(NonTerminalSymbol symbolType_) : symbolType(symbolType_) {}
    const NonTerminalSymbol symbolType;
    SymbolsAst children;
};

#endif // SYMBOLS_H
