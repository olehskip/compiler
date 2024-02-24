#include "syntax_analyzer.hpp"
#include <gtest/gtest.h>
#include <stack>
#include <string>

using namespace std;

static TerminalSymbolsAst flattenAst(SymbolAst::SharedPtr root)
{
    TerminalSymbolsAst ret;
    std::stack<SymbolAst::SharedPtr> symStack;
    symStack.push(root);

    while (!symStack.empty()) {
        auto currSym = symStack.top();
        symStack.pop();
        if (auto terminalSymbol = std::dynamic_pointer_cast<TerminalSymbolAst>(currSym)) {
            ret.push_back(terminalSymbol);
        } else if (auto nonTerminalSymbol =
                       std::dynamic_pointer_cast<NonTerminalSymbolAst>(currSym)) {
            // TODO: change to iterators
            for (int i = nonTerminalSymbol->children.size() - 1; i >= 0; --i) {
                symStack.push(nonTerminalSymbol->children[i]);
            }
        }
    }
    ret.push_back(std::make_shared<TerminalSymbolAst>(TerminalSymbol::FINISH, ""));
    return ret;
}

static void cmpAstTrees(SymbolAst::SharedPtr root1, SymbolAst::SharedPtr root2)
{
    auto terminalSymbol1 = std::dynamic_pointer_cast<TerminalSymbolAst>(root1);
    auto terminalSymbol2 = std::dynamic_pointer_cast<TerminalSymbolAst>(root2);
    auto nonTerminalSymbol1 = std::dynamic_pointer_cast<NonTerminalSymbolAst>(root1);
    auto nonTerminalSymbol2 = std::dynamic_pointer_cast<NonTerminalSymbolAst>(root2);
    ASSERT_TRUE((terminalSymbol1 && terminalSymbol2) || (!terminalSymbol1 && !terminalSymbol2));
    ASSERT_TRUE((nonTerminalSymbol1 && nonTerminalSymbol2) ||
                (!nonTerminalSymbol1 && !nonTerminalSymbol2));
    if (terminalSymbol1) {
        ASSERT_EQ(*terminalSymbol1, *terminalSymbol2);
    } else if (nonTerminalSymbol1) {
        ASSERT_EQ(*nonTerminalSymbol1, *nonTerminalSymbol2);
        ASSERT_EQ(nonTerminalSymbol1->children.size(), nonTerminalSymbol2->children.size());
        for (size_t i = 0; i < nonTerminalSymbol1->children.size(); ++i) {
            cmpAstTrees(nonTerminalSymbol1->children[i], nonTerminalSymbol2->children[i]);
        }
    }
}

TEST(Simple, SingleRuleSingleSymbol)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "=")});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}

TEST(Simple, SingleRuleSeveralSymbols)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM,
                            {TerminalSymbol::ASSIGN_OP, TerminalSymbol::ID, TerminalSymbol::BLANK});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "="),
                   std::make_shared<TerminalSymbolAst>(TerminalSymbol::ID, "ID"),
                   std::make_shared<TerminalSymbolAst>(TerminalSymbol::BLANK, " ")});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}

TEST(Simple, SeveralRulesSingleSymbol)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {NonTerminalSymbol::CALL});
    syntaxAnalyzer->addRule(NonTerminalSymbol::CALL, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
            NonTerminalSymbol::EXPR,
            SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
                NonTerminalSymbol::CALL, SymbolsAst{std::make_shared<TerminalSymbolAst>(
                                             TerminalSymbol::ASSIGN_OP, "=")})})});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}

TEST(Simple, SeveralRulesSeveralSymbols)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR,
                            {NonTerminalSymbol::CALL, TerminalSymbol::BLANK});
    syntaxAnalyzer->addRule(NonTerminalSymbol::CALL, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    // TODO: this looks terrible, I need to find a better way to represent it
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
            NonTerminalSymbol::EXPR,
            SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
                           NonTerminalSymbol::CALL, SymbolsAst{std::make_shared<TerminalSymbolAst>(
                                                        TerminalSymbol::ASSIGN_OP, "=")}),
                       std::make_shared<TerminalSymbolAst>(TerminalSymbol::BLANK, "")})});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}

TEST(Recursion, RightRecursion)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR,
                            {TerminalSymbol::ASSIGN_OP, NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
            NonTerminalSymbol::EXPR,
            SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
                           NonTerminalSymbol::EXPR, SymbolsAst{std::make_shared<TerminalSymbolAst>(
                                                        TerminalSymbol::ASSIGN_OP, "=")}),
                       std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "=")})});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}

TEST(Recursion, LeftRecursion)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR,
                            {NonTerminalSymbol::EXPR, TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
            NonTerminalSymbol::EXPR,
            SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
                           NonTerminalSymbol::EXPR, SymbolsAst{std::make_shared<TerminalSymbolAst>(
                                                        TerminalSymbol::ASSIGN_OP, "=")}),
                       std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "=")})});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}

TEST(Recursion, MiddleRecursion)
{
    auto syntaxAnalyzer =
        std::make_shared<SyntaxAnalyzer>(NonTerminalSymbol::PROGRAM, TerminalSymbol::FINISH);
    syntaxAnalyzer->addRule(NonTerminalSymbol::PROGRAM, {NonTerminalSymbol::EXPR});
    syntaxAnalyzer->addRule(
        NonTerminalSymbol::EXPR,
        {TerminalSymbol::ASSIGN_OP, NonTerminalSymbol::EXPR, TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->addRule(NonTerminalSymbol::EXPR, {TerminalSymbol::ASSIGN_OP});
    syntaxAnalyzer->start();
    // TODO: this looks terrible, I need to find a better way to represent it
    auto expectedTree = std::make_shared<NonTerminalSymbolAst>(
        NonTerminalSymbol::PROGRAM,
        SymbolsAst{std::make_shared<NonTerminalSymbolAst>(
            NonTerminalSymbol::EXPR,
            SymbolsAst{
                std::make_shared<NonTerminalSymbolAst>(
                    NonTerminalSymbol::EXPR,
                    SymbolsAst{
                        std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "="),
                        std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "=")}),
                std::make_shared<TerminalSymbolAst>(TerminalSymbol::ASSIGN_OP, "=")})});
    auto parseRes = syntaxAnalyzer->parse(flattenAst(expectedTree));
    cmpAstTrees(expectedTree, parseRes);
}
