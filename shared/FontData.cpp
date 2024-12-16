#include "FontData.hpp"

namespace Shared
{
    //==================================================================================================================

    FontData::FontData(int const glyphCount, GlyphData *glyphData, int const unitsPerEM)
        : unitsPerEm(unitsPerEM)
    {
        MFA_ASSERT(glyphCount >= 0);
        _glyphs.reserve(glyphCount);
        for (int i = 0; i < glyphCount; ++i)
        {
            _glyphs.emplace_back(glyphData[i]);
            auto const unicode = glyphData[i].UnicodeValue;
            MFA_ASSERT(_glyphLookup.contains(unicode) == false);
            _glyphLookup[unicode] = i;
            if (glyphData[i].GlyphIndex <= 0)
            {
                _missingGlyphs.emplace_back(glyphData[i]);
            }
        }

        MFA_ASSERT(_missingGlyphs.size() == 1);
    }

    //==================================================================================================================

    bool FontData::TryGetGlyph(uint32_t const unicode, GlyphData &character) const
    {
        if (auto const findResult = _glyphLookup.find(unicode); findResult != _glyphLookup.end())
        {
            character = _glyphs[findResult->second];
            return true;
        }
        character = _missingGlyphs.back();
        return false;
    }

    //==================================================================================================================
}