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
    VerifyOrReturnError(!EndOfContents, ASN1_END);
    VerifyOrReturnError(!IndefiniteLen, ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Defense-in-depth against integer overflow: mHeadLen + ValueLen are both uint32_t and
    // their sum could in principle wrap around, advancing mElemStart to an unintended
    // location. DecodeHead already caps ValueLen at the remaining buffer, so this cannot
    // wrap from any input reaching Next() through the public API; the guard hardens against
    // a future caller or a regression in those upstream bounds checks.
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    // Defend against an inconsistent reader state where mElemStart has advanced past
    // mContainerEnd (e.g. a prior failed unwind left stale pointers). Without this
    // check the pointer subtraction below would underflow when cast to size_t and the
    // bounds comparison would silently pass.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    // Compare in integer space rather than pointer space. On 32-bit targets size_t == uint32_t,
    // so the size_t cast provides no additional width; the preceding LENGTH_OVERFLOW guard is
    // what prevents wrap there. The size_t widening eliminates pointer-arithmetic UB on 64-bit
    // targets where computing `mElemStart + mHeadLen + ValueLen` could otherwise wrap past the
    // end of the address space before the comparison runs.
    //
    // BEHAVIOR CHANGE: a bounds-exceeded result here means a child element claims more bytes
    // than its parent container holds (malformed encoding). Previously this advanced mElemStart
    // past mContainerEnd and the subsequent `mElemStart != mContainerEnd` check returned ASN1_END,
    // which callers (and the ASN1_EXIT_* macros) treat as a clean end of container -- silently
    // swallowing the malformed input. It now surfaces as ASN1_ERROR_INVALID_ENCODING. A child
    // ending exactly at mContainerEnd still passes this `<=` check and reports ASN1_END as before,
    // so well-formed certificates are unaffected.
    VerifyOrReturnError(static_cast<size_t>(mHeadLen) + ValueLen <= static_cast<size_t>(mContainerEnd - mElemStart),
                        ASN1_ERROR_INVALID_ENCODING);

    mElemStart = mElemStart + mHeadLen + ValueLen;

    ResetElementState();

    VerifyOrReturnError(mElemStart != mContainerEnd, ASN1_END);

    return DecodeHead();
}

CHIP_ERROR ASN1Reader::EnterConstructedType()
{
    VerifyOrReturnError(Constructed, ASN1_ERROR_INVALID_STATE);

    return EnterContainer(0);
}

CHIP_ERROR ASN1Reader::ExitConstructedType()
{
    return ExitContainer();
}

CHIP_ERROR ASN1Reader::GetConstructedType(const uint8_t *& val, uint32_t & valLen)
{
    VerifyOrReturnError(Constructed, ASN1_ERROR_INVALID_STATE);

    // Defend against an inconsistent reader state where mElemStart has advanced past
    // mContainerEnd. Required ahead of any size_t-widened subtraction so the result
    // cannot underflow.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    // Guard against integer overflow: mHeadLen + ValueLen are both uint32_t and their sum
    // could wrap around, producing a bogus length for the caller.
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);

    val    = mElemStart;
    valLen = mHeadLen + ValueLen;

    return CHIP_NO_ERROR;
}
CHIP_ERROR ASN1Reader::EnterEncapsulatedType()
{
    VerifyOrReturnError(Class == kASN1TagClass_Universal &&
                            (Tag == kASN1UniversalTag_OctetString || Tag == kASN1UniversalTag_BitString),
                        ASN1_ERROR_INVALID_STATE);

    VerifyOrReturnError(!Constructed, ASN1_ERROR_UNSUPPORTED_ENCODING);

    return EnterContainer((Tag == kASN1UniversalTag_BitString) ? 1 : 0);
}

CHIP_ERROR ASN1Reader::ExitEncapsulatedType()
{
    return ExitContainer();
}

