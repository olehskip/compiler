#include "instructions.hpp"

std::string CallInst::pretty() const // override
{
    std::string ret = "call \"" + procedureName + "\" (";
    std::string argsPretty;
    for (auto argIt = args.begin(); argIt != args.end(); ++argIt) {
        if (argIt != args.begin()) {
            ret += ", ";
        }
        if (auto constantIntArg = std::dynamic_pointer_cast<ConstantInt>(*argIt)) {
            ret += std::to_string(constantIntArg->val);
        } else {
            ret += "$" + std::to_string(uint64_t(argIt->get()));
        }
    }
    ret += ")";
    return ret;
}
