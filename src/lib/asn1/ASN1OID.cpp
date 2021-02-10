/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      This file implements methods for manipulating for writing
 *      Abstract Syntax Notation One (ASN.1) Object Identifiers
 *      (OIDs).
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <support/DLLUtil.h>

#define ASN1_DEFINE_OID_TABLE
#define ASN1_DEFINE_OID_NAME_TABLE
#include <asn1/ASN1.h>

namespace chip {
namespace ASN1 {

DLL_EXPORT OID ParseObjectID(const uint8_t * encodedOID, uint16_t encodedOIDLen)
{
    if (encodedOID == nullptr or encodedOIDLen == 0)
        return kOID_NotSpecified;

    for (uint32_t i = 0; i < sOIDTableSize; i++)
        if (encodedOIDLen == sOIDTable[i].EncodedOIDLen && memcmp(encodedOID, sOIDTable[i].EncodedOID, encodedOIDLen) == 0)
            return sOIDTable[i].EnumVal;

    return kOID_Unknown;
}

bool GetEncodedObjectID(OID oid, const uint8_t *& encodedOID, uint16_t & encodedOIDLen)
{
    for (uint32_t i = 0; i < sOIDTableSize; i++)
        if (oid == sOIDTable[i].EnumVal)
        {
            encodedOID    = sOIDTable[i].EncodedOID;
            encodedOIDLen = sOIDTable[i].EncodedOIDLen;
            return true;
        }

    return false;
}

OIDCategory GetOIDCategory(OID oid)
{
    if (oid == kOID_Unknown)
        return kOIDCategory_Unknown;
    if (oid == kOID_NotSpecified)
        return kOIDCategory_NotSpecified;
    return (OIDCategory)(oid & kOIDCategory_Mask);
}

const char * GetOIDName(OID oid)
{
    if (oid == kOID_Unknown)
        return "Unknown";
    if (oid == kOID_NotSpecified)
        return "NotSpecified";
    for (uint32_t i = 0; i < sOIDTableSize; i++)
        if (oid == sOIDNameTable[i].EnumVal)
            return sOIDNameTable[i].Name;
    return "Unknown";
}

ASN1_ERROR ASN1Reader::GetObjectId(OID & oid)
{
    if (Value == nullptr)
        return ASN1_ERROR_INVALID_STATE;
    if (ValueLen < 1)
        return ASN1_ERROR_INVALID_ENCODING;
    if (mElemStart + mHeadLen + ValueLen > mContainerEnd)
        return ASN1_ERROR_UNDERRUN;
    oid = ParseObjectID(Value, ValueLen);
    return ASN1_NO_ERROR;
}

ASN1_ERROR ASN1Writer::PutObjectId(OID oid)
{
    const uint8_t * encodedOID;
    uint16_t encodedOIDLen;

    if (!GetEncodedObjectID(oid, encodedOID, encodedOIDLen))
        return ASN1_ERROR_UNKNOWN_OBJECT_ID;

    return PutObjectId(encodedOID, encodedOIDLen);
}

} // namespace ASN1
} // namespace chip
