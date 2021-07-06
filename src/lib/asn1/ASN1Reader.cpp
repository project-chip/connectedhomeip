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
 *      This file implements an object for reading Abstract Syntax
 *      Notation One (ASN.1) encoded data.
 *
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <asn1/ASN1.h>

namespace chip {
namespace ASN1 {

void ASN1Reader::Init(const uint8_t * buf, uint32_t len)
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
    if (EndOfContents)
        return ASN1_END;

    if (IndefiniteLen)
    {
        return ASN1_ERROR_UNSUPPORTED_ENCODING;
    }
    else
        mElemStart += (mHeadLen + ValueLen);

    ResetElementState();

    if (mElemStart == mContainerEnd)
        return ASN1_END;

    return DecodeHead();
}

CHIP_ERROR ASN1Reader::EnterConstructedType()
{
    if (!Constructed)
        return ASN1_ERROR_INVALID_STATE;

    return EnterContainer(0);
}

CHIP_ERROR ASN1Reader::ExitConstructedType()
{
    return ExitContainer();
}

CHIP_ERROR ASN1Reader::GetConstructedType(const uint8_t *& val, uint32_t & valLen)
{
    if (!Constructed)
        return ASN1_ERROR_INVALID_STATE;

    val    = mElemStart;
    valLen = mHeadLen + ValueLen;

    return CHIP_NO_ERROR;
}
CHIP_ERROR ASN1Reader::EnterEncapsulatedType()
{
    if (Class != kASN1TagClass_Universal || (Tag != kASN1UniversalTag_OctetString && Tag != kASN1UniversalTag_BitString))
        return ASN1_ERROR_INVALID_STATE;

    if (Constructed)
        return ASN1_ERROR_UNSUPPORTED_ENCODING;

    return EnterContainer((Tag == kASN1UniversalTag_BitString) ? 1 : 0);
}

CHIP_ERROR ASN1Reader::ExitEncapsulatedType()
{
    return ExitContainer();
}

CHIP_ERROR ASN1Reader::EnterContainer(uint32_t offset)
{
    if (mNumSavedContexts == kMaxContextDepth)
        return ASN1_ERROR_MAX_DEPTH_EXCEEDED;

    mSavedContexts[mNumSavedContexts].ElemStart     = mElemStart;
    mSavedContexts[mNumSavedContexts].HeadLen       = mHeadLen;
    mSavedContexts[mNumSavedContexts].ValueLen      = ValueLen;
    mSavedContexts[mNumSavedContexts].IndefiniteLen = IndefiniteLen;
    mSavedContexts[mNumSavedContexts].ContainerEnd  = mContainerEnd;
    mNumSavedContexts++;

    mElemStart = Value + offset;
    if (!IndefiniteLen)
        mContainerEnd = Value + ValueLen;

    ResetElementState();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::ExitContainer()
{
    if (mNumSavedContexts == 0)
        return ASN1_ERROR_INVALID_STATE;

    ASN1ParseContext & prevContext = mSavedContexts[--mNumSavedContexts];

    if (prevContext.IndefiniteLen)
    {
        return ASN1_ERROR_UNSUPPORTED_ENCODING;
    }
    else
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
    if (Value == nullptr)
        return ASN1_ERROR_INVALID_STATE;
    if (ValueLen < 1)
        return ASN1_ERROR_INVALID_ENCODING;
    if (ValueLen > sizeof(int64_t))
        return ASN1_ERROR_VALUE_OVERFLOW;
    if (mElemStart + mHeadLen + ValueLen > mContainerEnd)
        return ASN1_ERROR_UNDERRUN;
    const uint8_t * p = Value;
    val               = ((*p & 0x80) == 0) ? 0 : -1;
    for (uint32_t i = ValueLen; i > 0; i--, p++)
        val = (val << 8) | *p;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::GetBoolean(bool & val)
{
    if (Value == nullptr)
        return ASN1_ERROR_INVALID_STATE;
    if (ValueLen != 1)
        return ASN1_ERROR_INVALID_ENCODING;
    if (mElemStart + mHeadLen + ValueLen > mContainerEnd)
        return ASN1_ERROR_UNDERRUN;
    if (*Value == 0)
        val = false;
    else if (*Value == 0xFF)
        val = true;
    else
        return ASN1_ERROR_INVALID_ENCODING;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::GetUTCTime(ASN1UniversalTime & outTime)
{
    // Supported Encoding: YYMMDDHHMMSSZ

    if (Value == nullptr)
        return ASN1_ERROR_INVALID_STATE;
    if (ValueLen < 1)
        return ASN1_ERROR_INVALID_ENCODING;
    if (mElemStart + mHeadLen + ValueLen > mContainerEnd)
        return ASN1_ERROR_UNDERRUN;

    if (ValueLen != 13 || Value[12] != 'Z')
        return ASN1_ERROR_UNSUPPORTED_ENCODING;

    for (int i = 0; i < 12; i++)
        if (!isdigit(Value[i]))
            return ASN1_ERROR_INVALID_ENCODING;

    outTime.Year   = (Value[0] - '0') * 10 + (Value[1] - '0');
    outTime.Month  = (Value[2] - '0') * 10 + (Value[3] - '0');
    outTime.Day    = (Value[4] - '0') * 10 + (Value[5] - '0');
    outTime.Hour   = (Value[6] - '0') * 10 + (Value[7] - '0');
    outTime.Minute = (Value[8] - '0') * 10 + (Value[9] - '0');
    outTime.Second = (Value[10] - '0') * 10 + (Value[11] - '0');

    if (outTime.Year >= 50)
        outTime.Year += 1900;
    else
        outTime.Year += 2000;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::GetGeneralizedTime(ASN1UniversalTime & outTime)
{
    // Supported Encoding: YYYYMMDDHHMMSSZ

    if (Value == nullptr)
        return ASN1_ERROR_INVALID_STATE;
    if (ValueLen < 1)
        return ASN1_ERROR_INVALID_ENCODING;
    if (mElemStart + mHeadLen + ValueLen > mContainerEnd)
        return ASN1_ERROR_UNDERRUN;

    if (ValueLen != 15 || Value[14] != 'Z')
        return ASN1_ERROR_UNSUPPORTED_ENCODING;

    for (int i = 0; i < 14; i++)
        if (!isdigit(Value[i]))
            return ASN1_ERROR_INVALID_ENCODING;

    outTime.Year   = (Value[0] - '0') * 1000 + (Value[1] - '0') * 100 + (Value[2] - '0') * 10 + (Value[3] - '0');
    outTime.Month  = (Value[4] - '0') * 10 + (Value[5] - '0');
    outTime.Day    = (Value[6] - '0') * 10 + (Value[7] - '0');
    outTime.Hour   = (Value[8] - '0') * 10 + (Value[9] - '0');
    outTime.Minute = (Value[10] - '0') * 10 + (Value[11] - '0');
    outTime.Second = (Value[12] - '0') * 10 + (Value[13] - '0');

    return CHIP_NO_ERROR;
}

static uint8_t ReverseBits(uint8_t v)
{
    // swap adjacent bits
    v = ((v >> 1) & 0x55) | ((v & 0x55) << 1);
    // swap adjacent bit pairs
    v = ((v >> 2) & 0x33) | ((v & 0x33) << 2);
    // swap nibbles
    v = (v >> 4) | (v << 4);
    return v;
}

CHIP_ERROR ASN1Reader::GetBitString(uint32_t & outVal)
{
    // NOTE: only supports DER encoding.

    if (Value == nullptr)
        return ASN1_ERROR_INVALID_STATE;
    if (ValueLen < 1)
        return ASN1_ERROR_INVALID_ENCODING;
    if (ValueLen > 5)
        return ASN1_ERROR_UNSUPPORTED_ENCODING;
    if (mElemStart + mHeadLen + ValueLen > mContainerEnd)
        return ASN1_ERROR_UNDERRUN;

    if (ValueLen == 1)
        outVal = 0;
    else
    {
        outVal    = ReverseBits(Value[1]);
        int shift = 8;
        for (uint32_t i = 2; i < ValueLen; i++, shift += 8)
            outVal |= (ReverseBits(Value[i]) << shift);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::DecodeHead()
{
    const uint8_t * p = mElemStart;

    if (p >= mBufEnd)
        return ASN1_ERROR_UNDERRUN;

    Class       = *p & 0xC0;
    Constructed = (*p & 0x20) != 0;

    Tag = *p & 0x1F;
    p++;
    if (Tag == 0x1F)
    {
        Tag = 0;
        do
        {
            if (p >= mBufEnd)
                return ASN1_ERROR_UNDERRUN;
            if ((Tag & 0xFE000000) != 0)
                return ASN1_ERROR_TAG_OVERFLOW;
            Tag = (Tag << 7) | (*p & 0x7F);
            p++;
        } while ((*p & 0x80) != 0);
    }

    if (p >= mBufEnd)
        return ASN1_ERROR_UNDERRUN;

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
            if (p >= mBufEnd)
                return ASN1_ERROR_UNDERRUN;
            if ((ValueLen & 0xFF000000) != 0)
                return ASN1_ERROR_LENGTH_OVERFLOW;
            ValueLen = (ValueLen << 8) | *p;
        }
        IndefiniteLen = false;
    }

    mHeadLen = p - mElemStart;

    EndOfContents = (Class == kASN1TagClass_Universal && Tag == 0 && Constructed == false && ValueLen == 0);

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
                        printf("ASN1Reader::ExitConstructedType() failed: %ld\n", (long) err);
                        return err;
                    }
                    nestLevel--;
                    continue;
                }
                else
                    break;
            }
            printf("ASN1Reader::Next() failed: %ld\n", (long) err);
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
                printf("ASN1Reader::EnterConstructedType() failed: %ld\n", (long) err);
                return err;
            }
            nestLevel++;
        }
    }

    return err;
}

} // namespace ASN1
} // namespace chip
