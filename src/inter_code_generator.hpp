#ifndef INTER_CODE_GENERATOR_HPP
#define INTER_CODE_GENERATOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast_node.hpp"

class Procedure;

using VarIdx = uint64_t;

class SymbolTable
{
public:
    using SharedPtr = std::shared_ptr<SymbolTable>;

    std::unordered_map<std::string, Procedure> proceduresTable;
    std::unordered_map<VarIdx, int> variablesTable;
};

enum class SsaOp
{
    PLUS
};

enum class SsaFormType
{
    ASSIGN,
    CALL,
};

class SsaForm
{
public:
    const SsaFormType formType;

protected:
    SsaForm(SsaFormType formType_) : formType(formType_) {}
};

class SsaAssign : public SsaForm
{
public:
    SsaAssign(VarIdx outputVar_, VarIdx firstVar_, SsaOp op_, VarIdx secondVar_)
        : SsaForm(SsaFormType::ASSIGN), outputVar(outputVar_), firstVar(firstVar_), op(op_),
          secondVar(secondVar_)
    {
    }
    VarIdx outputVar;
    VarIdx firstVar;
    SsaOp op;
    VarIdx secondVar;
};

class SsaSeq
{
public:
    SymbolTable::SharedPtr symbolTable;
    std::vector<SsaForm> forms;
};

class Procedure
{
public:
    std::string name;
    unsigned int paramsCnt;
    SymbolTable::SharedPtr symbolTable;
    SsaSeq ssaSeq;
};

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram);

#endif // INTER_CODE_GENERATOR_HPP
