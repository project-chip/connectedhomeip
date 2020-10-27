#ifndef __ZAP_ATTRIBUTE_TYPE__
#define __ZAP_ATTRIBUTE_TYPE__

// ZCL attribute types
enum
{
    ZCL_NO_DATA_ATTRIBUTE_TYPE           = 0x00, // No data
    ZCL_DATA8_ATTRIBUTE_TYPE             = 0x08, // 8-bit data
    ZCL_DATA16_ATTRIBUTE_TYPE            = 0x09, // 16-bit data
    ZCL_DATA24_ATTRIBUTE_TYPE            = 0x0A, // 24-bit data
    ZCL_DATA32_ATTRIBUTE_TYPE            = 0x0B, // 32-bit data
    ZCL_DATA40_ATTRIBUTE_TYPE            = 0x0C, // 40-bit data
    ZCL_DATA48_ATTRIBUTE_TYPE            = 0x0D, // 48-bit data
    ZCL_DATA56_ATTRIBUTE_TYPE            = 0x0E, // 56-bit data
    ZCL_DATA64_ATTRIBUTE_TYPE            = 0x0F, // 64-bit data
    ZCL_BOOLEAN_ATTRIBUTE_TYPE           = 0x10, // Boolean
    ZCL_BITMAP8_ATTRIBUTE_TYPE           = 0x18, // 8-bit bitmap
    ZCL_BITMAP16_ATTRIBUTE_TYPE          = 0x19, // 16-bit bitmap
    ZCL_BITMAP24_ATTRIBUTE_TYPE          = 0x1A, // 24-bit bitmap
    ZCL_BITMAP32_ATTRIBUTE_TYPE          = 0x1B, // 32-bit bitmap
    ZCL_BITMAP40_ATTRIBUTE_TYPE          = 0x1C, // 40-bit bitmap
    ZCL_BITMAP48_ATTRIBUTE_TYPE          = 0x1D, // 48-bit bitmap
    ZCL_BITMAP56_ATTRIBUTE_TYPE          = 0x1E, // 56-bit bitmap
    ZCL_BITMAP64_ATTRIBUTE_TYPE          = 0x1F, // 64-bit bitmap
    ZCL_INT8U_ATTRIBUTE_TYPE             = 0x20, // Unsigned 8-bit integer
    ZCL_INT16U_ATTRIBUTE_TYPE            = 0x21, // Unsigned 16-bit integer
    ZCL_INT24U_ATTRIBUTE_TYPE            = 0x22, // Unsigned 24-bit integer
    ZCL_INT32U_ATTRIBUTE_TYPE            = 0x23, // Unsigned 32-bit integer
    ZCL_INT40U_ATTRIBUTE_TYPE            = 0x24, // Unsigned 40-bit integer
    ZCL_INT48U_ATTRIBUTE_TYPE            = 0x25, // Unsigned 48-bit integer
    ZCL_INT56U_ATTRIBUTE_TYPE            = 0x26, // Unsigned 56-bit integer
    ZCL_INT64U_ATTRIBUTE_TYPE            = 0x27, // Unsigned 64-bit integer
    ZCL_INT8S_ATTRIBUTE_TYPE             = 0x28, // Signed 8-bit integer
    ZCL_INT16S_ATTRIBUTE_TYPE            = 0x29, // Signed 16-bit integer
    ZCL_INT24S_ATTRIBUTE_TYPE            = 0x2A, // Signed 24-bit integer
    ZCL_INT32S_ATTRIBUTE_TYPE            = 0x2B, // Signed 32-bit integer
    ZCL_INT40S_ATTRIBUTE_TYPE            = 0x2C, // Signed 40-bit integer
    ZCL_INT48S_ATTRIBUTE_TYPE            = 0x2D, // Signed 48-bit integer
    ZCL_INT56S_ATTRIBUTE_TYPE            = 0x2E, // Signed 56-bit integer
    ZCL_INT64S_ATTRIBUTE_TYPE            = 0x2F, // Signed 64-bit integer
    ZCL_ENUM8_ATTRIBUTE_TYPE             = 0x30, // 8-bit enumeration
    ZCL_ENUM16_ATTRIBUTE_TYPE            = 0x31, // 16-bit enumeration
    ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE        = 0x38, // Semi-precision
    ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE      = 0x39, // Single precision
    ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE      = 0x3A, // Double precision
    ZCL_OCTET_STRING_ATTRIBUTE_TYPE      = 0x41, // Octet string
    ZCL_CHAR_STRING_ATTRIBUTE_TYPE       = 0x42, // Character string
    ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE = 0x43, // Long octet string
    ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE  = 0x44, // Long character string
    ZCL_ARRAY_ATTRIBUTE_TYPE             = 0x48, // Array
    ZCL_STRUCT_ATTRIBUTE_TYPE            = 0x4C, // Structure
    ZCL_SET_ATTRIBUTE_TYPE               = 0x50, // Set
    ZCL_BAG_ATTRIBUTE_TYPE               = 0x51, // Bag
    ZCL_TIME_OF_DAY_ATTRIBUTE_TYPE       = 0xE0, // Time of day
    ZCL_DATE_ATTRIBUTE_TYPE              = 0xE1, // Date
    ZCL_UTC_TIME_ATTRIBUTE_TYPE          = 0xE2, // UTC Time
    ZCL_CLUSTER_ID_ATTRIBUTE_TYPE        = 0xE8, // Cluster ID
    ZCL_ATTRIBUTE_ID_ATTRIBUTE_TYPE      = 0xE9, // Attribute ID
    ZCL_BACNET_OID_ATTRIBUTE_TYPE        = 0xEA, // BACnet OID
    ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE      = 0xF0, // IEEE address
    ZCL_SECURITY_KEY_ATTRIBUTE_TYPE      = 0xF1, // 128-bit security key
    ZCL_UNKNOWN_ATTRIBUTE_TYPE           = 0xFF, // Unknown
};

