#ifndef IR_INSTRUCTIONS_HPP
#define IR_INSTRUCTIONS_HPP

#include "procedure.hpp"

#include <string>
#include <vector>

class Instruction : public Value
{
public:
    enum class InstType
    {
        ALLOCA,
        STORE,
        LOAD,
        // ASSIGN_LITERAL,
        // OPERATION,
        CALL,
    };
    const InstType instType;
    virtual ~Instruction() {}

    using SharedPtr = std::shared_ptr<Instruction>;

protected:
    Instruction(InstType instType_, Type typeID) : Value(typeID), instType(instType_) {}
};

class AllocaInst : public Instruction
{
public:
    AllocaInst(Type ty) : Instruction(InstType::ALLOCA, ty) {}
};

class StoreInst : public Instruction
{
public:
    StoreInst(Value::SharedPtr dst_, Value::SharedPtr src_)
        : Instruction(InstType::STORE, Type(Type::TypeID::VOID)), dst(dst_), src(src_)
    {
    }

private:
    Value::SharedPtr dst, src;
};

class LoadInst : public Instruction
{
public:
    LoadInst(Type ty, Value::SharedPtr src_) : Instruction(InstType::LOAD, ty), src(src_) {}

private:
    Value::SharedPtr src;
};

class CallInst : public Instruction
{
public:
    CallInst(Procedure::SharedPtr procedure_, std::vector<Value::SharedPtr> args_,
             std::string procedureName_)
        : Instruction(InstType::CALL, procedure_->ty), procedure(procedure_), args(args_),
          procedureName(procedureName_)
    {
    }

    void pretty(std::stringstream &stream) const override;

    Procedure::SharedPtr procedure;
    std::vector<Value::SharedPtr> args;
    std::string procedureName;
};

#endif // IR_INSTRUCTIONS_HPP
