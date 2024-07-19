#include "x64_nasm_generator.hpp"
#include "IR/procedure.hpp"
#include "log.hpp"

#include <sstream>
#include <unordered_map>
#include <unordered_set>

enum class Register
{
    RET,
    FIRST_ARG,
    SECOND_ARG,
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
        default:
            NOT_IMPLEMENTED;
    }
    return "";
}

class ValueStored
{
public:
    using SharedPtr = std::shared_ptr<ValueStored>;
    ValueStored(Value::SharedConstPtr value_, bool isPreserved_ = false)
        : isPreserved(isPreserved_), value(value_)
    {
    }

    std::string getData() const
    {
        if (!regLocs.empty()) {
            return getRegName(*regLocs.begin());
        } else if (isRodata) {
            return "RODATA_" + std::to_string(value->id);
        }
        ASSERT("value is not stored");
        return "";
    }

    std::string getRodata() const
    {
        ASSERT(isRodata);
        return "RODATA_" + std::to_string(value->id);
    }

    void storeToRegister(Register reg)
    {
        regLocs.insert(reg);
    }

    void evictFromRegister(Register reg)
    {
        ASSERT_MSG(regLocs.contains(reg),
                   "evictFromRegister was requested, but value is not in any register");
        regLocs.erase(reg);
        if (!isRodata && isPreserved) {
            NOT_IMPLEMENTED;
        }
    }

    void putToRoData()
    {
        isRodata = true;
    }

    const bool isPreserved; // saved to stack on eviction if true

    const bool isJustValue = false;

    const Value::SharedConstPtr value;

private:
    std::unordered_set<Register> regLocs;
    // std::unordered_set<Stack> stacks;
    bool isRodata = false;
};

class ValueKeeper
{
public:
    ValueStored::SharedPtr storeToRodata(Constant::SharedConstPtr value)
    {
        ASSERT(value);
        auto valueStored = getOrCreateValueStored(value);
        valueStored->putToRoData();
        rodata.insert(valueStored);
        return valueStored;
    }

    bool storeToReg(Value::SharedConstPtr value, Register reg, bool forced)
    {
        ASSERT(value);
        auto previousRegData = registersData[reg];
        if (previousRegData && forced) {
            previousRegData->evictFromRegister(reg);
        }
        if (!previousRegData || forced) {
            auto valueStored = getOrCreateValueStored(value);
            valueStored->storeToRegister(reg);
            return true;
        }

        return false;
    }

    bool movRegToReg(Register src, Register dest, bool forced)
    {
        auto srcValueStored = registersData[src];
        ASSERT_MSG(srcValueStored, "requested to mov value from reg to reg, but src reg is empty");
        return storeToReg(srcValueStored->value, dest, forced);
    }

    // void newStack() {}

private:
    ValueStored::SharedPtr getOrCreateValueStored(Value::SharedConstPtr value)
    {
        ASSERT(value);
        auto valueStored = allValues[value];
        if (!valueStored) {
            valueStored = std::make_shared<ValueStored>(value);
            allValues[value] = valueStored;
        }
        return valueStored;
    }

    std::unordered_map<Value::SharedConstPtr, ValueStored::SharedPtr> allValues;
    std::unordered_set<ValueStored::SharedPtr> rodata;
    std::unordered_map<Register, ValueStored::SharedPtr> registersData;

public:
    const decltype(rodata) &getAllRodata()
    {
        return rodata;
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

static void storeValueToRegAndEmit(std::stringstream &body, Value::SharedPtr val, Register reg,
                                   ValueKeeper &valueKeeper, bool isForced = false)
{
    const bool wasInserted = valueKeeper.storeToReg(val, reg, isForced);
    if (wasInserted) {
        body << "mov " << getRegName(reg) << ", ";
        if (auto constInt = std::dynamic_pointer_cast<ConstantInt>(val)) {
            body << constInt->val;
        } else if (auto constString = std::dynamic_pointer_cast<ConstantString>(val)) {
            body << valueKeeper.storeToRodata(constString).get();
        } else {
            NOT_IMPLEMENTED;
        }

        body << "\n";
    }
}

// TODO: moke it methods of Instruction
static void _generateX64Asm(SimpleBlock::SharedPtr simpleBlock, std::stringstream &body,
                            ValueKeeper &valueKeeper, bool isMain = false)
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
                const auto reg = getRegByArgIdx(argIdx);
                storeValueToRegAndEmit(body, arg, reg, valueKeeper, true);
            }
            body << "call " << callInst->procedure->mangledName << "\n";
            if (!procedure->returnType->isVoid()) {
                valueKeeper.storeToReg(procedure, Register::RET, true);
            }
        } else if (auto retInst = std::dynamic_pointer_cast<RetInst>(inst)) {
            // valueKeeper.storeToReg(retInst->val, Register::RET, true);
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
    ValueKeeper valueKeeper;

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
            _generateX64Asm(procedure->block, body, valueKeeper);
        }
        // nextSimpleBlock->symbolTable->prc
        nextSimpleBlock = mainSimpleBlock->parent;
    }

    body << "_start:\n";
    body << "mov rbp, rsp\n";
    _generateX64Asm(mainSimpleBlock, body, valueKeeper, true);

    header << "section .rodata\n";
    const auto rodata = valueKeeper.getAllRodata();
    for (const auto &storedValue : rodata) {
        auto constStringArg = std::dynamic_pointer_cast<const ConstantString>(storedValue->value);
        ASSERT_MSG(constStringArg, "Only string can appear in rodata so far");
        // `` is used so \n works
        header << storedValue->getRodata() + " db " + "`" + constStringArg->str + "`,0\n";
    }

    std::stringstream end;
    end << "mov rax, 60\n";
    end << "mov rdi, 0\n";
    end << "syscall\n";

    stream << header.str() << "\n" << body.str() << end.str();
}
