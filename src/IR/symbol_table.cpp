#include "symbol_table.hpp"
#include "log.hpp"

SymbolTable::SymbolTable(std::weak_ptr<SymbolTable> parent_) : parent(parent_) {}

void SymbolTable::addNewProcedure(Procedure::SharedPtr procedure)
{
    proceduresTable[procedure->name].push_back(procedure);
}

Procedure::SharedPtr
SymbolTable::getProcedure(std::string name, std::vector<CompileTimeKnownType::SharedPtr> argsTypes)
{
    auto vec = proceduresTable[name];
    for (auto procedure : vec) {
        ASSERT(procedure);
        if (procedure->argsTypes.size() != argsTypes.size()) {
            continue;
        }
        // const bool areArgsSame =
        //     std::equal(procedure->argsTypes.begin(), procedure->argsTypes.end(),
        //     argsTypes.begin(),
        //                [](Type procedureArgType, Type givenArggType) {
        //                    return procedureArgType.typeID == givenArggType.typeID;
        //                });
        // if (areArgsSame) {
        //     return procedure;
        // }
        return procedure;
    }

    if (auto lockedParent = parent.lock()) {
        return lockedParent->getProcedure(name, argsTypes);
    }
    return nullptr;
}
void SymbolTable::addNewVar(std::string name, Value::SharedPtr varValue)
{
    varsTable[name] = varValue;
}

Value::SharedPtr SymbolTable::getVar(std::string name)
{
    auto it = varsTable.find(name);
    if (it != varsTable.end()) {
        return it->second;
    } else if (auto lockedParent = parent.lock()) {
        return lockedParent->getVar(name);
    } else {
        return nullptr;
    }
}
