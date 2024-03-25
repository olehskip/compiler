#ifndef INTER_CODE_GENERATOR_HPP
#define INTER_CODE_GENERATOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast_node.hpp"
#include "instructions.hpp"

class Procedure;

using FormIdx = uint64_t;
using VarContent = int64_t;

using VarLabel = uint64_t;

class SymbolTable
{
public:
    using SharedPtr = std::shared_ptr<SymbolTable>;

    FormIdx putNewVar(VarContent varContent);

    std::unordered_map<std::string, Procedure::SharedPtr> proceduresTable;
    void addNewProcedure(Procedure::SharedPtr procedure);
    std::vector<VarContent> variablesTable;

    const SharedPtr prevSymbolTable;
};

enum class SsaOp
{
    PLUS
};

class SsaSeq
{
public:
    SymbolTable::SharedPtr symbolTable;
    std::vector<std::shared_ptr<Instruction>> insts;
    void pretty(std::stringstream &stream);
};

SsaSeq generateSsaSeq(AstProgram::SharedPtr astProgram);

#endif // INTER_CODE_GENERATOR_HPP
