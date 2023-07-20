#ifndef TOKENS_H
#define TOKENS_H

#include <unordered_map>
#include <string>
#include "magic_enum.hpp"

enum class Token
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
	ID
};

inline std::string getTokenName(Token token)
{
	return std::string(magic_enum::enum_name(token));
}

#endif // TOKENS_h

