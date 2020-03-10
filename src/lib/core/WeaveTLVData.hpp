/*
 *
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file contains macros to create static Weave TLV data.
 *
 *    These macros are designed to be used in statically initializing a byte array
 *    containing various TLV elements. TLVWriter can achieve similar functionality
 *    but only on RAM, while macros here can be used to create TLV elements in ROM.
 *
 */


#ifndef WEAVETLVDATA_H_
#define WEAVETLVDATA_H_

#include <stdint.h>
#include <Weave/Core/WeaveTLV.h>

/*
 *  @brief Integral truncate L to the least significant 32-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_GetLower32From64(v)  ((uint32_t)(((uint64_t)(v) >>  0) & 0xFFFFFFFFUL))

/*
 *  @brief Integral truncate argument X to the next least significant 32-bit
 *
 *  @note Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_GetUpper32From64(v)  ((uint32_t)(((uint64_t)(v) >> 32) & 0xFFFFFFFFUL))

/*
 *  @brief Integral truncate L to the least significant 16-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_GetLower16From32(v)  ((uint16_t)(((uint32_t)(v) >>  0) & 0xFFFFU))

/*
 *  @brief Integral truncate argument X to the next least significant 16-bit
 *
 *  @note Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_GetUpper16From32(v)  ((uint16_t)(((uint32_t)(v) >> 16) & 0xFFFFU))

/*
 *  @brief Integral truncate L to the least significant 8-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_GetLower8From16(v)   ((uint8_t)(((uint16_t)(v) >> 0) & 0xFFU))

/*
 *  @brief Integral truncate argument X to the next least significant 8-bit
 *
 *  @note Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_GetUpper8From16(v)   ((uint8_t)(((uint16_t)(v) >> 8) & 0xFFU))

/*
 *  @brief Integral truncate argument v to 8-bit
 *
 *  @note Integral truncate would take the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_Serialize8(v) ((uint8_t)(v))

/*
 *  @brief Integral truncate argument v to 16-bit, and then serialize it using Weave standard Little Endian order
 *
 *  @note Integral truncate would preserve the least significant bits, regardless of hardware endianness.
 *     Right bit shift gets rid of the least significant bits, regardless of hardware endianness.
 */
#define nlWeaveTLV_Serialize16(v) nlWeaveTLV_GetLower8From16(v), nlWeaveTLV_GetUpper8From16(v)

/*
 *  @brief Integral truncate argument v to 32-bit, and then serialize it using Weave standard Little Endian order
 */
#define nlWeaveTLV_Serialize32(v) nlWeaveTLV_Serialize16(nlWeaveTLV_GetLower16From32(v)), nlWeaveTLV_Serialize16(nlWeaveTLV_GetUpper16From32(v))

/*
 *  @brief Integral truncate argument v to 64-bit, and then serialize it using Weave standard Little Endian order
 */
#define nlWeaveTLV_Serialize64(v) nlWeaveTLV_Serialize32(nlWeaveTLV_GetLower32From64(v)), nlWeaveTLV_Serialize32(nlWeaveTLV_GetUpper32From64(v))

/*
 *  @brief Specifies an anonymous TLV element, which doesn't have any tag
 */
#define nlWeaveTLV_TAG_ANONYMOUS nl::Weave::TLV::kTLVTagControl_Anonymous

/*
 *  @brief Specifies a TLV element with a context-specific tag
 *  @param Tag      The context-specific tag for this TLV element. Would be truncated to 8 bits.
 */
#define nlWeaveTLV_TAG_CONTEXT_SPECIFIC(Tag) nl::Weave::TLV::kTLVTagControl_ContextSpecific, nlWeaveTLV_Serialize8(Tag)

/*
 *  @brief Specifies a TLV element with a Common Profile tag
 *  @param Tag      The tag for this TLV element, defined under Common Profile.
 *                  Would be truncated to 16 bites.
 */
#define nlWeaveTLV_TAG_COMMON_PROFILE_2Bytes(Tag) \
    nl::Weave::TLV::kTLVTagControl_CommonProfile_2Bytes, nlWeaveTLV_Serialize16(Tag)

/*
 *  @brief Specifies a TLV element with a Common Profile tag
 *  @param Tag      The tag for this TLV element, defined under Common Profile.
 *                  Would be truncated to 32 bites.
 */
#define nlWeaveTLV_TAG_COMMON_PROFILE_4Bytes(Tag) \
    nl::Weave::TLV::kTLVTagControl_CommonProfile_4Bytes, nlWeaveTLV_Serialize32(Tag)

/*
 *  @brief Specifies a TLV element with an Implicit Profile tag
 *  @param Tag      The tag for this TLV element, defined under the current implicit profile.
 *                  Would be truncated to 16 bits.
 */
