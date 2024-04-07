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

        // check out the comments in IR/procedure.hpp to understand the flow

        if (!containsRunTimeType(argsTypes)) {
            auto compileTimeArgsTypes = toCompileTimeTypes(argsTypes);
            auto specificProcedure =
                symbolTable->getSpecificProcedure(astProcedureCall->name, compileTimeArgsTypes);
            if (specificProcedure) {
                auto callInst = std::make_shared<CallInst>(specificProcedure, args);
                simpleBlock->insts.push_back(callInst);
                return callInst;
            } else {
                LOG_WARNING << astProcedureCall->name;
            }
        }
        NOT_IMPLEMENTED;
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
        return std::make_shared<ConstantInt>(astInt->num);
    } else if (auto astFloat = std::dynamic_pointer_cast<AstFloat>(astNode)) {
        return std::make_shared<ConstantFloat>(astFloat->num);
    } else if (auto astString = std::dynamic_pointer_cast<AstString>(astNode)) {
        return std::make_shared<ConstantString>(astString->str);
    }

    LOG_FATAL << "Not processed AST node with type " << astNode->astNodeType;

    return nullptr;
}

SimpleBlock::SharedPtr generateIR(AstProgram::SharedPtr astProgram)
{
    auto mainBasicBlock = std::make_shared<SimpleBlock>();
    auto mainSymbolTable = std::make_shared<SymbolTable>();
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
