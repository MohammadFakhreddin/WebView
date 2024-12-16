#include "FontParser.hpp"

#include <unordered_map>

namespace Shared::FontParser
{

    //==================================================================================================================

    using Table = std::unordered_map<std::string, uint32_t>;

    //==================================================================================================================

    struct GlyphMap
    {
        uint32_t GlyphIndex;
        uint32_t Unicode;

        explicit GlyphMap(uint32_t const index, uint32_t const unicode)
        {
            GlyphIndex = index;
            Unicode = unicode;
        }
    };

    //==================================================================================================================

    struct HeadTableData
    {
        uint32_t UnitsPerEM;
        uint32_t NumBytesPerGlyphIndexToLocationEntry;

        HeadTableData(uint32_t const unitsPerEM, uint32_t const numBytesPerGlyphIndexToLocationEntry)
        {
            UnitsPerEM = unitsPerEM;
            NumBytesPerGlyphIndexToLocationEntry = numBytesPerGlyphIndexToLocationEntry;
        }
    };

    void ReadGlyph(
        BinaryReader & reader,
        uint32_t const * glyphLocations,
        uint32_t glyphIndex,
        FontData::GlyphData & outGlyphData
    );

    //==================================================================================================================

    static bool FlagBitIsSet(uint8_t const flag, int const bitIndex)
    {
        return ((flag >> bitIndex) & 1) == 1;
    }

    //==================================================================================================================

    static Table ReadTableLocations(BinaryReader reader)
    {
        Table tableLocations {};
        // -- offset subtable --
        reader.Skip<uint8_t>(4);                        // unused: scalerType
        int numTables = reader.ReadUInt16();
        reader.Skip<uint8_t>(6);                        // unused: searchRange, entrySelector, rangeShift

        // -- table directory --
        for (int i = 0; i < numTables; i++)
        {
            char tag[4];
            reader.ReadString(4, tag);
            uint32_t const checksum = reader.ReadUInt32();
            uint32_t const offset = reader.ReadUInt32();
            uint32_t const length = reader.ReadUInt32();

            MFA_ASSERT(tableLocations.find(tag) == tableLocations.end());
            tableLocations[tag] = offset;
        }

        return tableLocations;
    }

    //==================================================================================================================

    // Read a simple glyph from the 'glyf' table
    static void ReadSimpleGlyph(
        BinaryReader & reader,
        uint32_t const * glyphLocations,
        uint32_t glyphIndex,
        FontData::GlyphData & outGlyphData
    )
    {
        // Flag masks
        const int OnCurve = 0;
        const int IsSingleByteX = 1;
        const int IsSingleByteY = 2;
        const int Repeat = 3;
        const int InstructionX = 4;
        const int InstructionY = 5;

        reader.GoTo(glyphLocations[glyphIndex]);

        outGlyphData.GlyphIndex = glyphIndex;

        int contourCount = reader.ReadInt16();
        if (contourCount < 0) MFA_LOG_ERROR("Expected simple glyph, but found compound glyph instead");

        outGlyphData.MinX = reader.ReadInt16();
        outGlyphData.MinY = reader.ReadInt16();
        outGlyphData.MaxX = reader.ReadInt16();
        outGlyphData.MaxY = reader.ReadInt16();

        // Read contour ends
        int numPoints = 0;
        outGlyphData.ContourEndIndices.resize(contourCount);

        for (int i = 0; i < contourCount; i++)
        {
            int contourEndIndex = reader.ReadUInt16();
            numPoints = std::max(numPoints, contourEndIndex + 1);
            outGlyphData.ContourEndIndices[i] = contourEndIndex;
        }

        int instructionsLength = reader.ReadInt16();
        reader.Skip<uint8_t>(instructionsLength); // skip instructions (hinting stuff)

        std::vector<uint8_t> allFlags(numPoints);
        outGlyphData.Points.resize(numPoints);

        for (int i = 0; i < numPoints; i++)
        {
            auto const flag = reader.ReadByte();
            allFlags[i] = flag;

            if (FlagBitIsSet(flag, Repeat))
            {
                int repeatCount = reader.ReadByte();

                for (int r = 0; r < repeatCount; r++)
                {
                    i++;
                    allFlags[i] = flag;
                }
            }
        }

        auto const ReadCoords = [&](bool readingX)
        {
            int min = std::numeric_limits<int>::max();
            int max = std::numeric_limits<int>::min();

            int const singleByteFlagBit = readingX ? IsSingleByteX : IsSingleByteY;
            int const instructionFlagMask = readingX ? InstructionX : InstructionY;

            int coordVal = 0;

            for (int i = 0; i < numPoints; i++)
            {
                uint8_t const currFlag = allFlags[i];

                // Offset value is represented with 1 byte (unsigned)
                // Here the instruction flag tells us whether to add or subtract the offset
                if (FlagBitIsSet(currFlag, singleByteFlagBit))
                {
                    int coordOffset = reader.ReadByte();
                    bool positiveOffset = FlagBitIsSet(currFlag, instructionFlagMask);
                    coordVal += positiveOffset ? coordOffset : -coordOffset;
                }
                // Offset value is represented with 2 bytes (signed)
                // Here the instruction flag tells us whether an offset value exists or not
                else if (!FlagBitIsSet(currFlag, instructionFlagMask))
                {
                    coordVal += reader.ReadInt16();
                }

                if (readingX) outGlyphData.Points[i].X = coordVal;
                else outGlyphData.Points[i].Y = coordVal;
                outGlyphData.Points[i].OnCurve = FlagBitIsSet(currFlag, OnCurve);

                min = std::min(min, coordVal);
                max = std::max(max, coordVal);
            }
        };

        ReadCoords(true);
        ReadCoords(false);
    }

