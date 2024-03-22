#ifndef INTER_CODE_GENERATOR_HPP
#define INTER_CODE_GENERATOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast_node.hpp"
#include "type_system.hpp"

class Procedure;

using FormIdx = uint64_t;
using VarContent = int64_t;

using VarLabel = uint64_t;

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

    const Type ty;
    using SharedPtr = std::shared_ptr<Value>;
};

class SymbolTable
{
public:
    using SharedPtr = std::shared_ptr<SymbolTable>;

    FormIdx putNewVar(VarContent varContent);

    std::unordered_map<std::string, Procedure> proceduresTable;
    // std::unordered_map<VarIdx, VarContent> variablesTable;
    std::vector<VarContent> variablesTable;

    const SharedPtr prevSymbolTable;
};

enum class SsaOp
{
    PLUS
};

class Instruction
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
        PARAM,
    };
    const InstType instType;
    virtual ~Instruction() {}

protected:
    Instruction(InstType instType_) : instType(instType_) {}
};

class AllocaInst : public Instruction
{
public:
    AllocaInst(VarLabel varLabel_, uint64_t size_, uint64_t = 0)
        : Instruction(InstType::ALLOCA), varLabel(varLabel_), size(size_), alignment(0)
    {
    }

    const VarLabel varLabel;
    const uint64_t size;
    const uint64_t alignment;
};

class StoreInst : public Instruction
{
public:
    StoreInst(Value::SharedPtr dst_, Value::SharedPtr src_)
        : Instruction(InstType::STORE), dst(dst_), src(src_)
    {
    }

private:
    Value::SharedPtr dst, src;
};

class LoadInst : public Instruction
{
public:
    LoadInst(Value::SharedPtr src_) : Instruction(InstType::LOAD), src(src_) {}

private:
    Value::SharedPtr src;
};

// class SsaOperation : public Instruction
// {
// public:
//     SsaOperation(FormIdx firstVar_, SsaOp op_, FormIdx secondVar_)
//         : Instruction(SsaFormType::OPERATION), firstVar(firstVar_), op(op_),
//         secondVar(secondVar_)
//     {
//     }
//     FormIdx firstVar;
//     SsaOp op;
//     FormIdx secondVar;
// };

class ParamInst : public Instruction
{
public:
    ParamInst(Value::SharedPtr value_, unsigned long long paramIdx_)
        : Instruction(InstType::PARAM), paramIdx(paramIdx_), value(value_)
    {
    }

private:
    const unsigned long long paramIdx;
    Value::SharedPtr value;
};

class CallInst : public Instruction
{
public:
    CallInst(std::string procedureName_, unsigned long long paramsCnt_)
        : Instruction(InstType::CALL), procedureName(procedureName_), paramsCnt(paramsCnt_)
    {
    }
    std::string procedureName;
    unsigned long long paramsCnt;
};

class SsaSeq
{
public:
    SymbolTable::SharedPtr symbolTable;
    std::vector<std::shared_ptr<Instruction>> forms;
    void pretty(std::stringstream &stream);
};

class Procedure
{
public:
    std::string name;
    SsaSeq ssaSeq;
    Type returnType;
};

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram);

#endif // INTER_CODE_GENERATOR_HPP
