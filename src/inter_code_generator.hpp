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

using VarLabelOrValue = std::variant<VarLabel, uint64_t>;

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

enum class SsaFormType
{
    ALLOCA,
    STORE,
    LOAD,
    ASSIGN_LITERAL,
    OPERATION,
    CALL,
    PARAM,
};

class SsaForm
{
public:
    const SsaFormType formType;
    virtual ~SsaForm() {}

protected:
    SsaForm(SsaFormType formType_) : formType(formType_) {}
};

class SsaAlloca : public SsaForm
{
public:
    SsaAlloca(VarLabel varLabel_, uint64_t size_, uint64_t = 0)
        : SsaForm(SsaFormType::ALLOCA), varLabel(varLabel_), size(size_), alignment(0)
    {
    }

    const VarLabel varLabel;
    const uint64_t size;
    const uint64_t alignment;
};

// class SsaStore : public SsaForm
// {
// public:
//     SsaAlloca(VarLabel varLabel_, uint64_t size_, uint64_t = 0)
//         : SsaForm(SsaFormType::STORE), varLabel(varLabel_), size(size_), alignment(0)
//     {
//     }
//     const VarLabel varLabel;
// };

class SsaStoreLiteral : public SsaForm
{
public:
    SsaStoreLiteral(VarContent literal_) : SsaForm(SsaFormType::ASSIGN_LITERAL), literal(literal_)
    {
    }
    const VarContent literal;
};

class SsaOperation : public SsaForm
{
public:
    SsaOperation(FormIdx firstVar_, SsaOp op_, FormIdx secondVar_)
        : SsaForm(SsaFormType::OPERATION), firstVar(firstVar_), op(op_), secondVar(secondVar_)
    {
    }
    FormIdx firstVar;
    SsaOp op;
    FormIdx secondVar;
};

class SsaParam : public SsaForm
{
public:
    SsaParam(FormIdx var_, unsigned long long paramIdx_)
        : SsaForm(SsaFormType::PARAM), var(var_), paramIdx(paramIdx_)
    {
    }
    FormIdx var;
    unsigned long long paramIdx;
};

class SsaCall : public SsaForm
{
public:
    SsaCall(std::string procedureName_, unsigned long long paramsCnt_)
        : SsaForm(SsaFormType::CALL), procedureName(procedureName_), paramsCnt(paramsCnt_)
    {
    }
    std::string procedureName;
    unsigned long long paramsCnt;
};

class SsaSeq
{
public:
    SymbolTable::SharedPtr symbolTable;
    std::vector<std::shared_ptr<SsaForm>> forms;
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
