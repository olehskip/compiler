#include "x64_nasm_generator.hpp"
#include "IR/procedure.hpp"
#include "log.hpp"

#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>

enum class Register
{
    RET,
    FIRST_ARG,
    SECOND_ARG,
    R11,
    R12
};

static Register getRegByArgIdx(uint8_t idx)
{
    switch (idx) {
        case 0:
            return Register::FIRST_ARG;
        case 1:
            return Register::SECOND_ARG;
        default:
            NOT_IMPLEMENTED;
    }
    SHOULD_NOT_HAPPEN;
    return Register();
}

static std::string getRegName(Register reg)
{
    switch (reg) {
        case Register::RET:
            return "rax";
        case Register::FIRST_ARG:
            return "rdi";
        case Register::SECOND_ARG:
            return "rsi";
        case Register::R11:
            return "R11";
        case Register::R12:
            return "R12";
        default:
            NOT_IMPLEMENTED;
    }
    return "";
}

class StackEntry
{
public:
    StackEntry(uint64_t offset_) : offset(offset_) {}

    std::string get() const
    {
        return "[rbp - " + std::to_string(offset) + "]";
    }

private:
    const uint64_t offset;
};

class StackAllocator
{
public:
    StackEntry allocate(Value::SharedPtr value)
    {
        currentOffset += 8;
        const StackEntry ret(currentOffset);
        const bool wasInserted = container.insert({value, ret}).second;
        ASSERT(wasInserted);
        return ret;
    }

    StackEntry getStackEntry(Value::SharedPtr value)
    {
        ASSERT(container.contains(value));
        return container.at(value);
    }

    uint64_t getTotalSize() const
    {
        return currentOffset;
    }

private:
    std::unordered_map<Value::SharedPtr, StackEntry> container;
    uint64_t currentOffset = 0;
};

class RodataEntry
{
public:
    RodataEntry(uint64_t id) : name("RODATA_" + std::to_string(id)) {}

    std::string get() const
    {
        return "[" + name + "]";
    }

    const std::string name;
};

class RodataAllocator
{
public:
    RodataEntry allocate(Value::SharedPtr value)
    {
        const RodataEntry ret(currentIdx++);
        const bool wasInserted = container.insert({value, ret}).second;
        ASSERT(wasInserted);
        return ret;
    }

    RodataEntry get(Value::SharedPtr value)
    {
        ASSERT(container.contains(value));
        return container.at(value);
    }

