#include "code_generator.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>

void SymbolTable::addNewProcedure(Procedure::SharedPtr procedure)
{
    proceduresTable[procedure->name] = procedure;
}

void SsaSeq::pretty(std::stringstream &stream)
{
    std::cout << "pretty\n";
    for (size_t idx = 0; idx < insts.size(); ++idx) {
        stream << "$" << std::to_string(uint64_t(insts[idx].get())) << " = " << insts[idx]->pretty()
               << "\n";
    }
}

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram)
{
    SsaSeq ssaSeq;
    ssaSeq.symbolTable = std::make_shared<SymbolTable>();
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "display", std::vector<Type>{Type(Type::TypeID::UINT64)}, Type(Type::TypeID::VOID)));
    ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
        "+", std::vector<Type>{Type(Type::TypeID::UINT64), Type(Type::TypeID::UINT64)},
        Type(Type::TypeID::UINT64)));
    std::function<Value::SharedPtr(AstNode::SharedPtr, SsaSeq &)> _generateSsaEq =
        [&_generateSsaEq](AstNode::SharedPtr astNode, SsaSeq &ssaSeq) -> Value::SharedPtr {
        if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
            for (auto child : astProgram->children) {
                _generateSsaEq(child, ssaSeq);
            }
            return nullptr;
        } else if (auto astProcedureDef =
                       std::dynamic_pointer_cast<AstProcedureDefinition>(astNode)) {
            assert(!"Not implemented yet");
        } else if (auto astProcedureCall = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
            auto procedure = ssaSeq.symbolTable->proceduresTable[astProcedureCall->name];
            if (!procedure) {
                std::cerr << "Can't find procedure with name " << astProcedureCall->name
                          << " in symbol table\n";
                abort();
            }
            assert(procedure);
            const size_t childrenSize = astProcedureCall->children.size();
            std::vector<Value::SharedPtr> args;
            for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
                auto childInst = _generateSsaEq(astProcedureCall->children[childIdx], ssaSeq);
                assert(childInst);
                args.push_back(childInst);
            }
            auto callInst = std::make_shared<CallInst>(std::shared_ptr<Procedure>(procedure), args,
                                                       astProcedureCall->name);
            ssaSeq.insts.push_back(callInst);
            return callInst;
        } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
            assert(!"Not implemented yet");
        } else if (auto astNum = std::dynamic_pointer_cast<AstNum>(astNode)) {
            auto constInt = std::make_shared<ConstantInt>(astNum->num);
            return constInt;
        }
        assert(!"Not processed ast node");

        return nullptr;
    };

    _generateSsaEq(astProgram, ssaSeq);

    return ssaSeq;
}
