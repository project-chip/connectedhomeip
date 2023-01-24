/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file contains definitions for working with CHIP TLV types.
 *
 */

#pragma once

#include <cstdint>

namespace chip {
namespace TLV {

/**
 * An enumeration identifying the type of a TLV element.
 */
enum TLVType
{
    kTLVType_NotSpecified     = -1,
    kTLVType_UnknownContainer = -2,

    kTLVType_SignedInteger       = 0x00,
    kTLVType_UnsignedInteger     = 0x04,
    kTLVType_Boolean             = 0x08,
    kTLVType_FloatingPointNumber = 0x0A,
    kTLVType_UTF8String          = 0x0C,
    kTLVType_ByteString          = 0x10,
    // IMPORTANT: Values starting at Null must match the corresponding values of
    // TLVElementType.
    kTLVType_Null      = 0x14,
    kTLVType_Structure = 0x15,
    kTLVType_Array     = 0x16,
    kTLVType_List      = 0x17
};

// TODO: Move to private namespace
enum class TLVElementType : int8_t
{
    // IMPORTANT: All values here except NotSpecified must have no bits in
    // common with values of TagControl.
    NotSpecified           = -1,
    Int8                   = 0x00,
    Int16                  = 0x01,
    Int32                  = 0x02,
    Int64                  = 0x03,
    UInt8                  = 0x04,
    UInt16                 = 0x05,
    UInt32                 = 0x06,
    UInt64                 = 0x07,
    BooleanFalse           = 0x08,
    BooleanTrue            = 0x09,
    FloatingPointNumber32  = 0x0A,
    FloatingPointNumber64  = 0x0B,
    UTF8String_1ByteLength = 0x0C,
    UTF8String_2ByteLength = 0x0D,
    UTF8String_4ByteLength = 0x0E,
    UTF8String_8ByteLength = 0x0F,
    ByteString_1ByteLength = 0x10,
    ByteString_2ByteLength = 0x11,
    ByteString_4ByteLength = 0x12,
    ByteString_8ByteLength = 0x13,
    // IMPORTANT: Values starting at Null must match the corresponding values of
    // TLVType.
    Null           = 0x14,
    Structure      = 0x15,
    Array          = 0x16,
    List           = 0x17,
    EndOfContainer = 0x18
};

template <typename T>
inline bool operator<=(const T & lhs, TLVElementType rhs)
{
    return lhs <= static_cast<int8_t>(rhs);
}

template <typename T>
inline bool operator>=(const T & lhs, TLVElementType rhs)
{
    return lhs >= static_cast<int8_t>(rhs);
}

// TODO: Move to private namespace
enum TLVFieldSize
{
    kTLVFieldSize_0Byte = -1,
    kTLVFieldSize_1Byte = 0,
    kTLVFieldSize_2Byte = 1,
    kTLVFieldSize_4Byte = 2,
    kTLVFieldSize_8Byte = 3
};

// TODO: Move to private namespace
enum
{
    kTLVTypeMask     = 0x1F,
    kTLVTypeSizeMask = 0x03
};

/**
 * Returns true if the specified TLV type is valid.
 *
 * @return @p true if the specified TLV type is valid; otherwise @p false.
 */
inline bool IsValidTLVType(TLVElementType type)
{
    return type <= TLVElementType::EndOfContainer;
}

/**
 * Returns true if the specified TLV type implies the presence of an associated value field.
 *
 * @return @p true if the specified TLV type implies the presence of an associated value field; otherwise @p false.
 */
inline bool TLVTypeHasValue(TLVElementType type)
{
    return (type <= TLVElementType::UInt64 ||
            (type >= TLVElementType::FloatingPointNumber32 && type <= TLVElementType::ByteString_8ByteLength));
}

/**
 * Returns true if the specified TLV type implies the presence of an associated length field.
 *
 * @return @p true if the specified TLV type implies the presence of an associated length field; otherwise @p false.
 */
inline bool TLVTypeHasLength(TLVElementType type)
{
    return type >= TLVElementType::UTF8String_1ByteLength && type <= TLVElementType::ByteString_8ByteLength;
}

/**
 * Returns true if the specified TLV type is a container.
 *
 * @return @p true if the specified TLV type is a container; otherwise @p false.
 */
inline bool TLVTypeIsContainer(TLVElementType type)
{
    return type >= TLVElementType::Structure && type <= TLVElementType::List;
}

inline bool TLVTypeIsContainer(TLVType type)
{
    return type >= kTLVType_Structure && type <= kTLVType_List;
}

/**
 * Returns true if the specified TLV type is a UTF8 or byte string.
 *
 * @return @p true if the specified TLV type is a UTF8 or byte string; otherwise @p false.
 */
inline bool TLVTypeIsString(TLVElementType type)
{
    return type >= TLVElementType::UTF8String_1ByteLength && type <= TLVElementType::ByteString_8ByteLength;
}

/**
 * Returns true if the specified TLV type is a UTF8 string.
 *
 * @return @p true if the specified TLV type is a UTF8 string; otherwise @p false.
 */
inline bool TLVTypeIsUTF8String(TLVElementType type)
{
    return type >= TLVElementType::UTF8String_1ByteLength && type <= TLVElementType::UTF8String_8ByteLength;
}

/**
 * Returns true if the specified TLV type is a byte string.
 *
 * @return @p true if the specified TLV type is a byte string; otherwise @p false.
 */
inline bool TLVTypeIsByteString(TLVElementType type)
{
    return type >= TLVElementType::ByteString_1ByteLength && type <= TLVElementType::ByteString_8ByteLength;
}

// TODO: move to private namespace
inline TLVFieldSize GetTLVFieldSize(TLVElementType type)
{
    if (TLVTypeHasValue(type))
        return static_cast<TLVFieldSize>(static_cast<uint8_t>(type) & kTLVTypeSizeMask);
    return kTLVFieldSize_0Byte;
}

// TODO: move to private namespace
inline uint8_t TLVFieldSizeToBytes(TLVFieldSize fieldSize)
{
    // We would like to assert fieldSize < 7, but that gives us fatal
    // -Wtautological-constant-out-of-range-compare warnings...
    return static_cast<uint8_t>((fieldSize != kTLVFieldSize_0Byte) ? (1 << fieldSize) : 0);
}

} // namespace TLV
} // namespace chip
