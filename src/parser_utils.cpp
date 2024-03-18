#include "parser_utils.hpp"

#include <cassert>
#include <iostream>
#include <stack>

static std::vector<AstNode::SharedPtr> processGeneral(SymbolSt::SharedPtr node);

static std::string processProcedureName(SymbolSt::SharedPtr node)
{
    auto terminalSt = std::dynamic_pointer_cast<TerminalSymbolSt>(node);
    assert(terminalSt);
    assert(terminalSt->symbolType == TerminalSymbol::ID);
    return terminalSt->text;
}

static std::vector<AstId::SharedPtr> processProcedureParams(SymbolSt::SharedPtr node)
{
    if (auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node)) {
        if (nonTerminalSt->symbolType == NonTerminalSymbol::PROCEDURE_PARAMS) {
            assert(nonTerminalSt->children.size() == 2 || nonTerminalSt->children.size() == 1);
            std::vector<AstId::SharedPtr> processedParams;
            for (auto child : nonTerminalSt->children) {
                auto processedChild = processProcedureParams(child);
                assert(processedChild.size() > 0);
                processedParams.insert(processedParams.end(), processedChild.begin(),
                                       processedChild.end());
            }
            return processedParams;
        } else if (nonTerminalSt->symbolType == NonTerminalSymbol::PROCEDURE_PARAM) {
            auto processedParam = processProcedureParams(nonTerminalSt->children[0]);
            assert(processedParam.size() == 1);
            return processedParam;
        } else {
            assert(!"Unexpected symbolType");
            return {};
        }
    } else if (auto terminalSt = std::dynamic_pointer_cast<TerminalSymbolSt>(node)) {
        assert(terminalSt->symbolType == TerminalSymbol::ID);
        return {std::make_shared<AstId>(terminalSt->text)};

    } else {
        assert(!"Should never happen");
        return {};
    }
}

static AstProcedureDefinition::SharedPtr
processProcedureDefinition(NonTerminalSymbolSt::SharedPtr node)
{
    auto ret = std::make_shared<AstProcedureDefinition>();
    assert(node->children.size() >= 6); // ( define (PROCEDURE_NAME ARG*) body )
    ret->name = processProcedureName(node->children[3]);
    // TODO: add support for no params
    if (node->children.size() > 6) {
        ret->params = processProcedureParams(node->children[4]);
        assert(ret->params.size() > 0);
    }
    auto processedBody = processGeneral(node->children[6]);
    assert(processedBody.size() == 1);
    ret->body = processedBody.front();
    return ret;
}

static std::vector<AstNode::SharedPtr> processOperands(SymbolSt::SharedPtr node)
{
    auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node);
    assert(nonTerminalSt);
    if (nonTerminalSt->symbolType == NonTerminalSymbol::OPERANDS) {
        assert(nonTerminalSt->children.size() == 2 || nonTerminalSt->children.size() == 1);
        std::vector<AstNode::SharedPtr> processedOperands;
        for (auto child : nonTerminalSt->children) {
            auto processedChild = processOperands(child);
            processedOperands.insert(processedOperands.end(), processedChild.begin(),
                                     processedChild.end());
        }
        assert(processedOperands.size() > 0);
        return processedOperands;
    } else if (nonTerminalSt->symbolType == NonTerminalSymbol::OPERAND) {
        auto processedOperand = processGeneral(nonTerminalSt->children[0]);
        assert(processedOperand.size() == 1);
        return processedOperand;
    } else {
        assert(!"Unexpected symbolType");
        return {};
    }
}

static AstProcedureCall::SharedPtr processProcedureCall(NonTerminalSymbolSt::SharedPtr node)
{
    auto ret = std::make_shared<AstProcedureCall>();
    assert(node->children.size() >= 3); // ( PROCEDURE_NAME OPERATOR* )
    ret->name = processProcedureName(node->children[1]);
    if (node->children.size() > 3) {
        ret->children = processOperands(node->children[2]);
        assert(ret->children.size() > 0);
    }

    return ret;
}

static AstProgram::SharedPtr processProgram(NonTerminalSymbolSt::SharedPtr node)
{
    auto ret = std::make_shared<AstProgram>();
    for (auto child : node->children) {
        auto processedChildren = processGeneral(child);
        ret->children.insert(ret->children.end(), processedChildren.begin(),
                             processedChildren.end());
    }
    assert(ret->children.size() > 0);
    return ret;
}

