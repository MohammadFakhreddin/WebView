#pragma once

#include "BedrockAssert.hpp"
#include "BedrockCommon.hpp"

#include <atomic>

namespace MFA {
    class ScopeLock
    {
    public:
        explicit ScopeLock(std::atomic<bool> & lock);
        ~ScopeLock();

        ScopeLock(ScopeLock const &) noexcept = delete;
        ScopeLock(ScopeLock &&) noexcept = delete;
        ScopeLock & operator = (ScopeLock const &) noexcept = delete;
        ScopeLock & operator = (ScopeLock &&) noexcept = delete;

    private:
        std::atomic<bool> & mLock;
    };
}

#define SCOPE_LOCK(lock)        ScopeLock MFA_UNIQUE_NAME(__scopeLock) {lock};
