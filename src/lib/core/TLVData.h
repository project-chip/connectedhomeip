/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file contains macros to create static CHIP TLV data.
 *
 *    These macros are designed to be used in statically initializing a byte array
 *    containing various TLV elements. TLVWriter can achieve similar functionality
 *    but only on RAM, while macros here can be used to create TLV elements in ROM.
 *
 */

#pragma once

#include <stdint.h>

#include <lib/core/TLV.h>

/*
 *  @brief Integral truncate L to the least significant 32-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_GetLower32From64(v) ((uint32_t) (((uint64_t) (v) >> 0) & 0xFFFFFFFFUL))

/*
 *  @brief Integral truncate argument X to the next least significant 32-bit
 *
 *  @note Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_GetUpper32From64(v) ((uint32_t) (((uint64_t) (v) >> 32) & 0xFFFFFFFFUL))

/*
 *  @brief Integral truncate L to the least significant 16-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_GetLower16From32(v) ((uint16_t) (((uint32_t) (v) >> 0) & 0xFFFFU))

/*
 *  @brief Integral truncate argument X to the next least significant 16-bit
 *
 *  @note Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_GetUpper16From32(v) ((uint16_t) (((uint32_t) (v) >> 16) & 0xFFFFU))

/*
 *  @brief Integral truncate L to the least significant 8-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_GetLower8From16(v) ((uint8_t) (((uint16_t) (v) >> 0) & 0xFFU))

/*
 *  @brief Integral truncate argument X to the next least significant 8-bit
 *
 *  @note Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_GetUpper8From16(v) ((uint8_t) (((uint16_t) (v) >> 8) & 0xFFU))

/*
 *  @brief Integral truncate argument v to 8-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_Serialize8(v) ((uint8_t) (v))

/*
 *  @brief Integral truncate argument v to 16-bit, and then serialize it using CHIP standard Little Endian order
 *
 *  @note Integral truncate would preserve the least significant bits, regardless of hardware endianness.
 *     Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define CHIP_TLV_Serialize16(v) CHIP_TLV_GetLower8From16(v), CHIP_TLV_GetUpper8From16(v)

/*
 *  @brief Integral truncate argument v to 32-bit, and then serialize it using CHIP standard Little Endian order
 */
#define CHIP_TLV_Serialize32(v)                                                                                                    \
    CHIP_TLV_Serialize16(CHIP_TLV_GetLower16From32(v)), CHIP_TLV_Serialize16(CHIP_TLV_GetUpper16From32(v))

/*
 *  @brief Integral truncate argument v to 64-bit, and then serialize it using CHIP standard Little Endian order
 */
#define CHIP_TLV_Serialize64(v)                                                                                                    \
    CHIP_TLV_Serialize32(CHIP_TLV_GetLower32From64(v)), CHIP_TLV_Serialize32(CHIP_TLV_GetUpper32From64(v))

/*
 *  @brief Specifies an anonymous TLV element, which doesn't have any tag
 */
#define CHIP_TLV_TAG_ANONYMOUS chip::TLV::TLVTagControl::Anonymous

/*
 *  @brief Specifies a TLV element with a context-specific tag
 *  @param Tag      The context-specific tag for this TLV element. Would be truncated to 8 bits.
 */
#define CHIP_TLV_TAG_CONTEXT_SPECIFIC(Tag) chip::TLV::TLVTagControl::ContextSpecific, CHIP_TLV_Serialize8(Tag)

/*
 *  @brief Specifies a TLV element with a Common Profile tag
 *  @param Tag      The tag for this TLV element, defined under Common Profile.
 *                  Would be truncated to 16 bites.
 */
#define CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(Tag) chip::TLV::TLVTagControl::CommonProfile_2Bytes, CHIP_TLV_Serialize16(Tag)

/*
 *  @brief Specifies a TLV element with a Common Profile tag
 *  @param Tag      The tag for this TLV element, defined under Common Profile.
 *                  Would be truncated to 32 bites.
 */
#define CHIP_TLV_TAG_COMMON_PROFILE_4Bytes(Tag) chip::TLV::TLVTagControl::CommonProfile_4Bytes, CHIP_TLV_Serialize32(Tag)

