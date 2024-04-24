#ifndef IR_PROCEDURE_HPP
#define IR_PROCEDURE_HPP

#include "value.hpp"

/*
 * Some procedures (only in the STD so far), have parameters with arguments of specific types, we
 * call such procedures SpecificProcedure, otherwise if the parameters can be of any type, we call
 * such procedures GeneralProcedure.
 *
 * There are some obstacles to determine the needed SpecificProcedure or GeneraelProcedure,
 * when given only name of it.
 *
 * First of all the user may use STD procedures, which are all SpecificProcedure so far, but take a
 * look at this pseudocode:
 *  func f(a, b) => return standard_procedure(a, b)
 * Here we call some standard procedure, but we can't know the arguments' types in compile time. To
 * solve this we use a dispatcher, which in runtime checks the arguments' types and returns the
 * needed SpecificProcedure from STD. GeneralProcedure and SpecificProcedure should have different
 * names to avoid ambiguity
 *
 * The flow to determine the needed procedure:
 * 1) If all the arguments have compile time known types, then we try to find an Operator or a
 * SpecificProcecure with the given parameters' types, if we failed then we try find a
 * GeneralProcedure, which doesn't care about arguments types 2) If at least one of the arguments
 * has a runtime known type, then we search for GeneralProcedure with the same name, if we fail
 * then we check whether there is a potential Dispatcher, if yes we use it to determine the needed
 * SpecificProcedure in runtime
 *
 * Dispatcher determines the arguments' types and dispatches a procedure call to the correct
 * SpecificProcedure, this takes place in runtime. It is kind of a symbol table entry, that works in
 * runtime, it pseudocode it would look like:
 * some_standard_procedure_dispatcher args =>
 *  for specificProcedure in specificProcedures:
 *      if specificProcedure.types == args.types:
 *          call specificProcedure
 *  throw error
 *
 */

class Procedure : public Value
{
public:
    using SharedPtr = std::shared_ptr<Procedure>;
    virtual ~Procedure(){};
    const std::string name;
    const std::string mangledName;
    const std::vector<Type::SharedPtr> argsTypes;
    const Type::SharedPtr returnType;

protected:
    Procedure(std::string name_, std::vector<Type::SharedPtr> argsTypes_,
              Type::SharedPtr returnType_)
        : Value(CompileTimeType::getNew(TypeID::PROCEDURE)), name(name_),
          mangledName(mangleName(name, argsTypes_)), argsTypes(argsTypes_), returnType(returnType_)
    {
    }
    Procedure(std::string name_, std::string mangledName_, std::vector<Type::SharedPtr> argsTypes_,
              Type::SharedPtr returnType_)
        : Value(CompileTimeType::getNew(TypeID::PROCEDURE)), name(name_), mangledName(mangledName_),
          argsTypes(argsTypes_), returnType(returnType_)
    {
    }

private:
    static std::string mangleName(const std::string &nameToMangle,
                                  const std::vector<Type::SharedPtr> argsTypes)
    {
        std::string ret = nameToMangle;
        for (auto argType : argsTypes) {
            if (argType->knownInCompileTime()) {
                auto typeID = std::dynamic_pointer_cast<CompileTimeType>(argType)->typeID;
                ret += typeIdToString(typeID);
            } else {
                ret += "UnknownType";
            }
        }
        return ret;
    }
};

class GeneralProcedure : public Procedure
{
public:
    using SharedPtr = std::shared_ptr<GeneralProcedure>;
    GeneralProcedure(std::string name_, std::vector<RunTimeType::SharedPtr> argsTypes_,
                     RunTimeType::SharedPtr returnType_)
        : Procedure(name_, toTypes(argsTypes_), returnType_)
    {
    }
    ~GeneralProcedure() override {}

    void pretty(std::stringstream &stream) const override
    {
        (void)stream;
        NOT_IMPLEMENTED;
    }
};

class SpecificProcedure : public Procedure
{
public:
    using SharedPtr = std::shared_ptr<SpecificProcedure>;
    SpecificProcedure(std::string name_, std::vector<CompileTimeType::SharedPtr> argsTypes_,
                      CompileTimeType::SharedPtr returnType_)
        : Procedure(name_, toTypes(argsTypes_), returnType_)
    {
    }
    SpecificProcedure(std::string name_, std::string mangledName_,
                      std::vector<CompileTimeType::SharedPtr> argsTypes_,
                      CompileTimeType::SharedPtr returnType_)
        : Procedure(name_, mangledName_, toTypes(argsTypes_), returnType_)
    {
    }
    ~SpecificProcedure() override {}

    void pretty(std::stringstream &stream) const override
    {
        (void)stream;
        NOT_IMPLEMENTED;
    }
};

class Operator : public SpecificProcedure
{
public:
    using SharedPtr = std::shared_ptr<SpecificProcedure>;
    SpecificProcedure(std::string name_, std::vector<CompileTimeType::SharedPtr> argsTypes_,
                      CompileTimeType::SharedPtr returnType_)
        : Procedure(name_, toTypes(argsTypes_), returnType_)
    {
    }
    ~SpecificProcedure() override {}

    void pretty(std::stringstream &stream) const override
    {
        (void)stream;
        NOT_IMPLEMENTED;
    }
};

class ProcedureDispatcher : public Value
{
public:
    ProcedureDispatcher(std::string name_,
                        std::vector<SpecificProcedure::SharedPtr> specificProcedures_)
        : Value(CompileTimeType::getNew(TypeID::LABEL)), name(name_),
          specificProcedures(specificProcedures_)
    {
    }

    const std::string name;
    const std::vector<SpecificProcedure::SharedPtr> specificProcedures;
};

#endif // IR_PROCEDURE_HPP
