#include "inter_code_generator.hpp"

#include <functional>

// void SsaSeq::append(const SsaSeq &anotherSeq)
// {
//     forms.insert(forms.end(), anotherSeq.forms.begin(), anotherSeq.forms.end());
//     symbolTable->merge(*anotherSeq.symbolTable);
// }

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram)
{
    SsaSeq ret;
    std::function<void(AstNode::SharedPtr)> _generateSsaEq = [&_generateSsaEq, &ret](AstNode::SharedPtr astNode) -> void {
        if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
            for (auto child : astProgram->children) {
                _generateSsaEq(child);
            }
        } else if (auto astProcedure = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
            for (auto child : astProcedure->children) {
            }
        } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
        } else if (auto astNum = std::dynamic_pointer_cast<AstNum>(astNode)) {
        }
    };

    return ret;
}