// ZCL attribute sizes
#define ZAP_GENERATED_ATTRIBUTE_SIZES                                                                                              \
    {                                                                                                                              \
        ZCL_DATA8_ATTRIBUTE_TYPE, 1, ZCL_DATA16_ATTRIBUTE_TYPE, 2, ZCL_DATA24_ATTRIBUTE_TYPE, 3, ZCL_DATA32_ATTRIBUTE_TYPE, 4,     \
            ZCL_DATA40_ATTRIBUTE_TYPE, 5, ZCL_DATA48_ATTRIBUTE_TYPE, 6, ZCL_DATA56_ATTRIBUTE_TYPE, 7, ZCL_DATA64_ATTRIBUTE_TYPE,   \
            8, ZCL_BOOLEAN_ATTRIBUTE_TYPE, 1, ZCL_BITMAP8_ATTRIBUTE_TYPE, 1, ZCL_BITMAP16_ATTRIBUTE_TYPE, 2,                       \
            ZCL_BITMAP24_ATTRIBUTE_TYPE, 3, ZCL_BITMAP32_ATTRIBUTE_TYPE, 4, ZCL_BITMAP40_ATTRIBUTE_TYPE, 5,                        \
            ZCL_BITMAP48_ATTRIBUTE_TYPE, 6, ZCL_BITMAP56_ATTRIBUTE_TYPE, 7, ZCL_BITMAP64_ATTRIBUTE_TYPE, 8,                        \
            ZCL_INT8U_ATTRIBUTE_TYPE, 1, ZCL_INT16U_ATTRIBUTE_TYPE, 2, ZCL_INT24U_ATTRIBUTE_TYPE, 3, ZCL_INT32U_ATTRIBUTE_TYPE, 4, \
            ZCL_INT40U_ATTRIBUTE_TYPE, 5, ZCL_INT48U_ATTRIBUTE_TYPE, 6, ZCL_INT56U_ATTRIBUTE_TYPE, 7, ZCL_INT64U_ATTRIBUTE_TYPE,   \
            8, ZCL_INT8S_ATTRIBUTE_TYPE, 1, ZCL_INT16S_ATTRIBUTE_TYPE, 2, ZCL_INT24S_ATTRIBUTE_TYPE, 3, ZCL_INT32S_ATTRIBUTE_TYPE, \
            4, ZCL_INT40S_ATTRIBUTE_TYPE, 5, ZCL_INT48S_ATTRIBUTE_TYPE, 6, ZCL_INT56S_ATTRIBUTE_TYPE, 7,                           \
            ZCL_INT64S_ATTRIBUTE_TYPE, 8, ZCL_ENUM8_ATTRIBUTE_TYPE, 1, ZCL_ENUM16_ATTRIBUTE_TYPE, 2,                               \
            ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE, 2, ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE, 4, ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE, 8,              \
            ZCL_TIME_OF_DAY_ATTRIBUTE_TYPE, 4, ZCL_DATE_ATTRIBUTE_TYPE, 4, ZCL_UTC_TIME_ATTRIBUTE_TYPE, 4,                         \
            ZCL_CLUSTER_ID_ATTRIBUTE_TYPE, 2, ZCL_ATTRIBUTE_ID_ATTRIBUTE_TYPE, 2, ZCL_BACNET_OID_ATTRIBUTE_TYPE, 4,                \
            ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE, 8, ZCL_SECURITY_KEY_ATTRIBUTE_TYPE, 16,                                               \
    }

#endif // __ZAP_ATTRIBUTE_TYPE__
