#pragma once

#include "BedrockAssert.hpp"
#include "BedrockCommon.hpp"
#include "BedrockPlatforms.hpp"

#include <atomic>

namespace MFA {
    class ScopeProfiler
    {
    public:
        explicit ScopeProfiler(std::string text);
        ~ScopeProfiler();

        ScopeProfiler(ScopeProfiler const &) noexcept = delete;
        ScopeProfiler(ScopeProfiler &&) noexcept = delete;
        ScopeProfiler & operator = (ScopeProfiler const &) noexcept = delete;
        ScopeProfiler & operator = (ScopeProfiler &&) noexcept = delete;

    private:

        std::string const _text;
        supportedClock::time_point _start{};
    };
}

#define SCOPE_Profiler(lock)        ScopeProfiler MFA_UNIQUE_NAME(__scopeProfiler) {lock};
