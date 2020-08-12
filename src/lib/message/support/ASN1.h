/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef ASN1_H_
#define ASN1_H_

#include <support/DLLUtil.h>

#include "ASN1Error.h"

namespace chip {
namespace TLV {
class TLVReader;
} // namespace TLV
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

#include <support/ASN1OID.h>

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

class ASN1UniversalTime
{
public:
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

    uint8_t Class;
    uint32_t Tag;
    const uint8_t * Value;
    uint32_t ValueLen;
    bool IsConstructed;
    bool IsIndefiniteLen;
    bool IsEndOfContents;

    ASN1_ERROR Next(void);
    ASN1_ERROR EnterConstructedType(void);
    ASN1_ERROR ExitConstructedType(void);
    ASN1_ERROR EnterEncapsulatedType(void);
    ASN1_ERROR ExitEncapsulatedType(void);
    bool IsContained(void) const;
    ASN1_ERROR GetInteger(int64_t & val);
    ASN1_ERROR GetBoolean(bool & val);
    ASN1_ERROR GetObjectId(OID & oid);
    ASN1_ERROR GetUTCTime(ASN1UniversalTime & outTime);
    ASN1_ERROR GetGeneralizedTime(ASN1UniversalTime & outTime);
    ASN1_ERROR GetBitString(uint32_t & outVal);

private:
    struct ASN1ParseContext
    {
        const uint8_t * ElemStart;
        uint32_t HeadLen;
        uint32_t ValueLen;
        bool IsIndefiniteLen;
        const uint8_t * ContainerEnd;
    };

    enum
    {
        kMaxContextDepth = 32
    };

    const uint8_t * mBuf;
    const uint8_t * mBufEnd;
    const uint8_t * mElemStart;
    const uint8_t * mContainerEnd;
    uint32_t mHeadLen;
    ASN1ParseContext mSavedContexts[kMaxContextDepth];
    uint32_t mNumSavedContexts;

    ASN1_ERROR DecodeHead(void);
    void ResetElementState(void);
    ASN1_ERROR EnterContainer(uint32_t offset);
    ASN1_ERROR ExitContainer(void);
};

class DLL_EXPORT ASN1Writer
{
public:
    void Init(uint8_t * buf, uint32_t maxLen);
    void InitNullWriter(void);
    ASN1_ERROR Finalize(void);
    uint16_t GetLengthWritten(void) const;

    ASN1_ERROR PutInteger(int64_t val);
    ASN1_ERROR PutBoolean(bool val);
    ASN1_ERROR PutObjectId(const uint8_t * val, uint16_t valLen);
    ASN1_ERROR PutObjectId(OID oid);
    ASN1_ERROR PutString(uint32_t tag, const char * val, uint16_t valLen);
    ASN1_ERROR PutOctetString(const uint8_t * val, uint16_t valLen);
    ASN1_ERROR PutOctetString(uint8_t cls, uint32_t tag, const uint8_t * val, uint16_t valLen);
    ASN1_ERROR PutOctetString(uint8_t cls, uint32_t tag, chip::TLV::TLVReader & val);
    ASN1_ERROR PutBitString(uint32_t val);
    ASN1_ERROR PutBitString(uint8_t unusedBits, const uint8_t * val, uint16_t valLen);
    ASN1_ERROR PutBitString(uint8_t unusedBits, chip::TLV::TLVReader & val);
    ASN1_ERROR PutTime(const ASN1UniversalTime & val);
    ASN1_ERROR PutNull(void);
    ASN1_ERROR StartConstructedType(uint8_t cls, uint32_t tag);
    ASN1_ERROR EndConstructedType(void);
    ASN1_ERROR StartEncapsulatedType(uint8_t cls, uint32_t tag, bool bitStringEncoding);
    ASN1_ERROR EndEncapsulatedType(void);
    ASN1_ERROR PutValue(uint8_t cls, uint32_t tag, bool isConstructed, const uint8_t * val, uint16_t valLen);
    ASN1_ERROR PutValue(uint8_t cls, uint32_t tag, bool isConstructed, chip::TLV::TLVReader & val);

private:
    uint8_t * mBuf;
    uint8_t * mBufEnd;
    uint8_t * mWritePoint;
    uint8_t ** mDeferredLengthList;

    ASN1_ERROR EncodeHead(uint8_t cls, uint32_t tag, bool isConstructed, int32_t len);
    ASN1_ERROR WriteDeferredLength(void);
    static uint8_t GetLengthOfLength(int32_t len);
    static void EncodeLength(uint8_t * buf, uint8_t lenOfLen, int32_t lenToEncode);
};

extern OID ParseObjectID(const uint8_t * encodedOID, uint16_t encodedOIDLen);
extern bool GetEncodedObjectID(OID oid, const uint8_t *& encodedOID, uint16_t & encodedOIDLen);

extern OIDCategory GetOIDCategory(OID oid);

extern const char * GetOIDName(OID oid);

extern ASN1_ERROR DumpASN1(ASN1Reader & reader, const char * prefix, const char * indent);

} // namespace ASN1
} // namespace chip

#endif // ASN1_H_
