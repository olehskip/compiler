#include "parser_utils.hpp"
#include "log.hpp"
#include "utils.hpp"

#include <stack>

static std::vector<AstNode::SharedPtr> processGeneral(SymbolSt::SharedPtr node);

static AstProgram::SharedPtr processProgram(NonTerminalSymbolSt::SharedPtr node)
{
    auto ret = std::make_shared<AstProgram>();
    for (auto child : node->children) {
        auto processedChildren = processGeneral(child);
        ret->children.insert(ret->children.end(), processedChildren.begin(),
                             processedChildren.end());
    }
    ASSERT(ret->children.size() > 0);
    return ret;
}

static AstBeginExpr::SharedPtr processBeginExpr(NonTerminalSymbolSt::SharedPtr node)
{
    auto ret = std::make_shared<AstBeginExpr>();
    ASSERT(node->children.size() >= 4); // ( begin EXPR+)
    for (size_t i = 2; i < node->children.size() - 1; ++i) {
        auto processedChildren = processGeneral(node->children[i]);
        ret->children.insert(ret->children.end(), processedChildren.begin(),
                             processedChildren.end());
    }
    ASSERT(ret->children.size() > 0);
    return ret;
}

static std::string processName(SymbolSt::SharedPtr node)
{
    auto terminalSt = std::dynamic_pointer_cast<TerminalSymbolSt>(node);
    ASSERT(terminalSt);
    ASSERT(terminalSt->symbolType == TerminalSymbol::ID);
    return terminalSt->text;
}

static std::vector<AstId::SharedPtr> processProcedureParams(SymbolSt::SharedPtr node)
{
    if (auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node)) {
        if (nonTerminalSt->symbolType == NonTerminalSymbol::PROCEDURE_PARAMS) {
            ASSERT(nonTerminalSt->children.size() == 2 || nonTerminalSt->children.size() == 1);
            std::vector<AstId::SharedPtr> processedParams;
            for (auto child : nonTerminalSt->children) {
                auto processedChild = processProcedureParams(child);
                ASSERT(processedChild.size() > 0);
                processedParams.insert(processedParams.end(), processedChild.begin(),
                                       processedChild.end());
            }
            return processedParams;
        } else if (nonTerminalSt->symbolType == NonTerminalSymbol::PROCEDURE_PARAM) {
            auto processedParam = processProcedureParams(nonTerminalSt->children[0]);
            ASSERT(processedParam.size() == 1);
            return processedParam;
        } else {
            LOG_FATAL << "Unexpected symbolType";
            return {};
        }
    } else if (auto terminalSt = std::dynamic_pointer_cast<TerminalSymbolSt>(node)) {
        ASSERT(terminalSt->symbolType == TerminalSymbol::ID);
        return {std::make_shared<AstId>(terminalSt->text)};

    } else {
        SHOULD_NOT_HAPPEN;
        return {};
    }
}

static AstProcedureDef::SharedPtr processProcedureDef(NonTerminalSymbolSt::SharedPtr node)
{
    ASSERT(node->children.size() >= 6); // ( define (PROCEDURE_NAME ARG*) body )
    const auto name = processName(node->children[3]);
    std::vector<AstId::SharedPtr> params;
    // TODO: add support for no params
    if (node->children.size() > 6) {
        params = processProcedureParams(node->children[4]);
        ASSERT(params.size() > 0);
    }
    auto body = extractFromSingleVector(processGeneral(node->children[6]));
    return std::make_shared<AstProcedureDef>(name, params, body);
}

static AstVarDef::SharedPtr processVarDef(NonTerminalSymbolSt::SharedPtr node)
{
    ASSERT(node->children.size() >= 5); // ( define VAR_NAME EXPR )
    const auto name = processName(node->children[2]);
    const auto processedExpr = extractFromSingleVector(processGeneral(node->children[3]));
    return std::make_shared<AstVarDef>(name, processedExpr);
}