#define nlWeaveTLV_TAG_IMPLICIT_PROFILE_2Bytes(Tag) \
    nl::Weave::TLV::kTLVTagControl_ImplicitProfile_2Bytes, nlWeaveTLV_Serialize16(Tag)

/*
 *  @brief Specifies a TLV element with an Implicit Profile tag
 *  @param Tag      The tag for this TLV element, defined under the current implicit profile.
 *                  Would be truncated to 32 bits.
 */
#define nlWeaveTLV_TAG_IMPLICIT_PROFILE_4Bytes(Tag) \
    nl::Weave::TLV::kTLVTagControl_ImplicitProfile_4Bytes, nlWeaveTLV_Serialize32(Tag)

/*
 *  @brief Specifies a TLV element with a Fully Qualified tag
 *  @param ProfileId    {Vendor ID, Profile Number}, as in #WeaveProfileId
 *  @param Tag          The tag for this TLV element, defined under ProfileId.
 *                      Would be truncated to 16 bits.
 */
#define nlWeaveTLV_TAG_FULLY_QUALIFIED_6Bytes(ProfileId, Tag) \
    nl::Weave::TLV::kTLVTagControl_FullyQualified_6Bytes, nlWeaveTLV_Serialize16(ProfileId >> 16), nlWeaveTLV_Serialize16(ProfileId), nlWeaveTLV_Serialize16(Tag)

/*
 *  @brief Specifies a TLV element with a Fully Qualified tag
 *  @param ProfileId    {Vendor ID, Profile Number}, as in #WeaveProfileId
 *  @param Tag          The tag for this TLV element, defined under ProfileId.
 *                      Would be truncated to 32 bits.
 */
#define nlWeaveTLV_TAG_FULLY_QUALIFIED_8Bytes(ProfileId, Tag) \
    nl::Weave::TLV::kTLVTagControl_FullyQualified_8Bytes, nlWeaveTLV_Serialize16(ProfileId >> 16), nlWeaveTLV_Serialize16(ProfileId), nlWeaveTLV_Serialize32(Tag)

/*
 *  @brief Specifies a NULL TLV element, which has just the tag but no value
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 */
#define nlWeaveTLV_NULL(TagSpec) nl::Weave::TLV::kTLVElementType_Null | TagSpec

/*
 *  @brief Specifies a Structure TLV element, marking the beginning of a Structure
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 */
#define nlWeaveTLV_STRUCTURE(TagSpec) nl::Weave::TLV::kTLVElementType_Structure | TagSpec

/*
 *  @brief Specifies a Array TLV element, marking the beginning of an Array
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 */
#define nlWeaveTLV_ARRAY(TagSpec) nl::Weave::TLV::kTLVElementType_Array | TagSpec

/*
 *  @brief Specifies a Path TLV element, marking the beginning of a Path
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 */
#define nlWeaveTLV_PATH(TagSpec) nl::Weave::TLV::kTLVElementType_Path | TagSpec

/*
 *  @brief Specifies a Boolean TLV element, which can be either true or false
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Should be either true or false
 */
#define nlWeaveTLV_BOOL(TagSpec, Value) \
    ((Value) ? nl::Weave::TLV::kTLVElementType_BooleanTrue : nl::Weave::TLV::kTLVElementType_BooleanFalse) | TagSpec

/**
 *  @brief
 *    Specifies a Single Precision Floating Point TLV element, marking the beginning of 32-bit data
 *
 *  @param TagSpec      Should be filled with macros beginning with nlWeaveTLV_TAG_
 *
 *  @param ...          Bytes representing the floating point value to serialize
 */
#define nlWeaveTLV_FLOAT32(TagSpec, ...) \
    nl::Weave::TLV::kTLVElementType_FloatingPointNumber32 | TagSpec, ## __VA_ARGS__

/**
 *  @brief
 *    Specifies a Double Precision Floating Point TLV element, marking the beginning of 64-bit data
 *
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param ...          Bytes representing the floating point value to serialize
 */
#define nlWeaveTLV_FLOAT64(TagSpec, ...) \
    nl::Weave::TLV::kTLVElementType_FloatingPointNumber64 | TagSpec, ## __VA_ARGS__

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define nlWeaveTLV_END_OF_CONTAINER nl::Weave::TLV::kTLVElementType_EndOfContainer | nl::Weave::TLV::kTLVTagControl_Anonymous

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define nlWeaveTLV_END_OF_STRUCTURE nlWeaveTLV_END_OF_CONTAINER

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define nlWeaveTLV_END_OF_ARRAY nlWeaveTLV_END_OF_CONTAINER

/*
 *  @brief Specifies a EndOfContainer TLV element, marking the end of a Structure, Array, or Path
 */
#define nlWeaveTLV_END_OF_PATH nlWeaveTLV_END_OF_CONTAINER


