#include "BinaryReader.hpp"

namespace Shared
{

    //==================================================================================================================

    BinaryReader::BinaryReader(std::shared_ptr<MFA::Blob> data)
    : _data(std::move(data))
    {
        _ptr = _data->As<Byte>();
    }

    //==================================================================================================================

    void BinaryReader::GoTo(Offset const offsetFromOrigin, Offset &previousOffset)
    {
        previousOffset = _ptr - _data->As<Byte>();
        _ptr = _data->As<Byte>() + offsetFromOrigin * sizeof(Byte);
    }

    //==================================================================================================================

    void BinaryReader::GoTo(Offset const offsetFromOrigin)
    {
        _ptr = _data->As<Byte>() + offsetFromOrigin * sizeof(Byte);
    }

    //==================================================================================================================

    BinaryReader::Offset BinaryReader::GetLocation() const
    {
        return _ptr - _data->As<Byte>();
    }

    //==================================================================================================================

    void BinaryReader::ReadString(Offset const count, char *outChars)
    {
        for (int i = 0; i < count; i++)
        {
            static_assert(sizeof(Byte) == sizeof(char));
            outChars[i] = (char)ReadByte();
        }
    }

    //==================================================================================================================

    BinaryReader::Byte BinaryReader::ReadByte()
    {
        auto const value = (Byte)*_ptr;
        _ptr = _ptr + sizeof(Byte);
        return value;
    }

    //==================================================================================================================

    BinaryReader::SByte BinaryReader::ReadSByte()
    {
        auto const value = (SByte)*_ptr;
        _ptr = _ptr + sizeof(SByte);
        return value;
    }

    //==================================================================================================================

    BinaryReader::UInt16 BinaryReader::ReadUInt16()
    {
        auto value = (UInt16)*_ptr;
        _ptr = _ptr + sizeof(UInt16);
        if constexpr (isLittleEndian == false)
        {
            value = ToLittleEndian(value);
        }
        return value;
    }

    //==================================================================================================================

    BinaryReader::Int16 BinaryReader::ReadInt16()
    {
        return (Int16)ReadUInt16();
    }

    //==================================================================================================================

    BinaryReader::UInt32 BinaryReader::ReadUInt32()
    {
        auto value = (UInt32)*_ptr;
        _ptr = _ptr + sizeof(UInt32);
        if constexpr (isLittleEndian == false)
        {
            value = ToLittleEndian(value);
        }
        return value;
    }

    //==================================================================================================================

    BinaryReader::Int32 BinaryReader::ReadInt32()
    {
        return (Int32)ReadUInt32();
    }

    //==================================================================================================================

    BinaryReader::UInt32 BinaryReader::ToLittleEndian(UInt32 const bigEndianValue)
    {
        static constexpr Byte ByteMask = 0b11111111;

        UInt32 const a = (bigEndianValue >> 24) & ByteMask;
        UInt32 const b = (bigEndianValue >> 16) & ByteMask;
        UInt32 const c = (bigEndianValue >> 8) & ByteMask;
        UInt32 const d = (bigEndianValue >> 0) & ByteMask;
        return a | b << 8 | c << 16 | d << 24;
    }

    //==================================================================================================================

    BinaryReader::UInt16 BinaryReader::ToLittleEndian(UInt16 const bigEndianValue)
    {
        return (UInt16)(bigEndianValue >> 8 | bigEndianValue << 8);
    }

    //==================================================================================================================

    double BinaryReader::ReadFixedPoint2Dot14()
    {
        return UInt16ToFixedPoint2Dot14(ReadUInt16());
    }

    //==================================================================================================================

    double BinaryReader::UInt16ToFixedPoint2Dot14(UInt16 const raw)
    {
        return (Int16)(raw) / (double)(1 << 14);
    }

    //==================================================================================================================

} // namespace Shared
