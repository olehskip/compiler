#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <typeinfo>
#include <variant>

template <typename... Types>
std::string variantTypeToString(const std::variant<Types...> &var)
{
    return std::visit([](const auto &value) { return typeid(value).name(); }, var);
}

#endif // UTILS_HPP
