#ifndef IR_PROCEDURE_HPP
#define IR_PROCEDURE_HPP

#include "value.hpp"

class Procedure : public Value
{
public:
    Procedure(std::string name_, std::vector<Type> argsTypes_, Type returnType)
        : Value(returnType), name(name_), argsTypes(argsTypes_)
    {
    }
    void pretty(std::stringstream &stream) const override
    {
        NOT_IMPLEMENTED;
        (void)stream;
    }

    const std::string name;
    const std::vector<Type> argsTypes;
    using SharedPtr = std::shared_ptr<Procedure>;
};

#endif // IR_PROCEDURE_HPP
