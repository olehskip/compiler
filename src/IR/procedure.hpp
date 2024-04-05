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
 * 1) If all the arguments have compile time known types, then we try to find a "specific" procedure
 *  with such parameters' types, if we failed then we try find a "general" procedure,
 *  which doesn't care about arguments types
 * 2) If at least one of the arguments has a runtime known type, then we check whether there is a
 *  potential dispatcher, if yes we use to determine the needed procedure in runtime
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
    Procedure(std::string name_)
        : Value(CompileTimeKnownType::getNew(TypeID::PROCEDURE)), name(name_)
    {
    }
    virtual ~Procedure() = 0;

    const std::string name;
    // const std::vector<Type::UniquePtr> argsTypes;
    using SharedPtr = std::shared_ptr<Procedure>;
};

class GeneralProcedure : public Value
{
    Procedure(std::string name_, std::vector<CompileTimeKnownType::SharedPtr> argsTypes_,
              Type::SharedPtr returnType)
        : Value(returnType), name(name_), argsTypes(argsTypes_)
    {
    }
    const std::vector<CompileTimeKnownType::SharedPtr> argsTypes = {};
};

class SpecificProcedure : public Procedure
{
public:
    using SharedPtr = std::shared_ptr<SpecificProcedure>;
};

class SpecificProcedureMangled : public SpecificProcedure
{
public:
    SpecificProcedureMangled(std::string mangledName_, const SpecificProcedure &specificProcedure)
        : SpecificProcedure(specificProcedure), mangledName(mangledName_)
    {
    }
    using SharedPtr = std::shared_ptr<SpecificProcedure>;
    const std::string mangledName;
};

class ProcedureDispatcher : public Value
{
public:
    ProcedureDispatcher(std::string name_,
                        std::vector<SpecificProcedure::SharedPtr> specificProcedures_)
        : Value(CompileTimeKnownType::getNew(TypeID::LABEL)), name(name_)
    {
        for (auto specificProcedure : specificProcedures_) {
            ASSERT(specificProcedure->name == name);
            // specificProcedures.emplace_back(mangleName(name, specificProcedure->argsTypes),
            // *specificProcedure);
        }
    }

    const std::string name;
    std::vector<SpecificProcedureMangled::SharedPtr> specificProcedures;

private:
    static std::string mangleName(const std::string &name,
                                  std::vector<CompileTimeKnownType::SharedPtr> types)
    {
        return name;
    }
};

#endif // IR_PROCEDURE_HPP
