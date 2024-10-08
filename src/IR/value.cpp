#include "value.hpp"

#include <iomanip>
#include <sstream>

static uint64_t getUniqueNumber()
{
    static uint64_t number = 0;
    return number++;
}

Value::Value(Type::SharedPtr ty_, bool isConstant_)
    : ty(ty_), id(getUniqueNumber()), strid("$" + std::to_string(id)), isConstant(isConstant_)
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
    stream << "CONSTANT_STRING " << std::quoted(str);
}
