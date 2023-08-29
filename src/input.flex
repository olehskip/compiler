%{
#include <iostream>
#include "symbols.hpp"
static const char tokenDelimiter = ' ';
static const char endChar = '\n';
static void ret_token(Token token)
{
    std::cout << getTokenName(token) << tokenDelimiter;
}
%}

%%
"//".*|"/*".*"*/" { ret_token(TerminalSymbol::COMMENT); }

";" { ret_token(TerminalSymbol::SEMICOLON); }

[[:blank:]]* { ret_token(TerminalSymbol::BLANK); }

\n+ { ret_token(TerminalSymbol::NEWLINE); }

"+" { ret_token(TerminalSymbol::PLUS_OP); }
"-" { ret_token(TerminalSymbol::MINUS_OP); }
"/" { ret_token(TerminalSymbol::MULT_OP); }
"\*" { ret_token(TerminalSymbol::DIV_OP); }
"=" { ret_token(TerminalSymbol::ASSIGN_OP); }

"(" { ret_token(TerminalSymbol::OPEN_BRACKET); }
")" { ret_token(TerminalSymbol::CLOSED_BRACKET); }

[[:digit:]]+ { ret_token(TerminalSymbol::NUM_LIT); }
\".+\" { ret_token(TerminalSymbol::STR_LIT); }

[[:alpha:]]*[[:alpha:]|[:digit:]]+ { ret_token(TerminalSymbol::ID); }

. { ret_token(TerminalSymbol::ERROR); }
%%

int yywrap(void){return 1;}

int main(void)
{
    yylex();
    std::cout << endChar;
    return 0;
}


