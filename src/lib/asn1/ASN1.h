/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines types and objects for reading and writing
 *      Abstract Syntax Notation One (ASN.1) encoded data.
 *
 */

#pragma once

#include <lib/support/DLLUtil.h>

#include <lib/asn1/ASN1Error.h>

namespace chip {
namespace TLV {
class TLVReader;
}
} // namespace chip

/**
 *   @namespace chip::ASN1
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for
 *     working with Abstract Syntax Notation One (ASN.1).
 */

namespace chip {
namespace ASN1 {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <asn1/ASN1OID.h>
#endif

static constexpr size_t kMaxConstructedAndEncapsulatedTypesDepth = 10;

enum ASN1TagClasses
{
    kASN1TagClass_Universal       = 0x00,
    kASN1TagClass_Application     = 0x40,
    kASN1TagClass_ContextSpecific = 0x80,
    kASN1TagClass_Private         = 0xC0
};

enum ASN1UniversalTags
{
    kASN1UniversalTag_Boolean         = 1,
    kASN1UniversalTag_Integer         = 2,
    kASN1UniversalTag_BitString       = 3,
    kASN1UniversalTag_OctetString     = 4,
    kASN1UniversalTag_Null            = 5,
    kASN1UniversalTag_ObjectId        = 6,
    kASN1UniversalTag_ObjectDesc      = 7,
    kASN1UniversalTag_External        = 8,
    kASN1UniversalTag_Real            = 9,
    kASN1UniversalTag_Enumerated      = 10,
    kASN1UniversalTag_UTF8String      = 12,
    kASN1UniversalTag_Sequence        = 16,
    kASN1UniversalTag_Set             = 17,
    kASN1UniversalTag_NumericString   = 18,
    kASN1UniversalTag_PrintableString = 19,
    kASN1UniversalTag_T61String       = 20,
    kASN1UniversalTag_VideotexString  = 21,
    kASN1UniversalTag_IA5String       = 22,
    kASN1UniversalTag_UTCTime         = 23,
    kASN1UniversalTag_GeneralizedTime = 24,
    kASN1UniversalTag_GraphicString   = 25,
    kASN1UniversalTag_VisibleString   = 26,
    kASN1UniversalTag_GeneralString   = 27,
    kASN1UniversalTag_UniversalString = 28
};

struct ASN1UniversalTime
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
};

class DLL_EXPORT ASN1Reader
{
public:
    void Init(const uint8_t * buf, uint32_t len);

    uint8_t GetClass(void) const { return Class; };
    uint32_t GetTag(void) const { return Tag; };
    const uint8_t * GetValue(void) const { return Value; };
    uint32_t GetValueLen(void) const { return ValueLen; };
    bool IsConstructed(void) const { return Constructed; };
    bool IsIndefiniteLen(void) const { return IndefiniteLen; };
    bool IsEndOfContents(void) const { return EndOfContents; };

    CHIP_ERROR Next(void);
    CHIP_ERROR EnterConstructedType(void);
    CHIP_ERROR ExitConstructedType(void);
    CHIP_ERROR GetConstructedType(const uint8_t *& val, uint32_t & valLen);
    CHIP_ERROR EnterEncapsulatedType(void);
    CHIP_ERROR ExitEncapsulatedType(void);
    bool IsContained(void) const;
    CHIP_ERROR GetInteger(int64_t & val);
    CHIP_ERROR GetBoolean(bool & val);
    CHIP_ERROR GetObjectId(OID & oid);
    CHIP_ERROR GetUTCTime(ASN1UniversalTime & outTime);
    CHIP_ERROR GetGeneralizedTime(ASN1UniversalTime & outTime);
    CHIP_ERROR GetBitString(uint32_t & outVal);

private:
    static constexpr size_t kMaxContextDepth = kMaxConstructedAndEncapsulatedTypesDepth;

