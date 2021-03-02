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

namespace chip {
namespace ASN1 {

using namespace chip::Encoding;

enum
{
    kLengthFieldReserveSize = 5,
    kMaxElementLength       = INT32_MAX,
    kUnkownLength           = -1,
    kUnknownLengthMarker    = 0xFF
};

void ASN1Writer::Init(uint8_t * buf, uint32_t maxLen)
{
    mBuf                = buf;
    mWritePoint         = buf;
    mBufEnd             = buf + maxLen;
    mBufEnd             = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(mBufEnd) & ~3); // align on 32bit boundary
    mDeferredLengthList = reinterpret_cast<uint8_t **>(mBufEnd);
}

void ASN1Writer::InitNullWriter(void)
{
    mBuf                = nullptr;
    mWritePoint         = nullptr;
    mBufEnd             = nullptr;
    mDeferredLengthList = nullptr;
}

ASN1_ERROR ASN1Writer::Finalize()
{
    if (mBuf != nullptr)
    {
        uint8_t * compactPoint = mBuf;
        uint8_t * spanStart    = mBuf;

        for (uint8_t ** listEntry = reinterpret_cast<uint8_t **>(mBufEnd); listEntry > mDeferredLengthList;)
        {
            uint8_t * lenField        = *--listEntry;
            uint8_t lenFieldFirstByte = *lenField;

            if (lenFieldFirstByte == kUnknownLengthMarker)
                return ASN1_ERROR_INVALID_STATE;

            uint8_t lenOfLen = (lenFieldFirstByte < 128) ? 1 : (lenFieldFirstByte & 0x7f) + 1;

            uint8_t * spanEnd = lenField + lenOfLen;

            if (spanStart == compactPoint)
                compactPoint = spanEnd;
            else
            {
                uint32_t spanLen = spanEnd - spanStart;
                memmove(compactPoint, spanStart, spanLen);
                compactPoint += spanLen;
            }

            spanStart = lenField + kLengthFieldReserveSize;
        }

        if (spanStart > compactPoint)
        {
            uint32_t spanLen = mWritePoint - spanStart;
            memmove(compactPoint, spanStart, spanLen);
            compactPoint += spanLen;
        }

        mWritePoint = compactPoint;
    }

    return ASN1_NO_ERROR;
}

uint16_t ASN1Writer::GetLengthWritten() const
{
    return (mBuf != nullptr) ? mWritePoint - mBuf : 0;
}

ASN1_ERROR ASN1Writer::PutInteger(int64_t val)
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

ASN1_ERROR ASN1Writer::PutBoolean(bool val)
{
    ASN1_ERROR err;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    err = EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_Boolean, false, 1);
    SuccessOrExit(err);

    *mWritePoint++ = (val) ? 0xFF : 0;

exit:
    return err;
}

ASN1_ERROR ASN1Writer::PutObjectId(const uint8_t * val, uint16_t valLen)
{
    return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_ObjectId, false, val, valLen);
}

ASN1_ERROR ASN1Writer::PutString(uint32_t tag, const char * val, uint16_t valLen)
{
    return PutValue(kASN1TagClass_Universal, tag, false, (const uint8_t *) val, valLen);
}

ASN1_ERROR ASN1Writer::PutOctetString(const uint8_t * val, uint16_t valLen)
{
    return PutValue(kASN1TagClass_Universal, kASN1UniversalTag_OctetString, false, val, valLen);
}

ASN1_ERROR ASN1Writer::PutOctetString(uint8_t cls, uint32_t tag, const uint8_t * val, uint16_t valLen)
{
    return PutValue(cls, tag, false, val, valLen);
}

ASN1_ERROR ASN1Writer::PutOctetString(uint8_t cls, uint32_t tag, chip::TLV::TLVReader & val)
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

ASN1_ERROR ASN1Writer::PutBitString(uint32_t val)
{
    ASN1_ERROR err;
    uint8_t len;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

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

    err = EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_BitString, false, len);
    SuccessOrExit(err);

    if (val == 0)
        mWritePoint[0] = 0;
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

