#include "lexical_analyzer/thompson_constructor.hpp"
#include "log.hpp"
#include "parser_utils.hpp"
#include "symbols.hpp"
#include "syntax_analyzer.hpp"
#include "x64_nasm_generator.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

static std::string readCode(std::string filePath)
{
    std::ifstream t(filePath);
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

static void saveSt(NonTerminalSymbolSt::SharedPtr programSt, std::string filepath)
{
    std::stringstream stream;
    prettySt(programSt, stream);
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

static void saveAst(AstProgram::SharedPtr astNode, std::string filepath)
{
    std::stringstream stream;
    prettyAst(astNode, stream);
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

static void saveIR(SimpleBlock::SharedPtr mainBlock, std::string filepath)
{
    ASSERT(mainBlock);
    ASSERT(!filepath.empty());
    std::stringstream stream;
    mainBlock->pretty(stream);
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

static void saveNasm(std::stringstream &stream, std::string filepath)
{
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

int main(int argc, char *argv[])
{
    ASSERT(argc == 3);
    const std::string inputPath = argv[1], outputPath = argv[2];
    std::cout << "Input file path = " << inputPath << "\n";
    std::cout << "Output folder path = " << outputPath << "\n";
    if (std::filesystem::remove_all(outputPath)) {
        std::cout << "Deleted output folder\n";
    }

    const bool outputDirectoryWasCreated = std::filesystem::create_directories(outputPath);
    ASSERT(outputDirectoryWasCreated);

    std::shared_ptr<ThompsonConstructor> thompsonConstructor =
        std::make_shared<ThompsonConstructor>();
    thompsonConstructor->addRule(";" + thompsonConstructor->everything + "*\n",
                                 TerminalSymbol::COMMENT);
    thompsonConstructor->addRule("#[tT]", TerminalSymbol::TRUE_LIT);
    thompsonConstructor->addRule("#[fF]", TerminalSymbol::FALSE_LIT);
    thompsonConstructor->addRule("\\(", TerminalSymbol::OPEN_BRACKET);
    thompsonConstructor->addRule("\\)", TerminalSymbol::CLOSED_BRACKET);
    thompsonConstructor->addRule("#\\\\" + ThompsonConstructor::allLetters,
                                 TerminalSymbol::CHARACTER);
    thompsonConstructor->addRule("\"" + LexicalAnalyzerConstructor::everything + "+\"",
                                 TerminalSymbol::STRING);
    thompsonConstructor->addRule("'" + LexicalAnalyzerConstructor::allLettersDigits + "+",
                                 TerminalSymbol::SYMBOL);
    thompsonConstructor->addRule("define", TerminalSymbol::DEFINE);
    thompsonConstructor->addRule("begin", TerminalSymbol::BEGIN);
    thompsonConstructor->addRule("if", TerminalSymbol::IF);
    thompsonConstructor->addRule(LexicalAnalyzerConstructor::allLetters + "+" +
                                     LexicalAnalyzerConstructor::allLettersDigits + "*",
                                 TerminalSymbol::ID);
    thompsonConstructor->addRule("[\\+><(>=)(<=)]", TerminalSymbol::ID);
    thompsonConstructor->addRule(ThompsonConstructor::allDigits + "+", TerminalSymbol::INT);
    thompsonConstructor->addRule(" +", TerminalSymbol::BLANK);
    thompsonConstructor->addRule("\n+", TerminalSymbol::NEWLINE);
    std::cout << "Lexer rules were added\n";

    LexicalAnalyzer lexicalAnalyzer(thompsonConstructor);

    SyntaxAnalyzer syntaxAnalyzer(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::STARTS});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::STARTS,
        {{NonTerminalSymbol::STARTS, NonTerminalSymbol::START}, {NonTerminalSymbol::START}});
    syntaxAnalyzer.addRules(NonTerminalSymbol::START,
                            {{NonTerminalSymbol::PROCEDURE_DEF}, {NonTerminalSymbol::EXPR}});

    syntaxAnalyzer.addRules(
        NonTerminalSymbol::EXPRS,
        {{NonTerminalSymbol::EXPRS, NonTerminalSymbol::EXPR}, {NonTerminalSymbol::EXPR}});
    syntaxAnalyzer.addRules(NonTerminalSymbol::EXPR, {{NonTerminalSymbol::BEGIN_EXPR},
                                                      {NonTerminalSymbol::VAR_DEF},
                                                      {TerminalSymbol::ID},
                                                      {NonTerminalSymbol::LITERAL},
                                                      {NonTerminalSymbol::PROCEDURE_CALL},
                                                      {NonTerminalSymbol::COND_IF}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::BEGIN_EXPR,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::BEGIN,
                            NonTerminalSymbol::EXPRS, TerminalSymbol::CLOSED_BRACKET});

    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_DEF,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::DEFINE,
                            TerminalSymbol::OPEN_BRACKET, TerminalSymbol::ID,
                            NonTerminalSymbol::PROCEDURE_PARAMS, TerminalSymbol::CLOSED_BRACKET,
                            NonTerminalSymbol::EXPR, TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_DEF,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::DEFINE,
                            TerminalSymbol::OPEN_BRACKET, TerminalSymbol::ID,
                            TerminalSymbol::CLOSED_BRACKET, NonTerminalSymbol::EXPR,
                            TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::PROCEDURE_PARAMS,
        {{NonTerminalSymbol::PROCEDURE_PARAMS, NonTerminalSymbol::PROCEDURE_PARAM},
         {NonTerminalSymbol::PROCEDURE_PARAM}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_PARAM, {TerminalSymbol::ID});
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_CALL,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::ID,
                            NonTerminalSymbol::OPERANDS, TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRule(
        NonTerminalSymbol::PROCEDURE_CALL,
        {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::ID, TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::OPERANDS,
        {{NonTerminalSymbol::OPERANDS, NonTerminalSymbol::OPERAND}, {NonTerminalSymbol::OPERAND}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::OPERAND, {NonTerminalSymbol::EXPR});

    syntaxAnalyzer.addRules(
        NonTerminalSymbol::COND_IF,
        {{TerminalSymbol::OPEN_BRACKET, TerminalSymbol::IF, NonTerminalSymbol::COND_IF_TEST_EXPR,
          NonTerminalSymbol::COND_IF_THEN_EXPR, NonTerminalSymbol::COND_IF_ELSE_EXPR,
          TerminalSymbol::CLOSED_BRACKET},
         {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::IF, NonTerminalSymbol::COND_IF_TEST_EXPR,
          NonTerminalSymbol::COND_IF_THEN_EXPR, TerminalSymbol::CLOSED_BRACKET}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::COND_IF_TEST_EXPR, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer.addRule(NonTerminalSymbol::COND_IF_THEN_EXPR, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer.addRule(NonTerminalSymbol::COND_IF_ELSE_EXPR, {NonTerminalSymbol::EXPR});

    syntaxAnalyzer.addRule(NonTerminalSymbol::VAR_DEF,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::DEFINE,
                            TerminalSymbol::ID, NonTerminalSymbol::EXPR,
                            TerminalSymbol::CLOSED_BRACKET});

    syntaxAnalyzer.addRules(NonTerminalSymbol::BOOLEAN,
                            {{TerminalSymbol::TRUE_LIT}, {TerminalSymbol::FALSE_LIT}});
    syntaxAnalyzer.addRules(NonTerminalSymbol::LITERAL, {{TerminalSymbol::INT},
                                                         {NonTerminalSymbol::BOOLEAN},
                                                         {TerminalSymbol::CHARACTER},
                                                         {TerminalSymbol::STRING}});

    syntaxAnalyzer.start();
    std::cout << "Syntax rules were added\n";

    const std::string code = readCode(inputPath);
    std::cout << "Code was read\n";

    auto lexicalRet = lexicalAnalyzer.parse(code);
    lexicalRet.push_back(std::make_shared<TerminalSymbolSt>(TerminalSymbol::FINISH, ""));
    removeBlankNewlineTerminals(lexicalRet);
    ASSERT_MSG(!isLexicalError(lexicalRet), "Lexical analysis failed");
    std::cout << "Code was successfully parsed by lexical analyzer\n";
    auto syntaxRet = syntaxAnalyzer.parse(lexicalRet);
    ASSERT_MSG(syntaxRet, "Syntax analysis failed");
    std::cout << "Code was successfully parsed by syntax analyzer\n";
    std::cout << "Code was successfully fully parsed\n";
    saveSt(syntaxRet, outputPath + "/st.txt");
    std::cout << "ST was saved\n";

    auto ast = convertToAst(syntaxRet);
    std::cout << "AST was created\n";
    saveAst(ast, outputPath + "/ast.txt");
    std::cout << "AST was saved\n";

    auto ssaSeq = generateIR(ast);
    saveIR(ssaSeq, outputPath + "/ssa.txt");
    std::cout << "SSA sequence was saved\n";

    std::stringstream nasm;
    generateX64Asm(ssaSeq, nasm);
    saveNasm(nasm, outputPath + "/output.nasm");
    std::cout << "Nasm code was saved\n";
    return 0;
}
