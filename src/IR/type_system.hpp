#ifndef IR_TYPE_SYSTEM_HPP
#define IR_TYPE_SYSTEM_HPP

#include "log.hpp"

#include <magic_enum.hpp>
#include <memory>
#include <vector>

class CompileTimeType;
enum class TypeID
{
    INT64,
    FLOAT,
    STRING,
    LABEL,
    VOID,
    PROCEDURE
};

inline std::string typeIdToString(TypeID typeID)
{
    return std::string(magic_enum::enum_name(typeID));
}

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

class CompileTimeType : public Type
{
public:
    using SharedPtr = std::shared_ptr<CompileTimeType>;

    CompileTimeType(TypeID typeID_) : typeID(typeID_) {}
    ~CompileTimeType() override {}

    static SharedPtr getNew(TypeID typeID_)
    {
        return std::make_shared<CompileTimeType>(typeID_);
    }

    bool knownInCompileTime() const override
    {
        return true;
    }

    bool isNumber() const
    {
        return typeID == TypeID::INT64 || typeID == TypeID::FLOAT;
    }

    bool isVoid() const override
    {
        return typeID == TypeID::VOID;
    }

    const TypeID typeID;
};

class RunTimeType : public Type
{
public:
    using SharedPtr = std::shared_ptr<RunTimeType>;

    RunTimeType() {}
    ~RunTimeType() override {}

    bool knownInCompileTime() const override
    {
        return false;
    }

    bool isVoid() const override
    {
        return false;
    }
};

using Types = std::vector<Type::SharedPtr>;
using CompileTimeTypes = std::vector<CompileTimeType::SharedPtr>;
using RunTimeTypes = std::vector<RunTimeType::SharedPtr>;

inline Types toTypes(const CompileTimeTypes &compileTimeTypes)
{
    Types ret;
    ret.reserve(compileTimeTypes.size());
    for (auto compileTimeType : compileTimeTypes) {
        ret.push_back(compileTimeType);
    }

    return ret;
}

inline Types toTypes(const RunTimeTypes &runTimeTypes)
{
    Types ret;
    ret.reserve(runTimeTypes.size());
    for (auto runTimeType : runTimeTypes) {
        ret.push_back(runTimeType);
    }

    return ret;
}

inline CompileTimeTypes toCompileTimeTypes(const Types &types)
{
    CompileTimeTypes ret;
    ret.reserve(types.size());
    for (auto ty : types) {
        auto curr = std::dynamic_pointer_cast<CompileTimeType>(ty);
        ASSERT(curr);
        ret.push_back(curr);
    }

    return ret;
}

inline bool containsRunTimeType(const Types &types)
{
    return std::any_of(types.begin(), types.end(),
                       [](Type::SharedPtr ty) { return !ty->knownInCompileTime(); });
}

#endif // IR_TYPE_SYSTEM_HPP
