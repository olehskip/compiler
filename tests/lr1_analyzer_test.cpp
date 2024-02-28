#include "parser_utils.hpp"
#include "syntax_analyzer.hpp"

#include <gtest/gtest.h>
#include <stack>
#include <string>

using namespace std;

static void cmpSts(SymbolSt::SharedPtr root1, SymbolSt::SharedPtr root2)
{
    auto terminalSymbol1 = std::dynamic_pointer_cast<TerminalSymbolSt>(root1);
    auto terminalSymbol2 = std::dynamic_pointer_cast<TerminalSymbolSt>(root2);
    auto nonTerminalSymbol1 = std::dynamic_pointer_cast<NonTerminalSymbolSt>(root1);
    auto nonTerminalSymbol2 = std::dynamic_pointer_cast<NonTerminalSymbolSt>(root2);
    ASSERT_TRUE((terminalSymbol1 && terminalSymbol2) || (!terminalSymbol1 && !terminalSymbol2));
    ASSERT_TRUE((nonTerminalSymbol1 && nonTerminalSymbol2) ||
                (!nonTerminalSymbol1 && !nonTerminalSymbol2));
    if (terminalSymbol1) {
        ASSERT_EQ(*terminalSymbol1, *terminalSymbol2);
    } else if (nonTerminalSymbol1) {
        ASSERT_EQ(*nonTerminalSymbol1, *nonTerminalSymbol2);
        ASSERT_EQ(nonTerminalSymbol1->children.size(), nonTerminalSymbol2->children.size());
        for (size_t i = 0; i < nonTerminalSymbol1->children.size(); ++i) {
            cmpSts(nonTerminalSymbol1->children[i], nonTerminalSymbol2->children[i]);
        }
    }
}

TEST(Simple, SingleRuleSingleSymbol)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "=")});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}

TEST(Simple, SingleRuleSeveralSymbols)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM,
                            {TerminalSymbol::ASSIGN_OP, TerminalSymbol::ID, TerminalSymbol::BLANK});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "="),
                  std::make_shared<TerminalSymbolSt>(TerminalSymbol::ID, "ID"),
                  std::make_shared<TerminalSymbolSt>(TerminalSymbol::BLANK, " ")});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}

TEST(Simple, SeveralRulesSingleSymbol)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {NonTerminalSymbol::PROCEDURE_CALL});
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROCEDURE_CALL, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
            NonTerminalSymbol::EXPR,
            SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
                NonTerminalSymbol::PROCEDURE_CALL,
                SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "=")})})});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}

TEST(Simple, SeveralRulesSeveralSymbols)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR,
                            {NonTerminalSymbol::PROCEDURE_CALL, TerminalSymbol::BLANK});
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROCEDURE_CALL, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    // TODO: this looks terrible, I need to find a better way to represent it
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
            NonTerminalSymbol::EXPR,
            SymbolsSt{
                std::make_shared<NonTerminalSymbolSt>(
                    NonTerminalSymbol::PROCEDURE_CALL,
                    SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "=")}),
                std::make_shared<TerminalSymbolSt>(TerminalSymbol::BLANK, "")})});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}

TEST(Recursion, RightRecursion)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR,
                            {TerminalSymbol::ASSIGN_OP, NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {TerminalSymbol::BLANK});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
            NonTerminalSymbol::EXPR,
            SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "="),
                      std::make_shared<NonTerminalSymbolSt>(
                          NonTerminalSymbol::EXPR, SymbolsSt{std::make_shared<TerminalSymbolSt>(
                                                       TerminalSymbol::BLANK, " ")})})});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}

TEST(Recursion, LeftRecursion)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR,
                            {NonTerminalSymbol::EXPR, TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {TerminalSymbol::BLANK});
    std::cout << "before parsing\n";
    syntaxAnalyzer->start();
    std::cout << "after parsing\n";
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
            NonTerminalSymbol::EXPR,
            SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
                          NonTerminalSymbol::EXPR, SymbolsSt{std::make_shared<TerminalSymbolSt>(
                                                       TerminalSymbol::BLANK, " ")}),
                      std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "=")})});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}

TEST(Recursion, MiddleRecursion)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(
        NonTerminalSymbol::EXPR,
        {TerminalSymbol::ASSIGN_OP, NonTerminalSymbol::EXPR, TerminalSymbol::BLANK});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {TerminalSymbol::ID});
    syntaxAnalyzer->start();
    // TODO: this looks terrible, I need to find a better way to represent it
    auto expectedTree = std::make_shared<NonTerminalSymbolSt>(
        NonTerminalSymbol::PROGRAM,
        SymbolsSt{std::make_shared<NonTerminalSymbolSt>(
            NonTerminalSymbol::EXPR,
            SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ASSIGN_OP, "="),
                      std::make_shared<NonTerminalSymbolSt>(
                          NonTerminalSymbol::EXPR,
                          SymbolsSt{std::make_shared<TerminalSymbolSt>(TerminalSymbol::ID, "ID")}),
                      std::make_shared<TerminalSymbolSt>(TerminalSymbol::BLANK, " ")})});
    auto parseRes = syntaxAnalyzer->parse(getLeafsSt(expectedTree));
    cmpSts(expectedTree, parseRes);
}