    //==================================================================================================================

    static bool ReadNextComponentGlyph(
        BinaryReader & reader,
        uint32_t const * glyphLocations,
        uint32_t const glyphLocation,
        FontData::GlyphData & outGlyphData
    )
    {
        outGlyphData.Points.clear();
        outGlyphData.ContourEndIndices.clear();

        uint32_t const flag = reader.ReadUInt16();
        uint32_t const glyphIndex = reader.ReadUInt16();

        uint32_t const componentGlyphLocation = glyphLocations[glyphIndex];
        // If compound glyph refers to itself, return empty glyph to avoid infinite loop.
        // Had an issue with this on the 'carriage return' character in robotoslab.
        // There's likely a bug in my parsing somewhere, but this is my work-around for now...
        if (componentGlyphLocation == glyphLocation)
        {
            return false;
        }

        // Decode flags
        bool argsAre2Bytes = FlagBitIsSet(flag, 0);
        bool argsAreXYValues = FlagBitIsSet(flag, 1);
        bool roundXYToGrid = FlagBitIsSet(flag, 2);
        bool isSingleScaleValue = FlagBitIsSet(flag, 3);
        bool isMoreComponentsAfterThis = FlagBitIsSet(flag, 5);
        bool isXAndYScale = FlagBitIsSet(flag, 6);
        bool is2x2Matrix = FlagBitIsSet(flag, 7);
        bool hasInstructions = FlagBitIsSet(flag, 8);
        bool useThisComponentMetrics = FlagBitIsSet(flag, 9);
        bool componentsOverlap = FlagBitIsSet(flag, 10);

        // Read args (these are either x/y offsets, or point number)
        int arg1 = argsAre2Bytes ? reader.ReadInt16() : reader.ReadSByte();
        int arg2 = argsAre2Bytes ? reader.ReadInt16() : reader.ReadSByte();

        if (!argsAreXYValues) MFA_LOG_ERROR("TODO: Args1&2 are point indices to be matched, rather than offsets");

        double offsetX = arg1;
        double offsetY = arg2;

        double iHat_x = 1;
        double iHat_y = 0;
        double jHat_x = 0;
        double jHat_y = 1;

        if (isSingleScaleValue)
        {
            iHat_x = reader.ReadFixedPoint2Dot14();
            jHat_y = iHat_x;
        }
        else if (isXAndYScale)
        {
            iHat_x = reader.ReadFixedPoint2Dot14();
            jHat_y = reader.ReadFixedPoint2Dot14();
        }
        // Todo: incomplete implemntation
        else if (is2x2Matrix)
        {
            iHat_x = reader.ReadFixedPoint2Dot14();
            iHat_y = reader.ReadFixedPoint2Dot14();
            jHat_x = reader.ReadFixedPoint2Dot14();
            jHat_y = reader.ReadFixedPoint2Dot14();
        }

        auto const TransformPoint = [&](double x, double y)->std::tuple<double, double>
        {
            double xPrime = iHat_x * x + jHat_x * y + offsetX;
            double yPrime = iHat_y * x + jHat_y * y + offsetY;
            return std::tuple{xPrime, yPrime};
        };

        uint32_t const currentCompoundGlyphReadLocation = reader.GetLocation();
        ReadGlyph(reader, glyphLocations, glyphIndex, outGlyphData);
        reader.GoTo(currentCompoundGlyphReadLocation);

        for (int i = 0; i < outGlyphData.Points.size(); i++)
        {
            auto const [xPrime, yPrime] = TransformPoint(
                outGlyphData.Points[i].X,
                outGlyphData.Points[i].Y
            );
            outGlyphData.Points[i].X = (int)xPrime;
            outGlyphData.Points[i].Y = (int)yPrime;
        }

        return isMoreComponentsAfterThis;
    }

