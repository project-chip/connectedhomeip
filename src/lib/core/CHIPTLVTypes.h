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

#ifndef CHIPTLVTYPES_H_
#define CHIPTLVTYPES_H_

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
    kTLVType_Null                = 0x14,
    kTLVType_Structure           = 0x15,
    kTLVType_Array               = 0x16,
    kTLVType_Path                = 0x17
};

// TODO: Move to private namespace
enum TLVElementType
{
    kTLVElementType_NotSpecified           = -1,
    kTLVElementType_Int8                   = 0x00,
    kTLVElementType_Int16                  = 0x01,
    kTLVElementType_Int32                  = 0x02,
    kTLVElementType_Int64                  = 0x03,
    kTLVElementType_UInt8                  = 0x04,
    kTLVElementType_UInt16                 = 0x05,
    kTLVElementType_UInt32                 = 0x06,
    kTLVElementType_UInt64                 = 0x07,
    kTLVElementType_BooleanFalse           = 0x08,
    kTLVElementType_BooleanTrue            = 0x09,
    kTLVElementType_FloatingPointNumber32  = 0x0A,
    kTLVElementType_FloatingPointNumber64  = 0x0B,
    kTLVElementType_UTF8String_1ByteLength = 0x0C,
    kTLVElementType_UTF8String_2ByteLength = 0x0D,
    kTLVElementType_UTF8String_4ByteLength = 0x0E,
    kTLVElementType_UTF8String_8ByteLength = 0x0F,
    kTLVElementType_ByteString_1ByteLength = 0x10,
    kTLVElementType_ByteString_2ByteLength = 0x11,
    kTLVElementType_ByteString_4ByteLength = 0x12,
    kTLVElementType_ByteString_8ByteLength = 0x13,
    kTLVElementType_Null                   = 0x14,
    kTLVElementType_Structure              = 0x15,
    kTLVElementType_Array                  = 0x16,
    kTLVElementType_Path                   = 0x17,
    kTLVElementType_EndOfContainer         = 0x18
};

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
inline bool IsValidTLVType(uint8_t type)
{
    return type <= kTLVElementType_EndOfContainer;
}

/**
 * Returns true if the specified TLV type implies the presence of an associated value field.
 *
 * @return @p true if the specified TLV type implies the presence of an associated value field; otherwise @p false.
 */
inline bool TLVTypeHasValue(uint8_t type)
{
    return (type <= kTLVElementType_UInt64 ||
            (type >= kTLVElementType_FloatingPointNumber32 && type <= kTLVElementType_ByteString_8ByteLength));
}

/**
 * Returns true if the specified TLV type implies the presence of an associated length field.
 *
 * @return @p true if the specified TLV type implies the presence of an associated length field; otherwise @p false.
 */
inline bool TLVTypeHasLength(uint8_t type)
{
    return type >= kTLVElementType_UTF8String_1ByteLength && type <= kTLVElementType_ByteString_8ByteLength;
}

/**
 * Returns true if the specified TLV type is a container.
 *
 * @return @p true if the specified TLV type is a container; otherwise @p false.
 */
inline bool TLVTypeIsContainer(uint8_t type)
{
    return type >= kTLVElementType_Structure && type <= kTLVElementType_Path;
}

/**
 * Returns true if the specified TLV type is a UTF8 or byte string.
 *
 * @return @p true if the specified TLV type is a UTF8 or byte string; otherwise @p false.
 */
inline bool TLVTypeIsString(uint8_t type)
{
    return type >= kTLVElementType_UTF8String_1ByteLength && type <= kTLVElementType_ByteString_8ByteLength;
}

// TODO: move to private namespace
inline TLVFieldSize GetTLVFieldSize(uint8_t type)
{
    if (TLVTypeHasValue(type))
        return static_cast<TLVFieldSize>(type & kTLVTypeSizeMask);
    return kTLVFieldSize_0Byte;
}

// TODO: move to private namespace
inline uint8_t TLVFieldSizeToBytes(TLVFieldSize fieldSize)
{
    // We would like to assert fieldSize < 7, but that gives us fatal
    // -Wtautological-constant-out-of-range-compare warnings...
    return (fieldSize != kTLVFieldSize_0Byte) ? static_cast<uint8_t>(1 << fieldSize) : 0;
}

} // namespace TLV
} // namespace chip

#endif /* CHIPTLVTYPES_H_ */
