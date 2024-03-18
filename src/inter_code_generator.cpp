#include "inter_code_generator.hpp"

#include "type_system.hpp"

#include <cassert>
#include <functional>
#include <optional>
#include <sstream>

// void SsaSeq::append(const SsaSeq &anotherSeq)
// {
//     forms.insert(forms.end(), anotherSeq.forms.begin(), anotherSeq.forms.end());
//     symbolTable->merge(*anotherSeq.symbolTable);
// }

FormIdx SymbolTable::putNewVar(VarContent varContent)
{
    const FormIdx newVarIdx = variablesTable.size();
    variablesTable.push_back(varContent);
    return newVarIdx;
}

void SsaSeq::pretty(std::stringstream &stream)
{
    for (size_t idx = 0; idx < forms.size(); ++idx) {
        auto form = forms[idx];
        stream << "[" << idx << "] ";
        if (auto ssaCall = std::dynamic_pointer_cast<SsaCall>(form)) {
            stream << "CALL " << ssaCall->procedureName << " " << ssaCall->paramsCnt << "\n";
        } else if (auto ssaAssignLiteral = std::dynamic_pointer_cast<SsaStoreLiteral>(form)) {
            stream << "ASSIGN_LITERAL " << ssaAssignLiteral->literal << "\n";
        } else if (auto ssaParam = std::dynamic_pointer_cast<SsaParam>(form)) {
            stream << "PARAM " << ssaParam->var << "\n";
        } else {
            assert(!"Not processed ssa form");
        }
    }
}
#include <iostream>
SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram)
{
    SsaSeq ssaSeq;
    std::function<std::optional<FormIdx>(AstNode::SharedPtr, SsaSeq &)> _generateSsaEq =
        [&_generateSsaEq](AstNode::SharedPtr astNode, SsaSeq &ssaSeq) -> std::optional<FormIdx> {
        if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
            for (auto child : astProgram->children) {
                _generateSsaEq(child, ssaSeq);
            }
            return std::nullopt;
        } else if (auto astProcedureDef =
                       std::dynamic_pointer_cast<AstProcedureDefinition>(astNode)) {
            auto returnType = determineType(astProcedureDef->body, ssaSeq.symbolTable);
            std::cout << "type = " << returnType.name << "\n";
            Procedure procedure{astProcedureDef->name, {}, returnType};
            _generateSsaEq(astProcedureDef->body, procedure.ssaSeq);
            return std::nullopt;
        } else if (auto astProcedureCall = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
            const size_t childrenSize = astProcedureCall->children.size();
            for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
                auto childMaybeFormIdx =
                    _generateSsaEq(astProcedureCall->children[childIdx], ssaSeq);
                assert(childMaybeFormIdx);
                ssaSeq.forms.push_back(std::make_shared<SsaParam>(*childMaybeFormIdx, childIdx));
            }
            ssaSeq.forms.push_back(std::make_shared<SsaCall>(astProcedureCall->name, childrenSize));
            return ssaSeq.forms.size() - 1;
        } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
            // ret.forms.push_back(SsaCall{astProcedure->name, childrenSize});
            // return ret.forms.size() - 1;
            assert(!"Not implemented yet");
            return std::nullopt;
        } else if (auto astNum = std::dynamic_pointer_cast<AstNum>(astNode)) {
            ssaSeq.forms.push_back(std::make_shared<SsaStoreLiteral>(astNum->num));
            return ssaSeq.forms.size() - 1;
        }
        assert(!"Not processed ast node");

        return std::nullopt;
    };

    _generateSsaEq(astProgram, ssaSeq);

    return ssaSeq;
}
