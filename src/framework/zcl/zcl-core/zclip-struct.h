/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCLIP_STRUCT_H
#define ZCLIP_STRUCT_H

// Needed:
//  variable length strings - decoded point to strings in encoded data
//  substructs
//  arrays
//  storage allocation
//  decoding size determination
//  parse failure reporting

//----------------------------------------------------------------
// The types of values that can be found in structs.

// any new values should be added before CHIP_ZCLIP_START_MARKER
// as we error check for it in cbor-encoder.c:realReadCborValue()
// current design allows for not more than 32 entries here, three bits
// are taken by CHIP_ZCLIP_TYPE_MODIFIER_MANDATORY and other flags
enum {
  CHIP_ZCLIP_TYPE_BOOLEAN,

  CHIP_ZCLIP_TYPE_INTEGER,
  CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,

  CHIP_ZCLIP_TYPE_BINARY,
  CHIP_ZCLIP_TYPE_FIXED_LENGTH_BINARY,

  CHIP_ZCLIP_TYPE_STRING,
  CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING,

  // These are used for struct-based encoding/decoding from/to an
  // ChipZclStringType_t substructure.
  CHIP_ZCLIP_TYPE_UINT8_LENGTH_STRING,
  CHIP_ZCLIP_TYPE_UINT16_LENGTH_STRING,

  // Additional types for non-struct internal use.
  //
  // These allow direct encoding/decoding from/to a buffer containing a
  // length-prefixed binary or text string.
  CHIP_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY,
  CHIP_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY,
  CHIP_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING,
  CHIP_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING,

  // Used for CBOR_NULL etc.
  CHIP_ZCLIP_TYPE_MISC,

  // Markers that are not really value types.
  CHIP_ZCLIP_START_MARKER,
  CHIP_ZCLIP_ARRAY_MARKER
};

// Override CHIP_ZCLIP_TYPE bit 7 - indicates mandatory field.
#define CHIP_ZCLIP_TYPE_MODIFIER_MANDATORY  0x80
// Override CHIP_ZCLIP_TYPE bit 6 - indicates field represents an array.
#define CHIP_ZCLIP_TYPE_MODIFIER_ARRAY      0x40
// Override CHIP_ZCLIP_TYPE bit 5 - indicates that pName mchip contains the TLV tag
#define CHIP_ZCLIP_TYPE_MODIFIER_TLV        0x20

// All zclip structs (i.e. Struct spec and Field specs) have a
// header info block == 2 ZclipStructSpec fields, which are actually
// one FieldSpec followed by a char pointer.
#define CHIP_ZCLIP_STRUCT_HEADER_SIZE  2

//----------------------------------------------------------------
// Information about structs is encoded in arrays of ZclipStructSpec.
// This makes it easy to define macros for defining different kinds of fields.

// Defined as unsigned long to align with width of a pointer for both
// 32-bit and 64-bit architectures (host apps). This allows the "name"
// character string pointers to be cast to ZclipStructSpec when used as
// compile-time initializer constants in the macros below. DO NOT change
// to a fixed-width type (uint32_t, uint64_t) without first testing
// host app compilation for both 32- and 64-bit targets and making any
// other code changes needed to satisfy both.
typedef unsigned long ZclipStructSpec;

// CHIP_ZCLIP_STRUCT is defined to the name of the current struct type.
//
// A simple struct definition and its encoding look like:
//
// typedef struct {
//   bool     field0;
//   uint16_t field1;
//   uint8_t  field2;
//   bool     field3;
// } SomeStruct;
//
// #define CHIP_ZCLIP_STRUCT ZclipTestStruct
//
// unsigned long zclipTestStructSpec[] = {
//   CHIP_ZCLIP_OBJECT(sizeof(CHIP_ZCLIP_STRUCT),
//                      4,         // fieldCount
//                      NULL),     // names
//   CHIP_ZCLIP_FIELD_INDEXED(CHIP_ZCLIP_TYPE_BOOLEAN,          field0),
//   CHIP_ZCLIP_FIELD_INDEXED(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, field1),
//   CHIP_ZCLIP_FIELD_NAMED(  CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, field2, "e"),
//   CHIP_ZCLIP_FIELD_TLV(    CHIP_ZCLIP_TYPE_BOOLEAN,          field3, 7)
// };
//
// #undef CHIP_ZCLIP_STRUCT
//
// This struct will be encoded to or decoded from the CBOR map
// {0: <field0>, 1: <field1>, "e": <field2>, -7: <field3>}.
//
// Indexed fields must appear first in the spec and are automatically numbered
// starting from 0.  Named fields must appear after any indexed fields. TLV tag fields
// must apper after the indexed and named fields. ZCLIP uses -tlvTag as the key, tag
// is specified as unsigned in general-thread.xml, sign is applied during CBOR
// encoding and decoding.
//
//----------------------------------------------------------------
// Macros for encoding struct information

// The offset of a field within CHIP_ZCLIP_STRUCT.
#define ZCLIP_FIELD_OFFSET(field) \
  ((uint16_t) (long) &(((CHIP_ZCLIP_STRUCT *) NULL)->field))

// The size of a field within CHIP_ZCLIP_STRUCT.
#define ZCLIP_FIELD_SIZE(field) \
  (sizeof(((CHIP_ZCLIP_STRUCT *)NULL)->field))

#define TLV_TAG_MASK 0xffff
// A Field spec header is two x uint32 words:
// uint32:0 LS[type:8)][size:8][offset:16]
//   type- the enum type of the field value (3 top bits taken by flags)
//   size- the total size of the field in bytes (if field is an array type,
//         this is the size of the array descriptor struct).
//   offset- the offset of the field within the struct.
// uint32:1 [pName:32 or tlvTag:16] based on CHIP_ZCLIP_TYPE_MODIFIER_TLV
//          bit in the type field

