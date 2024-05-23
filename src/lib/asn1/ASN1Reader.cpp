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
 *      This file implements an object for reading Abstract Syntax
 *      Notation One (ASN.1) encoded data.
 *
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <lib/asn1/ASN1.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace ASN1 {

using namespace chip::Encoding;

void ASN1Reader::Init(const uint8_t * buf, size_t len)
{
    ResetElementState();
    mBuf              = buf;
    mBufEnd           = buf + len;
    mElemStart        = buf;
    mContainerEnd     = mBufEnd;
    mNumSavedContexts = 0;
}

CHIP_ERROR ASN1Reader::Next()
{
    ReturnErrorCodeIf(EndOfContents, ASN1_END);
    ReturnErrorCodeIf(IndefiniteLen, ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Note: avoid using addition assignment operator (+=), which may result in integer overflow
    // in the right hand side of an assignment (mHeadLen + ValueLen).
    mElemStart = mElemStart + mHeadLen + ValueLen;

    ResetElementState();

    ReturnErrorCodeIf(mElemStart == mContainerEnd, ASN1_END);

    return DecodeHead();
}

CHIP_ERROR ASN1Reader::EnterConstructedType()
{
    ReturnErrorCodeIf(!Constructed, ASN1_ERROR_INVALID_STATE);

    return EnterContainer(0);
}

CHIP_ERROR ASN1Reader::ExitConstructedType()
{
    return ExitContainer();
}

CHIP_ERROR ASN1Reader::GetConstructedType(const uint8_t *& val, uint32_t & valLen)
{
    ReturnErrorCodeIf(!Constructed, ASN1_ERROR_INVALID_STATE);

    val    = mElemStart;
    valLen = mHeadLen + ValueLen;

    return CHIP_NO_ERROR;
}
CHIP_ERROR ASN1Reader::EnterEncapsulatedType()
{
    VerifyOrReturnError(Class == kASN1TagClass_Universal &&
                            (Tag == kASN1UniversalTag_OctetString || Tag == kASN1UniversalTag_BitString),
                        ASN1_ERROR_INVALID_STATE);

    ReturnErrorCodeIf(Constructed, ASN1_ERROR_UNSUPPORTED_ENCODING);

    return EnterContainer((Tag == kASN1UniversalTag_BitString) ? 1 : 0);
}

CHIP_ERROR ASN1Reader::ExitEncapsulatedType()
{
    return ExitContainer();
}

CHIP_ERROR ASN1Reader::EnterContainer(uint32_t offset)
{
    ReturnErrorCodeIf(mNumSavedContexts == kMaxContextDepth, ASN1_ERROR_MAX_DEPTH_EXCEEDED);

    mSavedContexts[mNumSavedContexts].ElemStart     = mElemStart;
    mSavedContexts[mNumSavedContexts].HeadLen       = mHeadLen;
    mSavedContexts[mNumSavedContexts].ValueLen      = ValueLen;
    mSavedContexts[mNumSavedContexts].IndefiniteLen = IndefiniteLen;
    mSavedContexts[mNumSavedContexts].ContainerEnd  = mContainerEnd;
    mNumSavedContexts++;

    mElemStart = Value + offset;
    if (!IndefiniteLen)
    {
        VerifyOrReturnError(CanCastTo<uint32_t>(mBufEnd - Value), ASN1_ERROR_VALUE_OVERFLOW);
        VerifyOrReturnError(static_cast<uint32_t>(mBufEnd - Value) >= ValueLen, ASN1_ERROR_VALUE_OVERFLOW);
        mContainerEnd = Value + ValueLen;
    }

    ResetElementState();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::ExitContainer()
{
    ReturnErrorCodeIf(mNumSavedContexts == 0, ASN1_ERROR_INVALID_STATE);

    ASN1ParseContext & prevContext = mSavedContexts[--mNumSavedContexts];

    ReturnErrorCodeIf(prevContext.IndefiniteLen, ASN1_ERROR_UNSUPPORTED_ENCODING);

    mElemStart = prevContext.ElemStart + prevContext.HeadLen + prevContext.ValueLen;

    mContainerEnd = prevContext.ContainerEnd;

    ResetElementState();

    return CHIP_NO_ERROR;
}

bool ASN1Reader::IsContained() const
{
    return mNumSavedContexts > 0;
}

CHIP_ERROR ASN1Reader::GetInteger(int64_t & val)
{
    uint8_t encodedVal[sizeof(int64_t)] = { 0 };
    size_t valPaddingLen                = sizeof(int64_t) - ValueLen;

    ReturnErrorCodeIf(Value == nullptr, ASN1_ERROR_INVALID_STATE);
    ReturnErrorCodeIf(ValueLen < 1, ASN1_ERROR_INVALID_ENCODING);
    ReturnErrorCodeIf(ValueLen > sizeof(int64_t), ASN1_ERROR_VALUE_OVERFLOW);
    ReturnErrorCodeIf(mElemStart + mHeadLen + ValueLen > mContainerEnd, ASN1_ERROR_UNDERRUN);

    if ((*Value & 0x80) == 0x80)
    {
        for (size_t i = 0; i < valPaddingLen; i++)
        {
            encodedVal[i] = 0xFF;
        }
    }
    memcpy(&encodedVal[valPaddingLen], Value, ValueLen);

    val = static_cast<int64_t>(BigEndian::Get64(encodedVal));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::GetBoolean(bool & val)
{
    ReturnErrorCodeIf(Value == nullptr, ASN1_ERROR_INVALID_STATE);
    ReturnErrorCodeIf(ValueLen != 1, ASN1_ERROR_INVALID_ENCODING);
    ReturnErrorCodeIf(mElemStart + mHeadLen + ValueLen > mContainerEnd, ASN1_ERROR_UNDERRUN);
    VerifyOrReturnError(Value[0] == 0 || Value[0] == 0xFF, ASN1_ERROR_INVALID_ENCODING);

    val = (Value[0] != 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::GetUTCTime(ASN1UniversalTime & outTime)
{
    // Supported Encoding: YYMMDDHHMMSSZ
    ReturnErrorCodeIf(Value == nullptr, ASN1_ERROR_INVALID_STATE);
    ReturnErrorCodeIf(ValueLen < 1, ASN1_ERROR_INVALID_ENCODING);
    ReturnErrorCodeIf(mElemStart + mHeadLen + ValueLen > mContainerEnd, ASN1_ERROR_UNDERRUN);
    VerifyOrReturnError(ValueLen == 13 && Value[12] == 'Z', ASN1_ERROR_UNSUPPORTED_ENCODING);

    return outTime.ImportFrom_ASN1_TIME_string(CharSpan(reinterpret_cast<const char *>(Value), ValueLen));
}

CHIP_ERROR ASN1Reader::GetGeneralizedTime(ASN1UniversalTime & outTime)
{
    // Supported Encoding: YYYYMMDDHHMMSSZ
    ReturnErrorCodeIf(Value == nullptr, ASN1_ERROR_INVALID_STATE);
    ReturnErrorCodeIf(ValueLen < 1, ASN1_ERROR_INVALID_ENCODING);
    ReturnErrorCodeIf(mElemStart + mHeadLen + ValueLen > mContainerEnd, ASN1_ERROR_UNDERRUN);
    VerifyOrReturnError(ValueLen == 15 && Value[14] == 'Z', ASN1_ERROR_UNSUPPORTED_ENCODING);

    return outTime.ImportFrom_ASN1_TIME_string(CharSpan(reinterpret_cast<const char *>(Value), ValueLen));
}

static uint8_t ReverseBits(uint8_t v)
{
    // swap adjacent bits
    v = static_cast<uint8_t>(static_cast<uint8_t>((v >> 1) & 0x55) | static_cast<uint8_t>((v & 0x55) << 1));
    // swap adjacent bit pairs
    v = static_cast<uint8_t>(static_cast<uint8_t>((v >> 2) & 0x33) | static_cast<uint8_t>((v & 0x33) << 2));
    // swap nibbles
    v = static_cast<uint8_t>(static_cast<uint8_t>(v >> 4) | static_cast<uint8_t>(v << 4));
    return v;
}

CHIP_ERROR ASN1Reader::GetBitString(uint32_t & outVal)
{
    // NOTE: only supports DER encoding.
    ReturnErrorCodeIf(Value == nullptr, ASN1_ERROR_INVALID_STATE);
    ReturnErrorCodeIf(ValueLen < 1, ASN1_ERROR_INVALID_ENCODING);
    ReturnErrorCodeIf(ValueLen > 5, ASN1_ERROR_UNSUPPORTED_ENCODING);
    ReturnErrorCodeIf(mElemStart + mHeadLen + ValueLen > mContainerEnd, ASN1_ERROR_UNDERRUN);

    if (ValueLen == 1)
    {
        outVal = 0;
    }
    else
    {
        outVal    = ReverseBits(Value[1]);
        int shift = 8;
        for (uint32_t i = 2; i < ValueLen; i++, shift += 8)
        {
            outVal |= static_cast<uint32_t>(ReverseBits(Value[i]) << shift);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::DecodeHead()
{
    const uint8_t * p = mElemStart;
    ReturnErrorCodeIf(p >= mBufEnd, ASN1_ERROR_UNDERRUN);

    Class       = *p & 0xC0;
    Constructed = (*p & 0x20) != 0;
    Tag         = *p & 0x1F;

    // Only tags < 31 supported. The implication of this is that encoded tags are exactly 1 byte long.
    VerifyOrReturnError(Tag < 0x1F, ASN1_ERROR_UNSUPPORTED_ENCODING);

    p++;
    ReturnErrorCodeIf(p >= mBufEnd, ASN1_ERROR_UNDERRUN);

    if ((*p & 0x80) == 0)
    {
        ValueLen      = *p & 0x7F;
        IndefiniteLen = false;
        p++;
    }
    else if (*p == 0x80)
    {
        ValueLen      = 0;
        IndefiniteLen = true;
        p++;
    }
    else
    {
        ValueLen       = 0;
        uint8_t lenLen = *p & 0x7F;
        p++;
        for (; lenLen > 0; lenLen--, p++)
        {
            ReturnErrorCodeIf(p >= mBufEnd, ASN1_ERROR_UNDERRUN);
            ReturnErrorCodeIf((ValueLen & 0xFF000000) != 0, ASN1_ERROR_LENGTH_OVERFLOW);
            ValueLen = (ValueLen << 8) | *p;
        }
        IndefiniteLen = false;
    }

    VerifyOrReturnError(CanCastTo<uint32_t>(mBufEnd - p), ASN1_ERROR_VALUE_OVERFLOW);
    VerifyOrReturnError(static_cast<uint32_t>(mBufEnd - p) >= ValueLen, ASN1_ERROR_VALUE_OVERFLOW);
    VerifyOrReturnError(CanCastTo<uint32_t>(p - mElemStart), ASN1_ERROR_VALUE_OVERFLOW);
    mHeadLen = static_cast<uint32_t>(p - mElemStart);

    EndOfContents = (Class == kASN1TagClass_Universal && Tag == 0 && !Constructed && ValueLen == 0);

    Value = p;

    return CHIP_NO_ERROR;
}

void ASN1Reader::ResetElementState()
{
    Class         = 0;
    Tag           = 0;
    Value         = nullptr;
    ValueLen      = 0;
    Constructed   = false;
    IndefiniteLen = false;
    EndOfContents = false;
    mHeadLen      = 0;
}

CHIP_ERROR DumpASN1(ASN1Reader & asn1Parser, const char * prefix, const char * indent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (indent == nullptr)
        indent = "  ";

    int nestLevel = 0;
    while (true)
    {
        err = asn1Parser.Next();
        if (err != CHIP_NO_ERROR)
        {
            if (err == ASN1_END)
            {
                if (asn1Parser.IsContained())
                {
                    err = asn1Parser.ExitConstructedType();
                    if (err != CHIP_NO_ERROR)
                    {
                        printf("ASN1Reader::ExitConstructedType() failed: %" CHIP_ERROR_FORMAT "\n", err.Format());
                        return err;
                    }
                    nestLevel--;
                    continue;
                }
                break;
            }
            printf("ASN1Reader::Next() failed: %" CHIP_ERROR_FORMAT "\n", err.Format());
            return err;
        }
        if (prefix != nullptr)
            printf("%s", prefix);
        for (int i = nestLevel; i; i--)
            printf("%s", indent);
        if (asn1Parser.IsEndOfContents())
            printf("END-OF-CONTENTS ");
        else if (asn1Parser.GetClass() == kASN1TagClass_Universal)
            switch (asn1Parser.GetTag())
            {
            case kASN1UniversalTag_Boolean:
                printf("BOOLEAN ");
                break;
            case kASN1UniversalTag_Integer:
                printf("INTEGER ");
                break;
            case kASN1UniversalTag_BitString:
                printf("BIT STRING ");
                break;
            case kASN1UniversalTag_OctetString:
                printf("OCTET STRING ");
                break;
            case kASN1UniversalTag_Null:
                printf("NULL ");
                break;
            case kASN1UniversalTag_ObjectId:
                printf("OBJECT IDENTIFIER ");
                break;
            case kASN1UniversalTag_ObjectDesc:
                printf("OBJECT DESCRIPTOR ");
                break;
            case kASN1UniversalTag_External:
                printf("EXTERNAL ");
                break;
            case kASN1UniversalTag_Real:
                printf("REAL ");
                break;
            case kASN1UniversalTag_Enumerated:
                printf("ENUMERATED ");
                break;
            case kASN1UniversalTag_Sequence:
                printf("SEQUENCE ");
                break;
            case kASN1UniversalTag_Set:
                printf("SET ");
                break;
            case kASN1UniversalTag_UTF8String:
            case kASN1UniversalTag_NumericString:
            case kASN1UniversalTag_PrintableString:
            case kASN1UniversalTag_T61String:
            case kASN1UniversalTag_VideotexString:
            case kASN1UniversalTag_IA5String:
            case kASN1UniversalTag_GraphicString:
            case kASN1UniversalTag_VisibleString:
            case kASN1UniversalTag_GeneralString:
            case kASN1UniversalTag_UniversalString:
                printf("STRING ");
                break;
            case kASN1UniversalTag_UTCTime:
            case kASN1UniversalTag_GeneralizedTime:
                printf("TIME ");
                break;
            default:
                printf("[UNIVERSAL %lu] ", static_cast<unsigned long>(asn1Parser.GetTag()));
                break;
            }
        else if (asn1Parser.GetClass() == kASN1TagClass_Application)
            printf("[APPLICATION %lu] ", static_cast<unsigned long>(asn1Parser.GetTag()));
        else if (asn1Parser.GetClass() == kASN1TagClass_ContextSpecific)
            printf("[%lu] ", static_cast<unsigned long>(asn1Parser.GetTag()));
        else if (asn1Parser.GetClass() == kASN1TagClass_Private)
            printf("[PRIVATE %lu] ", static_cast<unsigned long>(asn1Parser.GetTag()));

        if (asn1Parser.IsConstructed())
            printf("(constructed) ");

        if (asn1Parser.IsIndefiniteLen())
            printf("Length = indefinite\n");
        else
            printf("Length = %ld\n", static_cast<long>(asn1Parser.GetValueLen()));

        if (asn1Parser.IsConstructed())
        {
            err = asn1Parser.EnterConstructedType();
            if (err != CHIP_NO_ERROR)
            {
                printf("ASN1Reader::EnterConstructedType() failed: %" CHIP_ERROR_FORMAT "\n", err.Format());
                return err;
            }
            nestLevel++;
        }
    }

    return err;
}

} // namespace ASN1
} // namespace chip
