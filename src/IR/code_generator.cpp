#include "code_generator.hpp"
#include "log.hpp"

#include <sstream>

void SimpleBlock::pretty(std::stringstream &stream) const // override
{
    for (size_t idx = 0; idx < insts.size(); ++idx) {
        stream << "$" << std::to_string(uint64_t(insts[idx].get())) << " = ";
        insts[idx]->pretty(stream);
        stream << "\n";
    }
}

static bool containsRunTimeKnownType(const std::vector<Type::SharedPtr> &types)
{
    return std::any_of(types.begin(), types.end(),
                       [](Type::SharedPtr ty) { return !ty->knownInCompileTime(); });
}

static Value::SharedPtr _generateSsaEq(AstNode::SharedPtr astNode,
                                       SimpleBlock::SharedPtr simpleBlock,
                                       SymbolTable::SharedPtr symbolTable)
{
    ASSERT(astNode);
    ASSERT(simpleBlock);
    if (auto astProgram = std::dynamic_pointer_cast<AstProgram>(astNode)) {
        for (auto child : astProgram->children) {
            _generateSsaEq(child, simpleBlock, symbolTable);
        }
        return nullptr;
    } else if (auto astBeginExpr = std::dynamic_pointer_cast<AstBeginExpr>(astNode)) {
        auto newSymbolTable = std::make_shared<SymbolTable>(symbolTable);
        Value::SharedPtr lastChildProcessed;
        for (auto child : astBeginExpr->children) {
            lastChildProcessed = _generateSsaEq(child, simpleBlock, newSymbolTable);
        }
        ASSERT(lastChildProcessed);
        return lastChildProcessed;
    } else if (auto astProcedureDef = std::dynamic_pointer_cast<AstProcedureDef>(astNode)) {

        NOT_IMPLEMENTED;
    } else if (auto astProcedureCall = std::dynamic_pointer_cast<AstProcedureCall>(astNode)) {
        const size_t childrenSize = astProcedureCall->children.size();
        std::vector<Value::SharedPtr> args;
        std::vector<Type::SharedPtr> argsTypes;
        for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
            auto childInst =
                _generateSsaEq(astProcedureCall->children[childIdx], simpleBlock, symbolTable);
            ASSERT(childInst);
            args.push_back(childInst);
            argsTypes.push_back(childInst->ty);
        }

        if (!containsRunTimeKnownType(argsTypes)) {
            auto procedure = symbolTable->getProcedure(astProcedureCall->name, argsTypes);
            if (procedure) {
                auto callInst = std::make_shared<CallInst>(std::shared_ptr<Procedure>(procedure),
                                                           args, astProcedureCall->name);
                simpleBlock->insts.push_back(callInst);
                return callInst;
            }
        }

    } else if (auto astVarDef = std::dynamic_pointer_cast<AstVarDef>(astNode)) {
        auto varExprProcessed = _generateSsaEq(astVarDef->expr, simpleBlock, symbolTable);
        ASSERT(varExprProcessed);
        symbolTable->addNewVar(astVarDef->name, varExprProcessed);
        return varExprProcessed;
    } else if (auto astId = std::dynamic_pointer_cast<AstId>(astNode)) {
        // TODO: it isn't clear that astId can be only variables
        auto var = symbolTable->getVar(astId->name);
        ASSERT_MSG(var, "Can't find variable with name = " << astId->name);
        return var;
    } else if (auto astInt = std::dynamic_pointer_cast<AstInt>(astNode)) {
        auto constInt = std::make_shared<ConstantInt>(astInt->num);
        return constInt;
    } else if (auto astFloat = std::dynamic_pointer_cast<AstFloat>(astNode)) {
        auto constFloat = std::make_shared<ConstantFloat>(astFloat->num);
        return constFloat;
    }

    LOG_FATAL << "Not processed AST node with type " << astNode->astNodeType;

    return nullptr;
}

SimpleBlock::SharedPtr generateIR(AstProgram::SharedPtr astProgram)
{
    auto mainBasicBlock = std::make_shared<SimpleBlock>();
    auto mainSymbolTable = std::make_shared<SymbolTable>();
    mainSymbolTable->addNewProcedure(std::make_shared<Procedure>(
        "display",
        std::vector<CompileTimeKnownType::SharedPtr>{CompileTimeKnownType::getNew(TypeID::UINT64)},
        CompileTimeKnownType::getNew(TypeID::VOID)));
    mainSymbolTable->addNewProcedure(std::make_shared<Procedure>(
        "+",
        std::vector<CompileTimeKnownType::SharedPtr>{CompileTimeKnownType::getNew(TypeID::UINT64),
                                                     CompileTimeKnownType::getNew(TypeID::UINT64)},
        CompileTimeKnownType::getNew(TypeID::UINT64)));
    _generateSsaEq(astProgram, mainBasicBlock, mainSymbolTable);
    // ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
    //     "+", std::vector<Type>{Type(Type::TypeID::UINT64), Type(Type::TypeID::FLOAT)},
    //     Type(Type::TypeID::FLOAT)));
    // ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
    //     "+", std::vector<Type>{Type(Type::TypeID::FLOAT), Type(Type::TypeID::UINT64)},
    //     Type(Type::TypeID::FLOAT)));
    // ssaSeq.symbolTable->addNewProcedure(std::make_shared<Procedure>(
    //     "+", std::vector<Type>{Type(Type::TypeID::FLOAT), Type(Type::TypeID::FLOAT)},
    //     Type(Type::TypeID::FLOAT)));

    return mainBasicBlock;
}