static std::vector<AstNode::SharedPtr> processGeneral(SymbolSt::SharedPtr node)
{
    if (auto terminalSt = std::dynamic_pointer_cast<TerminalSymbolSt>(node)) {
        if (terminalSt->symbolType == TerminalSymbol::ID) {
            return {std::make_shared<AstId>(terminalSt->text)};
        } else if (terminalSt->symbolType == TerminalSymbol::NUMBER) {
            auto ret = std::make_shared<AstNum>();
            ret->num = std::stoi(terminalSt->text);
            return {ret};
        } else {
            assert(!"terminal not implemented");
            return {};
        }
    } else if (auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node)) {
        switch (nonTerminalSt->symbolType) {
            case NonTerminalSymbol::START:
            case NonTerminalSymbol::STARTS:
            case NonTerminalSymbol::EXPR:
            case NonTerminalSymbol::EXPRS: {
                std::vector<AstNode::SharedPtr> ret;
                for (auto child : nonTerminalSt->children) {
                    auto processedChildren = processGeneral(child);
                    ret.insert(ret.end(), processedChildren.begin(), processedChildren.end());
                }
                return ret;
            }
            case NonTerminalSymbol::PROCEDURE_DEFINITION: {
                auto ret =
                    std::dynamic_pointer_cast<AstNode>(processProcedureDefinition(nonTerminalSt));
                assert(ret);
                return {ret};
            }
            case NonTerminalSymbol::PROCEDURE_CALL: {
                auto ret = std::dynamic_pointer_cast<AstNode>(processProcedureCall(nonTerminalSt));
                assert(ret);
                return {ret};
            }
            case NonTerminalSymbol::LITERAL: {
                assert(nonTerminalSt->children.size() == 1);
                return {processGeneral(nonTerminalSt->children.back())};
            }
            default: {
                std::cerr << "nonterminal " + getSymbolName(nonTerminalSt->symbolType) +
                                 " not implemented";
                abort();
            }
        }
    }
    assert(!"Should not happen");
    return {};
}

AstProgram::SharedPtr convertToAst(NonTerminalSymbolSt::SharedPtr root)
{
    return processProgram(root);
}

void removeBlankNewlineTerminals(TerminalSymbolsSt &terminalSymbolsSt)
{
    std::erase_if(terminalSymbolsSt, [](auto &symbol) {
        return symbol->symbolType == TerminalSymbol::BLANK ||
               symbol->symbolType == TerminalSymbol::NEWLINE;
    });
}

bool isLexicalError(const TerminalSymbolsSt &terminalSymbolsSt)
{
    return terminalSymbolsSt.end() !=
           std::find_if(
               terminalSymbolsSt.begin(), terminalSymbolsSt.end(),
               [](const auto &toCheck) { return toCheck->symbolType == TerminalSymbol::ERROR; });
}

void stVisitor(const SymbolSt::SharedPtr root,
               std::function<void(TerminalSymbolSt::SharedPtr)> terminalCallback,
               std::function<void(NonTerminalSymbolSt::SharedPtr)> nonTerminalCallback)
{
    assert(root);
    std::stack<SymbolSt::SharedPtr> symStack;
    symStack.push(root);

    while (!symStack.empty()) {
        auto currSym = symStack.top();
        assert(currSym);
        symStack.pop();
        if (auto terminalSymbol = std::dynamic_pointer_cast<TerminalSymbolSt>(currSym)) {
            terminalCallback(terminalSymbol);
        } else if (auto nonTerminalSymbol =
                       std::dynamic_pointer_cast<NonTerminalSymbolSt>(currSym)) {
            // TODO: change to iterators
            for (int i = nonTerminalSymbol->children.size() - 1; i >= 0; --i) {
                symStack.push(nonTerminalSymbol->children[i]);
            }
            nonTerminalCallback(nonTerminalSymbol);
        } else {
            assert(!"Should never happen");
        }
    }
}

TerminalSymbolsSt getLeafsSt(SymbolSt::SharedPtr root)
{
    TerminalSymbolsSt ret;
    stVisitor(
        root, [&ret](TerminalSymbolSt::SharedPtr terminalSymbol) { ret.push_back(terminalSymbol); },
        [](NonTerminalSymbolSt::SharedPtr) {});
    ret.push_back(
        std::make_shared<TerminalSymbolSt>(TerminalSymbol::FINISH, "")); // TODO: remove it
    return ret;
}
