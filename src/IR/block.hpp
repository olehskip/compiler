#ifndef IR_BLOCK_HPP
#define IR_BLOCK_HPP

#include "IR/instructions.hpp"
#include "IR/symbol_table.hpp"
#include "IR/value.hpp"

class SimpleBlock : public Value
{
public:
    using SharedPtr = std::shared_ptr<SimpleBlock>;
    SimpleBlock() : Value(CompileTimeType::getNew(TypeID::LABEL))
    {
        symbolTable = std::make_shared<SymbolTable>();
    }
    SimpleBlock(SimpleBlock::SharedPtr parent) : Value(CompileTimeType::getNew(TypeID::LABEL))
    {
        symbolTable = std::make_shared<SymbolTable>(parent->symbolTable);
    }
    SimpleBlock::SharedPtr parent;

    std::vector<std::shared_ptr<Instruction>> insts;
    SymbolTable::SharedPtr symbolTable;
    void pretty(std::stringstream &stream) const override
    {
        for (size_t idx = 0; idx < insts.size(); ++idx) {
            stream << "$" << std::to_string(uint64_t(insts[idx].get())) << " = ";
            insts[idx]->pretty(stream);
            stream << "\n";
        }
    }
};

#endif // IR_BLOCK_HPP
