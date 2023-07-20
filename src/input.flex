%{
#include <iostream>
#include "tokens.hpp"
static const char tokenDelimiter = ' ';
static const char endChar = '\n';
static void ret_token(Token token)
{
    std::cout << getTokenName(token) << tokenDelimiter;
}
%}

%%
"//".*|"/*".*"*/" { ret_token(Token::COMMENT); }

";" { ret_token(Token::SEMICOLON); }

[[:blank:]]* { ret_token(Token::BLANK); }

\n+ { ret_token(Token::NEWLINE); }

"+" { ret_token(Token::PLUS_OP); }
"-" { ret_token(Token::MINUS_OP); }
"/" { ret_token(Token::MULT_OP); }
"\*" { ret_token(Token::DIV_OP); }
"=" { ret_token(Token::ASSIGN_OP); }

"(" { ret_token(Token::OPEN_BRACKET); }
")" { ret_token(Token::CLOSED_BRACKET); }

[[:digit:]]+ { ret_token(Token::NUM_LIT); }
\".+\" { ret_token(Token::STR_LIT); }

[[:alpha:]]*[[:alpha:]|[:digit:]]+ { ret_token(Token::ID); }

. { ret_token(Token::ERROR); }
%%

int yywrap(void){return 1;}

int main(void)
{
    yylex();
    std::cout << endChar;
    return 0;
}


