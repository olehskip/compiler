#ifndef X64_NASM_GENERATOR_HPP
#define X64_NASM_GENERATOR_HPP

#include <string>
#include <vector>

#include "inter_code_generator.hpp"

void generateX64Asm(SsaSeq &ssaSeq, std::stringstream &stream);

#endif // X64_NASM_GENERATOR_HPP
