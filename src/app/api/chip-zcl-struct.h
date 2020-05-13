/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file provides declarations required by the CHIP ZCL codec.
 *      An implementation of the codec needs to implement the functions
 *      declared in this header.
 *
 */
#ifndef CHIP_ZCL_STRUCT
#define CHIP_ZCL_STRUCT

/**
 * Base types for the codec. This is a smaller subset than the actual ZCL types, and the
 * generated layer for a specific code is responsible for mapping ZCL types onto these
 * base types.
 */
enum
{
    CHIP_ZCL_STRUCT_TYPE_BOOLEAN,

    CHIP_ZCL_STRUCT_TYPE_INTEGER,
    CHIP_ZCL_STRUCT_TYPE_UNSIGNED_INTEGER,

    CHIP_ZCL_STRUCT_TYPE_BINARY,
    CHIP_ZCL_STRUCT_TYPE_FIXED_LENGTH_BINARY,

    CHIP_ZCL_STRUCT_TYPE_STRING,
    CHIP_ZCL_STRUCT_TYPE_MAX_LENGTH_STRING,

    // These are used for struct-based encoding/decoding from/to an
    // ChipZclStringType_t substructure.
    CHIP_ZCL_STRUCT_TYPE_UINT8_LENGTH_STRING,
    CHIP_ZCL_STRUCT_TYPE_UINT16_LENGTH_STRING,

    // Additional types for non-struct internal use.
    //
    // These allow direct encoding/decoding from/to a buffer containing a
    // length-prefixed binary or text string.
    CHIP_ZCL_STRUCT_TYPE_UINT8_LENGTH_PREFIXED_BINARY,
    CHIP_ZCL_STRUCT_TYPE_UINT16_LENGTH_PREFIXED_BINARY,
    CHIP_ZCL_STRUCT_TYPE_UINT8_LENGTH_PREFIXED_STRING,
    CHIP_ZCL_STRUCT_TYPE_UINT16_LENGTH_PREFIXED_STRING,

    // Used for NULL and other weird cases.
    CHIP_ZCL_STRUCT_TYPE_MISC,

    // Markers that are not really value types.
    CHIP_ZCL_STRUCT_START_MARKER,
    CHIP_ZCL_STRUCT_ARRAY_MARKER
};

// Override CHIP_ZCL_STRUCT_TYPE bit 7 - indicates mandatory field.
#define CHIP_ZCL_STRUCT_TYPE_MODIFIER_MANDATORY 0x80
// Override CHIP_ZCL_STRUCT_TYPE bit 6 - indicates field represents an array.
#define CHIP_ZCL_STRUCT_TYPE_MODIFIER_ARRAY 0x40
// Override CHIP_ZCL_STRUCT_TYPE bit 5 - indicates that pName member contains the TLV tag
#define CHIP_ZCL_STRUCT_TYPE_MODIFIER_TLV 0x20

// All zclip structs (i.e. Struct spec and Field specs) have a
// header info block == 2 ZclipStructSpec fields, which are actually
// one FieldSpec followed by a char pointer.
#define CHIP_ZCL_STRUCT_STRUCT_HEADER_SIZE 2

// Defined as unsigned long to align with width of a pointer for both
// 32-bit and 64-bit architectures (host apps). This allows the "name"
// character string pointers to be cast to ZclipStructSpec when used as
// compile-time initializer constants in the macros below. DO NOT change
// to a fixed-width type (uint32_t, uint64_t) without first testing
// host app compilation for both 32- and 64-bit targets and making any
// other code changes needed to satisfy both.
typedef unsigned long ChipZclStructSpec;

//----------------------------------------------------------------
// Macros for encoding struct information

// The offset of a field within CHIP_ZCL_STRUCT.
#define CHIP_ZCL_FIELD_OFFSET(field) ((uint16_t)(long) &(((CHIP_ZCL_STRUCT *) NULL)->field))

// The size of a field within CHIP_ZCL_STRUCT.
#define CHIP_ZCL_FIELD_SIZE(field) (sizeof(((CHIP_ZCL_STRUCT *) NULL)->field))

#define TLV_TAG_MASK 0xffff
// A Field spec header is two x uint32 words:
// uint32:0 LS[type:8)][size:8][offset:16]
//   type- the enum type of the field value (3 top bits taken by flags)
//   size- the total size of the field in bytes (if field is an array type,
//         this is the size of the array descriptor struct).
//   offset- the offset of the field within the struct.
// uint32:1 [pName:32 or tlvTag:16] based on CHIP_ZCL_TYPE_MODIFIER_TLV
//          bit in the type field

#define CHIP_ZCL_FIELD_SPEC(type, size, offset, name) ((type) | ((size) << 8) | ((offset) << 16)), (ZclipStructSpec) name
#define CHIP_ZCL_FIELD_TLV(type, field, tlv)                                                                                       \
    CHIP_ZCL_FIELD_SPEC(CHIP_ZCL_TYPE_MODIFIER_TLV | (type), ZCLIP_FIELD_SIZE(field), ZCLIP_FIELD_OFFSET(field),                   \
                        (tlv) &TLV_TAG_MASK)
