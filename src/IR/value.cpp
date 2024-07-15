#include "value.hpp"

#include <sstream>

static uint64_t getUniqueNumber()
{
    static uint64_t number = 0;
    return number++;
}

Value::Value(Type::SharedPtr ty_) : ty(ty_), id(getUniqueNumber()), strid("$" + std::to_string(id))
{
}

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
