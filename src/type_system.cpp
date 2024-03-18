#include "type_system.hpp"

#include "inter_code_generator.hpp"

Type determineType(AstNode::SharedPtr astNode, SymbolTable::SharedPtr symbolTable)
{
    return DefaultTypes::floatType;
}