    struct ASN1ParseContext
    {
        const uint8_t * ElemStart;
        uint32_t HeadLen;
        uint32_t ValueLen;
        bool IndefiniteLen;
        const uint8_t * ContainerEnd;
    };

    uint8_t Class;
    uint32_t Tag;
    const uint8_t * Value;
    uint32_t ValueLen;
    bool Constructed;
    bool IndefiniteLen;
    bool EndOfContents;

    const uint8_t * mBuf;
    const uint8_t * mBufEnd;
    const uint8_t * mElemStart;
    const uint8_t * mContainerEnd;
    uint32_t mHeadLen;
    ASN1ParseContext mSavedContexts[kMaxContextDepth];
    uint32_t mNumSavedContexts;

    CHIP_ERROR DecodeHead(void);
    void ResetElementState(void);
    CHIP_ERROR EnterContainer(uint32_t offset);
    CHIP_ERROR ExitContainer(void);
};

class DLL_EXPORT ASN1Writer
{
public:
    void Init(uint8_t * buf, uint32_t maxLen);
    void InitNullWriter(void);
    uint16_t GetLengthWritten(void) const;

    CHIP_ERROR PutInteger(int64_t val);
    CHIP_ERROR PutBoolean(bool val);
    CHIP_ERROR PutObjectId(const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutObjectId(OID oid);
    CHIP_ERROR PutString(uint32_t tag, const char * val, uint16_t valLen);
    CHIP_ERROR PutOctetString(const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutOctetString(uint8_t cls, uint32_t tag, const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutOctetString(uint8_t cls, uint32_t tag, chip::TLV::TLVReader & val);
    CHIP_ERROR PutBitString(uint32_t val);
    CHIP_ERROR PutBitString(uint8_t unusedBits, const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutBitString(uint8_t unusedBits, chip::TLV::TLVReader & val);
    CHIP_ERROR PutTime(const ASN1UniversalTime & val);
    CHIP_ERROR PutNull(void);
    CHIP_ERROR PutConstructedType(const uint8_t * val, uint16_t valLen);
    CHIP_ERROR StartConstructedType(uint8_t cls, uint32_t tag);
    CHIP_ERROR EndConstructedType(void);
    CHIP_ERROR StartEncapsulatedType(uint8_t cls, uint32_t tag, bool bitStringEncoding);
    CHIP_ERROR EndEncapsulatedType(void);
    CHIP_ERROR PutValue(uint8_t cls, uint32_t tag, bool isConstructed, const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutValue(uint8_t cls, uint32_t tag, bool isConstructed, chip::TLV::TLVReader & val);

private:
    static constexpr size_t kMaxDeferredLengthDepth = kMaxConstructedAndEncapsulatedTypesDepth;

    uint8_t * mBuf;
    uint8_t * mBufEnd;
    uint8_t * mWritePoint;
    uint8_t * mDeferredLengthLocations[kMaxDeferredLengthDepth];
    uint8_t mDeferredLengthCount;

    CHIP_ERROR EncodeHead(uint8_t cls, uint32_t tag, bool isConstructed, int32_t len);
    CHIP_ERROR WriteDeferredLength(void);
    static uint8_t BytesForLength(int32_t len);
    static void EncodeLength(uint8_t * buf, uint8_t bytesForLen, int32_t lenToEncode);
};

OID ParseObjectID(const uint8_t * encodedOID, uint16_t encodedOIDLen);
bool GetEncodedObjectID(OID oid, const uint8_t *& encodedOID, uint16_t & encodedOIDLen);

OIDCategory GetOIDCategory(OID oid);

const char * GetOIDName(OID oid);

CHIP_ERROR DumpASN1(ASN1Reader & reader, const char * prefix, const char * indent);

inline OID GetOID(OIDCategory category, uint8_t id)
{
    return static_cast<OID>(category | id);
}

inline uint8_t GetOIDEnum(OID oid)
{
    return static_cast<uint8_t>(oid & kOID_EnumMask);
}

} // namespace ASN1
} // namespace chip