#define CHIP_ZCLIP_FIELD_SPEC(type, size, offset, name) \
  ((type) | ((size) << 8) | ((offset) << 16)),           \
  (ZclipStructSpec)name
#define CHIP_ZCLIP_FIELD_TLV(type, field, tlv)                  \
  CHIP_ZCLIP_FIELD_SPEC(CHIP_ZCLIP_TYPE_MODIFIER_TLV | (type), \
                         ZCLIP_FIELD_SIZE(field),                \
                         ZCLIP_FIELD_OFFSET(field),              \
                         (tlv) & TLV_TAG_MASK)
#define CHIP_ZCLIP_FIELD_NAMED(type, field, name)  \
  CHIP_ZCLIP_FIELD_SPEC(type,                      \
                         ZCLIP_FIELD_SIZE(field),   \
                         ZCLIP_FIELD_OFFSET(field), \
                         name)

#define CHIP_ZCLIP_FIELD_INDEXED(type, field) \
  CHIP_ZCLIP_FIELD_NAMED(type, field, NULL)

#define CHIP_ZCLIP_FIELD   CHIP_ZCLIP_FIELD_INDEXED

//---- Mandatory type fields----

#define CHIP_ZCLIP_FIELD_NAMED_MANDATORY(type, field, name)            \
  CHIP_ZCLIP_FIELD_NAMED((type) | CHIP_ZCLIP_TYPE_MODIFIER_MANDATORY, \
                          field,                                        \
                          name)

#define CHIP_ZCLIP_FIELD_INDEXED_MANDATORY(type, field) \
  CHIP_ZCLIP_FIELD_NAMED_MANDATORY(type, field, NULL)

#define CHIP_ZCLIP_FIELD_MANDATORY \
  CHIP_ZCLIP_FIELD_INDEXED_MANDATORY

//---- Array type fields----

#define CHIP_ZCLIP_FIELD_NAMED_ARRAY(type, field, name)            \
  CHIP_ZCLIP_FIELD_NAMED((type) | CHIP_ZCLIP_TYPE_MODIFIER_ARRAY, \
                          field,                                    \
                          name)

#define CHIP_ZCLIP_FIELD_INDEXED_ARRAY(type, field) \
  CHIP_ZCLIP_FIELD_NAMED_ARRAY(type, field, NULL)

#define CHIP_ZCLIP_FIELD_ARRAY \
  CHIP_ZCLIP_FIELD_INDEXED_ARRAY

//---- the combo of Mandatory + Array type fields----

#define CHIP_ZCLIP_FIELD_NAMED_MANDATORY_ARRAY(type, field, name)                                        \
  CHIP_ZCLIP_FIELD_NAMED((type) | CHIP_ZCLIP_TYPE_MODIFIER_MANDATORY | CHIP_ZCLIP_TYPE_MODIFIER_ARRAY, \
                          field,                                                                          \
                          NULL)

#define CHIP_ZCLIP_FIELD_INDEXED_MANDATORY_ARRAY(type, field) \
  CHIP_ZCLIP_FIELD_NAMED_MANDATORY_ARRAY(type, field, NULL)

#define CHIP_ZCLIP_FIELD_MANDATORY_ARRAY \
  CHIP_ZCLIP_FIELD_INDEXED_MANDATORY_ARRAY

// The first few uint32_ts encode information about the struct itself.
// The start marker is for safety - we don't really need it.  The other
// values are the number of fields, the size of the struct, and a string
// holding the names of the struct and its fields (not yet implemented).
#define CHIP_ZCLIP_OBJECT(size, fieldCount, names)                \
  (CHIP_ZCLIP_START_MARKER | ((fieldCount) << 8) | (size) << 16), \
  (ZclipStructSpec)(names)

//----------------------------------------------------------------
// To avoid having too much code depend on the exact encoding, information
// about structs and fields can be expanded.

typedef struct {
  const ZclipStructSpec *spec;
  uint16_t size;                // size of the struct in bytes
  uint8_t  fieldCount;          // num of fields

  // Values used for iterating through the fields.
  uint16_t fieldIndex;          // the index of the next field in the struct
  const ZclipStructSpec *next;  // next field to be processed
} ZclipStructData;

bool emExpandZclipStructData(const ZclipStructSpec *spec,
                             ZclipStructData *structData);

typedef struct {
  uint8_t valueType;
  // 0xFF for strings and substructs. For field arrays this will be the size
  // of the array descriptor structure.
  uint16_t valueSize;
  uint16_t valueOffset;
  bool isArray;         // if true, the field represents an array of values
  bool isMandatory;     // if true, the field presence is mandatory
  bool isTlv;           // if true, the field represents a TLV
  const char *name;
  uint16_t tlvTag;
} ZclipFieldData;

void emResetZclipFieldData(ZclipStructData *structData);
bool chZclipFieldDataFinished(ZclipStructData *structData);
void emGetNextZclipFieldData(ZclipStructData *structData,
                             ZclipFieldData *fieldData);

//----------------------------------------------------------------
// Utilities for reading and writing integer fields.  These should probably
// go somewhere else.

uint32_t emFetchInt32uValue(const uint8_t *valueLoc, uint16_t valueSize);
int32_t emFetchInt32sValue(const uint8_t *valueLoc, uint16_t valueSize);
void emStoreInt32sValue(uint8_t* valueLoc, int32_t value, uint8_t valueSize);
void emStoreInt32uValue(uint8_t* valueLoc, uint32_t value, uint8_t valueSize);

#endif // ZCLIP_STRUCT_H