CHIP_ERROR ASN1Reader::EnterContainer(uint32_t offset)
{
    VerifyOrReturnError(mNumSavedContexts != kMaxContextDepth, ASN1_ERROR_MAX_DEPTH_EXCEEDED);

    // Peek-then-commit: validate all preconditions BEFORE mutating reader
    // state. If any guard fires the saved-context stack, mElemStart, and
    // mContainerEnd are all left untouched so the reader stays in a
    // consistent state.
    if (!IndefiniteLen)
    {
        VerifyOrReturnError(CanCastTo<uint32_t>(mBufEnd - Value), ASN1_ERROR_VALUE_OVERFLOW);
        VerifyOrReturnError(static_cast<uint32_t>(mBufEnd - Value) >= ValueLen, ASN1_ERROR_VALUE_OVERFLOW);
    }

    // All checks passed - now commit by pushing the saved context and updating reader state.
    mSavedContexts[mNumSavedContexts].ElemStart     = mElemStart;
    mSavedContexts[mNumSavedContexts].HeadLen       = mHeadLen;
    mSavedContexts[mNumSavedContexts].ValueLen      = ValueLen;
    mSavedContexts[mNumSavedContexts].IndefiniteLen = IndefiniteLen;
    mSavedContexts[mNumSavedContexts].ContainerEnd  = mContainerEnd;
    mNumSavedContexts++;

    mElemStart = Value + offset;
    if (!IndefiniteLen)
    {
        mContainerEnd = Value + ValueLen;
    }

    ResetElementState();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::ExitContainer()
{
    VerifyOrReturnError(mNumSavedContexts != 0, ASN1_ERROR_INVALID_STATE);

    // Peek-then-commit: validate the saved context BEFORE mutating any reader
    // state. On any error the saved-context stack is left untouched so the
    // reader stays in a consistent state and a follow-up call sees the same
    // failure (rather than silently operating on a half-popped stack).
    const ASN1ParseContext & prevContext = mSavedContexts[mNumSavedContexts - 1];

    VerifyOrReturnError(!prevContext.IndefiniteLen, ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Guard against integer overflow: HeadLen + ValueLen are both uint32_t and their sum
    // could wrap around, potentially advancing mElemStart to an unintended location.
    VerifyOrReturnError(prevContext.HeadLen <= UINT32_MAX - prevContext.ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    // Defend against a saved context with ContainerEnd < ElemStart so the pointer
    // subtraction below cannot underflow when widened to size_t.
    VerifyOrReturnError(prevContext.ContainerEnd >= prevContext.ElemStart, ASN1_ERROR_INVALID_STATE);
    // Compare in integer space rather than pointer space. On 32-bit targets size_t == uint32_t
    // and the LENGTH_OVERFLOW guard above is what prevents wrap; the size_t widening eliminates
    // pointer-arithmetic UB on 64-bit targets where `prevContext.ElemStart + prevContext.HeadLen
    // + prevContext.ValueLen` could otherwise wrap past the end of the address space. A
    // bounds-exceeded result here means the saved container declared more bytes than its parent
    // holds (malformed encoding), NOT a clean end of container, so it must surface as
    // INVALID_ENCODING rather than ASN1_END.
    VerifyOrReturnError(static_cast<size_t>(prevContext.HeadLen) + prevContext.ValueLen <=
                            static_cast<size_t>(prevContext.ContainerEnd - prevContext.ElemStart),
                        ASN1_ERROR_INVALID_ENCODING);

    // All checks passed - now commit by popping the saved context and updating reader state.
    --mNumSavedContexts;
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

    VerifyOrReturnError(Value != nullptr, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(ValueLen >= 1, ASN1_ERROR_INVALID_ENCODING);
    VerifyOrReturnError(ValueLen <= sizeof(int64_t), ASN1_ERROR_VALUE_OVERFLOW);
    // Mirrors the guard sequence in Next(): (1) reject inconsistent state where
    // mContainerEnd < mElemStart (without this the size_t-widened subtraction below would
    // underflow to a huge value and the bounds compare would trivially pass), (2) reject
    // mHeadLen + ValueLen wrap on 32-bit targets where size_t == uint32_t, (3) bounds-check
    // via the size_t-widened comparison.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    VerifyOrReturnError(static_cast<size_t>(mHeadLen) + ValueLen <= static_cast<size_t>(mContainerEnd - mElemStart),
                        ASN1_ERROR_UNDERRUN);

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
    VerifyOrReturnError(Value != nullptr, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(ValueLen == 1, ASN1_ERROR_INVALID_ENCODING);
    // Mirrors the guard sequence in Next(): (1) reject inconsistent state where
    // mContainerEnd < mElemStart, (2) reject mHeadLen + ValueLen wrap on 32-bit
    // (size_t == uint32_t), (3) bounds-check via size_t-widened comparison.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    VerifyOrReturnError(static_cast<size_t>(mHeadLen) + ValueLen <= static_cast<size_t>(mContainerEnd - mElemStart),
                        ASN1_ERROR_UNDERRUN);
    VerifyOrReturnError(Value[0] == 0 || Value[0] == 0xFF, ASN1_ERROR_INVALID_ENCODING);

    val = (Value[0] != 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::GetUTCTime(ASN1UniversalTime & outTime)
{
    // Supported Encoding: YYMMDDHHMMSSZ
    VerifyOrReturnError(Value != nullptr, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(ValueLen >= 1, ASN1_ERROR_INVALID_ENCODING);
    // Mirrors the guard sequence in Next(): (1) reject inconsistent state where
    // mContainerEnd < mElemStart, (2) reject mHeadLen + ValueLen wrap on 32-bit
    // (size_t == uint32_t), (3) bounds-check via size_t-widened comparison.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    VerifyOrReturnError(static_cast<size_t>(mHeadLen) + ValueLen <= static_cast<size_t>(mContainerEnd - mElemStart),
                        ASN1_ERROR_UNDERRUN);
    VerifyOrReturnError(ValueLen == 13 && Value[12] == 'Z', ASN1_ERROR_UNSUPPORTED_ENCODING);

    return outTime.ImportFrom_ASN1_TIME_string(CharSpan(reinterpret_cast<const char *>(Value), ValueLen));
}

CHIP_ERROR ASN1Reader::GetGeneralizedTime(ASN1UniversalTime & outTime)
{
    // Supported Encoding: YYYYMMDDHHMMSSZ
    VerifyOrReturnError(Value != nullptr, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(ValueLen >= 1, ASN1_ERROR_INVALID_ENCODING);
    // Mirrors the guard sequence in Next(): (1) reject inconsistent state where
    // mContainerEnd < mElemStart, (2) reject mHeadLen + ValueLen wrap on 32-bit
    // (size_t == uint32_t), (3) bounds-check via size_t-widened comparison.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    VerifyOrReturnError(static_cast<size_t>(mHeadLen) + ValueLen <= static_cast<size_t>(mContainerEnd - mElemStart),
                        ASN1_ERROR_UNDERRUN);
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
    VerifyOrReturnError(Value != nullptr, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(ValueLen >= 1, ASN1_ERROR_INVALID_ENCODING);
    VerifyOrReturnError(ValueLen <= 5, ASN1_ERROR_UNSUPPORTED_ENCODING);
    // Mirrors the guard sequence in Next(): (1) reject inconsistent state where
    // mContainerEnd < mElemStart, (2) reject mHeadLen + ValueLen wrap on 32-bit
    // (size_t == uint32_t), (3) bounds-check via size_t-widened comparison.
    VerifyOrReturnError(mContainerEnd >= mElemStart, ASN1_ERROR_INVALID_STATE);
    VerifyOrReturnError(mHeadLen <= UINT32_MAX - ValueLen, ASN1_ERROR_LENGTH_OVERFLOW);
    VerifyOrReturnError(static_cast<size_t>(mHeadLen) + ValueLen <= static_cast<size_t>(mContainerEnd - mElemStart),
                        ASN1_ERROR_UNDERRUN);

    if (ValueLen == 1)
    {
        outVal = 0;
    }
    else
    {
        outVal             = ReverseBits(Value[1]);
        unsigned int shift = 8;
        for (uint32_t i = 2; i < ValueLen; i++, shift += 8)
        {
            // Cast to uint32_t before shifting: ReverseBits returns uint8_t which
            // would be promoted to (signed) int, and shifts of 24+ on a value with
            // the high bit set are undefined behavior on signed integers.
            outVal |= (static_cast<uint32_t>(ReverseBits(Value[i])) << shift);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Reader::DecodeHead()
{
    const uint8_t * p = mElemStart;
    VerifyOrReturnError(p < mBufEnd, ASN1_ERROR_UNDERRUN);

    Class       = *p & 0xC0;
    Constructed = (*p & 0x20) != 0;
    Tag         = *p & 0x1F;

    // Only tags < 31 supported. The implication of this is that encoded tags are exactly 1 byte long.
    VerifyOrReturnError(Tag < 0x1F, ASN1_ERROR_UNSUPPORTED_ENCODING);

    p++;
    VerifyOrReturnError(p < mBufEnd, ASN1_ERROR_UNDERRUN);

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
            VerifyOrReturnError(p < mBufEnd, ASN1_ERROR_UNDERRUN);
            VerifyOrReturnError((ValueLen & 0xFF000000) == 0, ASN1_ERROR_LENGTH_OVERFLOW);
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
