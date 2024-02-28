#ifndef AST_NODE_HPP
#define AST_NODE_HPP

#include <memory>
#include <string>
#include <vector>

class AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstNode>;
    AstNode::SharedPtr parent;
};

class AstProgram : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProgram>;
    std::vector<AstNode::SharedPtr> children;
};

class AstProcedureCall : public AstNode
{
public:
    using SharedPtr = std::shared_ptr<AstProcedureCall>;
    std::string name;
    std::vector<AstNode::SharedPtr> children;
};

class AstId : public AstNode
{
public:
    std::string name;
    using SharedPtr = std::shared_ptr<AstId>;
};

class AstNum : public AstNode
{
public:
    int num;
    using SharedPtr = std::shared_ptr<AstId>;
};

// class AstProcedureCall : AstNode
// {
// public:
//     std::string name;
//     std::vector<AstNode::SharedPtr> children;
// };

#endif // AST_NODE_HPP
