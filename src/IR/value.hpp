#ifndef IR_VALUE_HPP
#define IR_VALUE_HPP

#include "log.hpp"

#include <memory>
#include <optional>
#include <vector>

class CompileTimeKnownType;
enum class TypeID
{
    UINT64,
    FLOAT,
    LABEL,
    VOID,
    PROCEDURE,
};

/* It isn't always possile to inference all the types in compile time, so Type can also store a type
 * that is to be deduced in runtime
 */
class Type
{
public:
    virtual ~Type(){};
    virtual bool knownInCompileTime() const = 0;
    using SharedPtr = std::shared_ptr<Type>;

    // we can always tell if a type is void, because
    // type is never void if it cannot be deduced in compile time
    virtual bool isVoid() const = 0;

protected:
    Type() {}
};

class CompileTimeKnownType : public Type
{
public:
    using SharedPtr = std::shared_ptr<Type>;

    CompileTimeKnownType(TypeID typeID_)
    {
        typeID = typeID_;
    }
    ~CompileTimeKnownType() override {}

    static SharedPtr getNew(TypeID typeID_)
    {
        return std::make_shared<CompileTimeKnownType>(typeID_);
    }

    bool knownInCompileTime() const override
    {
        return true;
    }

    TypeID getTypeID() const
    {
        return typeID;
    }

    bool isNumber() const
    {
        return typeID == TypeID::UINT64 || typeID == TypeID::FLOAT;
    }

    bool isVoid() const override
    {
        return typeID == TypeID::VOID;
    }

private:
    TypeID typeID;
};

class RunTimeKnownType : public Type
{
public:
    RunTimeKnownType() {}
    ~RunTimeKnownType() override {}

    bool knownInCompileTime() const override
    {
        return false;
    }

    bool isVoid() const override
    {
        return false;
    }
};

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
    ConstantInt(uint64_t val_) : Value(CompileTimeKnownType::getNew(TypeID::UINT64)), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const uint64_t val;
};

class ConstantFloat : public Value
{
public:
    ConstantFloat(uint64_t val_) : Value(CompileTimeKnownType::getNew(TypeID::FLOAT)), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const uint64_t val;
};

#endif // IR_VALUE_HPP
