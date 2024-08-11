#ifndef IR_SYMBOL_TABLE_HPP
#define IR_SYMBOL_TABLE_HPP

#include "IR/type_system.hpp"
#include "IR/value.hpp"

#include <memory>
#include <unordered_map>

/* As in the implementation of LLVM IR (do not confuse the implementation and what is printed),
 * emitSsa doesn't use "raw" names of objects for instructions,
 * e.g. if we wanted to call a procedure "display", it would emit:
 *  call 0x1245678
 * where 0x12345678i is address to Procedure, however the generator needs to store the names because
 * AST contains only names, so SymbolTable translates names to Value, hence after IR generation, the
 * symbol table is not needed anymore and it isn't passed to next steps of the compilation
 */

class GeneralProcedure;
class SpecificProcedure;

class SymbolTable
{
public:
    using SharedPtr = std::shared_ptr<SymbolTable>;
    SymbolTable(std::weak_ptr<SymbolTable> parent_ = std::weak_ptr<SymbolTable>());

    void addGeneralProcedure(std::shared_ptr<GeneralProcedure> procedure);
    std::shared_ptr<GeneralProcedure> getGeneralProcedure(std::string name);

    void addSpecificProcedure(std::shared_ptr<SpecificProcedure> procedure);
    std::shared_ptr<SpecificProcedure> getSpecificProcedure(std::string name,
                                                            CompileTimeTypes types);

    // if a variable with such name already exists, it gets overwritten
    void addNewVar(std::string name, Value::SharedPtr varValue);

    // if var with such name doesn't exists, the function returns nullptr
    Value::SharedPtr getVar(std::string name);

    const std::unordered_map<std::string, std::shared_ptr<GeneralProcedure>> &
    getGeneralProceduresTable() const;
    const std::unordered_map<std::string, std::vector<std::shared_ptr<SpecificProcedure>>> &
    getSpecificProceduresTable() const;

private:
    std::unordered_map<std::string, std::shared_ptr<GeneralProcedure>> generalProceduresTable;
    std::unordered_map<std::string, std::vector<std::shared_ptr<SpecificProcedure>>>
        specificProceduresTable;
    std::unordered_map<std::string, Value::SharedPtr> varsTable;
    std::weak_ptr<SymbolTable> parent;
};

#endif // IR_SYMBOL_TABLE_HPP
