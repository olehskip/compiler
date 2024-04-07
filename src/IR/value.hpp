#ifndef IR_VALUE_HPP
#define IR_VALUE_HPP

#include "log.hpp"
#include "type_system.hpp"

#include <optional>
#include <vector>

class Value
{
public:
    Value(Type::SharedPtr ty_) : ty(ty_) {}
    virtual ~Value() {}
    virtual void pretty(std::stringstream &stream) const = 0;
    const Type::SharedPtr ty;
    using SharedPtr = std::shared_ptr<Value>;
};

class ConstantInt : public Value
{
public:
    ConstantInt(int64_t val_) : Value(CompileTimeType::getNew(TypeID::INT64)), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const int64_t val;
};

class ConstantFloat : public Value
{
public:
    ConstantFloat(uint64_t val_) : Value(CompileTimeType::getNew(TypeID::FLOAT)), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const uint64_t val;
};

class ConstantString : public Value
{
public:
    ConstantString(std::string str_) : Value(CompileTimeType::getNew(TypeID::STRING)), str(str_) {}
    void pretty(std::stringstream &stream) const override;

    const std::string str;
};

#endif // IR_VALUE_HPP
