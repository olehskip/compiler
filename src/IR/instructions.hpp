#ifndef IR_INSTRUCTIONS_HPP
#define IR_INSTRUCTIONS_HPP

#include "IR/procedure.hpp"

#include <memory>
#include <string>
#include <vector>

enum class InstType
{
    ALLOCA,
    STORE,
    LOAD,
    // ASSIGN_LITERAL,
    // OPERATION,
    CALL,
    RET,
    COND_JUMP
};

class Instruction : public Value
{
public:
    const InstType instType;
    virtual ~Instruction() {}

    using SharedPtr = std::shared_ptr<Instruction>;
    void refPretty(std::stringstream &stream) const override
    {
        stream << strid;
    }

protected:
    Instruction(InstType instType_, Type::SharedPtr ty) : Value(ty), instType(instType_) {}
};

class AllocaInst : public Instruction
{
public:
    AllocaInst(Type::SharedPtr ty) : Instruction(InstType::ALLOCA, ty) {}
};

class StoreInst : public Instruction
{
public:
    StoreInst(Value::SharedPtr dst_, Value::SharedPtr src_)
        : Instruction(InstType::STORE, CompileTimeType::getNew(TypeID::VOID)), dst(dst_), src(src_)
    {
    }

private:
    Value::SharedPtr dst, src;
};

class LoadInst : public Instruction
{
public:
    LoadInst(Type::SharedPtr ty, Value::SharedPtr src_) : Instruction(InstType::LOAD, ty), src(src_)
    {
    }

private:
    Value::SharedPtr src;
};

class CallInst : public Instruction
{
public:
    CallInst(Procedure::SharedPtr procedure_, std::vector<Value::SharedPtr> args_)
        : Instruction(InstType::CALL, procedure_->returnType), procedure(procedure_), args(args_)
    {
    }

    void pretty(std::stringstream &stream) const override;

    const Procedure::SharedPtr procedure;
    const std::vector<Value::SharedPtr> args;
};

class RetInst : public Instruction
{
public:
    RetInst(Value::SharedPtr val_) : Instruction(InstType::RET, val_->ty), val(val_) {}

    void pretty(std::stringstream &stream) const override;

    const Value::SharedPtr val;
};

class CondJumpInst : public Instruction
{
public:
    CondJumpInst(Value::SharedPtr valToTest_, std::shared_ptr<SimpleBlock> thenBlock_,
                 std::shared_ptr<SimpleBlock> elseBlock_)
        : Instruction(InstType::COND_JUMP, CompileTimeType::getNew(TypeID::VOID)),
          valToTest(valToTest_), thenBlock(thenBlock_), elseBlock(elseBlock_)
    {
    }
    void pretty(std::stringstream &stream) const override;

    const Value::SharedPtr valToTest;
    const std::shared_ptr<SimpleBlock> thenBlock;
    const std::shared_ptr<SimpleBlock> elseBlock;
};

#endif // IR_INSTRUCTIONS_HPP
