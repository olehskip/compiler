#include "x64_nasm_generator.hpp"
#include "IR/procedure.hpp"
#include "log.hpp"

#include <sstream>

class Stack
{
public:
    StackRegister allocate(Value::SharedPtr value)
    {
        currentOffset += 8;
        const StackRegister ret(currentOffset);
        const bool wasInserted = container.insert({value, ret}).second;
        ASSERT(wasInserted);
        return ret;
    }

    StackRegister getStackRegister(Value::SharedPtr value)
    {
        ASSERT(container.contains(value));
        return container.at(value);
    }

    uint64_t getTotalSize() const
    {
        return currentOffset;
    }

private:
    std::unordered_map<Value::SharedPtr, StackRegister> container;
    uint64_t currentOffset = 0;
};

static std::string getParamRegisterName(unsigned long long paramIdx)
{
    switch (paramIdx) {
        case 0:
            return "rdi";
        case 1:
            return "rsi";
    }

    ASSERT("Invalid paramIdx");
    return {};
}

void generateX64Asm(SimpleBlock::SharedPtr mainSimpleBlock, std::stringstream &stream)
{
    ASSERT(mainSimpleBlock);
    Stack stack;

    stream << "extern display_int\n";
    stream << "extern plus_int\n";
    stream << "section .text\n";
    stream << "global _start\n";
    stream << "_start:\n";
    stream << "mov rbp, rsp\n";
    for (auto inst : mainSimpleBlock->insts) {
        if (auto callInst = std::dynamic_pointer_cast<CallInst>(inst)) {
            std::vector<Type> argsTypes;
            for (auto arg : callInst->args) {
                argsTypes.push_back(arg->ty);
            }
            auto procedure = callInst->procedure;
            ASSERT(procedure);
            std::string asmProcedureName;
            if (callInst->procedureName == "+") {
                asmProcedureName = "plus_int";
            } else if (callInst->procedureName == "display") {
                asmProcedureName = "display_int";
            }
            ASSERT(asmProcedureName.size());
            for (size_t argIdx = 0; argIdx < callInst->args.size(); ++argIdx) {
                auto arg = callInst->args[argIdx];
                if (auto constIntArg = std::dynamic_pointer_cast<ConstantInt>(arg)) {
                    stream << "mov " << getParamRegisterName(argIdx) << ", " << constIntArg->val
                           << "\n";
                } else {
                    auto registerVal = stack.getStackRegister(arg);
                    stream << "mov " << getParamRegisterName(argIdx) << ", "
                           << registerVal.getData() << "\n";
                }
            }
            stream << "call " << asmProcedureName << "\n";
            if (!procedure->ty.isVoid()) {
                stream << "push rax\n";
                stack.allocate(callInst);
            }
        } else {
            ASSERT("Not precessed ssa form type");
        }
    }

    stream << "mov rax, 60\n";
    stream << "mov rdi, 0\n";
    stream << "syscall\n";
}
