#include "IR/symbol_table.hpp"
#include "log.hpp"
#include "IR/procedure.hpp"

SymbolTable::SymbolTable(std::weak_ptr<SymbolTable> parent_) : parent(parent_) {}

void SymbolTable::addGeneralProcedure(GeneralProcedure::SharedPtr procedure)
{
    // TODO: add checking in parent symbol tables as well
    ASSERT_MSG(!specificProceduresTable.contains(procedure->name),
               "Can't add GeneralProcedure with name = "
                   << procedure->name
                   << " because SpecificProcedure with the same name already exists");
    ASSERT_MSG(!generalProceduresTable.contains(procedure->name),
               "GeneralProcedure with name = " << procedure->name << " already exists");
    generalProceduresTable.insert({procedure->name, procedure});
}

GeneralProcedure::SharedPtr SymbolTable::getGeneralProcedure(std::string name)
{
    auto procedureIt = generalProceduresTable.find(name);
    if (procedureIt != generalProceduresTable.end()) {
        return procedureIt->second;
    }
    if (auto lockedParent = parent.lock()) {
        return lockedParent->getGeneralProcedure(name);
    }
    return nullptr;
}

void SymbolTable::addSpecificProcedure(SpecificProcedure::SharedPtr procedure)
{
    // TODO: add checking in parent symbol tables as well
    ASSERT_MSG(!generalProceduresTable.contains(procedure->name),
               "Can't add Specificprocedure with name = "
                   << procedure->name
                   << " because GeneralProcedure with the same name already exists");
    specificProceduresTable[procedure->name].push_back(procedure);
}

SpecificProcedure::SharedPtr SymbolTable::getSpecificProcedure(std::string name,
                                                               CompileTimeTypes types)
{
    auto procedureIt = specificProceduresTable.find(name);
    if (procedureIt != specificProceduresTable.end()) {
        for (auto procedureCandidate : procedureIt->second) {
            const auto typesEqual =
                std::equal(types.begin(), types.end(),
                           toCompileTimeTypes(procedureCandidate->argsTypes).begin(),
                           [](auto typeA, auto typeB) { return typeA->typeID == typeB->typeID; });
            if (typesEqual) {
                return procedureCandidate;
            }
        }
    }
    if (auto lockedParent = parent.lock()) {
        return lockedParent->getSpecificProcedure(name, types);
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

const std::unordered_map<std::string, GeneralProcedure::SharedPtr> &
SymbolTable::getGeneralProceduresTable() const
{
    return generalProceduresTable;
}

const std::unordered_map<std::string, std::vector<SpecificProcedure::SharedPtr>> &
SymbolTable::getSpecificProceduresTable() const
{
    return specificProceduresTable;
}