static std::vector<AstNode::SharedPtr> processOperands(SymbolSt::SharedPtr node)
{
    auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node);
    ASSERT(nonTerminalSt);
    if (nonTerminalSt->symbolType == NonTerminalSymbol::OPERANDS) {
        ASSERT(nonTerminalSt->children.size() == 2 || nonTerminalSt->children.size() == 1);
        std::vector<AstNode::SharedPtr> processedOperands;
        for (auto child : nonTerminalSt->children) {
            auto processedChild = processOperands(child);
            processedOperands.insert(processedOperands.end(), processedChild.begin(),
                                     processedChild.end());
        }
        ASSERT(processedOperands.size() > 0);
        return processedOperands;
    } else if (nonTerminalSt->symbolType == NonTerminalSymbol::OPERAND) {
        auto processedOperand = processGeneral(nonTerminalSt->children[0]);
        ASSERT(processedOperand.size() == 1);
        return processedOperand;
    } else {
        LOG_FATAL << "Unexpected symbolType";
        return {};
    }
}

static AstProcedureCall::SharedPtr processProcedureCall(NonTerminalSymbolSt::SharedPtr node)
{
    ASSERT(node->children.size() >= 3); // ( PROCEDURE_NAME OPERATOR* )
    const auto name = processName(node->children[1]);
    std::vector<AstNode::SharedPtr> children;
    if (node->children.size() > 3) {
        children = processOperands(node->children[2]);
        ASSERT(children.size() > 0);
    }

    return std::make_shared<AstProcedureCall>(name, children);
}

static AstNode::SharedPtr processIfCondTest(SymbolSt::SharedPtr node)
{
    auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node);
    ASSERT(nonTerminalSt);
    ASSERT(nonTerminalSt->symbolType == NonTerminalSymbol::IF_COND_TEST);
    ASSERT(nonTerminalSt->children.size() == 1);
    return extractFromSingleVector(processGeneral(nonTerminalSt->children[0]));
}

static AstNode::SharedPtr processIfCondBody(SymbolSt::SharedPtr node)
{
    auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node);
    ASSERT(nonTerminalSt);
    ASSERT(nonTerminalSt->symbolType == NonTerminalSymbol::IF_COND_BODY);
    ASSERT(nonTerminalSt->children.size() == 1);
    return extractFromSingleVector(processGeneral(nonTerminalSt->children[0]));
}

static AstNode::SharedPtr processIfCondElseBody(SymbolSt::SharedPtr node)
{
    auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node);
    ASSERT(nonTerminalSt);
    ASSERT(nonTerminalSt->symbolType == NonTerminalSymbol::IF_COND_ELSE_BODY);
    ASSERT(nonTerminalSt->children.size() == 1);
    return extractFromSingleVector(processGeneral(nonTerminalSt->children[0]));
}
static AstCondIf::SharedPtr processCondIf(NonTerminalSymbolSt::SharedPtr node)
{
    ASSERT(node->children.size() == 5 ||
           node->children.size() == 6); // ( if EXPR_TO_TEST BODY ELSE_BODY? )
    auto exprToTest = processIfCondTest(node->children[2]);
    auto body = processIfCondBody(node->children[3]);
    AstNode::SharedPtr elseBody;
    if (node->children.size() == 6) {
        elseBody = processIfCondElseBody(node->children[4]);
    }

    return std::make_shared<AstCondIf>(exprToTest, body, elseBody);
}

