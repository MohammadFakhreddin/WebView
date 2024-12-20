#include "FontParser.hpp"

#include "BinaryReader.hpp"

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

    static Table ReadTableLocations(BinaryReader & reader)
    {
        Table tableLocations {};
        // -- offset subtable --
        reader.Skip<uint8_t>(4);                        // unused: scalerType
        int const numTables = (int)reader.ReadUInt16();
        reader.Skip<uint8_t>(6);                        // unused: searchRange, entrySelector, rangeShift

        // -- table directory --
        for (int i = 0; i < numTables; i++)
        {
            char tag[4]{};
            reader.ReadString(4, tag);

            uint32_t const checksum = reader.ReadUInt32();
            uint32_t const offset = reader.ReadUInt32();
            uint32_t const length = reader.ReadUInt32();

            // MFA_ASSERT(tableLocations.find(tag) == tableLocations.end());
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
        uint32_t const glyphIndex,
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
        uint32_t const glyphIndex,
        FontData::GlyphData & outGlyphData
    )
    {
        uint32_t const glyphLocation = glyphLocations[glyphIndex];

        reader.GoTo(glyphLocation);
        int const contourCount = reader.ReadInt16();

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
        uint32_t const * glyphLocations,
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

    static void GetAllGlyphLocations(
        BinaryReader & reader,
        int const numGlyphs,
        int const bytesPerEntry,
        uint32_t const localTableLocation,
        uint32_t const glyphTableLocation,
        uint32_t * glyphLocations
    )
    {
        bool const isTwoByteEntry = bytesPerEntry == 2;

        for (int glyphIndex = 0; glyphIndex < numGlyphs; glyphIndex++)
        {
            reader.GoTo(localTableLocation + glyphIndex * bytesPerEntry);
            // If 2-byte format is used, the stored location is half of actual location (so multiply by 2)
            auto const glyphDataOffset = isTwoByteEntry ? reader.ReadUInt16() * 2u : reader.ReadUInt32();
            glyphLocations[glyphIndex] = glyphTableLocation + glyphDataOffset;
        }
    }

    //==================================================================================================================

    // Create a lookup from unicode to font's internal glyph index
    static std::vector<GlyphMap> GetUnicodeToGlyphIndexMappings(BinaryReader & reader, uint32_t const cmapOffset)
    {
        std::vector<GlyphMap> glyphPairs {};
        reader.GoTo(cmapOffset);

        uint version = reader.ReadUInt16();
        uint numSubtables = reader.ReadUInt16(); // font can contain multiple character maps for different platforms

        // --- Read through metadata for each character map to find the one we want to use ---
        uint cmapSubtableOffset = 0;
        int selectedUnicodeVersionID = -1;

        for (int i = 0; i < numSubtables; i++)
        {
            int platformID = reader.ReadUInt16();
            int platformSpecificID = reader.ReadUInt16();
            uint offset = reader.ReadUInt32();

            // Unicode encoding
            if (platformID == 0)
            {
                // Use highest supported unicode version
                // if (platformSpecificID is 0 or 1 or 3 or 4 && platformSpecificID > selectedUnicodeVersionID)
                if (
                    (platformSpecificID == 0 || platformSpecificID == 1 || platformSpecificID == 3 || platformSpecificID == 4) &&
                    platformSpecificID > selectedUnicodeVersionID)
                {
                    cmapSubtableOffset = offset;
                    selectedUnicodeVersionID = platformSpecificID;
                }
            }
            // Microsoft Encoding
            else if (platformID == 3 && selectedUnicodeVersionID == -1)
            {
                if (platformSpecificID == 1 || platformSpecificID == 10)
                {
                    cmapSubtableOffset = offset;
                }
            }
        }

        if (cmapSubtableOffset == 0)
        {
            MFA_LOG_ERROR("Font does not contain supported character map type (TODO)");
        }

        // Go to the character map
        reader.GoTo(cmapOffset + cmapSubtableOffset);
        int format = reader.ReadUInt16();
        bool hasReadMissingCharGlyph = false;

        if (format != 12 && format != 4)
        {
            MFA_LOG_ERROR("Font cmap format not supported (TODO): " + format);
        }

        // ---- Parse Format 4 ----
        if (format == 4)
        {
            int length = reader.ReadUInt16();
            int languageCode = reader.ReadUInt16();
            // Number of contiguous segments of character codes
            int segCount2X = reader.ReadUInt16();
            int segCount = segCount2X / 2;
            reader.Skip<uint8_t>(6); // Skip: searchRange, entrySelector, rangeShift

            // Ending character code for each segment (last = 2^16 - 1)
            // int[] endCodes = new int[segCount];
            std::vector<int> endCodes(segCount);
            for (int i = 0; i < segCount; i++)
            {
                endCodes[i] = reader.ReadUInt16();
            }

            // Skip16BitEntries(1);
            reader.Skip<uint16_t>(1); // Reserved pad

            // int[] startCodes = new int[segCount];
            std::vector<int> startCodes(segCount);
            for (int i = 0; i < segCount; i++)
            {
                startCodes[i] = reader.ReadUInt16();
            }

            // int[] idDeltas = new int[segCount];
            std::vector<int> idDeltas(segCount);
            for (int i = 0; i < segCount; i++)
            {
                idDeltas[i] = reader.ReadUInt16();
            }

            // (int offset, int readLoc)[] idRangeOffsets = new (int, int)[segCount];
            struct IdRangeOffset {int offset; int readLoc; };
            std::vector<IdRangeOffset> idRangeOffsets(segCount);
            for (int i = 0; i < segCount; i++)
            {
                int readLoc = (int)reader.GetLocation();
                int offset = reader.ReadUInt16();
                idRangeOffsets[i] = IdRangeOffset{ .offset = offset, .readLoc = readLoc};
            }

            for (int i = 0; i < startCodes.size(); i++)
            {
                int endCode = endCodes[i];
                int currCode = startCodes[i];

                if (currCode == 65535) break; // not sure about this (hack to avoid out of bounds on a specific font)

                while (currCode <= endCode)
                {
                    int glyphIndex;
                    // If idRangeOffset is 0, the glyph index can be calculated directly
                    if (idRangeOffsets[i].offset == 0)
                    {
                        glyphIndex = (currCode + idDeltas[i]) % 65536;
                    }
                    // Otherwise, glyph index needs to be looked up from an array
                    else
                    {
                        uint readerLocationOld = reader.GetLocation();
                        int rangeOffsetLocation = idRangeOffsets[i].readLoc + idRangeOffsets[i].offset;
                        int glyphIndexArrayLocation = 2 * (currCode - startCodes[i]) + rangeOffsetLocation;

                        reader.GoTo(glyphIndexArrayLocation);
                        glyphIndex = reader.ReadUInt16();

                        if (glyphIndex != 0)
                        {
                            glyphIndex = (glyphIndex + idDeltas[i]) % 65536;
                        }

                        reader.GoTo(readerLocationOld);
                    }

                    glyphPairs.emplace_back((uint32_t)glyphIndex, (uint32_t)currCode);
                    hasReadMissingCharGlyph |= glyphIndex == 0;
                    currCode++;
                }
            }
        }
        // ---- Parse Format 12 ----
        else if (format == 12)
        {
            reader.Skip<uint8_t>(10); // Skip: reserved, subtableByteLengthInlcudingHeader, languageCode
            uint numGroups = reader.ReadUInt32();

            for (int i = 0; i < numGroups; i++)
            {
                uint startCharCode = reader.ReadUInt32();
                uint endCharCode = reader.ReadUInt32();
                uint startGlyphIndex = reader.ReadUInt32();

                uint numChars = endCharCode - startCharCode + 1;
                for (int charCodeOffset = 0; charCodeOffset < numChars; charCodeOffset++)
                {
                    uint charCode = (uint)(startCharCode + charCodeOffset);
                    uint glyphIndex = (uint)(startGlyphIndex + charCodeOffset);

                    glyphPairs.emplace_back(glyphIndex, charCode);
                    hasReadMissingCharGlyph |= glyphIndex == 0;
                }
            }
        }

        if (!hasReadMissingCharGlyph)
        {
            glyphPairs.emplace_back(0, 65535);
        }

        return glyphPairs;
    }

    //==================================================================================================================

    FontData Parse(std::shared_ptr<MFA::Blob> rawFontData)
    {
        BinaryReader reader(std::move(rawFontData));

        auto tableLocationLookup = ReadTableLocations(reader);

        MFA_ASSERT(tableLocationLookup.contains("glyf"));
        auto const glyphTableLocation = tableLocationLookup["glyf"];
        MFA_ASSERT(tableLocationLookup.contains("loca"));
        auto const locaTableLocation = tableLocationLookup["loca"];
        MFA_ASSERT(tableLocationLookup.contains("cmap"));
        auto const cmapLocation = tableLocationLookup["cmap"];

        // ---- Read Head Table ----
        MFA_ASSERT(tableLocationLookup.contains("head"));
        auto const headLocation = tableLocationLookup["head"];
        reader.GoTo(headLocation);
        reader.Skip<uint8_t>(18);
        // Design units to Em size (range from 64 to 16384)
        int unitsPerEm = reader.ReadUInt16();
        reader.Skip<uint8_t>(30);
        // Number of bytes used by the offsets in the 'loca' table (for looking up glyph locations)
        int numBytesPerLocationLookup = (reader.ReadInt16() == 0 ? 2 : 4);

        // --- Read 'maxp' table ---
        reader.GoTo(tableLocationLookup["maxp"]);
        reader.Skip<uint8_t>(4);

        int const numGlyphs = reader.ReadUInt16();
        std::vector<uint32_t> glyphLocations(numGlyphs);

        GetAllGlyphLocations(
            reader,
            numGlyphs,
            numBytesPerLocationLookup,
            locaTableLocation,
            glyphTableLocation,
            glyphLocations.data()
        );

        auto const mappings = GetUnicodeToGlyphIndexMappings(reader, cmapLocation);
        std::vector<FontData::GlyphData> glyphs(mappings.size());
        ReadAllGlyphs(reader, glyphLocations.data(), mappings.size(), mappings.data(), glyphs.data());

        {
            struct LayoutData {int advance; int left;};
            std::vector<LayoutData> layoutData(numGlyphs);

            // Get number of metrics from the 'hhea' table
            reader.GoTo(tableLocationLookup["hhea"]);

            reader.Skip<uint8_t>(8); // unused: version, ascent, descent
            int lineGap = reader.ReadInt16();
            int advanceWidthMax = reader.ReadInt16();
            reader.Skip<uint8_t>(22); // unused: minLeftSideBearing, minRightSideBearing, xMaxExtent, caretSlope/Offset, reserved, metricDataFormat
            int numAdvanceWidthMetrics = reader.ReadInt16();

            // Get the advance width and leftsideBearing metrics from the 'hmtx' table
            reader.GoTo(tableLocationLookup["hmtx"]);
            int lastAdvanceWidth = 0;

            for (int i = 0; i < numAdvanceWidthMetrics; i++)
            {
                int advanceWidth = reader.ReadUInt16();
                int leftSideBearing = reader.ReadInt16();
                lastAdvanceWidth = advanceWidth;

                layoutData[i] = LayoutData{ .advance = advanceWidth, .left = leftSideBearing};
            }

            // Some fonts have a run of monospace characters at the end
            int numRem = numGlyphs - numAdvanceWidthMetrics;

            for (int i = 0; i < numRem; i++)
            {
                int leftSideBearing = reader.ReadInt16();
                int glyphIndex = numAdvanceWidthMetrics + i;

                layoutData[glyphIndex] = LayoutData{ .advance = lastAdvanceWidth, .left = leftSideBearing};
            }

            // Apply
            for (auto & c : glyphs)
            {
                c.AdvanceWidth = layoutData[c.GlyphIndex].advance;
                c.LeftSideBearing = layoutData[c.GlyphIndex].left;
            }
        }

        FontData fontData((int)glyphs.size(), glyphs.data(), unitsPerEm);
        return fontData;

    }

    //==================================================================================================================

} // namespace Shared::FontParser
