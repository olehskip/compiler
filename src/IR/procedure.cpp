#include "IR/procedure.hpp"
#include "IR/block.hpp"

void GeneralProcedure::pretty(std::stringstream &stream) const
{
    stream << strid << " = procedure " << name;
    if (name != mangledName) {
        stream << " " << mangledName;
    }
    if (isOnlyDeclaration()) {
        stream << " ONLY_DECLARATION\n";
    } else {
        stream << ":\n";
        block->pretty(stream);
    }
}
