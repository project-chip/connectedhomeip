/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <asn1/ASN1OID.h>
#endif

#include <lib/asn1/ASN1Error.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

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

static constexpr size_t kMaxConstructedAndEncapsulatedTypesDepth = 10;

enum ASN1TagClasses
{
    kASN1TagClass_Universal       = 0x00,
    kASN1TagClass_Application     = 0x40,
    kASN1TagClass_ContextSpecific = 0x80,
    kASN1TagClass_Private         = 0xC0
};

enum ASN1UniversalTags : uint8_t
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

/**
 *  @struct ASN1UniversalTime
 *
 *  @brief
 *    A data structure representing ASN1 universal time in a calendar format.
 */
struct ASN1UniversalTime
{
    uint16_t Year;  /**< Year component. Legal interval is 0..9999. */
    uint8_t Month;  /**< Month component. Legal interval is 1..12. */
    uint8_t Day;    /**< Day of month component. Legal interval is 1..31. */
    uint8_t Hour;   /**< Hour component. Legal interval is 0..23. */
    uint8_t Minute; /**< Minute component. Legal interval is 0..59. */
    uint8_t Second; /**< Second component. Legal interval is 0..59. */

    static constexpr size_t kASN1UTCTimeStringLength         = 13;
    static constexpr size_t kASN1GeneralizedTimeStringLength = 15;
    static constexpr size_t kASN1TimeStringMaxLength         = 15;

    /**
     * @brief Set time from ASN1_TIME string.
     *        Two string formats are supported:
     *            YYMMDDHHMMSSZ - for years in the range 1950 - 2049
     *          YYYYMMDDHHMMSSZ - other years
     **/
    CHIP_ERROR ImportFrom_ASN1_TIME_string(const CharSpan & asn1_time);

    /**
     * @brief Encode time as an ASN1_TIME string.
     *        Two string formats are supported:
     *            YYMMDDHHMMSSZ - for years in the range 1950 - 2049
     *          YYYYMMDDHHMMSSZ - other years
     **/
    CHIP_ERROR ExportTo_ASN1_TIME_string(MutableCharSpan & asn1_time) const;

    /**
     * @brief Encode time as Unix epoch time.
     **/
    bool ExportTo_UnixTime(uint32_t & unixEpoch);
};

class DLL_EXPORT ASN1Reader
{
public:
    void Init(const uint8_t * buf, size_t len);
    void Init(const ByteSpan & data) { Init(data.data(), data.size()); }
    template <size_t N>
    void Init(const uint8_t (&data)[N])
    {
        Init(data, N);
    }

    uint8_t GetClass(void) const { return Class; };
    uint8_t GetTag(void) const { return Tag; };
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
    uint8_t Tag;
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
    void Init(uint8_t * buf, size_t maxLen);
    void Init(const MutableByteSpan & data) { Init(data.data(), data.size()); }
    template <size_t N>
    void Init(uint8_t (&data)[N])
    {
        Init(data, N);
    }
    void InitNullWriter(void);
    size_t GetLengthWritten(void) const;

    bool IsNullWriter() const { return mBuf == nullptr; }

    CHIP_ERROR PutInteger(int64_t val);
    CHIP_ERROR PutBoolean(bool val);
    CHIP_ERROR PutObjectId(const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutObjectId(OID oid);
    CHIP_ERROR PutString(uint8_t tag, const char * val, uint16_t valLen);
    CHIP_ERROR PutOctetString(const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutOctetString(uint8_t cls, uint8_t tag, const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutOctetString(uint8_t cls, uint8_t tag, chip::TLV::TLVReader & tlvReader);
    CHIP_ERROR PutBitString(uint32_t val);
    CHIP_ERROR PutBitString(uint8_t unusedBits, const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutBitString(uint8_t unusedBits, chip::TLV::TLVReader & tlvReader);
    CHIP_ERROR PutTime(const ASN1UniversalTime & val);
    CHIP_ERROR PutNull(void);
    CHIP_ERROR PutConstructedType(const uint8_t * val, uint16_t valLen);
    CHIP_ERROR StartConstructedType(uint8_t cls, uint8_t tag);
    CHIP_ERROR EndConstructedType(void);
    CHIP_ERROR StartEncapsulatedType(uint8_t cls, uint8_t tag, bool bitStringEncoding);
    CHIP_ERROR EndEncapsulatedType(void);
    CHIP_ERROR PutValue(uint8_t cls, uint8_t tag, bool isConstructed, const uint8_t * val, uint16_t valLen);
    CHIP_ERROR PutValue(uint8_t cls, uint8_t tag, bool isConstructed, chip::TLV::TLVReader & tlvReader);

private:
    static constexpr size_t kMaxDeferredLengthDepth = kMaxConstructedAndEncapsulatedTypesDepth;

    uint8_t * mBuf;
    uint8_t * mBufEnd;
    uint8_t * mWritePoint;
    uint8_t * mDeferredLengthLocations[kMaxDeferredLengthDepth];
    uint8_t mDeferredLengthCount;

    CHIP_ERROR EncodeHead(uint8_t cls, uint8_t tag, bool isConstructed, int32_t len);
    CHIP_ERROR WriteDeferredLength(void);
    static uint8_t BytesForLength(int32_t len);
    static void EncodeLength(uint8_t * buf, uint8_t bytesForLen, int32_t lenToEncode);
    void WriteData(const uint8_t * p, size_t len);
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
