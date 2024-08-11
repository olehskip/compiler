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
    ID,
    DEFINE,
    BEGIN,

    FINISH,

    TRUE_LIT,
    FALSE_LIT,
    CHARACTER,
    STRING,
    SYMBOL,
    INT,
};

enum class NonTerminalSymbol
{
    PROGRAM,
    STARTS,
    START,

    EXPRS,
    EXPR,
    BEGIN_EXPR,
    EPS, // TODO: remove it

    DATUM,
    SIMPLE_DATUM,
    COMPOUND_DATUM,

    VAR_DEF,
    LITERAL,

    PROCEDURE_CALL,
    OPERANDS,
    OPERAND,

    PROCEDURE_DEF,
    PROCEDURE_PARAMS,
    PROCEDURE_PARAM,

    DEFINITION,

    BOOLEAN,
    LIST,
    VECTOR,
    BYTEVECTOR
};

using Symbol = std::variant<TerminalSymbol, NonTerminalSymbol>;
using Symbols = std::vector<Symbol>;
using SymbolsSet = std::set<Symbol>;
using NonTerminalsSet = std::set<NonTerminalSymbol>;

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

class NonTerminalSymbolSt;
class SymbolSt
{
public:
    virtual ~SymbolSt(){};
    auto operator<=>(const SymbolSt &) const = default;

    // std::shared_ptr<NonTerminalSymbolSt> parent = nullptr;
    using SharedPtr = std::shared_ptr<SymbolSt>;
};

using SymbolsSt = std::vector<SymbolSt::SharedPtr>;

class TerminalSymbolSt : public SymbolSt
{
public:
    TerminalSymbolSt(TerminalSymbol symbolType_, std::string text_)
        : symbolType(symbolType_), text(std::move(text_))
    {
    }
    ~TerminalSymbolSt() {}

    const TerminalSymbol symbolType;
    const std::string text;

    using SharedPtr = std::shared_ptr<TerminalSymbolSt>;
};

using TerminalSymbolsSt = std::vector<TerminalSymbolSt::SharedPtr>;

class NonTerminalSymbolSt : public SymbolSt
{
public:
    NonTerminalSymbolSt(NonTerminalSymbol symbolType_) : symbolType(symbolType_), children({}) {}
    NonTerminalSymbolSt(NonTerminalSymbol symbolType_, SymbolsSt children_)
        : symbolType(symbolType_), children(children_)
    {
    }
    ~NonTerminalSymbolSt() {}

    const NonTerminalSymbol symbolType;
    SymbolsSt children;

    using SharedPtr = std::shared_ptr<NonTerminalSymbolSt>;
};

#endif // SYMBOLS_H
