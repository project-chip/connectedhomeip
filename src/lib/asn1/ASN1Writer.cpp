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
 *      This file implements an object for writing Abstract Syntax
 *      Notation One (ASN.1) encoded data.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asn1/ASN1.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>

namespace chip {
namespace ASN1 {

using namespace chip::Encoding;

enum
{
    kLengthFieldReserveSize = 1,
    kUnknownLength          = -1,
    kUnknownLengthMarker    = 0xFF
};

void ASN1Writer::Init(uint8_t * buf, uint32_t maxLen)
{
    mBuf                 = buf;
    mWritePoint          = buf;
    mBufEnd              = buf + maxLen;
    mDeferredLengthCount = 0;
}

void ASN1Writer::InitNullWriter(void)
{
    mBuf                 = nullptr;
    mWritePoint          = nullptr;
    mBufEnd              = nullptr;
    mDeferredLengthCount = 0;
}

uint16_t ASN1Writer::GetLengthWritten() const
{
    return (mBuf != nullptr) ? mWritePoint - mBuf : 0;
}

CHIP_ERROR ASN1Writer::PutInteger(int64_t val)
{
    uint8_t encodedVal[8];
    uint8_t valStart, valLen;

    BigEndian::Put64(encodedVal, static_cast<uint64_t>(val));

    for (valStart = 0; valStart < 7; valStart++)
    {
        if (encodedVal[valStart] == 0x00 && (encodedVal[valStart + 1] & 0x80) == 0)
            continue;
        if (encodedVal[valStart] == 0xFF && (encodedVal[valStart + 1] & 0x80) == 0x80)
            continue;
        break;
    }
    valLen = 8 - valStart;

    return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false, encodedVal + valStart, valLen);
}

CHIP_ERROR ASN1Writer::PutBoolean(bool val)
{
    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    ReturnErrorOnFailure(EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_Boolean, false, 1));

    *mWritePoint++ = (val) ? 0xFF : 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::PutObjectId(const uint8_t * val, uint16_t valLen)
{
    return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId, false, val, valLen);
}

CHIP_ERROR ASN1Writer::PutString(uint32_t tag, const char * val, uint16_t valLen)
{
    return PutValue(kASN1TagClass_Universal, tag, false, (const uint8_t *) val, valLen);
}

CHIP_ERROR ASN1Writer::PutOctetString(const uint8_t * val, uint16_t valLen)
{
    return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_OctetString, false, val, valLen);
}

CHIP_ERROR ASN1Writer::PutOctetString(uint8_t cls, uint32_t tag, const uint8_t * val, uint16_t valLen)
{
    return PutValue(cls, tag, false, val, valLen);
}

CHIP_ERROR ASN1Writer::PutOctetString(uint8_t cls, uint32_t tag, chip::TLV::TLVReader & val)
{
    return PutValue(cls, tag, false, val);
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

static uint8_t HighestBit(uint32_t v)
{
    uint32_t highestBit = 0;

    if (v > 0xFFFF)
    {
        highestBit = 16;
        v >>= 16;
    }
    if (v > 0xFF)
    {
        highestBit |= 8;
        v >>= 8;
    }
    if (v > 0xF)
    {
        highestBit |= 4;
        v >>= 4;
    }
    if (v > 0x3)
    {
        highestBit |= 2;
        v >>= 2;
    }
    highestBit |= (v >> 1);

    return highestBit;
}

CHIP_ERROR ASN1Writer::PutBitString(uint32_t val)
{
    uint8_t len;

    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    if (val == 0)
        len = 1;
    else if (val < 256)
        len = 2;
    else if (val < 65536)
        len = 3;
    else if (val < (1 << 24))
        len = 4;
    else
        len = 5;

    ReturnErrorOnFailure(EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_BitString, false, len));

    if (val == 0)
    {
        mWritePoint[0] = 0;
    }
    else
    {
        mWritePoint[1] = ReverseBits(static_cast<uint8_t>(val));
        if (len >= 3)
        {
            val >>= 8;
            mWritePoint[2] = ReverseBits(static_cast<uint8_t>(val));
            if (len >= 4)
            {
                val >>= 8;
                mWritePoint[3] = ReverseBits(static_cast<uint8_t>(val));
                if (len == 5)
                {
                    val >>= 8;
                    mWritePoint[4] = ReverseBits(static_cast<uint8_t>(val));
                }
            }
        }
        mWritePoint[0] = 7 - HighestBit(val);
    }

    mWritePoint += len;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::PutBitString(uint8_t unusedBitCount, const uint8_t * encodedBits, uint16_t encodedBitsLen)
{
    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    ReturnErrorOnFailure(EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_BitString, false, encodedBitsLen + 1));

    *mWritePoint++ = unusedBitCount;

    memcpy(mWritePoint, encodedBits, encodedBitsLen);
    mWritePoint += encodedBitsLen;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::PutBitString(uint8_t unusedBitCount, chip::TLV::TLVReader & encodedBits)
{
    uint32_t encodedBitsLen;

    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    encodedBitsLen = encodedBits.GetLength();

    ReturnErrorOnFailure(EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_BitString, false, encodedBitsLen + 1));

    *mWritePoint++ = unusedBitCount;

    encodedBits.GetBytes(mWritePoint, encodedBitsLen);
    mWritePoint += encodedBitsLen;

    return CHIP_NO_ERROR;
}

