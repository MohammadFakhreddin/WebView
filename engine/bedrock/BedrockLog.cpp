#include "BedrockLog.hpp"

#include "BedrockAssert.hpp"

namespace MFA::Log {
    void Debug(char const * message, ...)
    {
    #ifdef MFA_DEBUG
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);
        
        printf("\n-----------DEBUG------------\n%s\n---------------------------\n", displayMessage.c_str());
    #endif
    }

    void Info(char const * message, ...)
    {
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);
        
        printf("\n-----------INFO------------\n%s\n---------------------------\n", displayMessage.c_str());
    }

    void Warn(char const * message, ...)
    {
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);
        
        printf("\n-----------WARN------------\n%s\n---------------------------\n", displayMessage.c_str());
    }

    void Error(char const * message, ...)
    {
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);

        printf("\n-----------ERROR------------\n%s\n---------------------------\n", displayMessage.c_str());
    }

    void _Debug(char const * file, int line, char const * function, char const * message, ...)
    {
    #ifdef MFA_DEBUG
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);

        printf("\n-----------DEBUG------------\nFile: %s\nLine: %d\nFunction: %s\n%s\n---------------------------\n", file, line, function, displayMessage.c_str());
    #endif
    }

    void _Info(char const * file, int line, char const * function, char const * message, ...)
    {
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);
        
        printf("\n-----------INFO------------\nFile: %s\nLine: %d\nFunction: %s\n%s\n---------------------------\n", file, line, function, displayMessage.c_str());
    }

    void _Warn(char const * file, int line, char const * function, char const * message, ...)
    {
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);
        
        printf("\n-----------WARN------------\nFile: %s\nLine: %d\nFunction: %s\n%s\n---------------------------\n", file, line, function, displayMessage.c_str());
    }

    void _Error(char const * file, int line, char const * function, char const * message, ...)
    {
        va_list args;
        va_start(args, message);
        std::string displayMessage {};
        MFA_STRING_VA(displayMessage, message, args);
        va_end(args);
        
        printf("\n-----------ERROR------------\nFile: %s\nLine: %d\nFunction: %s\n%s\n---------------------------\n", file, line, function, displayMessage.c_str());
    #ifdef MFA_DEBUG
        assert(false);
    #endif
    }
};