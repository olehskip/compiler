#include "IR/procedure.hpp"
#include "IR/block.hpp"

void Procedure::pretty(std::stringstream &stream) const
{
    stream << strid << " = procedure " << name;
    if (name != mangledName) {
        stream << " " << mangledName;
    }
    stream << "(";
    for (auto argTypeIt = argsTypes.begin(); argTypeIt != argsTypes.end(); argTypeIt++) {
        (*argTypeIt)->pretty(stream);
        if (std::next(argTypeIt) != argsTypes.end()) {
            stream << ", ";
        }
    }
    stream << ") ";
    if (isOnlyDeclaration()) {
        stream << " ONLY_DECLARATION\n";
    } else {
        stream << ":\n";
        block->pretty(stream);
    }
}
