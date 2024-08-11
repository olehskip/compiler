#ifndef IR_VALUE_HPP
#define IR_VALUE_HPP

#include "log.hpp"
#include "type_system.hpp"

#include <optional>
#include <vector>

class Value : public Printable
{
public:
    virtual ~Value() {}
    const Type::SharedPtr ty;
    using SharedPtr = std::shared_ptr<Value>;
    using SharedConstPtr = std::shared_ptr<const Value>;

    const uint64_t id;
    const std::string strid;
    const bool isConstant;

    virtual void refPretty(std::stringstream &stream) const override
    {
        stream << strid;
    }

protected:
    Value(Type::SharedPtr ty_, bool isConstant = false);
};

class Constant : public Value
{
public:
    using SharedPtr = std::shared_ptr<Constant>;
    using SharedConstPtr = std::shared_ptr<const Constant>;

    void refPretty(std::stringstream &stream) const override
    {
        pretty(stream);
    }

protected:
    Constant(Type::SharedPtr ty_) : Value(ty_, true){};
};

class ConstantInt : public Constant
{
public:
    ConstantInt(int64_t val_) : Constant(CompileTimeType::getNew(TypeID::INT64)), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const int64_t val;
};

class ConstantFloat : public Constant
{
public:
    ConstantFloat(uint64_t val_) : Constant(CompileTimeType::getNew(TypeID::FLOAT)), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const uint64_t val;
};

class ConstantString : public Constant
{
public:
    ConstantString(std::string str_) : Constant(CompileTimeType::getNew(TypeID::STRING)), str(str_)
    {
    }
    void pretty(std::stringstream &stream) const override;

    const std::string str;
};

#endif // IR_VALUE_HPP
