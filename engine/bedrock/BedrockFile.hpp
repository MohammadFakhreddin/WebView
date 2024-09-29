#pragma once

#include <string>

#include "BedrockMemory.hpp"

namespace MFA::File
{
    // Should accept char const * or string_view for better compatibility
    std::shared_ptr<Blob> Read(std::string const & path);
}