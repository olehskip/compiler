#include "value.hpp"

#include <sstream>

void ConstantInt::pretty(std::stringstream &stream) const // override
{
    stream << "CONSTANT_INT " << val;
}

void ConstantFloat::pretty(std::stringstream &stream) const // override
{
    stream << "CONSTANT_FLOAT " << val;
}

void ConstantString::pretty(std::stringstream &stream) const // override
{
    stream << "CONSTANT_STRING " << str;
}