#define CHIP_ZCL_FIELD_NAMED(type, field, name) CHIP_ZCL_FIELD_SPEC(type, ZCLIP_FIELD_SIZE(field), ZCLIP_FIELD_OFFSET(field), name)

#define CHIP_ZCL_FIELD_INDEXED(type, field) CHIP_ZCL_FIELD_NAMED(type, field, NULL)

#define CHIP_ZCL_FIELD CHIP_ZCL_FIELD_INDEXED

//---- Mandatory type fields----

#define CHIP_ZCL_FIELD_NAMED_MANDATORY(type, field, name)                                                                          \
    CHIP_ZCL_FIELD_NAMED((type) | CHIP_ZCL_TYPE_MODIFIER_MANDATORY, field, name)

#define CHIP_ZCL_FIELD_INDEXED_MANDATORY(type, field) CHIP_ZCL_FIELD_NAMED_MANDATORY(type, field, NULL)

#define CHIP_ZCL_FIELD_MANDATORY CHIP_ZCL_FIELD_INDEXED_MANDATORY

//---- Array type fields----

#define CHIP_ZCL_FIELD_NAMED_ARRAY(type, field, name) CHIP_ZCL_FIELD_NAMED((type) | CHIP_ZCL_TYPE_MODIFIER_ARRAY, field, name)

#define CHIP_ZCL_FIELD_INDEXED_ARRAY(type, field) CHIP_ZCL_FIELD_NAMED_ARRAY(type, field, NULL)

#define CHIP_ZCL_FIELD_ARRAY CHIP_ZCL_FIELD_INDEXED_ARRAY

//---- the combo of Mandatory + Array type fields----

#define CHIP_ZCL_FIELD_NAMED_MANDATORY_ARRAY(type, field, name)                                                                    \
    CHIP_ZCL_FIELD_NAMED((type) | CHIP_ZCL_TYPE_MODIFIER_MANDATORY | CHIP_ZCL_TYPE_MODIFIER_ARRAY, field, NULL)

#define CHIP_ZCL_FIELD_INDEXED_MANDATORY_ARRAY(type, field) CHIP_ZCL_FIELD_NAMED_MANDATORY_ARRAY(type, field, NULL)

#define CHIP_ZCL_FIELD_MANDATORY_ARRAY CHIP_ZCL_FIELD_INDEXED_MANDATORY_ARRAY

// The first few uint32_ts encode information about the struct itself.
// The start marker is for safety - we don't really need it.  The other
// values are the number of fields, the size of the struct, and a string
// holding the names of the struct and its fields (not yet implemented).
#define CHIP_ZCL_OBJECT(size, fieldCount, names)                                                                                   \
    (CHIP_ZCL_START_MARKER | ((fieldCount) << 8) | (size) << 16), (ZclipStructSpec)(names)

//----------------------------------------------------------------
//----------------------------------------------------------------
// To avoid having too much code depend on the exact encoding, information
// about structs and fields can be expanded.

typedef struct
{
    const ChipZclStructSpec * spec;
    uint16_t size;      // size of the struct in bytes
    uint8_t fieldCount; // num of fields

    // Values used for iterating through the fields.
    uint16_t fieldIndex;            // the index of the next field in the struct
    const ChipZclStructSpec * next; // next field to be processed
} ChipZclStructData;

bool chipZclExpandChipZclStructData(const ChipZclStructSpec * spec, ChipZclStructData * structData);

typedef struct
{
    uint8_t valueType;
    // 0xFF for strings and substructs. For field arrays this will be the size
    // of the array descriptor structure.
    uint16_t valueSize;
    uint16_t valueOffset;
    bool isArray;     // if true, the field represents an array of values
    bool isMandatory; // if true, the field presence is mandatory
    bool isTlv;       // if true, the field represents a TLV
    const char * name;
    uint16_t tlvTag;
} ChipZclFieldData;

void chipZclResetChipZclFieldData(ChipZclStructData * structData);
bool chipZclFieldDataFinished(ChipZclStructData * structData);
void chipZclGetNextChipZclFieldData(ChipZclStructData * structData, ChipZclFieldData * fieldData);

//----------------------------------------------------------------
// Utilities for reading and writing integer fields.  These should probably
// go somewhere else.

uint32_t chipZclFetchInt32uValue(const uint8_t * valueLoc, uint16_t valueSize);
int32_t chipZclFetchInt32sValue(const uint8_t * valueLoc, uint16_t valueSize);
void chipZclStoreInt32sValue(uint8_t * valueLoc, int32_t value, uint8_t valueSize);
void chipZclStoreInt32uValue(uint8_t * valueLoc, uint32_t value, uint8_t valueSize);

#endif // CHIP_ZCL_STRUCT
