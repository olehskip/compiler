#include "x64_nasm_generator.hpp"
#include "IR/procedure.hpp"
#include "log.hpp"

#include <sstream>

class StackAllocator
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

class RodataAllocator
{
public:
    RoDataRegister allocate(Value::SharedPtr value)
    {
        const RoDataRegister ret(currentIdx++);
        const bool wasInserted = container.insert({value, ret}).second;
        ASSERT(wasInserted);
        return ret;
    }

    RoDataRegister get(Value::SharedPtr value)
    {
        ASSERT(container.contains(value));
        return container.at(value);
    }

    RoDataRegister getOrAllocate(Value::SharedPtr value)
    {
        const auto it = container.find(value);
        if (it != container.end()) {
            return it->second;
        }
        return allocate(value);
    }

private:
    std::unordered_map<Value::SharedPtr, RoDataRegister> container;
    uint64_t currentIdx = 0;

    using ContainerIterator = decltype(std::begin(container));

public:
    ContainerIterator begin()
    {
        return container.begin();
    }
    ContainerIterator end()
    {
        return container.end();
    }
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

static void addProcedurePrologue(std::stringstream &stream)
{
    stream << "push rbp ; prologue #2\n";
    stream << "mov rbp, rsp ; prologue #2\n";
}

static void addProcedureEpilogue(std::stringstream &stream)
{
    stream << "mov rsp, rbp ; epilogue #1\n";
    stream << "pop rbp ; prologue #2\n";
}

// TODO: moke it methods of Instruction
static void _generateX64Asm(SimpleBlock::SharedPtr simpleBlock, std::stringstream &body,
                            RodataAllocator &rodata, StackAllocator &stack, bool isMain = false)
{
    if (!isMain) {
        addProcedurePrologue(body);
    }
    for (auto inst : simpleBlock->insts) {
        if (auto callInst = std::dynamic_pointer_cast<CallInst>(inst)) {
            std::vector<Type::SharedPtr> argsTypes;
            for (auto arg : callInst->args) {
                argsTypes.push_back(arg->ty);
            }
            auto procedure = callInst->procedure;
            ASSERT(procedure);
            for (size_t argIdx = 0; argIdx < callInst->args.size(); ++argIdx) {
                auto arg = callInst->args[argIdx];
                if (auto constIntArg = std::dynamic_pointer_cast<ConstantInt>(arg)) {
                    body << "mov " << getParamRegisterName(argIdx) << ", " << constIntArg->val
                         << "\n";
                } else if (auto constStringArg = std::dynamic_pointer_cast<ConstantString>(arg)) {
                    auto registerVal = rodata.getOrAllocate(constStringArg);
                    body << "mov " << getParamRegisterName(argIdx) << ", " << registerVal.getPtr()
                         << "\n";
                } else {
                    auto registerVal = stack.getStackRegister(arg);
                    body << "mov " << getParamRegisterName(argIdx) << ", " << registerVal.getData()
                         << "\n";
                }
            }
            body << "call " << callInst->procedure->mangledName << "\n";
            if (!procedure->returnType->isVoid()) {
                body << "push rax\n";
                stack.allocate(callInst);
            }
        } else {
            ASSERT("Not precessed ssa form type");
        }
    }
    if (!isMain) {
        addProcedureEpilogue(body);
        body << "ret\n";
    }
}

void generateX64Asm(SimpleBlock::SharedPtr mainSimpleBlock, std::stringstream &stream)
{
    ASSERT(mainSimpleBlock);
    StackAllocator stack;
    RodataAllocator rodata;

    std::stringstream header;
    // TODO: make it automatically
    header << "extern plusINT64\n";
    header << "extern displayINT64\n";
    header << "extern displaySTRING\n";
    header << "global _start\n";

    std::stringstream body;
    body << "section .text\n";
    auto nextSimpleBlock = mainSimpleBlock;
    while (nextSimpleBlock) {
        const auto generalProcedureTable =
            nextSimpleBlock->symbolTable->getGeneralProceduresTable();
        for (const auto &[name, procedure] : generalProcedureTable) {
            body << name << ":\n";
            _generateX64Asm(procedure->block, body, rodata, stack);
        }
        // nextSimpleBlock->symbolTable->prc
        nextSimpleBlock = mainSimpleBlock->parent;
    }

    body << "_start:\n";
    body << "mov rbp, rsp\n";
    _generateX64Asm(mainSimpleBlock, body, rodata, stack, true);

    header << "section .rodata\n";
    for (const auto &allocation : rodata) {
        auto constStringArg = std::dynamic_pointer_cast<ConstantString>(allocation.first);
        ASSERT_MSG(constStringArg, "Only string can appear in rodata so far");
        // `` is used so \n works
        header << allocation.second.name + " db " + "`" + constStringArg->str + "`,0\n";
    }

    std::stringstream end;
    end << "mov rax, 60\n";
    end << "mov rdi, 0\n";
    end << "syscall\n";

    stream << header.str() << body.str() << end.str();
}
