#include "ScopeProfiler.hpp"

#include "BedrockAssert.hpp"
#include "BedrockLog.hpp"

#include <thread>

namespace MFA {

    ScopeProfiler::ScopeProfiler(std::string text)
        : _text(std::move(text))
        , _start(std::chrono::high_resolution_clock::now())
    {
        
    }

    ScopeProfiler::~ScopeProfiler()
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - _start;
        MFA_LOG_INFO("Profiler: %s took %f seconds", _text.c_str(), duration.count());
    }
}
