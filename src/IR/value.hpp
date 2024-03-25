#ifndef IR_VALUE_HPP
#define IR_VALUE_HPP

#include <memory>
#include <vector>
#include <unordered_map>

class Type
{
public:
    enum class TypeID
    {
        UINT64,
        VOID
    };
    Type(TypeID typeID_) : typeID(typeID_) {}
    const TypeID typeID;
};

class Value
{
public:
    Value(Type ty_) : ty(ty_) {}
    virtual ~Value() {}
    virtual std::string pretty() const = 0;
    const Type ty;
    using SharedPtr = std::shared_ptr<Value>;
};

class ConstantInt : public Value
{
public:
    ConstantInt(uint64_t val_) : Value(Type::TypeID::UINT64), val(val_) {}
    std::string pretty() const override;

    const uint64_t val;
};

class Procedure : public Value
{
public:
    Procedure(std::string name_, std::vector<Type> argsTypes_, Type returnType)
        : Value(returnType), name(name_), argsTypes(argsTypes_)
    {
    }
    std::string pretty() const override;

    const std::string name;
    const std::vector<Type> argsTypes;
    using SharedPtr = std::shared_ptr<Procedure>;
};

#endif // IR_VALUE_HPP
