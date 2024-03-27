#include "value.hpp"

std::string ConstantInt::pretty() const // override
{
    return "CONSTANT_INT " + std::to_string(val);
}

std::string ConstantFloat::pretty() const // override
{
    return "CONSTANT_FLOAT " + std::to_string(val);
}

std::string Procedure::pretty() const // override
{
    return "SOME PROCEDURE";
}
