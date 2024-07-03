#ifndef INTER_CODE_GENERATOR_HPP
#define INTER_CODE_GENERATOR_HPP

#include "ast_node.hpp"
#include "IR/block.hpp"


SimpleBlock::SharedPtr generateIR(AstProgram::SharedPtr astProgram);

#endif // INTER_CODE_GENERATOR_HPP