exit:
    return err;
}

ASN1_ERROR ASN1Writer::PutBitString(uint8_t unusedBitCount, const uint8_t * encodedBits, uint16_t encodedBitsLen)
{
    ASN1_ERROR err;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    err = EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_BitString, false, encodedBitsLen + 1);
    SuccessOrExit(err);

    *mWritePoint++ = unusedBitCount;

    memcpy(mWritePoint, encodedBits, encodedBitsLen);
    mWritePoint += encodedBitsLen;

exit:
    return err;
}

ASN1_ERROR ASN1Writer::PutBitString(uint8_t unusedBitCount, chip::TLV::TLVReader & encodedBits)
{
    ASN1_ERROR err;
    uint32_t encodedBitsLen;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    encodedBitsLen = encodedBits.GetLength();

    err = EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_BitString, false, encodedBitsLen + 1);
    SuccessOrExit(err);

    *mWritePoint++ = unusedBitCount;

    encodedBits.GetBytes(mWritePoint, encodedBitsLen);
    mWritePoint += encodedBitsLen;

exit:
    return err;
}

static void itoa2(uint32_t val, uint8_t * buf)
{
    buf[1] = '0' + (val % 10);
    val /= 10;
    buf[0] = '0' + (val % 10);
}

ASN1_ERROR ASN1Writer::PutTime(const ASN1UniversalTime & val)
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

ASN1_ERROR ASN1Writer::PutNull()
{
    return EncodeHead(kASN1TagClass_Universal, kASN1UniversalTag_Null, false, 0);
}

ASN1_ERROR ASN1Writer::StartConstructedType(uint8_t cls, uint32_t tag)
{
    return EncodeHead(cls, tag, true, kUnkownLength);
}

ASN1_ERROR ASN1Writer::EndConstructedType()
{
    return WriteDeferredLength();
}

ASN1_ERROR ASN1Writer::StartEncapsulatedType(uint8_t cls, uint32_t tag, bool bitStringEncoding)
{
    ASN1_ERROR err;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    err = EncodeHead(cls, tag, false, kUnkownLength);
    SuccessOrExit(err);

    // If the encapsulating type is BIT STRING, encode the unused bit count field.  Since the BIT
    // STRING contains an ASN.1 DER encoding, and ASN.1 DER encodings are always multiples of 8 bits,
    // the unused bit count is always 0.
    if (bitStringEncoding)
    {
        if (mWritePoint == reinterpret_cast<uint8_t *>(mDeferredLengthList))
            return ASN1_ERROR_OVERFLOW;
        *mWritePoint++ = 0;
    }

exit:
    return err;
}

ASN1_ERROR ASN1Writer::EndEncapsulatedType()
{
    return WriteDeferredLength();
}

ASN1_ERROR ASN1Writer::PutValue(uint8_t cls, uint32_t tag, bool isConstructed, const uint8_t * val, uint16_t valLen)
{
    ASN1_ERROR err;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    err = EncodeHead(cls, tag, isConstructed, valLen);
    SuccessOrExit(err);

    memcpy(mWritePoint, val, valLen);
    mWritePoint += valLen;

exit:
    return err;
}

ASN1_ERROR ASN1Writer::PutValue(uint8_t cls, uint32_t tag, bool isConstructed, chip::TLV::TLVReader & val)
{
    ASN1_ERROR err;
    uint32_t valLen;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    valLen = val.GetLength();

    err = EncodeHead(cls, tag, isConstructed, valLen);
    SuccessOrExit(err);

    val.GetBytes(mWritePoint, valLen);
    mWritePoint += valLen;

exit:
    return err;
}

