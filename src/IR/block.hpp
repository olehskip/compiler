#ifndef IR_BLOCK_HPP
#define IR_BLOCK_HPP

#include "IR/instructions.hpp"
#include "IR/symbol_table.hpp"
#include "IR/value.hpp"

class SimpleBlock : public Value
{
public:
    using SharedPtr = std::shared_ptr<SimpleBlock>;
    std::vector<SimpleBlock::SharedPtr> children; // maybe use a weak_ptr, but then who owns it?
    SimpleBlock() : Value(CompileTimeType::getNew(TypeID::LABEL))
    {
        symbolTable = std::make_shared<SymbolTable>();
    }

    std::vector<std::shared_ptr<Instruction>> insts;
    SymbolTable::SharedPtr symbolTable;
    void pretty(std::stringstream &stream) const override
    {
        stream << strid << " = block:\n";
        const auto generalProcedureTable = symbolTable->getGeneralProceduresTable();
        for (const auto &[name, procedure] : generalProcedureTable) {
            procedure->pretty(stream);
        }
        for (auto inst : insts) {
            stream << inst->strid << " = ";
            inst->pretty(stream);
            stream << "\n";
        }
    }

    // this is done so a weak_ptr can be created and passed to the parent
    static SimpleBlock::SharedPtr createWithParent(SimpleBlock::SharedPtr parent)
    {
        const auto simpleBlock = std::shared_ptr<SimpleBlock>(new SimpleBlock(parent));
        parent->children.push_back(simpleBlock);
        return simpleBlock;
    }

private:
    SimpleBlock(SimpleBlock::SharedPtr parent) : Value(CompileTimeType::getNew(TypeID::LABEL))
    {
        symbolTable = std::make_shared<SymbolTable>(parent->symbolTable);
    }
};

#endif // IR_BLOCK_HPP
