#ifndef UTILS_HPP
#define UTILS_HPP

#include "log.hpp"

#include <string>
#include <typeinfo>
#include <variant>
#include <vector>

template <typename... Types>
std::string variantTypeToString(const std::variant<Types...> &var)
{
    return std::visit([](const auto &value) { return typeid(value).name(); }, var);
}

template <typename T>
T extractFromSingleVector(const std::vector<T> &vec)
{
    ASSERT(vec.size() == 1);
    return vec[0];
}

#endif // UTILS_HPP
