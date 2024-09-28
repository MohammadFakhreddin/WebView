#pragma once

#include "BedrockPlatforms.hpp"
#include "BedrockString.hpp"

#include <cstdarg>
#include <string>


namespace MFA::Log {

    void Debug(char const * message, ...);

    void Info(char const * message, ...);

    void Warn(char const * message, ...);

    void Error(char const * message, ...);

    void _Debug(char const * file, int line, char const * function, char const * message, ...);

    void _Info(char const * file, int line, char const * function, char const * message, ...);

    void _Warn(char const * file, int line, char const * function, char const * message, ...);

    void _Error(char const * file, int line, char const * function, char const * message, ...);

} // MFA::Log


#ifdef MFA_DEBUG
    #define MFA_LOG_DEBUG(fmt_, ...)                MFA::Log::_Debug(__FILE__, __LINE__, __FUNCTION__, fmt_, ##__VA_ARGS__)
#else
    #define MFA_LOG_DEBUG(fmt_, ...)                         
#endif

#define MFA_LOG_INFO(fmt_, ...)                 MFA::Log::_Info(__FILE__, __LINE__, __FUNCTION__, fmt_, ##__VA_ARGS__)
#define MFA_LOG_WARN(fmt_, ...)                 MFA::Log::_Warn(__FILE__, __LINE__, __FUNCTION__, fmt_, ##__VA_ARGS__)
#define MFA_LOG_ERROR(fmt_, ...)                MFA::Log::_Error(__FILE__, __LINE__, __FUNCTION__, fmt_, ##__VA_ARGS__)
