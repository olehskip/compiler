#include "instructions.hpp"

std::string CallInst::pretty() const // override
{
    return std::to_string((uint64_t)this) + " [CALL]";
}
