#ifndef IR_VALUE_HPP
#define IR_VALUE_HPP

#include "log.hpp"

#include <memory>
#include <optional>
#include <vector>

/* It isnt' always possile to inference all the types in compile time, so Type can also indicate
 * that it doesn't know the type, however we always know whether it is void or not
 */
class Type
{
public:
    enum class TypeID
    {
        UINT64,
        FLOAT,
        LABEL,
        VOID,
    };

    Type() {}
    Type(TypeID typeID_)
    {
        typeID = typeID_;
    }

    static Type createRuntimeType()
    {
        return Type();
    }

    bool knownInCompileTime() const
    {
        return typeID.has_value();
    }

    // throws assertion if typeID cannot be infered in compile time
    TypeID getTypeID() const
    {
        assertKnownInCompileTime();
        return *typeID;
    }

    bool isNumber() const
    {
        assertKnownInCompileTime();
        return typeID == TypeID::UINT64 || typeID == TypeID::FLOAT;
    }

    // if type is uknown in compile time it means it can't be void
    bool isVoid() const
    {
        return knownInCompileTime() && typeID == TypeID::VOID;
    }

private:
    void assertKnownInCompileTime() const
    {
        ASSERT_MSG(knownInCompileTime(), "Type ID cannot be infered in compile time");
    }

    std::optional<TypeID> typeID =
        std::nullopt; // typeId is nullopt means it can't be infered in compile time
};

class Value
{
public:
    Value(Type ty_) : ty(ty_) {}
    virtual ~Value() {}
    virtual void pretty(std::stringstream &stream) const = 0;
    const Type ty;
    using SharedPtr = std::shared_ptr<Value>;
};

class ConstantInt : public Value
{
public:
    ConstantInt(uint64_t val_) : Value(Type::TypeID::UINT64), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const uint64_t val;
};

class ConstantFloat : public Value
{
public:
    ConstantFloat(uint64_t val_) : Value(Type::TypeID::FLOAT), val(val_) {}
    void pretty(std::stringstream &stream) const override;

    const uint64_t val;
};

#endif // IR_VALUE_HPP