/*
 *  @brief Specifies a TLV element with an Implicit Profile tag
 *  @param Tag      The tag for this TLV element, defined under the current implicit profile.
 *                  Would be truncated to 16 bits.
 */
#define CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(Tag) chip::TLV::TLVTagControl::ImplicitProfile_2Bytes, CHIP_TLV_Serialize16(Tag)

/*
 *  @brief Specifies a TLV element with an Implicit Profile tag
 *  @param Tag      The tag for this TLV element, defined under the current implicit profile.
 *                  Would be truncated to 32 bits.
 */
#define CHIP_TLV_TAG_IMPLICIT_PROFILE_4Bytes(Tag) chip::TLV::TLVTagControl::ImplicitProfile_4Bytes, CHIP_TLV_Serialize32(Tag)

/*
 *  @brief Specifies a TLV element with a Fully Qualified tag
 *  @param ProfileId    {Vendor ID, Profile Number}, as in #CHIPProfileId
 *  @param Tag          The tag for this TLV element, defined under ProfileId.
 *                      Would be truncated to 16 bits.
 */
#define CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(ProfileId, Tag)                                                                        \
    chip::TLV::TLVTagControl::FullyQualified_6Bytes, CHIP_TLV_Serialize16(ProfileId >> 16), CHIP_TLV_Serialize16(ProfileId),       \
        CHIP_TLV_Serialize16(Tag)

/*
 *  @brief Specifies a TLV element with a Fully Qualified tag
 *  @param ProfileId    {Vendor ID, Profile Number}, as in #CHIPProfileId
 *  @param Tag          The tag for this TLV element, defined under ProfileId.
 *                      Would be truncated to 32 bits.
 */
#define CHIP_TLV_TAG_FULLY_QUALIFIED_8Bytes(ProfileId, Tag)                                                                        \
    chip::TLV::TLVTagControl::FullyQualified_8Bytes, CHIP_TLV_Serialize16(ProfileId >> 16), CHIP_TLV_Serialize16(ProfileId),       \
        CHIP_TLV_Serialize32(Tag)

/*
 *  @brief Specifies a NULL TLV element, which has just the tag but no value
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 */
#define CHIP_TLV_NULL(TagSpec) chip::TLV::TLVElementType::Null | TagSpec

/*
 *  @brief Specifies a Structure TLV element, marking the beginning of a Structure
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 */
#define CHIP_TLV_STRUCTURE(TagSpec) chip::TLV::TLVElementType::Structure | TagSpec

/*
 *  @brief Specifies a Array TLV element, marking the beginning of an Array
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 */
#define CHIP_TLV_ARRAY(TagSpec) chip::TLV::TLVElementType::Array | TagSpec

/*
 *  @brief Specifies a List TLV element, marking the beginning of a List
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 */
#define CHIP_TLV_LIST(TagSpec) chip::TLV::TLVElementType::List | TagSpec

/*
 *  @brief Specifies a Boolean TLV element, which can be either true or false
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Should be either true or false
 */
#define CHIP_TLV_BOOL(TagSpec, Value)                                                                                              \
    ((Value) ? chip::TLV::TLVElementType::BooleanTrue : chip::TLV::TLVElementType::BooleanFalse) | TagSpec

/**
 *  @brief
 *    Specifies a Single Precision Floating Point TLV element, marking the beginning of 32-bit data
 *
 *  @param TagSpec      Should be filled with macros beginning with CHIP_TLV_TAG_
 *
 *  @param ...          Bytes representing the floating point value to serialize
 */
#define CHIP_TLV_FLOAT32(TagSpec, ...) chip::TLV::TLVElementType::FloatingPointNumber32 | TagSpec, ##__VA_ARGS__

/**
 *  @brief
 *    Specifies a Double Precision Floating Point TLV element, marking the beginning of 64-bit data
 *
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param ...          Bytes representing the floating point value to serialize
 */
#define CHIP_TLV_FLOAT64(TagSpec, ...) chip::TLV::TLVElementType::FloatingPointNumber64 | TagSpec, ##__VA_ARGS__

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define CHIP_TLV_END_OF_CONTAINER chip::TLV::TLVElementType::EndOfContainer | chip::TLV::TLVTagControl::Anonymous

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define CHIP_TLV_END_OF_STRUCTURE CHIP_TLV_END_OF_CONTAINER

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define CHIP_TLV_END_OF_ARRAY CHIP_TLV_END_OF_CONTAINER

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define CHIP_TLV_END_OF_PATH CHIP_TLV_END_OF_CONTAINER

