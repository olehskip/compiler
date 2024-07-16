#ifndef X64_NASM_GENERATOR_HPP
#define X64_NASM_GENERATOR_HPP

#include "IR/code_generator.hpp"

#include <string>
#include <vector>

class Register
{
public:
    virtual std::string get() const = 0;
};

class RealRegister : public Register
{
public:
    std::string get() const override
    {
        return "REAL_REGISTER_RAX";
    }
};

class StackRegister : public Register
{
public:
    StackRegister(uint64_t offset_) : offset(offset_) {}

    std::string get() const override
    {
        return "[rbp - " + std::to_string(offset) + "]";
    }

private:
    const uint64_t offset;
};

class RoDataRegister : public Register
{
public:
    RoDataRegister(uint64_t id) : name("RODATA_" + std::to_string(id)) {}

    std::string get() const override
    {
        return "[" + name + "]";
    }

    std::string getPtr() const
    {
        return name;
    }

    const std::string name;
};

void generateX64Asm(SimpleBlock::SharedPtr ssaSeq, std::stringstream &stream);

#endif // X64_NASM_GENERATOR_HPP
