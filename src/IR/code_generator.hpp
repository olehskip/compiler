#ifndef INTER_CODE_GENERATOR_HPP
#define INTER_CODE_GENERATOR_HPP

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast_node.hpp"
#include "instructions.hpp"
#include "symbol_table.hpp"

class SimpleBlock : public Value
{
public:
    using SharedPtr = std::shared_ptr<SimpleBlock>;
    SimpleBlock() : Value(CompileTimeType::getNew(TypeID::LABEL)) {}
    SimpleBlock::SharedPtr parent;

    std::vector<std::shared_ptr<Instruction>> insts;
    void pretty(std::stringstream &stream) const override;
};

SimpleBlock::SharedPtr generateIR(AstProgram::SharedPtr astProgram);

#endif // INTER_CODE_GENERATOR_HPP
