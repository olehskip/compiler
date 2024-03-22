#include "lexical_analyzer/thompson_constructor.hpp"
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

static void prettyAst(AstNode::SharedPtr astNode, std::stringstream &stream)
{
    const auto id = std::to_string((unsigned long long)astNode.get());
    if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
        stream << "digraph G {\n";
        for (auto child : astProgram->children) {
            stream << "\t" << '"' << "[PROGRAM] " << id << '"' << " -> ";
            prettyAst(child, stream);
        }
        stream << "\n}\n";
    } else if (auto astProcedureDef = std::dynamic_pointer_cast<AstProcedureDefinition>(astNode)) {
        stream << '"' << "[PROCEDURE DEF] " << id << " " << astProcedureDef->name << '"' << "\n";
        for (auto child : astProcedureDef->params) {
            stream << "\t" << '"' << "[PROCEDURE DEF] " << id << " " << astProcedureDef->name << '"'
                   << " -> ";
            prettyAst(child, stream);
        }
        stream << "\t" << '"' << "[PROCEDURE DEF] " << id << " " << astProcedureDef->name << '"'
               << " -> ";
        prettyAst(astProcedureDef->body, stream);
    } else if (auto astProcedureCall = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
        stream << '"' << "[PROCEDURE CALL] " << id << " " << astProcedureCall->name << '"' << "\n";
        for (auto child : astProcedureCall->children) {
            stream << "\t" << '"' << "[PROCEDURE CALL] " << id << " " << astProcedureCall->name
                   << '"' << " -> ";
            prettyAst(child, stream);
        }
    } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
        stream << '"' << "[ID] " << id << " " << astId->name << '"' << "\n";
    } else if (auto astNum = std::dynamic_pointer_cast<AstNum>(astNode)) {
        stream << '"' << "[NUM] " << id << " " << astNum->num << '"' << "\n";
    }
}

static void saveAst(AstProgram::SharedPtr astNode, std::string filepath)
{
    std::stringstream stream;
    prettyAst(astNode, stream);
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

void saveSeq(SsaSeq &seq, std::string filepath)
{
    std::stringstream stream;
    seq.pretty(stream);
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

void saveNasm(std::stringstream &stream, std::string filepath)
{
    std::ofstream file(filepath);
    file << stream.rdbuf();
    file.close();
}

int main(int argc, char *argv[])
{
    assert(argc == 3);
    const std::string inputPath = argv[1], outputPath = argv[2];
    std::cout << "Input file path = " << inputPath << "\n";
    std::cout << "Output folder path = " << outputPath << "\n";
    const bool outputDirectoryWasCreated = std::filesystem::create_directories(outputPath);
    assert(outputDirectoryWasCreated);

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
    thompsonConstructor->addRule("define", TerminalSymbol::DEFINE);
    thompsonConstructor->addRule(LexicalAnalyzerConstructor::allLetters + "+" +
                                     LexicalAnalyzerConstructor::allLettersAndDigits + "*",
                                 TerminalSymbol::ID);
    thompsonConstructor->addRule("\\+", TerminalSymbol::ID);
    thompsonConstructor->addRule(ThompsonConstructor::allDigits, TerminalSymbol::NUMBER);
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
                            {{NonTerminalSymbol::PROCEDURE_DEFINITION}, {NonTerminalSymbol::EXPR}});

    syntaxAnalyzer.addRules(
        NonTerminalSymbol::EXPRS,
        {{NonTerminalSymbol::EXPRS, NonTerminalSymbol::EXPR}, {NonTerminalSymbol::EXPR}});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::EXPR,
        {{TerminalSymbol::ID}, {NonTerminalSymbol::LITERAL}, {NonTerminalSymbol::PROCEDURE_CALL}});

    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_CALL,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::ID,
                            NonTerminalSymbol::OPERANDS, TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::OPERANDS,
        {{NonTerminalSymbol::OPERANDS, NonTerminalSymbol::OPERAND}, {NonTerminalSymbol::OPERAND}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::OPERAND, {NonTerminalSymbol::EXPR});

    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_DEFINITION,
                           {TerminalSymbol::OPEN_BRACKET, TerminalSymbol::DEFINE,
                            TerminalSymbol::OPEN_BRACKET, TerminalSymbol::ID,
                            NonTerminalSymbol::PROCEDURE_PARAMS, TerminalSymbol::CLOSED_BRACKET,
                            NonTerminalSymbol::EXPR, TerminalSymbol::CLOSED_BRACKET});
    syntaxAnalyzer.addRules(
        NonTerminalSymbol::PROCEDURE_PARAMS,
        {{NonTerminalSymbol::PROCEDURE_PARAMS, NonTerminalSymbol::PROCEDURE_PARAM},
         {NonTerminalSymbol::PROCEDURE_PARAM}});
    syntaxAnalyzer.addRule(NonTerminalSymbol::PROCEDURE_PARAM, {TerminalSymbol::ID});

    syntaxAnalyzer.addRules(NonTerminalSymbol::BOOLEAN,
                            {{TerminalSymbol::TRUE_LIT}, {TerminalSymbol::FALSE_LIT}});
    syntaxAnalyzer.addRules(NonTerminalSymbol::LITERAL, {{TerminalSymbol::NUMBER},
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
    assert(!isLexicalError(lexicalRet));
    std::cout << "Code was successfully parsed by lexical analyzer\n";
    auto syntaxRet = syntaxAnalyzer.parse(lexicalRet);
    assert(syntaxRet);
    std::cout << "Code was successfully parsed by syntax analyzer\n";
    std::cout << "Code was successfully fully parsed\n";
    saveSt(syntaxRet, outputPath + "/st.txt");
    std::cout << "ST was saved\n";

    auto ast = convertToAst(syntaxRet);
    saveAst(ast, outputPath + "/ast.txt");
    std::cout << "AST was saved\n";

    auto ssaSeq = generateSsaSeq(ast);
    saveSeq(ssaSeq, outputPath + "/ssa.txt");
    std::cout << "SSA sequence was saved\n";

    std::stringstream nasm;
    generateX64Asm(ssaSeq, nasm);
    saveNasm(nasm, outputPath + "/output.nasm");
    std::cout << "Nasm code was saved\n";

    return 0;
}
