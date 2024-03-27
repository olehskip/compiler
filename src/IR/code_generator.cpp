#include "code_generator.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>

void SymbolTable::addNewProcedure(Procedure::SharedPtr procedure)
{
    proceduresTable[procedure->name].push_back(procedure);
}

Procedure::SharedPtr SymbolTable::getProcedure(std::string name, std::vector<Type> argsTypes)
{
    auto vec = proceduresTable[name];
    for (auto procedure : vec) {
        assert(procedure);
        if (procedure->argsTypes.size() != argsTypes.size()) {
            continue;
        }
        const bool areArgsSame =
            std::equal(procedure->argsTypes.begin(), procedure->argsTypes.end(), argsTypes.begin(),
                       [](Type procedureArgType, Type givenArggType) {
                           return procedureArgType.typeID == givenArggType.typeID;
                       });
        if (areArgsSame) {
            return procedure;
        }
    }

    return nullptr;
}

void SsaSeq::pretty(std::stringstream &stream)
{
    std::cout << "pretty\n";
    for (size_t idx = 0; idx < insts.size(); ++idx) {
        stream << "$" << std::to_string(uint64_t(insts[idx].get())) << " = " << insts[idx]->pretty()
               << "\n";
    }
}

static Value::SharedPtr _generateSsaEq(AstNode::SharedPtr astNode, SsaSeq &ssaSeq)
{
    if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
        for (auto child : astProgram->children) {
            _generateSsaEq(child, ssaSeq);
        }
        return nullptr;
    } else if (auto astProcedureDef = std::dynamic_pointer_cast<AstProcedureDefinition>(astNode)) {
        assert(!"Not implemented yet");
    } else if (auto astProcedureCall = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
        const size_t childrenSize = astProcedureCall->children.size();
        std::vector<Value::SharedPtr> args;
        std::vector<Type> argsTypes;
        for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
            auto childInst = _generateSsaEq(astProcedureCall->children[childIdx], ssaSeq);
            assert(childInst);
            args.push_back(childInst);
            argsTypes.push_back(childInst->ty);
        }
        auto procedure = ssaSeq.symbolTable->getProcedure(astProcedureCall->name, argsTypes);
        if (!procedure) {
            std::cerr << "Can't find procedure with name " << astProcedureCall->name
                      << " in symbol table\n";
            abort();
        }
        assert(procedure);
        auto callInst = std::make_shared<CallInst>(std::shared_ptr<Procedure>(procedure), args,
                                                   astProcedureCall->name);
        ssaSeq.insts.push_back(callInst);
        return callInst;
    } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
        assert(!"Not implemented yet");
    } else if (auto astInt = std::dynamic_pointer_cast<AstInt>(astNode)) {
        auto constInt = std::make_shared<ConstantInt>(astInt->num);
        return constInt;
    } else if (auto astFloat = std::dynamic_pointer_cast<AstFloat>(astNode)) {
        auto constFloat = std::make_shared<ConstantFloat>(astFloat->num);
        return constFloat;
    }
    assert(!"Not processed ast node");

    return nullptr;
}

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram)
{
    SsaSeq ssaSeq;
    ssaSeq.symbolTable = std::make_shared<SymbolTable>();
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "display", std::vector<Type>{Type(Type::TypeID::UINT64)}, Type(Type::TypeID::VOID)));
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "+", std::vector<Type>{Type(Type::TypeID::UINT64), Type(Type::TypeID::UINT64)},
        Type(Type::TypeID::FLOAT)));
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "+", std::vector<Type>{Type(Type::TypeID::UINT64), Type(Type::TypeID::FLOAT)},
        Type(Type::TypeID::FLOAT)));
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "+", std::vector<Type>{Type(Type::TypeID::FLOAT), Type(Type::TypeID::UINT64)},
        Type(Type::TypeID::FLOAT)));
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "+", std::vector<Type>{Type(Type::TypeID::FLOAT), Type(Type::TypeID::FLOAT)},
        Type(Type::TypeID::FLOAT)));

    _generateSsaEq(astProgram, ssaSeq);

    return ssaSeq;
}
