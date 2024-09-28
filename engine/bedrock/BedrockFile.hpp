#pragma once

#include <string>

#include "BedrockMemory.hpp"

namespace MFA::File
{
    std::shared_ptr<Blob> Read(std::string const & path);
}