static void itoa2(uint32_t val, uint8_t * buf)
{
    buf[1] = '0' + (val % 10);
    val /= 10;
    buf[0] = '0' + (val % 10);
}

CHIP_ERROR ASN1Writer::PutTime(const ASN1UniversalTime & val)
{
    uint8_t buf[15];

    itoa2(val.Year / 100, buf);
    itoa2(val.Year, buf + 2);
    itoa2(val.Month, buf + 4);
    itoa2(val.Day, buf + 6);
    itoa2(val.Hour, buf + 8);
    itoa2(val.Minute, buf + 10);
    itoa2(val.Second, buf + 12);
    buf[14] = 'Z';

    // X.509/RFC5280 mandates that times before 2050 UTC must be encoded as ASN.1 UTCTime values, while
    // times equal or greater than 2050 must be encoded as GeneralizedTime values.  The only difference
    // (in the context of X.509 DER) is that GeneralizedTimes are encoded with a 4 digit year, while
    // UTCTimes are encoded with a two-digit year.
    //
    if (val.Year >= 2050)
        return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_GeneralizedTime, false, buf, 15);
    else
        return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_UTCTime, false, buf + 2, 13);
}

CHIP_ERROR ASN1Writer::PutNull()
{
    return EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_Null, false, 0);
}

