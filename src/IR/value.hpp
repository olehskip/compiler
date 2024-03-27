#ifndef IR_VALUE_HPP
#define IR_VALUE_HPP

#include <memory>
#include <unordered_map>
#include <vector>

class Type
{
public:
    enum class TypeID
    {
        UINT64,
        FLOAT,
        VOID,
    };
    const TypeID typeID;
    Type(TypeID typeID_) : typeID(typeID_) {}

    bool isNumber()
    {
        return typeID == TypeID::UINT64 || typeID == TypeID::FLOAT;
    }
};

class TypeManager
{
public:
    TypeManager() {}

    template <class T, std::enable_if<std::is_base_of_v<Type, T>, bool> = true>
    std::shared_ptr<T> getType()
    {
    }

private:
    std::vector<Type> types;
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

class ConstantFloat : public Value
{
public:
    ConstantFloat(uint64_t val_) : Value(Type::TypeID::FLOAT), val(val_) {}
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
