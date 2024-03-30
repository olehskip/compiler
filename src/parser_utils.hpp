#ifndef PARSER_UTILS_HPP
#define PARSER_UTILS_HPP

#include "ast_node.hpp"
#include "symbols.hpp"

#include <functional>

// removes from the ST all the nonterminals that are not in the whitelist
AstProgram::SharedPtr convertToAst(NonTerminalSymbolSt::SharedPtr root);
// TODO: rename it
void removeBlankNewlineTerminals(TerminalSymbolsSt &terminalSymbolsSt);
bool isLexicalError(const TerminalSymbolsSt &terminalSymbolsSt);

void stVisitor(const SymbolSt::SharedPtr root,
               std::function<void(TerminalSymbolSt::SharedPtr)> terminalCallback,
               std::function<void(NonTerminalSymbolSt::SharedPtr)> nonTerminalCallback);

TerminalSymbolsSt getLeafsSt(SymbolSt::SharedPtr root);

void prettyAst(AstNode::SharedPtr astNode, std::stringstream &stream);

#endif // PARSER_UTILS_HPP
