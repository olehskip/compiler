#include "x64_nasm_generator.hpp"

#include <cassert>

#include <sstream>

class Stack
{
public:
    struct Allocation
    {
        using Offset = unsigned long long;
        using Size = unsigned long long;
        Offset offset;
        Size size;
    };

    Allocation allocate(FormIdx formIdx, Allocation::Size size)
    {
        currentOffset += size;
        const Allocation ret{currentOffset, size};
        const bool wasInserted = formStackOffset.insert({formIdx, ret}).second;
        assert(wasInserted);
        return ret;
    }

    Allocation getAllocation(FormIdx formIdx)
    {
        return formStackOffset.at(formIdx);
    }

    Allocation::Offset getTotalSize() const
    {
        return currentOffset;
    }

private:
    std::unordered_map<FormIdx, Allocation> formStackOffset;
    Allocation::Offset currentOffset = 0;
};

static std::string getParamRegisterName(unsigned long long paramIdx)
{
    switch (paramIdx) {
        case 0:
            return "rdi";
        case 1:
            return "rsi";
    }

    assert(!"Invalid paramIdx");
}

void generateX64Asm(SsaSeq &ssaSeq, std::stringstream &stream)
{
    Stack stack;
    // std::unordered_map<
    // Heap heap;
    // std::unordered_map<FormIdx, Heap::Allocation> allocations;

    stream << "extern display_int\n";
    stream << "extern plus_int\n";
    stream << "section .text\n";
    stream << "global _start\n";
    stream << "_start:\n";
    stream << "mov rbp, rsp\n";
    auto forms = ssaSeq.forms;
    for (size_t idx = 0; idx < forms.size(); ++idx) {
        auto form = forms[idx];
        if (auto ssaCall = std::dynamic_pointer_cast<SsaCall>(form)) {
            std::string asmProcedureName;
            if (ssaCall->procedureName == "+") {
                asmProcedureName = "plus_int";
            } else if (ssaCall->procedureName == "display") {
                asmProcedureName = "display_int";
            }
            assert(asmProcedureName.size());
            stream << "call " << asmProcedureName << "\n";
            stack.allocate(idx, 8);
            stream << "push rax\n";
        } else if (auto ssaAssignLiteral = std::dynamic_pointer_cast<SsaAssignLiteral>(form)) {
        } else if (auto ssaParam = std::dynamic_pointer_cast<SsaParam>(form)) {
            const FormIdx varIdxToPush = ssaParam->var;
            assert(forms.size() > varIdxToPush);
            const auto paramForm = forms[varIdxToPush];
            assert(paramForm);
            auto registerName = getParamRegisterName(ssaParam->paramIdx);
            if (auto literalParamForm = std::dynamic_pointer_cast<SsaAssignLiteral>(paramForm)) {
                stream << "mov " << registerName << ", " << literalParamForm->literal << "\n";
            } else {
                auto allocation = stack.getAllocation(varIdxToPush);
                stream << "mov " << registerName << ", [rbp - " << allocation.offset << "]\n";
            }
        } else {
            assert(!"Not precessed ssa form type");
        }
    }

    stream << "mov rax, 60\n";
    stream << "mov rdi, 0\n";
    stream << "syscall\n";
}
