#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <memory>
#include <string>
#include <vector>

enum class AstNodeType
{
    UKNOWN,
    PROGRAM,
    BEGIN_EXPR,
    PROCEDURE_DEF,
    PROCEDURE_CALL,
    VAR_DEF,
    ID,
    INT,
    FLOAT
};

class AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstNode>;
    virtual ~AstNode() {}

    AstNode(AstNodeType astNodeType_ = AstNodeType::UKNOWN) : astNodeType(astNodeType_) {}

    AstNode::SharedPtr parent;

    // it exists rather for debug
    const AstNodeType astNodeType;
};

class AstProgram : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProgram>;

    AstProgram() : AstNode(AstNodeType::PROGRAM) {}

    std::vector<AstNode::SharedPtr> children;
};

class AstBeginExpr : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstBeginExpr>;

    AstBeginExpr() : AstNode(AstNodeType::BEGIN_EXPR) {}

    std::vector<AstNode::SharedPtr> children;
};

class AstId : public AstNode
{
public:
    AstId(std::string name_) : AstNode(AstNodeType::ID), name(name_) {}

    std::string name;
    using SharedPtr = std::shared_ptr<AstId>;
};

class AstInt : public AstNode
{
public:
    AstInt() : AstNode(AstNodeType::INT) {}

    int64_t num;
    using SharedPtr = std::shared_ptr<AstInt>;
};

class AstFloat : public AstNode
{
public:
    AstFloat() : AstNode(AstNodeType::FLOAT) {}

    long double num;
    using SharedPtr = std::shared_ptr<AstFloat>;
};

class AstProcedureDef : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureDef>;

    AstProcedureDef() : AstNode(AstNodeType::PROCEDURE_DEF) {}

    std::string name;
    std::vector<AstId::SharedPtr> params;
    AstNode::SharedPtr body;
};

class AstProcedureCall : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureCall>;

    AstProcedureCall() : AstNode(AstNodeType::PROCEDURE_CALL) {}

    std::string name;
    std::vector<AstNode::SharedPtr> children;
};

class AstVarDef : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstVarDef>;

    AstVarDef() : AstNode(AstNodeType::VAR_DEF) {}

    std::string name;
    AstNode::SharedPtr expr;
};

#endif // AST_NODE_HPP
