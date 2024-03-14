#include "inter_code_generator.hpp"

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
        } else if (auto ssaAssignLiteral = std::dynamic_pointer_cast<SsaAssignLiteral>(form)) {
            stream << "ASSIGN_LITERAL " << ssaAssignLiteral->literal << "\n";
        } else if (auto ssaParam = std::dynamic_pointer_cast<SsaParam>(form)) {
            stream << "PARAM " << ssaParam->var << "\n";
        } else {
            assert(!"Not processed ssa form");
        }
    }
}

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram)
{
    SsaSeq ret;
    std::function<std::optional<FormIdx>(AstNode::SharedPtr)> _generateSsaEq =
        [&_generateSsaEq, &ret](AstNode::SharedPtr astNode) -> std::optional<FormIdx> {
        if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
            for (auto child : astProgram->children) {
                _generateSsaEq(child);
            }
            return std::nullopt;
        } else if (auto astProcedure = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
            const size_t childrenSize = astProcedure->children.size();
            for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
                auto childMaybeFormIdx = _generateSsaEq(astProcedure->children[childIdx]);
                assert(childMaybeFormIdx);
                ret.forms.push_back(std::make_shared<SsaParam>(*childMaybeFormIdx, childIdx));
            }
            ret.forms.push_back(std::make_shared<SsaCall>(astProcedure->name, childrenSize));
            return ret.forms.size() - 1;
        } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
            // ret.forms.push_back(SsaCall{astProcedure->name, childrenSize});
            // return ret.forms.size() - 1;
            assert(!"Not implemented yet");
            return std::nullopt;
        } else if (auto astNum = std::dynamic_pointer_cast<AstNum>(astNode)) {
            ret.forms.push_back(std::make_shared<SsaAssignLiteral>(astNum->num));
            return ret.forms.size() - 1;
        }
        assert(!"Not processed ast node");
        return std::nullopt;
    };

    _generateSsaEq(astProgram);

    return ret;
}
