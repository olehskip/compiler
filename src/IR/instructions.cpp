#include "IR/instructions.hpp"
#include "IR/block.hpp"

#include <sstream>

void CallInst::pretty(std::stringstream &stream) const // override
{
    stream << "call \"" << procedure->name << "\" (";
    for (auto argIt = args.begin(); argIt != args.end(); ++argIt) {
        if (argIt != args.begin()) {
            stream << ", ";
        }
        (*argIt)->refPretty(stream);
    }
    stream << ")";
}

void RetInst::pretty(std::stringstream &stream) const // override
{
    stream << "ret ";
    val->refPretty(stream);
}

void CondJumpInst::pretty(std::stringstream &stream) const // override
{
    stream << "CondJump ";
    elseBlock->refPretty(stream);
    stream << " ";
    thenBlock->refPretty(stream);
    if (elseBlock) {
        stream << " ";
        elseBlock->refPretty(stream);
    }
}
