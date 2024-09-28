#pragma once

#include <string>
#include <regex>
#include <stdarg.h>
#include <sstream>

#define MFA_STRING(variable, format, ...)           \
{                                                   \
    char charBufer[1024] {};                        \
    auto const strLength = sprintf(                 \
        charBufer,                                  \
        format,                                     \
        __VA_ARGS__                                 \
    );                                              \
    variable.assign(charBufer, strLength);          \
}                                                   \

#define MFA_STRING_VA(variable, format, ...)        \
{                                                   \
	char charBufer[1024]{};                         \
	auto const strLength = vsnprintf(               \
	    charBufer,                                  \
		1024,                                       \
		format,                                     \
	    __VA_ARGS__                                 \
	);                                              \
	variable.assign(charBufer, strLength);          \
}                                                   \

#define MFA_APPEND(variable, format, ...)           \
{                                                   \
    char charBufer[1024] {};                        \
    auto const strLength = sprintf(                 \
        charBufer,                                  \
        format,                                     \
        __VA_ARGS__                                 \
    );                                              \
    variable.append(charBufer, strLength);          \
}                                                   \

namespace MFA::String
{
    // https://en.cppreference.com/w/cpp/string/byte/tolower
    [[nodiscard]]
    std::string ToLowerCase(std::string s);

    [[nodiscard]]
    std::vector<std::string> Split(std::string const & text, std::string const & separator);
    
}
