#include "x64_nasm_generator.hpp"

#include <cassert>

#include <sstream>

class Heap
{
public:
    using HeapOffset = unsigned long long;
    using AllocationSize = unsigned long long;
    struct Allocation
    {
        HeapOffset offset;
        AllocationSize size;
    };

    Allocation allocate(AllocationSize size)
    {
        assert(currentOffset + size <= HEAP_SIZE);
        const Allocation ret{currentOffset, size};
        currentOffset += size;
        return ret;
    }

    static constexpr AllocationSize HEAP_SIZE = 1024;

private:
    HeapOffset currentOffset = 0;
};

void generateX64Asm(SsaSeq &ssaSeq, std::stringstream &stream)
{
    Heap heap;
    std::unordered_map<FormIdx, Heap::Allocation> allocations;

    stream << "extern display_int\n";
    stream << "extern plus_int\n";
    stream << "%define HEAP_SIZE " << heap.HEAP_SIZE << "\n";
    stream << "section .bss\n";
    stream << "heap resb HEAP_SIZE\n";
    stream << "section .text\n";
    stream << "global _start\n";
    stream << "_start:\n";
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
            auto allocation = heap.allocate(8);
            allocations.insert({idx, allocation});
            stream << "mov QWORD [heap + " << allocation.offset << "], rax\n";
            for (size_t i = 0; i < ssaCall->paramsCnt; ++i) {
                stream << "pop rax\n";
            }
        } else if (auto ssaAssignLiteral = std::dynamic_pointer_cast<SsaAssignLiteral>(form)) {
            auto allocation = heap.allocate(8);
            allocations.insert({idx, allocation});
            stream << "mov QWORD [heap + " << allocation.offset << "], "
                   << ssaAssignLiteral->literal << "\n";
        } else if (auto ssaParam = std::dynamic_pointer_cast<SsaParam>(form)) {
            const FormIdx varIdxToPush = ssaParam->var;
            assert(forms.size() > varIdxToPush);
            const auto paramForm = forms[varIdxToPush];
            assert(paramForm);
            Heap::Allocation allocation = allocations.at(varIdxToPush);
            stream << "push QWORD [heap + " << allocation.offset << "]\n";
        } else {
            assert(!"Not precessed ssa form type");
        }
    }

    stream << "mov rax, 60\n";
    stream << "mov rdi, 0\n";
    stream << "syscall\n";
}
