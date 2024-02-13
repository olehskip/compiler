#include "symbols.hpp"
#include "syntax_analyzer.hpp"
#include <iostream>
#include <queue>

// auto my_program = R""""(
//
// )"""";
//
void astPrinter(SymbolAst::SharedPtr symbolAst, std::string prefix)
{
    std::queue<SymbolAst::SharedPtr> q, qn;
    q.push(symbolAst);
    // std::cout << prefix << "{" << getSymbolName(symbolAst->symbolType) << "}\n";
    std::cout << "\n" << prefix;
    if (auto terminalSymbol = std::dynamic_pointer_cast<TerminalSymbolAst>(symbolAst)) {
        std::cout << "<" << getSymbolName(terminalSymbol->symbolType) << "|" << terminalSymbol->text
                  << ">";
    } else if (auto nonTerminalSymbol =
                   std::dynamic_pointer_cast<NonTerminalSymbolAst>(symbolAst)) {
        for (auto child : nonTerminalSymbol->children) {
            std::cout << getSymbolName(nonTerminalSymbol->symbolType) << "; ";
            astPrinter(child, prefix + "-");
        }
    } else {
        std::cout << "\nsomething is wrong\n";
        abort();
    }
}

int main()
{
    SyntaxAnalyzer syntaxAnalyzer(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::STMT});
    syntaxAnalyzer.addRule(NonTerminalSymbol::STMT,
                           {NonTerminalSymbol::STMT, TerminalSymbol::PLUS_OP, TerminalSymbol::ID});
    syntaxAnalyzer.addRule(NonTerminalSymbol::STMT,
                           {NonTerminalSymbol::STMT, TerminalSymbol::MINUS_OP, TerminalSymbol::ID});
    syntaxAnalyzer.addRule(NonTerminalSymbol::STMT, {TerminalSymbol::ID});
    syntaxAnalyzer.start();
    auto ret = syntaxAnalyzer.parse({
        std::make_shared<TerminalSymbolAst>(TerminalSymbol::ID, "a"),
        std::make_shared<TerminalSymbolAst>(TerminalSymbol::PLUS_OP, "+"),
        std::make_shared<TerminalSymbolAst>(TerminalSymbol::ID, "b"),
        std::make_shared<TerminalSymbolAst>(TerminalSymbol::MINUS_OP, "+"),
        std::make_shared<TerminalSymbolAst>(TerminalSymbol::ID, "c"),
        std::make_shared<TerminalSymbolAst>(TerminalSymbol::FINISH, ""),
    });
    std::cout << "finish\n";
    astPrinter(ret, "");

    return 0;
}
