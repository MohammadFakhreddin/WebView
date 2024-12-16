#pragma once

#include "BedrockAssert.hpp"
#include "BedrockMemory.hpp"

#include <memory>

namespace Shared
{
    class BinaryReader
    {
    public:

        static constexpr bool isLittleEndian = (std::endian::native == std::endian::little);

        using Byte = uint8_t;
        using SByte = int8_t;
        using UInt16 = uint16_t;
        using Int16 = int16_t;
        using UInt32 = uint32_t;
        using Int32 = int32_t;
        using Offset = uint64_t;

        explicit BinaryReader(std::shared_ptr<MFA::Blob> data);

        template <typename T>
        void Skip(Offset const count)
        {
            _ptr += count * sizeof(T);
        }

        void GoTo(Offset offsetFromOrigin, Offset &previousOffset);

        void GoTo(Offset offsetFromOrigin);

        [[nodiscard]] Offset GetLocation() const;

        void ReadString(Offset count, char *outChars);

        [[nodiscard]] Byte ReadByte();

        [[nodiscard]] SByte ReadSByte();

        [[nodiscard]] UInt16 ReadUInt16();

        [[nodiscard]] Int16 ReadInt16();

        [[nodiscard]] UInt32 ReadUInt32();

        [[nodiscard]] Int32 ReadInt32();

        [[nodiscard]] double ReadFixedPoint2Dot14();

        [[nodiscard]] static double UInt16ToFixedPoint2Dot14(UInt16 raw);

    private:

        static UInt32 ToLittleEndian(UInt32 bigEndianValue);

        static UInt16 ToLittleEndian(UInt16 bigEndianValue);

        std::shared_ptr<MFA::Blob> _data{};
        Byte * _ptr{};

    };
}
