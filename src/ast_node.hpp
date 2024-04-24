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
    COND_IF,
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

    const std::string name;
    using SharedPtr = std::shared_ptr<AstId>;
};

class AstInt : public AstNode
{
public:
    AstInt(int64_t num_) : AstNode(AstNodeType::INT), num(num_) {}

    const int64_t num;
    using SharedPtr = std::shared_ptr<AstInt>;
};

class AstFloat : public AstNode
{
public:
    AstFloat(long double num_) : AstNode(AstNodeType::FLOAT), num(num_) {}

    const long double num;
    using SharedPtr = std::shared_ptr<AstFloat>;
};

class AstString : public AstNode
{
public:
    AstString(std::string str_) : AstNode(AstNodeType::STRING), str(str_) {}

    const std::string str;
    using SharedPtr = std::shared_ptr<AstString>;
};

class AstProcedureDef : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureDef>;

    AstProcedureDef(std::string name_, std::vector<AstId::SharedPtr> params_,
                    AstNode::SharedPtr body_)
        : AstNode(AstNodeType::PROCEDURE_DEF), name(name_), params(params_), body(body_)
    {
    }

    const std::string name;
    const std::vector<AstId::SharedPtr> params;
    const AstNode::SharedPtr body;
};

class AstProcedureCall : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureCall>;

    AstProcedureCall(std::string name_, std::vector<AstNode::SharedPtr> children_)
        : AstNode(AstNodeType::PROCEDURE_CALL), name(name_), children(children_)
    {
    }

    const std::string name;
    const std::vector<AstNode::SharedPtr> children;
};

class AstVarDef : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstVarDef>;

    AstVarDef(std::string name_, AstNode::SharedPtr expr_)
        : AstNode(AstNodeType::VAR_DEF), name(name_), expr(expr_)
    {
    }

    const std::string name;
    const AstNode::SharedPtr expr;
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
