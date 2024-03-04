# JSON to TLV and TLV to JSON Converter

## Introduction

Helper functions for converting TLV-encoded data to Json format and vice versa.

### Supported payloads

The library supports

-   full bi-directional conversion for matter data model payloads

-   When the MEI prefix encodes a standard/scoped source, the tag is encoded
    using ContextSpecific tag if tag_id is less than or equal to UINT8_MAX, and
    ImplicitProfile tag if tag_id is larger than UINT8_MAX. The reason for
    allowing such IDs is to support json formats where keys contain ids
    typically found in paths, like `{"1234:INT": 10}` meaning
    `"Attribute 1234 has value 10"`.

-   When the MEI prefix encodes a manufacturer code, the tag is encoded using
    FullyQualified_6Bytes tag, the Vendor ID SHALL be set to the manufacturer
    code, the profile number set to 0 and the tag number set to the MEI suffix.

### Format details

In order for the Json format to represent the TLV format without loss of
information, the Json name of each element should contain the TLV element tag
and type information.

The Json name schema is as follows:

```
{ [field_name:]field_id:element_type[-sub_element_type] }
```

Specific rules:

-   'element_type' MUST map to the TLV Type as specified in the table below.
-   For 'element_type' of type "ARRAY", the sub_element_type MUST always be
    present and be equal to the 'element_type' of the elements in the array.
-   Array elements MUST have the same 'element_type'.
-   Array elements MUST have anonymous TLV tags.
-   'field_name' is always an optional field and can be provided if the semantic
    name for the field is available.
-   In the case of an empty array, 'sub_element_type' should be "?" (unknown
    element type).
-   The unknown element type “?” MUST never occur outside of "ARRAY-?".
-   'field_id' is equivalent to the Field ID for all cluster payloads (commands,
    events, attributes), encoded as a decimal number.

The table below summarizes all element types and their corresponding encoding in
the Json element name:

| TLV Type                               | element_type           | JSON Type                       |
| -------------------------------------- | ---------------------- | ------------------------------- |
| Unsigned Integer ( < 2^32 )            | UINT                   | Number                          |
| Unsigned Integer ( >= 2^32 )           | UINT                   | String                          |
| Signed Integer ( >= -2^31 and < 2^31 ) | INT                    | Number                          |
| Signed Integer ( < -2^31 or >= 2^31 )  | INT                    | String                          |
| Boolean                                | BOOL                   | Boolean                         |
| Float (positive/negative infinity)     | FLOAT                  | String ("Infinity"/"-Infinity") |
| Double (positive/negative infinity)    | DOUBLE                 | String ("Infinity"/"-Infinity") |
| Float (not infinity)                   | FLOAT                  | Number                          |
| Double (not infinity)                  | DOUBLE                 | Number                          |
| Octet String                           | BYTES                  | String (Base64 encoded)         |
| UTF-8 String                           | STRING                 | String                          |
| Null Value                             | NULL                   | null                            |
| Struct                                 | STRUCT                 | Dict                            |
| Array                                  | ARRAY-sub_element_type | Array                           |
| Unknown                                | ?                      | Empty Array                     |

Note that this is NOT a generalized JSON representation of TLV and does not
support arbitrary TLV conversions. The main goal of this Json format is to
support data model payloads for events/commands/attributes. Some of the
limitations of this format are:

-   TLV List types are not supported.
-   TLV Array cannot contain another TLV Array.
-   The top-level container MUST be an anonymous STRUCT.
-   Elements of the TLV Structure MUST have Context or Implicit Profile Tags for
    standard/scoped source and Fully Qualified Profile Tags for an MC source.
-   Implicit Profile Tag number MUST be larger or equal to 256 and smaller than
    2^32 + 1.
-   TLV Structure element MUST be sorted by tag numbers from low to high, where
    sorted elements with Context Tags MUST appear first followed by sorted
    elements with Implicit Profile Tags and then Profile Specific Tags.

## Format Example

The following is an example of a Json string. It represents various TLV
elements, arrays, and structures.

```
{                                    // top-level anonymous structure
    "0:ARRAY-STRUCT" : [             // array of structures
        {
            "0:INT" : 8,             // Struct.elem0
            "1:BOOL" : true          // Struct.elem1
        }
    ],
    "1:STRUCT" : {                   // structure
        "0:INT" : 12,                // Struct.elem0
        "1:BOOL" : false,            // Struct.elem1
        "2:STRING" : "example"       // Struct.elem2
    },
    "2:INT" : "40000000000",         // int as string
    "isQualified:3:BOOL" : true,     // boolean with field_name in the Json name
    "4:ARRAY-?" : [],                // empty array
    "5:ARRAY-DOUBLE" : [             // array of doubles
        1.1,
        134.2763,
        -12345.87,
        "Infinity",                  // positive infinity
        62534,                       // positive integer-valued double
        -62534                       // negative integer-valued double
    ],
    "6:ARRAY-BYTES" : [              // array of Octet Strings: [{00 01 02 03 04}, {FF}, {4A EF 88}]
        "AAECAwQ=",                  // base64( {00 01 02 03 04} )
        "/w==",                      // base64( {FF} )
        "Su+I"                       // base64( {4A EF 88} )
    ],
    "7:BYTES" : "VGVzdCBCeXRlcw==",  // base64( "Test Bytes" )
    "8:DOUBLE" : 17.9,               // 17.9 as double
    "9:FLOAT" : 17.9,                // 17.9 as float
    "10:FLOAT" : "-Infinity",        // Negative infinity float
    "4293984426:UINT" : 3,           // Vendor ID = 0xFFF1, Profile ID = 0, Tag ID = 0x00AA
    "contact:11:STRUCT" : {          // structure example with field_name in the Json name
        "name:1:STRING" : "John",
        "age:2:UINT" : 34,
        "approved:3:BOOL" : true,
        "kids:4:ARRAY-INT" : [
            5,
            9,
            10
        ]
    }
}
```
