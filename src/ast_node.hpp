#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <memory>
#include <string>
#include <vector>

#include "IR/block.hpp"
#include "IR/symbol_table.hpp"
#include "log.hpp"

enum class AstNodeType
{
    UKNOWN,
    PROGRAM,
    BEGIN_EXPR,
    PROCEDURE_DEF,
    PROCEDURE_CALL,
    COND_IF,
    VAR_DEF,
    ID,
    INT,
    FLOAT,
    STRING
};

class AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstNode>;
    virtual ~AstNode() {}

    AstNode(AstNodeType astNodeType_ = AstNodeType::UKNOWN) : astNodeType(astNodeType_) {}
    virtual Value::SharedPtr emitSsa(SimpleBlock::SharedPtr /* simpleBlock */)
    {
        NOT_IMPLEMENTED;
        return nullptr;
    };

    AstNode::SharedPtr parent;

    // it exists rather for debug
    const AstNodeType astNodeType;
};

class AstProgram : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProgram>;

    AstProgram() : AstNode(AstNodeType::PROGRAM) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    std::vector<AstNode::SharedPtr> children;
};

class AstBeginExpr : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstBeginExpr>;

    AstBeginExpr() : AstNode(AstNodeType::BEGIN_EXPR) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    std::vector<AstNode::SharedPtr> children;
};

class AstId : public AstNode
{
public:
    AstId(std::string name_) : AstNode(AstNodeType::ID), name(name_) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    const std::string name;
    using SharedPtr = std::shared_ptr<AstId>;
};

class AstInt : public AstNode
{
public:
    AstInt(int64_t num_) : AstNode(AstNodeType::INT), num(num_) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    const int64_t num;
    using SharedPtr = std::shared_ptr<AstInt>;
};

class AstFloat : public AstNode
{
public:
    AstFloat(long double num_) : AstNode(AstNodeType::FLOAT), num(num_) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    const long double num;
    using SharedPtr = std::shared_ptr<AstFloat>;
};

class AstString : public AstNode
{
public:
    AstString(std::string str_) : AstNode(AstNodeType::STRING), str(str_) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    const std::string str;
    using SharedPtr = std::shared_ptr<AstString>;
};

class AstProcedureDef : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureDef>;

    AstProcedureDef() : AstNode(AstNodeType::PROCEDURE_DEF) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    std::string name;
    std::vector<AstId::SharedPtr> params;
    AstNode::SharedPtr body;
};

class AstProcedureCall : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureCall>;

    AstProcedureCall() : AstNode(AstNodeType::PROCEDURE_CALL) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    std::string name;
    std::vector<AstNode::SharedPtr> children;
};

class AstVarDef : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstVarDef>;

    AstVarDef() : AstNode(AstNodeType::VAR_DEF) {}
    Value::SharedPtr emitSsa(SimpleBlock::SharedPtr simpleBlock) override;

    std::string name;
    AstNode::SharedPtr expr;
};

class AstCondIf : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstCondIf>;

    AstCondIf(AstNode::SharedPtr exprToTest_, AstNode::SharedPtr body_,
              AstNode::SharedPtr elseBody_)
        : AstNode(AstNodeType::COND_IF), exprToTest(exprToTest_), body(body_), elseBody(elseBody_)
    {
    }

    const AstNode::SharedPtr exprToTest;
    const AstNode::SharedPtr body;
    const AstNode::SharedPtr elseBody; // may be nullptr if no else
};

#endif // AST_NODE_HPP
