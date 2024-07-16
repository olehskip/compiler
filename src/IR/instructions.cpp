#include "instructions.hpp"

#include <sstream>

void CallInst::pretty(std::stringstream &stream) const // override
{
    stream << "call \"" << procedure->name << "\" (";
    for (auto argIt = args.begin(); argIt != args.end(); ++argIt) {
        if (argIt != args.begin()) {
            stream << ", ";
        }
        if (auto constantIntArg = std::dynamic_pointer_cast<ConstantInt>(*argIt)) {
            constantIntArg->pretty(stream);
        } else if (auto constantStrArg = std::dynamic_pointer_cast<ConstantString>(*argIt)) {
            constantStrArg->pretty(stream);
        } else {
            stream << (*argIt)->strid;
        }
    }
    stream << ")";
}

void RetInst::pretty(std::stringstream &stream) const // override
{
    stream << "ret ";
    if (auto constantIntArg = std::dynamic_pointer_cast<ConstantInt>(val)) {
        constantIntArg->pretty(stream);
    } else if (auto constantStrArg = std::dynamic_pointer_cast<ConstantString>(val)) {
        constantStrArg->pretty(stream);
    } else {
        stream << val->strid;
    }
}