    //==================================================================================================================

    static void ReadCompoundGlyph(
        BinaryReader & reader,
        uint32_t const * glyphLocations,
        uint32_t glyphIndex,
        FontData::GlyphData & outGlyphData
    )
    {
        outGlyphData.GlyphIndex = glyphIndex;

        uint32_t glyphLocation = glyphLocations[glyphIndex];
        reader.GoTo(glyphLocation);
        reader.Skip<uint8_t>(2);

        outGlyphData.MinX = reader.ReadInt16();
        outGlyphData.MinY = reader.ReadInt16();
        outGlyphData.MaxX = reader.ReadInt16();
        outGlyphData.MaxY = reader.ReadInt16();

        outGlyphData.Points.clear();
        outGlyphData.ContourEndIndices.clear();

        bool hasMoreGlyph = false;
        FontData::GlyphData componentGlyph {};

        do
        {
            hasMoreGlyph = ReadNextComponentGlyph(reader, glyphLocations, glyphLocation, componentGlyph);

            // Add all contour end indices from the simple glyph component to the compound glyph's data
            // Note: indices must be offset to account for previously-added component glyphs
            for (int endIndex : componentGlyph.ContourEndIndices)
            {
                outGlyphData.ContourEndIndices.emplace_back(endIndex + outGlyphData.Points.size());
            }
            outGlyphData.Points.insert(
                outGlyphData.Points.end(),
                componentGlyph.Points.begin(),
                componentGlyph.Points.end()
            );
        } while (hasMoreGlyph == true);
    }

    //==================================================================================================================

    void ReadGlyph(
        BinaryReader & reader,
        uint32_t const * glyphLocations,
        uint32_t glyphIndex,
        FontData::GlyphData & outGlyphData
    )
    {
        uint32_t glyphLocation = glyphLocations[glyphIndex];

        reader.GoTo(glyphLocation);
        int contourCount = reader.ReadInt16();

        // Glyph is either simple or compound
        // * Simple: outline data is stored here directly
        // * Compound: two or more simple glyphs need to be looked up, transformed, and combined
        bool const isSimpleGlyph = contourCount >= 0;

        if (isSimpleGlyph)
        {
            ReadSimpleGlyph(reader, glyphLocations, glyphIndex, outGlyphData);
        }
        else
        {
            ReadCompoundGlyph(reader, glyphLocations, glyphIndex, outGlyphData);
        }
    }

    //==================================================================================================================

    void ReadAllGlyphs(
        BinaryReader & reader,
        uint32_t * glyphLocations,
        uint32_t const mappingsCount,
        GlyphMap const * mappings,
        FontData::GlyphData * outGlyphDataList
    )
    {
        for (int i = 0; i < mappingsCount; i++)
        {
            auto const & mapping = mappings[i];
            auto & glyphData = outGlyphDataList[i];
            ReadGlyph(reader, glyphLocations, mapping.GlyphIndex, glyphData);
            glyphData.UnicodeValue = mapping.Unicode;
        }
    }

    //==================================================================================================================

}