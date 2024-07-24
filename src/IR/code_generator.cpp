#include "IR/code_generator.hpp"
#include "ast_node.hpp"
#include "log.hpp"

#include <iomanip>
#include <sstream>

Value::SharedPtr AstProgram::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    for (auto child : children) {
        child->emitSsa(simpleBlock);
    }
    return nullptr;
}

Value::SharedPtr AstBeginExpr::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    auto newBlock = std::make_shared<SimpleBlock>(simpleBlock);
    Value::SharedPtr lastChildProcessed;
    for (auto child : children) {
        lastChildProcessed = child->emitSsa(simpleBlock);
    }
    ASSERT(lastChildProcessed);
    return lastChildProcessed;
}

Value::SharedPtr AstId::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    // TODO: it isn't clear that astId can be only variables
    auto var = simpleBlock->symbolTable->getVar(name);
    ASSERT_MSG(var, "Can't find variable with name = " << name);
    return var;
}

Value::SharedPtr AstInt::emitSsa(SimpleBlock::SharedPtr)
{
    return std::make_shared<ConstantInt>(num);
}

Value::SharedPtr AstFloat::emitSsa(SimpleBlock::SharedPtr)
{
    return std::make_shared<ConstantFloat>(num);
}

Value::SharedPtr AstString::emitSsa(SimpleBlock::SharedPtr)
{
    return std::make_shared<ConstantString>(str);
}

Value::SharedPtr AstProcedureDef::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    auto newBlock = std::make_shared<SimpleBlock>(simpleBlock);
    std::vector<RunTimeType::SharedPtr> argsTypes;
    for (size_t i = 0; i < params.size(); ++i) {
        newBlock->symbolTable->addNewVar(params[i]->name, std::make_shared<ProcParameter>(i));
        argsTypes.push_back(RunTimeType::getNew());
    }
    auto procedureSsa = body->emitSsa(newBlock);
    if (!procedureSsa->ty->isVoid()) {
        // TODO: should we return anything if void?
        // TODO: backend should be flexible, but now we always return the last expr
        auto retInst = std::make_shared<RetInst>(procedureSsa);
        newBlock->insts.push_back(retInst);
        procedureSsa = retInst;
    }
    simpleBlock->symbolTable->addGeneralProcedure(
        std::make_shared<GeneralProcedure>(name, argsTypes, procedureSsa->ty, newBlock));
    return procedureSsa;
}

Value::SharedPtr AstProcedureCall::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    const size_t childrenSize = children.size();
    std::vector<Value::SharedPtr> args;
    std::vector<Type::SharedPtr> argsTypes;
    for (size_t childIdx = 0; childIdx < childrenSize; ++childIdx) {
        auto childInst = children[childIdx]->emitSsa(simpleBlock);
        ASSERT(childInst);
        args.push_back(childInst);
        argsTypes.push_back(childInst->ty);
    }

    // check out the comments in IR/procedure.hpp to understand the flow

    Procedure::SharedPtr procedure;
    if (!containsRunTimeType(argsTypes)) {
        auto compileTimeArgsTypes = toCompileTimeTypes(argsTypes);
        procedure = simpleBlock->symbolTable->getSpecificProcedure(name, compileTimeArgsTypes);
    }
    if (!procedure) {
        procedure = simpleBlock->symbolTable->getGeneralProcedure(name);
    }
    if (!procedure) {
        LOG_FATAL << "There is no procedure with name " << std::quoted(name);
    }
    auto callInst = std::make_shared<CallInst>(procedure, args);
    simpleBlock->insts.push_back(callInst);
    return callInst;
}

Value::SharedPtr AstVarDef::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    auto varExprProcessed = expr->emitSsa(simpleBlock);
    ASSERT(varExprProcessed);
    simpleBlock->symbolTable->addNewVar(name, varExprProcessed);
    return varExprProcessed;
}

SimpleBlock::SharedPtr generateIR(AstProgram::SharedPtr astProgram)
{
    auto mainBasicBlock = std::make_shared<SimpleBlock>();
    auto mainSymbolTable = mainBasicBlock->symbolTable;
    mainSymbolTable->addSpecificProcedure(std::make_shared<SpecificProcedure>(
        "display", "displayINT64",
        std::vector<CompileTimeType::SharedPtr>{CompileTimeType::getNew(TypeID::INT64)},
        CompileTimeType::getNew(TypeID::VOID)));
    mainSymbolTable->addSpecificProcedure(std::make_shared<SpecificProcedure>(
        "display", "displaySTRING",
        std::vector<CompileTimeType::SharedPtr>{CompileTimeType::getNew(TypeID::STRING)},
        CompileTimeType::getNew(TypeID::VOID)));
    mainSymbolTable->addSpecificProcedure(std::make_shared<SpecificProcedure>(
        "+", "plusINT64",
        std::vector<CompileTimeType::SharedPtr>{CompileTimeType::getNew(TypeID::INT64),
                                                CompileTimeType::getNew(TypeID::INT64)},
        CompileTimeType::getNew(TypeID::INT64)));
    astProgram->emitSsa(mainBasicBlock);
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