static std::vector<AstNode::SharedPtr> processGeneral(SymbolSt::SharedPtr node)
{
    if (auto terminalSt = std::dynamic_pointer_cast<TerminalSymbolSt>(node)) {
        if (terminalSt->symbolType == TerminalSymbol::ID) {
            return {std::make_shared<AstId>(terminalSt->text)};
        } else if (terminalSt->symbolType == TerminalSymbol::INT) {
            return {std::make_shared<AstInt>(std::stoi(terminalSt->text))};
        } else if (terminalSt->symbolType == TerminalSymbol::STRING) {
            return {std::make_shared<AstString>(
                terminalSt->text.substr(1, terminalSt->text.size() - 2))}; // remove quotes
        } else {
            LOG_FATAL << "terminal " + getSymbolName(terminalSt->symbolType) + " not implemented";
        }
    } else if (auto nonTerminalSt = std::dynamic_pointer_cast<NonTerminalSymbolSt>(node)) {
        switch (nonTerminalSt->symbolType) {
            case NonTerminalSymbol::BEGIN_EXPR: {
                auto ret = std::dynamic_pointer_cast<AstNode>(processBeginExpr(nonTerminalSt));
                ASSERT(ret);
                return {ret};
            }
            case NonTerminalSymbol::STARTS:
            case NonTerminalSymbol::START:
            case NonTerminalSymbol::EXPR:
            case NonTerminalSymbol::EXPRS: {
                std::vector<AstNode::SharedPtr> ret;
                for (auto child : nonTerminalSt->children) {
                    auto processedChildren = processGeneral(child);
                    ret.insert(ret.end(), processedChildren.begin(), processedChildren.end());
                }
                return ret;
            }
            case NonTerminalSymbol::PROCEDURE_DEF: {
                auto ret = std::dynamic_pointer_cast<AstNode>(processProcedureDef(nonTerminalSt));
                ASSERT(ret);
                return {ret};
            }
            case NonTerminalSymbol::PROCEDURE_CALL: {
                auto ret = std::dynamic_pointer_cast<AstNode>(processProcedureCall(nonTerminalSt));
                ASSERT(ret);
                return {ret};
            }
            case NonTerminalSymbol::VAR_DEF: {
                auto ret = std::dynamic_pointer_cast<AstNode>(processVarDef(nonTerminalSt));
                ASSERT(ret);
                return {ret};
            }
            case NonTerminalSymbol::IF_COND: {
                auto ret = std::dynamic_pointer_cast<AstNode>(processCondIf(nonTerminalSt));
                ASSERT(ret);
                return {ret};
            }
            case NonTerminalSymbol::LITERAL: {
                ASSERT(nonTerminalSt->children.size() == 1);
                return {processGeneral(nonTerminalSt->children.back())};
            }
            default: {
                LOG_FATAL << "nonterminal " + getSymbolName(nonTerminalSt->symbolType) +
                                 " not implemented";
            }
        }
    }
    SHOULD_NOT_HAPPEN;
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
               symbol->symbolType == TerminalSymbol::NEWLINE ||
               symbol->symbolType == TerminalSymbol::COMMENT;
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
    ASSERT(root);
    std::stack<SymbolSt::SharedPtr> symStack;
    symStack.push(root);

    while (!symStack.empty()) {
        auto currSym = symStack.top();
        ASSERT(currSym);
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
            SHOULD_NOT_HAPPEN;
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

// TODO: this function is super ugly
void prettyAst(AstNode::SharedPtr astNode, std::stringstream &stream)
{
    const auto id = std::to_string((unsigned long long)astNode.get());
    if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
        stream << "digraph G {\n";
        for (auto child : astProgram->children) {
            stream << "\t" << '"' << "[PROGRAM] " << id << '"' << " -> ";
            prettyAst(child, stream);
        }
        stream << "\n}\n";
    } else if (auto astBeginExpr = std::dynamic_pointer_cast<AstBeginExpr>(astNode)) {
        for (auto child : astBeginExpr->children) {
            stream << "\t" << '"' << "[BEGIN_EXPR] " << id << '"' << " -> ";
            prettyAst(child, stream);
        }
    } else if (auto astProcedureDef = std::dynamic_pointer_cast<AstProcedureDef>(astNode)) {
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
    } else if (auto astVarDef = std::dynamic_pointer_cast<AstVarDef>(astNode)) {
        stream << '"' << "[VAR DEF] " << id << " " << astVarDef->name << '"' << "\n";
        stream << "\t" << '"' << "[VAR DEF] " << id << " " << astVarDef->name << '"' << " -> ";
        prettyAst(astVarDef->expr, stream);
    } else if (auto astCondIf = std::dynamic_pointer_cast<AstCondIf>(astNode)) {
        // stream << '"' << "[COND_IF] " << id << '"' << "\n";
        // // stream << '"' << "[COND_IF] " << id < '"' << "\n";
        // stream << "\t" << '"' << "[COND_IF] " << id << '"' << " -> ";
        // for (auto next: {astCondIf->exprToTest, astCondIf->body}) {
        //     prettyAst(next, stream);
        // }
    } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
        stream << '"' << "[ID] " << id << " " << astId->name << '"' << "\n";
    } else if (auto astInt = std::dynamic_pointer_cast<AstInt>(astNode)) {
        stream << '"' << "[INT] " << id << " " << astInt->num << '"' << "\n";
    } else if (auto astFloat = std::dynamic_pointer_cast<AstFloat>(astNode)) {
        stream << '"' << "[FLOAT] " << id << " " << astFloat->num << '"' << "\n";
    } else if (auto astString = std::dynamic_pointer_cast<AstString>(astNode)) {
        stream << '"' << "[STRING] " << id << " " << astString->str << '"' << "\n";
    } else {
        LOG_FATAL << "not processed AST node with type " << astNode->astNodeType;
    }
}
