/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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

#include <lib/support/DLLUtil.h>
#include <lib/support/SafeInt.h>

#define ASN1_DEFINE_OID_TABLE
#define ASN1_DEFINE_OID_NAME_TABLE
#include <lib/asn1/ASN1.h>

namespace chip {
namespace ASN1 {

DLL_EXPORT OID ParseObjectID(const uint8_t * encodedOID, uint16_t encodedOIDLen)
{
    if (encodedOID == nullptr or encodedOIDLen == 0)
    {
        return kOID_NotSpecified;
    }

    for (uint32_t i = 0; i < sOIDTableSize; i++)
    {
        if (encodedOIDLen == sOIDTable[i].EncodedOIDLen && memcmp(encodedOID, sOIDTable[i].EncodedOID, encodedOIDLen) == 0)
        {
            return sOIDTable[i].EnumVal;
        }
    }

    return kOID_Unknown;
}

bool GetEncodedObjectID(OID oid, const uint8_t *& encodedOID, uint16_t & encodedOIDLen)
{
    for (uint32_t i = 0; i < sOIDTableSize; i++)
    {
        if (oid == sOIDTable[i].EnumVal)
        {
            encodedOID    = sOIDTable[i].EncodedOID;
            encodedOIDLen = sOIDTable[i].EncodedOIDLen;
            return true;
        }
    }

    return false;
}

OIDCategory GetOIDCategory(OID oid)
{
    if (oid == kOID_Unknown)
    {
        return kOIDCategory_Unknown;
    }
    if (oid == kOID_NotSpecified)
    {
        return kOIDCategory_NotSpecified;
    }
    return static_cast<OIDCategory>(oid & kOIDCategory_Mask);
}

const char * GetOIDName(OID oid)
{
    if (oid == kOID_Unknown)
    {
        return "Unknown";
    }
    if (oid == kOID_NotSpecified)
    {
        return "NotSpecified";
    }
    for (uint32_t i = 0; i < sOIDTableSize; i++)
    {
        if (oid == sOIDNameTable[i].EnumVal)
        {
            return sOIDNameTable[i].Name;
        }
    }
    return "Unknown";
}

CHIP_ERROR ASN1Reader::GetObjectId(OID & oid)
{
    ReturnErrorCodeIf(Value == nullptr, ASN1_ERROR_INVALID_STATE);
    ReturnErrorCodeIf(ValueLen < 1, ASN1_ERROR_INVALID_ENCODING);
    ReturnErrorCodeIf(mElemStart + mHeadLen + ValueLen > mContainerEnd, ASN1_ERROR_UNDERRUN);
    VerifyOrReturnError(CanCastTo<uint16_t>(ValueLen), ASN1_ERROR_INVALID_ENCODING);
    oid = ParseObjectID(Value, static_cast<uint16_t>(ValueLen));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::PutObjectId(OID oid)
{
    const uint8_t * encodedOID;
    uint16_t encodedOIDLen;

    VerifyOrReturnError(GetEncodedObjectID(oid, encodedOID, encodedOIDLen), ASN1_ERROR_UNKNOWN_OBJECT_ID);

    return PutObjectId(encodedOID, encodedOIDLen);
}

} // namespace ASN1
} // namespace chip
