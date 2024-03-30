#ifndef IR_SYMBOL_TABLE_HPP
#define IR_SYMBOL_TABLE_HPP

#include "value.hpp"

#include <memory>

/* As in the implementation of LLVM IR (do not confuse the implementation and what is printed),
 * generateIR doesn't use "raw" names of objects for instructions, e. g. if we want to call a
 * procedure "display", it would emit:
 *  call 0x1245678
 * where 0x12345678i is address to Procedure, however the generator needs to store the names because
 * AST contains only names, so SymbolTable translates names to Value, hence after IR generation, the
 * symbol table is not needed anymore and it isn't passed to next steps of the compilation
 */

class SymbolTable
{
public:
    using SharedPtr = std::shared_ptr<SymbolTable>;
    SymbolTable(std::weak_ptr<SymbolTable> parent_ = std::weak_ptr<SymbolTable>());
    void addNewProcedure(Procedure::SharedPtr procedure);
    Procedure::SharedPtr getProcedure(std::string name, std::vector<Type> argsTypes);

private:
    std::unordered_map<std::string, std::vector<Procedure::SharedPtr>>
        proceduresTable; // same procedure may have different types
    std::weak_ptr<SymbolTable> parent;
};

#endif // IR_SYMBOL_TABLE_HPP
