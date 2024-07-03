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
 * needed SpecificProcedure from STD.
 *
 * The flow to determine the needed procedure:
 * if all the arguments have compile time known types:
 *   if a SpecificProcecure with the given parameters' types:
 *     use the SpecificProcedure
 *   else if a GeneralProcedure exists:
 *     use the GeneralProceedure
 *   else:
 *     throw error
 * else if at least one of the arguments has a runtime known type:
 *   if at least one SpecificProcedure with the given name exist:
 *     use a dispatcher to determine SpecificProcedure or GeneralProcedure (if exists)
 *   else if a GeneralProcedure exists:
 *     use the GeneralProcedure
 *   else:
 *     throw error
 *
 * To sum the code above:
 * we always prefer SpecificProcedure over GeneralProcedure, but if we don't know the types at
 * compile time we have to use either a Dispatcher if there are several options to choose between or
 * just GeneralProcedure if it is the only option
 *
 * Dispatcher determines the arguments' types and dispatches a procedure call to the correct
 * SpecificProcedure or GeneralProcedure if no SpecificProcedure suits, this takes place at runtime.
 * If a SpecificProcedure exists, then a Dispatcher for it also exists
 * It is kind of a symbol table entry, that works in runtime, its pseudocode looks like:
 *
 * some_standard_procedure_dispatcher args =>
 *   for specificProcedure in specificProcedures:
 *     if specificProcedure.types == args.types:
 *       call specificProcedure
 *       return true
 *
 *  if a GeneralProcedure exists:
 *    call the GeneralProcedure
 *  else:
 *    return false
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
    GeneralProcedure(std::string name_, std::string mangledName_,
                     std::vector<RunTimeType::SharedPtr> argsTypes_,
                     RunTimeType::SharedPtr returnType_)
        : Procedure(name_, mangledName_, toTypes(argsTypes_), returnType_)
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
