#include "lexical_analyzer/thompson_constructor.hpp"
#include "parser_utils.hpp"
#include "symbols.hpp"
#include "syntax_analyzer.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

static std::string readCode(std::string filePath)
{
    std::ifstream t(filePath);
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

void printAst(AstNode::SharedPtr astNode)
{
    if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
        std::cout << '"' << "[PROGRAM] " << astProgram.get() << '"' << "\n";
        for (auto child : astProgram->children) {
            std::cout << '"' << "[PROGRAM] " << astProgram.get() << '"' << " -> ";
            printAst(child);
        }
    } else if (auto astProcedure = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
        std::cout << '"' << "[PROCEDURE] " << astNode.get() << " " << astProcedure->name << '"'
                  << "\n";
        for (auto child : astProcedure->children) {
            std::cout << '"' << "[PROCEDURE] " << astNode.get() << " " << astProcedure->name << '"'
                      << " -> ";
            printAst(child);
        }
    } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
        std::cout << '"' << "[ID] " << astNode.get() << " " << astId->name << '"' << "\n";
    } else if (auto astNum = std::dynamic_pointer_cast<AstNum>(astNode)) {
        std::cout << '"' << "[NUM] " << astNode.get() << " " << astNum->num << '"' << "\n";
    }
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
    syntaxAnalyzer.addRule(NonTerminalSymbol::OPERATOR, {TerminalSymbol::ID});
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
    lexicalRet.push_back(std::make_shared<TerminalSymbolSt>(TerminalSymbol::FINISH, ""));
    removeBlankNewlineTerminals(lexicalRet);
    assert(!isLexicalError(lexicalRet));
    auto syntaxRet = syntaxAnalyzer.parse(lexicalRet);
    assert(syntaxRet);
    std::cout << "successfully parsed\n";
    auto ast = convertToAst(syntaxRet);
    printAst(ast);

    return 0;
}
