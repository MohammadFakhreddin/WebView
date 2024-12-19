#pragma once

#include "BedrockMemory.hpp"
#include "FontData.hpp"

namespace Shared::FontParser
{

    [[nodiscard]]
    FontData Parse(std::shared_ptr<MFA::Blob> rawFontData);

}