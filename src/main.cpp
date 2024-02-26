#include "lexical_analyzer/thompson_constructor.hpp"
#include "symbols.hpp"
#include "syntax_analyzer.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>

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

static void removeBlankTerminals(TerminalSymbolsAst &terminalSymbolsAst)
{
    std::erase_if(terminalSymbolsAst, [](auto &symbol) {
        return symbol->symbolType == TerminalSymbol::BLANK ||
               symbol->symbolType == TerminalSymbol::NEWLINE;
    });
}

static void assertNoLexicalErrors(const TerminalSymbolsAst &terminalSymbolsAst)
{
    for (const auto &terminalSymbolAst : terminalSymbolsAst) {
        assert(terminalSymbolAst->symbolType != TerminalSymbol::ERROR);
    }
}

static std::string readCode(std::string filePath)
{
    std::ifstream t(filePath);
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

int main(int argc, char *argv[])
{
    assert(argc == 2);
    std::shared_ptr<ThompsonConstructor> thompsonConstructor =
        std::make_shared<ThompsonConstructor>();
    thompsonConstructor->addRule("#[tT]", TerminalSymbol::TRUE_LIT);
    thompsonConstructor->addRule("#[fF]", TerminalSymbol::FALSE_LIT);
    thompsonConstructor->addRule("\\(", TerminalSymbol::OPEN_BRACKET);
    thompsonConstructor->addRule("\\)", TerminalSymbol::CLOSED_BRACKET);
    thompsonConstructor->addRule("#\\\\" + ThompsonConstructor::allLetters,
                                 TerminalSymbol::CHARACTER);
    thompsonConstructor->addRule("\"" + LexicalAnalyzerConstructor::allLettersAndDigits + "+\"",
                                 TerminalSymbol::STRING);
    thompsonConstructor->addRule("'" + LexicalAnalyzerConstructor::allLettersAndDigits + "+",
                                 TerminalSymbol::SYMBOL);
    thompsonConstructor->addRule(LexicalAnalyzerConstructor::allLetters + "+" +
                                     LexicalAnalyzerConstructor::allLettersAndDigits,
                                 TerminalSymbol::ID);
    thompsonConstructor->addRule("\\+", TerminalSymbol::ID);
    thompsonConstructor->addRule(ThompsonConstructor::allDigits, TerminalSymbol::NUMBER);
    thompsonConstructor->addRule(" +", TerminalSymbol::BLANK);
    thompsonConstructor->addRule("\n+", TerminalSymbol::NEWLINE);
    std::cout << "lexer rules were added" << std::endl;

    LexicalAnalyzer lexicalAnalyzer(thompsonConstructor);

    SyntaxAnalyzer syntaxAnalyzer(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPRS});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::EXPRS,
        {{NonTerminalSymbol::EXPRS, NonTerminalSymbol::EXPR}, {NonTerminalSymbol::EXPR}});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::EXPR,
        {{TerminalSymbol::ID}, {NonTerminalSymbol::LITERAL}, {NonTerminalSymbol::PROCEDURE_CALL}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_CALL,
                           {TerminalSymbol::OPEN_BRACKET, NonTerminalSymbol::OPERATOR,
                            NonTerminalSymbol::OPERANDS, TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRule(NonTerminalSymbol::OPERATOR, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::OPERANDS,
        {{NonTerminalSymbol::OPERANDS, NonTerminalSymbol::OPERAND}, {NonTerminalSymbol::OPERAND}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::OPERAND, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer.addRules(NonTerminalSymbol::BOOLEAN,
                            {{TerminalSymbol::TRUE_LIT}, {TerminalSymbol::FALSE_LIT}});
    syntaxAnalyzer.addRules(NonTerminalSymbol::LITERAL, {{TerminalSymbol::NUMBER},
                                                         {NonTerminalSymbol::BOOLEAN},
                                                         {TerminalSymbol::CHARACTER},
                                                         {TerminalSymbol::STRING}});

    syntaxAnalyzer.start();
    std::cout << "syntax rules were added" << std::endl;

    const std::string code = readCode(argv[1]);
    std::cout << "code = \n" << std::quoted(code) << "\n";

    auto lexicalRet = lexicalAnalyzer.parse(code);
    lexicalRet.push_back(std::make_shared<TerminalSymbolAst>(TerminalSymbol::FINISH, ""));
    removeBlankTerminals(lexicalRet);
    assertNoLexicalErrors(lexicalRet);
    auto syntaxRet = syntaxAnalyzer.parse(lexicalRet);
    assert(syntaxRet);
    std::cout << "successfully parsed\n";
    return 0;
}