CHIP_ERROR ASN1Writer::PutConstructedType(const uint8_t * val, uint16_t valLen)
{
    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    // Make sure we have enough space to write
    VerifyOrReturnError((mWritePoint + valLen) <= mBufEnd, ASN1_ERROR_OVERFLOW);

    memcpy(mWritePoint, val, valLen);
    mWritePoint += valLen;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::StartConstructedType(uint8_t cls, uint32_t tag)
{
    return EncodeHead(cls, tag, true, kUnknownLength);
}

CHIP_ERROR ASN1Writer::EndConstructedType()
{
    return WriteDeferredLength();
}

CHIP_ERROR ASN1Writer::StartEncapsulatedType(uint8_t cls, uint32_t tag, bool bitStringEncoding)
{
    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    ReturnErrorOnFailure(EncodeHead(cls, tag, false, kUnknownLength));

    // If the encapsulating type is BIT STRING, encode the unused bit count field.  Since the BIT
    // STRING contains an ASN.1 DER encoding, and ASN.1 DER encodings are always multiples of 8 bits,
    // the unused bit count is always 0.
    if (bitStringEncoding)
    {
        VerifyOrReturnError(mWritePoint < mBufEnd, ASN1_ERROR_OVERFLOW);
        *mWritePoint++ = 0;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::EndEncapsulatedType()
{
    return WriteDeferredLength();
}

CHIP_ERROR ASN1Writer::PutValue(uint8_t cls, uint32_t tag, bool isConstructed, const uint8_t * val, uint16_t valLen)
{
    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    ReturnErrorOnFailure(EncodeHead(cls, tag, isConstructed, valLen));

    memcpy(mWritePoint, val, valLen);
    mWritePoint += valLen;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::PutValue(uint8_t cls, uint32_t tag, bool isConstructed, chip::TLV::TLVReader & val)
{
    uint32_t valLen;

    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    valLen = val.GetLength();

    ReturnErrorOnFailure(EncodeHead(cls, tag, isConstructed, valLen));

    val.GetBytes(mWritePoint, valLen);
    mWritePoint += valLen;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::EncodeHead(uint8_t cls, uint32_t tag, bool isConstructed, int32_t len)
{
    uint8_t bytesForLen;
    uint32_t totalLen;

    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    // Only tags <= 31 supported. The implication of this is that encoded tags are exactly 1 byte long.
    VerifyOrReturnError(tag <= 0x1F, ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Only positive and kUnknownLength values are supported for len input.
    VerifyOrReturnError(len >= 0 || len == kUnknownLength, ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Compute the number of bytes required to encode the length.
    bytesForLen = BytesForLength(len);

    // Make sure there's enough space to encode the entire value.
    totalLen = 1 + bytesForLen + (len != kUnknownLength ? len : 0);
    VerifyOrReturnError((mWritePoint + totalLen) <= mBufEnd, ASN1_ERROR_OVERFLOW);

    // Write the tag byte.
    *mWritePoint++ = cls | (isConstructed ? 0x20 : 0) | tag;

    // Encode the length if it is known.
    if (len != kUnknownLength)
    {
        EncodeLength(mWritePoint, bytesForLen, len);
    }
    // ... otherwise place a marker in the first byte of the length to indicate that the length is unknown
    // and save a pointer to the length field in the deferred-length array.
    //
    // The deferred-length is an array of "pointers" to length fields for which the length of the
    // element was unknown at the time the element head was written. Examples include constructed
    // types such as SEQUENCE and SET, as well non-constructed types that encapsulate other ASN.1 types
    // (e.g. OCTET STRINGS that contain BER/DER encodings). The final lengths are filled in later,
    // at the time the encoding of the element is complete (e.g. when EndConstructed() is called).
    else
    {
        VerifyOrReturnError(mDeferredLengthCount < kMaxDeferredLengthDepth, ASN1_ERROR_INVALID_STATE);

        *mWritePoint                                     = kUnknownLengthMarker;
        mDeferredLengthLocations[mDeferredLengthCount++] = mWritePoint;
    }

    mWritePoint += bytesForLen;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASN1Writer::WriteDeferredLength()
{
    // Do nothing for a null writer.
    VerifyOrReturnError(mBuf != nullptr, CHIP_NO_ERROR);

    VerifyOrReturnError(mDeferredLengthCount > 0, ASN1_ERROR_INVALID_STATE);

    uint8_t * lenField = mDeferredLengthLocations[mDeferredLengthCount - 1];

    VerifyOrReturnError(*lenField == kUnknownLengthMarker, ASN1_ERROR_INVALID_STATE);

    // Compute the length of the element's value.
    uint32_t elemLen = (mWritePoint - lenField) - kLengthFieldReserveSize;

    VerifyOrReturnError(CanCastTo<int32_t>(elemLen), ASN1_ERROR_LENGTH_OVERFLOW);

    uint8_t bytesForLen = BytesForLength(static_cast<int32_t>(elemLen));

    // Move the element data if the number of bytes consumed by the final length field
    // is different than the space that was reserved for the field.
    if (bytesForLen != kLengthFieldReserveSize)
    {
        mWritePoint += (bytesForLen - kLengthFieldReserveSize);

        VerifyOrReturnError(mWritePoint <= mBufEnd, ASN1_ERROR_OVERFLOW);

        memmove(lenField + bytesForLen, lenField + kLengthFieldReserveSize, elemLen);
    }

    // Encode the final length of the element, overwriting the unknown length marker
    // in the process.
    EncodeLength(lenField, bytesForLen, elemLen);

    mDeferredLengthCount--;

    return CHIP_NO_ERROR;
}

/**
 * Returns the number of bytes required to encode the length value.
 *
 * @param[in]   len     Parameter, which encoding length to be calculated.
 *
 * @return number of bytes required to encode the length value.
 */
uint8_t ASN1Writer::BytesForLength(int32_t len)
{
    if (len == kUnknownLength)
        return kLengthFieldReserveSize;
    if (len < 128)
        return 1;
    if (len < 256)
        return 2;
    if (len < 65536)
        return 3;
    if (len < (1 << 24))
        return 4;
    return 5;
}

void ASN1Writer::EncodeLength(uint8_t * buf, uint8_t bytesForLen, int32_t lenToEncode)
{
    if (bytesForLen == 1)
    {
        buf[0] = static_cast<uint8_t>(lenToEncode);
    }
    else
    {
        --bytesForLen;
        buf[0] = 0x80 | bytesForLen;
        do
        {
            buf[bytesForLen] = static_cast<uint8_t>(lenToEncode);
            lenToEncode >>= 8;
        } while (--bytesForLen);
    }
}

} // namespace ASN1
} // namespace chip
