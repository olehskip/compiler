#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include <variant>
#include <set>

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
	std::visit([&res](auto &&arg) {
		using T = std::decay_t<decltype(arg)>;
		res = std::is_same_v<T, TerminalSymbol>;
    }, symbol);
	return res;
}

inline bool isNonTermional(Symbol symbol)
{
	return !isTerminal(symbol);
}

inline std::string getSymbolName(Symbol symbol)
{
	std::string res;
	std::visit([&res](auto &&arg) {
        res = std::string(magic_enum::enum_name(arg));
    }, symbol);
	return res;
}

inline std::string getTerminalSymbolName(TerminalSymbol terminalSymbol)
{
	return getSymbolName(Symbol(terminalSymbol));
}

inline std::string getNonTerminalSymbolName(TerminalSymbol nonTerminalSymbol)
{
	return getSymbolName(Symbol(nonTerminalSymbol));
}


#endif // SYMBOLS_H

