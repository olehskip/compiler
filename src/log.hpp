#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream>

#include "magic_enum.hpp"
#include <iostream>
#include <source_location>
#include <type_traits>
#include <variant>

#if EXIT_FUNC == STD_EXIT
#include <cstdlib>
#endif

// #define EXIT_FUNC abort

enum class LogLevel
{
    WARNING,
    ASSERT,
    FATAL
};

// TODO: I don't like the fact, that one can do auto a = Logger(LogLevel::FATAL);
// TODO: add INFO LogLevel that doesn't print source location
class Logger
{
public:
    Logger(LogLevel logLevel_,
           const std::source_location location = std::source_location::current())
        : logLevel(logLevel_)
    {
        *this << "[" << logLevel << "] " << location.file_name() << ":" << location.line() << " "
              << location.function_name() << " ";
    }

    template <class E, std::enable_if_t<std::is_enum_v<E>, bool> = false>
    Logger &operator<<(E e)
    {
        appendEnumValToBuffer(e);
        return *this;
    }

    template <class T, std::enable_if_t<!std::is_enum_v<T>, bool> = true>
    Logger &operator<<(T t)
    {
        buffer << t;
        return *this;
    }

    ~Logger()
    {
        std::cout << buffer.str() << std::endl;
        if (logLevel == LogLevel::FATAL || logLevel == LogLevel::ASSERT) {
            std::cout << "Aborting..." << std::endl;
#if !defined(LOG_EXIT_FUNC)
#error "LOG_EXIT_FUNC is not defined"
#elif LOG_EXIT_FUNC == STD_EXIT
            std::exit(1);
#elif LOG_EXIT_FUNC == ABORT
            abort()
#else
#error "LOG_EXIT_FUNC is not correct"
#endif
        }
    }

private:
    template <class E, std::enable_if_t<std::is_enum_v<E>, bool> = false>
    void appendEnumValToBuffer(E e)
    {
        buffer << std::string(magic_enum::enum_name(e));
    }

    std::stringstream buffer;
    const LogLevel logLevel;
};

using LOG = Logger;
#define LOG_WARNING LOG(LogLevel::WARNING)
#define LOG_FATAL LOG(LogLevel::FATAL)

// TODO: add NOT_LIKELY
#define ASSERT(pred) \
    if (!(pred))     \
    LOG(LogLevel::ASSERT) << #pred
#define ASSERT_MSG(pred, msg) \
    if (!(pred))              \
    LOG(LogLevel::ASSERT) << #pred << "; " msg
#define SHOULD_NOT_HAPPEN LOG(LogLevel::FATAL) << "This should never happen"
#define NOT_IMPLEMENTED LOG(LogLevel::FATAL) << "Not implemented yet"

#endif // LOG_HPP
