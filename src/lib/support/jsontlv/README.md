# JSON to TLV and TLV to JSON Converter

## Introduction

Helper functions for converting TLV-encoded data to Json format and vice versa.

In order for the Json format to represent the TLV format without loss of
information, the Json key of each element should contain the TLV element tag and
type information.

The Json key schema is as follows:

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
    events, attributes).

The table below summarizes all element types and their corresponding encoding in
the Json key:

| TLV Type                               | element_type           | JSON Type               |
| -------------------------------------- | ---------------------- | ----------------------- |
| Unsigned Integer ( < 2^32 )            | UINT                   | Number                  |
| Unsigned Integer ( >= 2^32 )           | UINT                   | String                  |
| Signed Integer ( >= -2^31 and < 2^31 ) | INT                    | Number                  |
| Signed Integer ( < -2^31 or >= 2^31 )  | INT                    | String                  |
| Boolean                                | BOOL                   | Boolean                 |
| Float (32-bit)                         | FLOAT                  | Number                  |
| Float/Double (64-bit)                  | DOUBLE                 | Number                  |
| Octet String                           | BYTES                  | String (Base64 encoded) |
| UTF-8 String                           | STRING                 | String                  |
| Null Value                             | NULL                   | null                    |
| Struct                                 | STRUCT                 | Dict                    |
| Array                                  | ARRAY-sub_element_type | Array                   |
| Unknown                                | ?                      | Empty Array             |

Note that this is NOT a generalized JSON representation of TLV and does not
support arbitrary TLV conversions. The main goal of this Json format is to
support data model payloads for events/commands/attributes. Some of the
limitations of this format are:

-   TLV List types are not supported.
-   TLV Array cannot contain another TLV Array.
-   The top-level container MUST be an anonymous STRUCT.
-   Infinity Float/Double values are not supported.

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
    "3:BOOL" : true,                 // boolean
    "4:ARRAY-?" : [],                // empty array
    "5:ARRAY-DOUBLE" : [             // array of doubles
        1.1,
        134.2763,
        -12345.87
    ],
    "6:ARRAY-BYTES" : [              // array of Octet Strings: [{00 01 02 03 04}, {FF}, {4A EF 88}]
        "AAECAwQ=",                  // base64( {00 01 02 03 04} )
        "/w==",                      // base64( {FF} )
        "Su+I"                       // base64( {4A EF 88} )
    ],
    "7:BYTES" : "VGVzdCBCeXRlcw==",  // base64( "Test Bytes" )
    "8:DOUBLE" : 17.9,               // 17.9 as double
    "9:FLOAT" : 17.9,                // 17.9 as float
    "contact:10:STRUCT" : {          // structure example with field_name in the Json keys
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
