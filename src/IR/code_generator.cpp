#include "code_generator.hpp"

#include <cassert>
#include <functional>
#include <optional>
#include <sstream>
#include <iostream>

// FormIdx SymbolTable::putNewVar(VarContent varContent)
// {
//     const FormIdx newVarIdx = variablesTable.size();
//     variablesTable.push_back(varContent);
//     return newVarIdx;
// }
//
void SsaSeq::pretty(std::stringstream &stream)
{
    std::cout << "pretty\n";
    for (size_t idx = 0; idx < insts.size(); ++idx) {
        stream << insts[idx]->pretty() << "\n";
        // stream << "[" << idx << "] ";
        // if (auto ssaCall = std::dynamic_pointer_cast<CallInst>(form)) {
        //     stream << "CALL " << ssaCall->procedureName << " " << ssaCall->paramsCnt << "\n";
        // } else if (auto ssaAssignLiteral = std::dynamic_pointer_cast<SsaStoreLiteral>(form)) {
        //     stream << "ASSIGN_LITERAL " << ssaAssignLiteral->literal << "\n";
        // } else if (auto ssaParam = std::dynamic_pointer_cast<ParamInst>(form)) {
        //     stream << "PARAM " << ssaParam->var << "\n";
        // } else {
        //     assert(!"Not processed ssa form");
        // }
    }
}
#include <iostream>
SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram)
{
    SsaSeq ssaSeq;
    std::function<Value::SharedPtr(AstNode::SharedPtr, SsaSeq &)> _generateSsaEq =
        [&_generateSsaEq](AstNode::SharedPtr astNode, SsaSeq &ssaSeq) -> Value::SharedPtr {
        if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
            for (auto child : astProgram->children) {
                _generateSsaEq(child, ssaSeq);
            }
            return nullptr;
            // } else if (auto astProcedureDef =
            //                std::dynamic_pointer_cast<AstProcedureDefinition>(astNode)) {
            //     auto returnType = determineType(astProcedureDef->body, ssaSeq.symbolTable);
            //     std::cout << "type = " << returnType.name << "\n";
            //     Procedure procedure{astProcedureDef->name, {}, returnType};
            //     _generateSsaEq(astProcedureDef->body, procedure.ssaSeq);
            //     return std::nullopt;
        } else if (auto astProcedureCall = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
            const size_t childrenSize = astProcedureCall->children.size();
            std::vector<Value::SharedPtr> args;
            for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
                auto childInst = _generateSsaEq(astProcedureCall->children[childIdx], ssaSeq);
                assert(childInst);
                args.push_back(childInst);
            }
            auto callInst = std::make_shared<CallInst>(std::shared_ptr<Procedure>(nullptr), args,
                                                       astProcedureCall->name);
            ssaSeq.insts.push_back(callInst);
            return callInst;
        } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
            // auto value = std::make_shared<Value>(Type::TypeID::UINT64);
            // return value;
            // auto dstValue = std::make_shared<Value>();
            // auto storeInst = std::make_shared<StoreInst>();
            // ret.forms.push_back(SsaCall{astProcedure->name, childrenSize});
            // return ret.forms.size() - 1;
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
