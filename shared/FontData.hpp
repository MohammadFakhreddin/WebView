#pragma once

#include "BedrockAssert.hpp"

#include <cstdint>
#include <vector>

namespace Shared
{
    class FontData
    {
    public:

        //==============================================================================================================

        struct Point
        {
            int X {};
            int Y {};
            bool OnCurve {};

            explicit Point() = default;

            explicit Point(int const x, int const y)
                : X(x)
                , Y(y)
            {}

            explicit Point(int const x, int const y, bool const onCurve)
                : X(x)
                , Y(y)
                , OnCurve(onCurve)
            {}
        };

        //==============================================================================================================

        struct GlyphData
        {
            uint32_t UnicodeValue{};
            uint32_t GlyphIndex{};
            std::vector<Point> Points{};
            std::vector<int> ContourEndIndices{};
            int AdvanceWidth{};
            int LeftSideBearing{};

            int MinX{};
            int MaxX{};
            int MinY{};
            int MaxY{};

            [[nodiscard]]
            int width() const
            {
                return MaxX - MinX;
            }

            [[nodiscard]]
            int height() const
            {
                return MaxY - MinY;
            }
        };

        //==============================================================================================================

        explicit FontData(int glyphCount, GlyphData *glyphData, int unitsPerEM);

        //==============================================================================================================

        bool TryGetGlyph(uint32_t unicode, GlyphData &character) const;

        //==============================================================================================================

    public:

        int const unitsPerEm{};

    private:

        std::vector<GlyphData> _glyphs{};
        std::unordered_map<uint32_t, int> _glyphLookup{};
        std::vector<GlyphData> _missingGlyphs{};

    };
}