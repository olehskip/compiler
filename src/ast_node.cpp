#include "ast_node.hpp"

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
    auto ret = body->emitSsa(newBlock);
    std::vector<RunTimeType::SharedPtr> types;
    for (size_t i = 0; i < params.size(); ++i) {
        types.push_back(RunTimeType::getNew());
    }
    simpleBlock->symbolTable->addGeneralProcedure(
        std::make_shared<GeneralProcedure>(name, types, ret->ty, newBlock));

    return ret;
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

    if (!containsRunTimeType(argsTypes)) {
        auto compileTimeArgsTypes = toCompileTimeTypes(argsTypes);
        Procedure::SharedPtr procedure =
            simpleBlock->symbolTable->getSpecificProcedure(name, compileTimeArgsTypes);
        if (!procedure) {
            procedure = simpleBlock->symbolTable->getGeneralProcedure(name);
            if (!procedure) {
                LOG_FATAL << "There is no procedure with name" << name;
            }
        }
        auto callInst = std::make_shared<CallInst>(procedure, args);
        simpleBlock->insts.push_back(callInst);
        return callInst;
    }
    NOT_IMPLEMENTED;
    return nullptr;
}

Value::SharedPtr AstVarDef::emitSsa(SimpleBlock::SharedPtr simpleBlock)
{
    auto varExprProcessed = expr->emitSsa(simpleBlock);
    ASSERT(varExprProcessed);
    simpleBlock->symbolTable->addNewVar(name, varExprProcessed);
    return varExprProcessed;
}
