#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <memory>
#include <string>
#include <vector>

enum class AstNodeType
{
    UKNOWN,
    PROGRAM,
    PROCEDURE_DEFINITION,
    PROCEDURE_CALL,
    ID,
    NUM,
};

class AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstNode>;
    virtual ~AstNode() {}

    AstNode(AstNodeType astNodeType_ = AstNodeType::UKNOWN) : astNodeType(astNodeType_) {}

    AstNode::SharedPtr parent;

    const AstNodeType astNodeType;
};

class AstProgram : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProgram>;

    AstProgram() : AstNode(AstNodeType::PROGRAM) {}

    std::vector<AstNode::SharedPtr> children;
};

class AstId : public AstNode
{
public:
    AstId(std::string name_) : AstNode(AstNodeType::ID), name(name_) {}

    std::string name;
    using SharedPtr = std::shared_ptr<AstId>;
};

class AstNum : public AstNode
{
public:
    AstNum() : AstNode(AstNodeType::NUM) {}

    int num;
    using SharedPtr = std::shared_ptr<AstId>;
};

class AstProcedureDefinition : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureDefinition>;

    AstProcedureDefinition() : AstNode(AstNodeType::PROCEDURE_DEFINITION) {}

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

#endif // AST_NODE_HPP
