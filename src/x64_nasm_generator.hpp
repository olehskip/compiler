#ifndef X64_NASM_GENERATOR_HPP
#define X64_NASM_GENERATOR_HPP

#include <string>
#include <vector>

#include "IR/code_generator.hpp"

class Register
{
public:
    virtual std::string getData() const = 0;
    uint64_t value;
};

class RealRegister : Register
{
public:
    std::string getData() const override
    {
        return "REAL_REGISTER_RAX";
    }
};

class StackRegister : Register
{
public:
    StackRegister(uint64_t offset_) : offset(offset_) {}

    std::string getData() const override
    {
        return "[rbp - " + std::to_string(offset) + "]";
    }

private:
    const uint64_t offset;
};

class ConstantRegister : Register
{
public:
    ConstantRegister(uint64_t data_) : data(data_) {}
    
    std::string getData() const override
    {
        return std::to_string(data);
    }

private:
    const uint64_t data;
};

void generateX64Asm(SsaSeq &ssaSeq, std::stringstream &stream);

#endif // X64_NASM_GENERATOR_HPP