/*
 *  @brief Specifies an 8-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to int8_t, and then serialized
 */
#define nlWeaveTLV_INT8(TagSpec, Value) nl::Weave::TLV::kTLVElementType_Int8 | TagSpec, nlWeaveTLV_Serialize8(int8_t(Value))

/*
 *  @brief Specifies a 16-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to int16_t, and then serialized
 */
#define nlWeaveTLV_INT16(TagSpec, Value) nl::Weave::TLV::kTLVElementType_Int16 | TagSpec, nlWeaveTLV_Serialize16(int16_t(Value))

/*
 *  @brief Specifies a 32-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to int32_t, and then serialized
 */
#define nlWeaveTLV_INT32(TagSpec, Value) nl::Weave::TLV::kTLVElementType_Int32 | TagSpec, nlWeaveTLV_Serialize32(int32_t(Value))

/*
 *  @brief Specifies a 32-bit Signed Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to int64_t, and then serialized
 */
#define nlWeaveTLV_INT64(TagSpec, Value) nl::Weave::TLV::kTLVElementType_Int64 | TagSpec, nlWeaveTLV_Serialize64(int64_t(Value))

/*
 *  @brief Specifies an 8-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to (uint8_t), and then serialized
 */
#define nlWeaveTLV_UINT8(TagSpec, Value) nl::Weave::TLV::kTLVElementType_UInt8 | TagSpec, nlWeaveTLV_Serialize8((uint8_t)(Value))

/*
 *  @brief Specifies a 16-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to (uint16_t), and then serialized
 */
#define nlWeaveTLV_UINT16(TagSpec, Value) nl::Weave::TLV::kTLVElementType_UInt16 | TagSpec, nlWeaveTLV_Serialize16((uint16_t)(Value))

/*
 *  @brief Specifies a 32-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to (uint32_t), and then serialized
 */
#define nlWeaveTLV_UINT32(TagSpec, Value) nl::Weave::TLV::kTLVElementType_UInt32 | TagSpec, nlWeaveTLV_Serialize32((uint32_t)(Value))

/*
 *  @brief Specifies a 64-bit Unsigned Integer TLV element
 *  @param TagSpec      Should be filled with macros begin with nlWeaveTLV_TAG_
 *  @param Value        Would be first converted to (uint64_t), and then serialized
 */
#define nlWeaveTLV_UINT64(TagSpec, Value) nl::Weave::TLV::kTLVElementType_UInt64 | TagSpec, nlWeaveTLV_Serialize64((uint64_t)(Value))

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define nlWeaveTLV_UTF8_STRING_1ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_UTF8String_1ByteLength | TagSpec, nlWeaveTLV_Serialize8((uint8_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define nlWeaveTLV_UTF8_STRING_2ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_UTF8String_2ByteLength | TagSpec, nlWeaveTLV_Serialize16((uint16_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100000000
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define nlWeaveTLV_UTF8_STRING_4ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_UTF8String_4ByteLength | TagSpec, nlWeaveTLV_Serialize32((uint32_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies an UTF8 String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000000000000000
 *
 *  @param ...                  Bytes representing the string characters to serialize
 */
#define nlWeaveTLV_UTF8_STRING_8ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_UTF8String_8ByteLength | TagSpec, nlWeaveTLV_Serialize64((uint64_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100
 *
 *  @param ...                  Bytes to serialize
 */
#define nlWeaveTLV_BYTE_STRING_1ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_ByteString_1ByteLength | TagSpec, nlWeaveTLV_Serialize8((uint8_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000
 *
 *  @param ...                  Bytes to serialize
 */
#define nlWeaveTLV_BYTE_STRING_2ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_ByteString_2ByteLength | TagSpec, nlWeaveTLV_Serialize16((uint16_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x100000000
 *
 *  @param ...                  Bytes to serialize
 */
#define nlWeaveTLV_BYTE_STRING_4ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_ByteString_4ByteLength | TagSpec, nlWeaveTLV_Serialize32((uint32_t)(StringLength)), ## __VA_ARGS__

/**
 *  @brief
 *    Specifies a BYTE String TLV element, marking the beginning of String data
 *
 *  @param TagSpec              Should be filled with macros begin with nlWeaveTLV_TAG_
 *
 *  @param StringLength         Number of bytes in this string, must be less than 0x10000000000000000
 *
 *  @param ...                  Bytes to serialize
 */
#define nlWeaveTLV_BYTE_STRING_8ByteLength(TagSpec, StringLength, ...) \
    nl::Weave::TLV::kTLVElementType_ByteString_8ByteLength | TagSpec, nlWeaveTLV_Serialize64((uint64_t)(StringLength)), ## __VA_ARGS__

#endif /* WEAVETLVDATA_H_ */
