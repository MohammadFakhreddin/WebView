#pragma once

#include "BinaryReader.hpp"
#include "FontData.hpp"

namespace Shared::FontParser
{
    struct GlyphMap;
    void ReadAllGlyphs(
        BinaryReader & reader,
        uint32_t * glyphLocations,
        uint32_t mappingsCount,
        GlyphMap const * mappings,
        FontData::GlyphData * outGlyphDataList
    );
}