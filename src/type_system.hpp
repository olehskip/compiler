#ifndef TYPE_SYSTEM_HPP
#define TYPE_SYSTEM_HPP

#include "ast_node.hpp"

#include <memory>
#include <string>

// class Type
// {
// public:
//     Type(std::string name_): name(name_) {}
//     const std::string name;
// };
//
namespace DefaultTypes {
// const Type floatType("float");
// const Type intType("int");
}; // namespace DefaultTypes

class SymbolTable;
class AstNode;
// Type determineType(std::shared_ptr<AstNode> astNode, std::shared_ptr<SymbolTable> symbolTable);

#endif // TYPE_SYSTEM_HPP