ASN1_ERROR ASN1Writer::EncodeHead(uint8_t cls, uint32_t tag, bool isConstructed, int32_t len)
{
    ASN1_ERROR err = ASN1_NO_ERROR;
    uint8_t bytesForLen;
    uint32_t totalLen;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    // Only tags <= 31 supported. The implication of this is that encoded tags are exactly 1 byte long.
    VerifyOrExit(tag <= 0x1F, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Only positive and kUnkownLength values are supported for len input.
    VerifyOrExit(len >= 0 || len == kUnkownLength, err = ASN1_ERROR_UNSUPPORTED_ENCODING);

    // Compute the number of bytes required to encode the length.
    bytesForLen = BytesForLength(len);

    // If the element length is unknown, allocate a new entry in the deferred-length list.
    //
    // The deferred-length list is a list of "pointers" (represented as offsets into mBuf)
    // to length fields for which the length of the element was unknown at the time the element
    // head was written. Examples include constructed types such as SEQUENCE and SET, as well
    // non-constructed types that encapsulate other ASN.1 types (e.g. OCTET STRINGS that contain
    // BER/DER encodings). The final lengths are filled in later, at the time the encoding is
    // complete (e.g. when EndConstructed() is called).
    //
    if (len == kUnkownLength)
        mDeferredLengthList--;

    // Make sure there's enough space to encode the entire value without bumping into the deferred length
    // list at the end of the buffer.
    totalLen = 1 + bytesForLen + (len != kUnkownLength ? len : 0);
    VerifyOrExit((mWritePoint + totalLen) <= reinterpret_cast<uint8_t *>(mDeferredLengthList), err = ASN1_ERROR_OVERFLOW);

    // Write the tag byte.
    *mWritePoint++ = cls | (isConstructed ? 0x20 : 0) | tag;

    // Encode the length if it is known.
    if (len != kUnkownLength)
        EncodeLength(mWritePoint, bytesForLen, len);

    // ... otherwise place a marker in the first byte of the length to indicate that the length is unknown
    // and save a pointer to the length field in the deferred-length list.
    else
    {
        *mWritePoint         = kUnknownLengthMarker;
        *mDeferredLengthList = mWritePoint;
    }

    mWritePoint += bytesForLen;

exit:
    return err;
}

ASN1_ERROR ASN1Writer::WriteDeferredLength()
{
    ASN1_ERROR err = ASN1_NO_ERROR;
    uint8_t ** listEntry;
    uint32_t lenAdj;

    // Do nothing for a null writer.
    VerifyOrExit(mBuf != nullptr, err = ASN1_NO_ERROR);

    lenAdj = kLengthFieldReserveSize;

    // Scan the deferred-length list in reverse order looking for the most recent entry where
    // the length is still unknown. This entry represents the "container" element whose encoding
    // is now complete.
    for (listEntry = mDeferredLengthList; listEntry < reinterpret_cast<uint8_t **>(mBufEnd); listEntry++)
    {
        // Get a pointer to the deferred-length field.
        uint8_t * lenField = *listEntry;

        // Get the first byte of the length field.
        uint8_t lenFieldFirstByte = *lenField;

        // If the length is marked as unknown...
        if (lenFieldFirstByte == kUnknownLengthMarker)
        {
            // Compute the final length of the element's value (3 = bytes reserved for length).
            uint32_t elemLen = (mWritePoint - lenField) - lenAdj;

            // Return an error if the length exceeds the maximum value that can be encoded in the
            // space reserved for the length.
            VerifyOrExit(elemLen <= kMaxElementLength, err = ASN1_ERROR_LENGTH_OVERFLOW);

            // Encode the final length of the element, overwriting the unknown length marker
            // in the process.  Note that the number of bytes consumed by the final length field
            // may be smaller than the space that was reserved for the field.  This will be fixed
            // up when the Finalize() method is called.
            uint8_t bytesForLen = BytesForLength(static_cast<int32_t>(elemLen));
            EncodeLength(lenField, bytesForLen, elemLen);

            ExitNow(err = ASN1_NO_ERROR);
        }
        else
        {
            uint8_t bytesForLen = (lenFieldFirstByte < 128) ? 1 : (lenFieldFirstByte & 0x7f) + 1;
            lenAdj += (kLengthFieldReserveSize - bytesForLen);
        }
    }

    err = ASN1_ERROR_INVALID_STATE;

exit:
    return err;
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
    if (len == kUnkownLength)
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
        buf[0] = static_cast<uint8_t>(lenToEncode);
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
