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
        } else {
            stream << (*argIt)->strid;
        }
    }
    stream << ")";
}