    RodataEntry getOrAllocate(Value::SharedPtr value)
    {
        const auto it = container.find(value);
        if (it != container.end()) {
            return it->second;
        }
        return allocate(value);
    }

private:
    std::unordered_map<Value::SharedPtr, RodataEntry> container;
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

static void movValueToReg(std::stringstream &body, Value::SharedPtr value, Register reg,
                          StackAllocator &stackAllocator, RodataAllocator &rodataAllocator)
{
    body << "mov " << getRegName(reg) << ", ";
    if (auto constInt = std::dynamic_pointer_cast<ConstantInt>(value)) {
        body << constInt->val;
    } else if (auto constString = std::dynamic_pointer_cast<ConstantString>(value)) {
        body << rodataAllocator.allocate(constString).name;
    } else if (auto procParam = std::dynamic_pointer_cast<ProcParameter>(value)) {
        body << getRegName(getRegByArgIdx(procParam->idx));
    } else {
        // TODO: redo it, it's stupid
        body << stackAllocator.getStackEntry(value).get();
    }

    body << "\n";
}

// TODO: moke it methods of Instruction
static void _generateX64Asm(SimpleBlock::SharedPtr simpleBlock, std::stringstream &body,
                            StackAllocator &stackAllocator, RodataAllocator &rodataAllocator,
                            bool isMain, bool isProcedure)
{
    if (isMain) {
        body << "mov rbp, rsp\n";
    } else if (isProcedure) {
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
                const auto reg = getRegByArgIdx(argIdx);
                movValueToReg(body, arg, reg, stackAllocator, rodataAllocator);
            }
            body << "call " << callInst->procedure->mangledName << "\n";
            if (!procedure->returnType->isVoid()) {
                body << "push rax\n";
                stackAllocator.allocate(callInst);
            }
        } else if (auto retInst = std::dynamic_pointer_cast<RetInst>(inst)) {
            movValueToReg(body, retInst->val, Register::RET, stackAllocator, rodataAllocator);
        } else if (auto condJumpInst = std::dynamic_pointer_cast<CondJumpInst>(inst)) {
            const auto testReg = Register::R11, oneReg = Register::R12;
            const auto testRegName = getRegName(testReg), oneRegName = getRegName(oneReg);
            const std::string thenBlockName = ".if" + condJumpInst->strid + "_then_block";
            const std::string elseBlockName = ".if" + condJumpInst->strid + "_else_block";
            const std::string endName = ".if" + condJumpInst->strid + "_end";
            movValueToReg(body, condJumpInst->valToTest, testReg, stackAllocator, rodataAllocator);
            body << "mov " << oneRegName << ", 1\n";
            body << "cmp " << testRegName << ", " << oneRegName << "\n";
            body << "je " << thenBlockName << "\n";
            body << "jne " << elseBlockName << "\n";

            StackAllocator thenBlockAllocator;
            body << thenBlockName << ":\n";
            _generateX64Asm(condJumpInst->thenBlock, body, thenBlockAllocator, rodataAllocator,
                            false, false);
            body << "jmp " << endName << "\n";

            body << elseBlockName << ":\n";
            if (condJumpInst->elseBlock) {
                StackAllocator elseBlockAllocator;
                _generateX64Asm(condJumpInst->elseBlock, body, elseBlockAllocator, rodataAllocator,
                                false, false);
            }

            body << endName << ":\n";

        } else {
            ASSERT("Not precessed ssa form type");
        }
    }
    if (isMain) {
        body << "mov rax, 60\n";
        body << "mov rdi, 0\n";
        body << "syscall\n";
    } else if (isProcedure) {
        addProcedureEpilogue(body);
        body << "ret\n";
    }

    body << "\n";
}

void generateX64Asm(SimpleBlock::SharedPtr mainSimpleBlock, std::stringstream &stream)
{
    ASSERT(mainSimpleBlock);

    std::stringstream header;
    // TODO: make it automatically
    header << "extern greaterINT64\n";
    header << "extern plusINT64\n";
    header << "extern displayINT64\n";
    header << "extern displaySTRING\n";
    header << "global _start\n\n";

    std::stringstream body;
    body << "section .text\n";

    RodataAllocator rodataAllocator;
    StackAllocator mainStackAllocator;
    body << "_start:\n";
    _generateX64Asm(mainSimpleBlock, body, mainStackAllocator, rodataAllocator, true, false);

    std::stack<SimpleBlock::SharedPtr> stack;
    stack.push(mainSimpleBlock);
    while (!stack.empty()) {
        const auto nextSimpleBlock = stack.top();
        ASSERT(nextSimpleBlock);
        stack.pop();
        const auto generalProcedureTable =
            nextSimpleBlock->symbolTable->getGeneralProceduresTable();
        for (const auto &[_, procedure] : generalProcedureTable) {
            body << procedure->mangledName << ":\n";
            StackAllocator procedureStackAllocator;
            _generateX64Asm(procedure->block, body, procedureStackAllocator, rodataAllocator, false,
                            true);
        }
    }

    header << "section .rodata\n";
    for (const auto &[value, rodataEntry] : rodataAllocator) {
        auto stringRodata = std::dynamic_pointer_cast<const ConstantString>(value);
        ASSERT_MSG(stringRodata, "Only string can appear in rodata so far");
        // `` is used so \n works
        header << rodataEntry.name + " db " + "`" + stringRodata->str + "`,0\n";
    }

    stream << header.str() << "\n" << body.str();
}