/*
 *  @brief Specifies an 8-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to int8_t, and then serialized
 */
#define CHIP_TLV_INT8(TagSpec, Value) chip::TLV::TLVElementType::Int8 | TagSpec, CHIP_TLV_Serialize8(int8_t(Value))

/*
 *  @brief Specifies a 16-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to int16_t, and then serialized
 */
#define CHIP_TLV_INT16(TagSpec, Value) chip::TLV::TLVElementType::Int16 | TagSpec, CHIP_TLV_Serialize16(int16_t(Value))

/*
 *  @brief Specifies a 32-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to int32_t, and then serialized
 */
#define CHIP_TLV_INT32(TagSpec, Value) chip::TLV::TLVElementType::Int32 | TagSpec, CHIP_TLV_Serialize32(int32_t(Value))

/*
 *  @brief Specifies a 32-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to int64_t, and then serialized
 */
#define CHIP_TLV_INT64(TagSpec, Value) chip::TLV::TLVElementType::Int64 | TagSpec, CHIP_TLV_Serialize64(int64_t(Value))

/*
 *  @brief Specifies an 8-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to (uint8_t), and then serialized
 */
#define CHIP_TLV_UINT8(TagSpec, Value) chip::TLV::TLVElementType::UInt8 | TagSpec, CHIP_TLV_Serialize8((uint8_t) (Value))

/*
 *  @brief Specifies a 16-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to (uint16_t), and then serialized
 */
#define CHIP_TLV_UINT16(TagSpec, Value) chip::TLV::TLVElementType::UInt16 | TagSpec, CHIP_TLV_Serialize16((uint16_t) (Value))

/*
 *  @brief Specifies a 32-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to (uint32_t), and then serialized
 */
#define CHIP_TLV_UINT32(TagSpec, Value) chip::TLV::TLVElementType::UInt32 | TagSpec, CHIP_TLV_Serialize32((uint32_t) (Value))

/*
 *  @brief Specifies a 64-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with CHIP_TLV_TAG_
 *  @param Value        Would be first converted to (uint64_t), and then serialized
 */
#define CHIP_TLV_UINT64(TagSpec, Value) chip::TLV::TLVElementType::UInt64 | TagSpec, CHIP_TLV_Serialize64((uint64_t) (Value))

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define CHIP_TLV_UTF8_STRING_1ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::UTF8String_1ByteLength | TagSpec, CHIP_TLV_Serialize8((uint8_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define CHIP_TLV_UTF8_STRING_2ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::UTF8String_2ByteLength | TagSpec, CHIP_TLV_Serialize16((uint16_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100000000
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define CHIP_TLV_UTF8_STRING_4ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::UTF8String_4ByteLength | TagSpec, CHIP_TLV_Serialize32((uint32_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000000000000000
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define CHIP_TLV_UTF8_STRING_8ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::UTF8String_8ByteLength | TagSpec, CHIP_TLV_Serialize64((uint64_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100
 *
 *  @param ...                  Bytes to serialize
 */
#define CHIP_TLV_BYTE_STRING_1ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::ByteString_1ByteLength | TagSpec, CHIP_TLV_Serialize8((uint8_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000
 *
 *  @param ...                  Bytes to serialize
 */
#define CHIP_TLV_BYTE_STRING_2ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::ByteString_2ByteLength | TagSpec, CHIP_TLV_Serialize16((uint16_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100000000
 *
 *  @param ...                  Bytes to serialize
 */
#define CHIP_TLV_BYTE_STRING_4ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::ByteString_4ByteLength | TagSpec, CHIP_TLV_Serialize32((uint32_t) (StringLength)), ##__VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with CHIP_TLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000000000000000
 *
 *  @param ...                  Bytes to serialize
 */
#define CHIP_TLV_BYTE_STRING_8ByteLength(TagSpec, StringLength, ...)                                                               \
    chip::TLV::TLVElementType::ByteString_8ByteLength | TagSpec, CHIP_TLV_Serialize64((uint64_t) (StringLength)), ##__VA_ARGS__
