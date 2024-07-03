#include "code_generator.hpp"
#include "log.hpp"

#include <sstream>

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
