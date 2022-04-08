/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements an encoder for the CHIP TLV (Tag-Length-Value) encoding format.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <lib/core/CHIPTLV.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

// Doxygen is confused by the __attribute__ annotation
#ifndef DOXYGEN
#define NO_INLINE __attribute__((noinline))
#endif // DOXYGEN

namespace chip {
namespace TLV {

using namespace chip::Encoding;

NO_INLINE void TLVWriter::Init(uint8_t * buf, size_t maxLen)
{
    // TODO: Maybe we can just make mMaxLen, mLenWritten, mRemainingLen size_t instead?
    uint32_t actualMaxLen = maxLen > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(maxLen);
    mBackingStore         = nullptr;
    mBufStart = mWritePoint = buf;
    mRemainingLen           = actualMaxLen;
    mLenWritten             = 0;
    mMaxLen                 = actualMaxLen;
    mContainerType          = kTLVType_NotSpecified;
    mReservedSize           = 0;
    SetContainerOpen(false);
    SetCloseContainerReserved(true);

    ImplicitProfileId = kProfileIdNotSpecified;
}

CHIP_ERROR TLVWriter::Init(TLVBackingStore & backingStore, uint32_t maxLen)
{
    mBackingStore  = &backingStore;
    mBufStart      = nullptr;
    mRemainingLen  = 0;
    CHIP_ERROR err = mBackingStore->OnInit(*this, mBufStart, mRemainingLen);
    if (err != CHIP_NO_ERROR)
        return err;

    mWritePoint    = mBufStart;
    mLenWritten    = 0;
    mMaxLen        = maxLen;
    mContainerType = kTLVType_NotSpecified;
    mReservedSize  = 0;
    SetContainerOpen(false);
    SetCloseContainerReserved(true);

    ImplicitProfileId = kProfileIdNotSpecified;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::Finalize()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;
    if (mBackingStore != nullptr)
        err = mBackingStore->FinalizeBuffer(*this, mBufStart, static_cast<uint32_t>(mWritePoint - mBufStart));
    return err;
}

CHIP_ERROR TLVWriter::PutBoolean(Tag tag, bool v)
{
    return WriteElementHead((v) ? TLVElementType::BooleanTrue : TLVElementType::BooleanFalse, tag, 0);
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v)
{
    return Put(tag, static_cast<uint64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint8_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt8, tag, v);
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint16_t v)
{
    return Put(tag, static_cast<uint64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint16_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt16, tag, v);
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint32_t v)
{
    return Put(tag, static_cast<uint64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint32_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt32, tag, v);
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint64_t v)
{
    TLVElementType elemType;
    if (v <= UINT8_MAX)
        elemType = TLVElementType::UInt8;
    else if (v <= UINT16_MAX)
        elemType = TLVElementType::UInt16;
    else if (v <= UINT32_MAX)
        elemType = TLVElementType::UInt32;
    else
        elemType = TLVElementType::UInt64;
    return WriteElementHead(elemType, tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, uint64_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::UInt64, tag, v);
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v)
{
    return Put(tag, static_cast<int64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, int8_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int8, tag, static_cast<uint8_t>(v));
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, int16_t v)
{
    return Put(tag, static_cast<int64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, int16_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int16, tag, static_cast<uint16_t>(v));
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, int32_t v)
{
    return Put(tag, static_cast<int64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, int32_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int32, tag, static_cast<uint32_t>(v));
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, int64_t v)
{
    TLVElementType elemType;
    if (v >= INT8_MIN && v <= INT8_MAX)
        elemType = TLVElementType::Int8;
    else if (v >= INT16_MIN && v <= INT16_MAX)
        elemType = TLVElementType::Int16;
    else if (v >= INT32_MIN && v <= INT32_MAX)
        elemType = TLVElementType::Int32;
    else
        elemType = TLVElementType::Int64;
    return WriteElementHead(elemType, tag, static_cast<uint64_t>(v));
}

CHIP_ERROR TLVWriter::Put(Tag tag, int64_t v, bool preserveSize)
{
    if (preserveSize)
        return WriteElementHead(TLVElementType::Int64, tag, static_cast<uint64_t>(v));
    return Put(tag, v);
}

CHIP_ERROR TLVWriter::Put(Tag tag, const float v)
{
    uint32_t u32;
    memcpy(&u32, &v, sizeof(u32));
    return WriteElementHead(TLVElementType::FloatingPointNumber32, tag, u32);
}

CHIP_ERROR TLVWriter::Put(Tag tag, const double v)
{
    uint64_t u64;
    memcpy(&u64, &v, sizeof(u64));
    return WriteElementHead(TLVElementType::FloatingPointNumber64, tag, u64);
}

CHIP_ERROR TLVWriter::Put(Tag tag, ByteSpan data)
{
    VerifyOrReturnError(CanCastTo<uint32_t>(data.size()), CHIP_ERROR_MESSAGE_TOO_LONG);
    return PutBytes(tag, data.data(), static_cast<uint32_t>(data.size()));
}

CHIP_ERROR TLVWriter::PutBytes(Tag tag, const uint8_t * buf, uint32_t len)
{
    return WriteElementWithData(kTLVType_ByteString, tag, buf, len);
}

CHIP_ERROR TLVWriter::PutString(Tag tag, const char * buf)
{
    size_t len = strlen(buf);
    if (!CanCastTo<uint32_t>(len))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return PutString(tag, buf, static_cast<uint32_t>(len));
}

CHIP_ERROR TLVWriter::PutString(Tag tag, const char * buf, uint32_t len)
{
    return WriteElementWithData(kTLVType_UTF8String, tag, reinterpret_cast<const uint8_t *>(buf), len);
}

CHIP_ERROR TLVWriter::PutString(Tag tag, Span<const char> str)
{
    if (!CanCastTo<uint32_t>(str.size()))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return PutString(tag, str.data(), static_cast<uint32_t>(str.size()));
}

CHIP_ERROR TLVWriter::PutStringF(Tag tag, const char * fmt, ...)
{
    CHIP_ERROR err;
    va_list ap;

    va_start(ap, fmt);

    err = VPutStringF(tag, fmt, ap);

    va_end(ap);

    return err;
}

#if CONFIG_HAVE_VCBPRINTF
// We have a variant of the printf function that takes a callback that
// emits a single character.  The callback performs a function
// identical to putchar.

void TLVWriter::CHIPTLVWriterPutcharCB(uint8_t c, void * appState)
{
    TLVWriter * w = static_cast<TLVWriter *>(appState);
    w->WriteData(&c, sizeof(c));
}
#endif

CHIP_ERROR TLVWriter::VPutStringF(Tag tag, const char * fmt, va_list ap)
{
    va_list aq;
    size_t dataLen;
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVFieldSize lenFieldSize;
#if CONFIG_HAVE_VSNPRINTF_EX
    size_t skipLen;
    size_t writtenBytes;
#elif CONFIG_HAVE_VCBPRINTF
#elif CONFIG_TLV_TRUNCATE
    size_t maxLen;
#else
    char * tmpBuf;
#endif
    va_copy(aq, ap);

    dataLen = static_cast<size_t>(vsnprintf(nullptr, 0, fmt, aq));

    va_end(aq);

    if (!CanCastTo<uint32_t>(dataLen))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (dataLen <= UINT8_MAX)
        lenFieldSize = kTLVFieldSize_1Byte;
    else if (dataLen <= UINT16_MAX)
        lenFieldSize = kTLVFieldSize_2Byte;
    else
        lenFieldSize = kTLVFieldSize_4Byte;

#if !(CONFIG_HAVE_VCBPRINTF) && !(CONFIG_HAVE_VSNPRINTF_EX) && CONFIG_TLV_TRUNCATE
    // no facilities for splitting the stream across multiple buffers,
    // just write however much fits in the current buffer.
    // assume conservative tag length at this time (8 bytes)
    maxLen = mRemainingLen -
        (1 + 8 + (1 << static_cast<uint8_t>(lenFieldSize)) +
         1); // 1 : control byte, 8 : tag length, stringLen + 1 for null termination
    if (maxLen < dataLen)
        dataLen = maxLen;
#endif

    // write length.
    err = WriteElementHead(
        static_cast<TLVElementType>(static_cast<uint8_t>(kTLVType_UTF8String) | static_cast<uint8_t>(lenFieldSize)), tag, dataLen);
    SuccessOrExit(err);

    VerifyOrExit((mLenWritten + dataLen) <= mMaxLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // write data
#if CONFIG_HAVE_VSNPRINTF_EX

    skipLen = 0;

    do
    {
        va_copy(aq, ap);

        vsnprintf_ex(reinterpret_cast<char *>(mWritePoint), mRemainingLen, skipLen, fmt, aq);

        va_end(aq);

        writtenBytes = (mRemainingLen >= (dataLen - skipLen)) ? dataLen - skipLen : mRemainingLen;
        skipLen += writtenBytes;
        mWritePoint += writtenBytes;
        mRemainingLen -= writtenBytes;
        mLenWritten += writtenBytes;
        if (skipLen < dataLen)
        {
            VerifyOrExit(mBackingStore != NULL, err = CHIP_ERROR_NO_MEMORY);

            err = mBackingStore->FinalizeBuffer(*this, mBufHandle, mBufStart, mWritePoint - mBufStart);
            SuccessOrExit(err);

            err = mBackingStore->GetNewBuffer(*this, mBufHandle, mBufStart, mRemainingLen);
            SuccessOrExit(err);

            mWritePoint = mBufStart;
        }

    } while (skipLen < dataLen);

#elif CONFIG_HAVE_VCBPRINTF

    va_copy(aq, ap);

    vcbprintf(CHIPTLVWriterPutcharCB, this, dataLen, fmt, aq);

    va_end(aq);

#else // CONFIG_HAVE_VSNPRINTF_EX

    tmpBuf = static_cast<char *>(chip::Platform::MemoryAlloc(dataLen + 1));
    VerifyOrExit(tmpBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    va_copy(aq, ap);

    vsnprintf(tmpBuf, dataLen + 1, fmt, aq);

    va_end(aq);

    err = WriteData(reinterpret_cast<uint8_t *>(tmpBuf), static_cast<uint32_t>(dataLen));
    chip::Platform::MemoryFree(tmpBuf);

#endif // CONFIG_HAVE_VSNPRINTF_EX

exit:

    return err;
}

CHIP_ERROR TLVWriter::PutNull(Tag tag)
{
    return WriteElementHead(TLVElementType::Null, tag, 0);
}

CHIP_ERROR TLVWriter::CopyElement(TLVReader & reader)
{
    return CopyElement(reader.GetTag(), reader);
}

const size_t kCHIPTLVCopyChunkSize = 16;

CHIP_ERROR TLVWriter::CopyElement(Tag tag, TLVReader & reader)
{
    TLVElementType elemType = reader.ElementType();
    uint64_t elemLenOrVal   = reader.mElemLenOrVal;
    TLVReader readerHelper; // used to figure out the length of the element and read data of the element
    uint32_t copyDataLen;
    uint8_t chunk[kCHIPTLVCopyChunkSize];

    VerifyOrReturnError(elemType != TLVElementType::NotSpecified && elemType != TLVElementType::EndOfContainer,
                        CHIP_ERROR_INCORRECT_STATE);

    // Initialize the helper
    readerHelper.Init(reader);

    // Skip to the end of the element.
    ReturnErrorOnFailure(reader.Skip());

    // Compute the amount of value data to copy from the reader.
    copyDataLen = reader.GetLengthRead() - readerHelper.GetLengthRead();

    // Write the head of the new element with the same type and length/value, but using the
    // specified tag.
    ReturnErrorOnFailure(WriteElementHead(elemType, tag, elemLenOrVal));

    while (copyDataLen > 0)
    {
        uint32_t chunkSize = copyDataLen > kCHIPTLVCopyChunkSize ? kCHIPTLVCopyChunkSize : copyDataLen;
        ReturnErrorOnFailure(readerHelper.ReadData(chunk, chunkSize));

        ReturnErrorOnFailure(WriteData(chunk, chunkSize));

        copyDataLen -= chunkSize;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::OpenContainer(Tag tag, TLVType containerType, TLVWriter & containerWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(TLVTypeIsContainer(containerType), CHIP_ERROR_WRONG_TLV_TYPE);

    if (IsCloseContainerReserved())
    {
        VerifyOrReturnError(mMaxLen >= kEndOfContainerMarkerSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        mMaxLen -= kEndOfContainerMarkerSize;
    }
    err = WriteElementHead(static_cast<TLVElementType>(containerType), tag, 0);

    if (err != CHIP_NO_ERROR)
    {
        // undo the space reservation, as the container is not actually open
        if (IsCloseContainerReserved())
            mMaxLen += kEndOfContainerMarkerSize;

        return err;
    }

    containerWriter.mBackingStore  = mBackingStore;
    containerWriter.mBufStart      = mBufStart;
    containerWriter.mWritePoint    = mWritePoint;
    containerWriter.mRemainingLen  = mRemainingLen;
    containerWriter.mLenWritten    = 0;
    containerWriter.mMaxLen        = mMaxLen - mLenWritten;
    containerWriter.mContainerType = containerType;
    containerWriter.SetContainerOpen(false);
    containerWriter.SetCloseContainerReserved(IsCloseContainerReserved());
    containerWriter.ImplicitProfileId = ImplicitProfileId;

    SetContainerOpen(true);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::CloseContainer(TLVWriter & containerWriter)
{
    if (!TLVTypeIsContainer(containerWriter.mContainerType))
        return CHIP_ERROR_INCORRECT_STATE;

    if (containerWriter.IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;

    mBackingStore = containerWriter.mBackingStore;
    mBufStart     = containerWriter.mBufStart;
    mWritePoint   = containerWriter.mWritePoint;
    mRemainingLen = containerWriter.mRemainingLen;
    mLenWritten += containerWriter.mLenWritten;

    if (IsCloseContainerReserved())
        mMaxLen += kEndOfContainerMarkerSize;

    SetContainerOpen(false);

    // Reset the container writer so that it can't accidentally be used again.
    containerWriter.Init(static_cast<uint8_t *>(nullptr), 0);

    return WriteElementHead(TLVElementType::EndOfContainer, AnonymousTag(), 0);
}

CHIP_ERROR TLVWriter::StartContainer(Tag tag, TLVType containerType, TLVType & outerContainerType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(TLVTypeIsContainer(containerType), CHIP_ERROR_WRONG_TLV_TYPE);

    if (IsCloseContainerReserved())
    {
        VerifyOrReturnError(mMaxLen >= kEndOfContainerMarkerSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        mMaxLen -= kEndOfContainerMarkerSize;
    }

    err = WriteElementHead(static_cast<TLVElementType>(containerType), tag, 0);
    if (err != CHIP_NO_ERROR)
    {
        // undo the space reservation, as the container is not actually open
        if (IsCloseContainerReserved())
            mMaxLen += kEndOfContainerMarkerSize;

        return err;
    }
    outerContainerType = mContainerType;
    mContainerType     = containerType;

    SetContainerOpen(false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::EndContainer(TLVType outerContainerType)
{
    if (!TLVTypeIsContainer(mContainerType))
        return CHIP_ERROR_INCORRECT_STATE;

    mContainerType = outerContainerType;

    if (IsCloseContainerReserved())
        mMaxLen += kEndOfContainerMarkerSize;

    return WriteElementHead(TLVElementType::EndOfContainer, AnonymousTag(), 0);
}

CHIP_ERROR TLVWriter::PutPreEncodedContainer(Tag tag, TLVType containerType, const uint8_t * data, uint32_t dataLen)
{
    if (!TLVTypeIsContainer(containerType))
        return CHIP_ERROR_INVALID_ARGUMENT;

    CHIP_ERROR err = WriteElementHead(static_cast<TLVElementType>(containerType), tag, 0);
    if (err != CHIP_NO_ERROR)
        return err;

    return WriteData(data, dataLen);
}

CHIP_ERROR TLVWriter::CopyContainer(TLVReader & container)
{
    return CopyContainer(container.GetTag(), container);
}

CHIP_ERROR TLVWriter::CopyContainer(Tag tag, TLVReader & container)
{
    // NOTE: This function MUST be used with a TVLReader that is reading from a contiguous buffer.
    if (container.mBackingStore != nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    CHIP_ERROR err;
    TLVType containerType, outerContainerType;
    const uint8_t * containerStart;

    containerType = container.GetType();

    err = container.EnterContainer(outerContainerType);
    if (err != CHIP_NO_ERROR)
        return err;

    containerStart = container.GetReadPoint();

    err = container.ExitContainer(outerContainerType);
    if (err != CHIP_NO_ERROR)
        return err;

    return PutPreEncodedContainer(tag, containerType, containerStart,
                                  static_cast<uint32_t>(container.GetReadPoint() - containerStart));
}

CHIP_ERROR TLVWriter::CopyContainer(Tag tag, const uint8_t * encodedContainer, uint16_t encodedContainerLen)
{
    TLVReader reader;

    reader.Init(encodedContainer, encodedContainerLen);

    ReturnErrorOnFailure(reader.Next());

    ReturnErrorOnFailure(PutPreEncodedContainer(tag, reader.GetType(), reader.GetReadPoint(), reader.GetRemainingLength()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::WriteElementHead(TLVElementType elemType, Tag tag, uint64_t lenOrVal)
{
    uint8_t * p;
    uint8_t stagingBuf[17]; // 17 = 1 control byte + 8 tag bytes + 8 length/value bytes

    if (IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;

    uint32_t tagNum = TagNumFromTag(tag);

    if ((mRemainingLen >= sizeof(stagingBuf)) && (mMaxLen >= sizeof(stagingBuf)))
        p = mWritePoint;
    else
        p = stagingBuf;

    if (IsSpecialTag(tag))
    {
        if (tagNum <= kContextTagMaxNum)
        {
            if (mContainerType != kTLVType_Structure && mContainerType != kTLVType_List)
                return CHIP_ERROR_INVALID_TLV_TAG;

            Write8(p, TLVTagControl::ContextSpecific | elemType);
            Write8(p, static_cast<uint8_t>(tagNum));
        }
        else
        {
            if (elemType != TLVElementType::EndOfContainer && mContainerType != kTLVType_NotSpecified &&
                mContainerType != kTLVType_Array && mContainerType != kTLVType_List)
                return CHIP_ERROR_INVALID_TLV_TAG;

            Write8(p, TLVTagControl::Anonymous | elemType);
        }
    }
    else
    {
        uint32_t profileId = ProfileIdFromTag(tag);

        if (mContainerType != kTLVType_NotSpecified && mContainerType != kTLVType_Structure && mContainerType != kTLVType_List)
            return CHIP_ERROR_INVALID_TLV_TAG;

        if (profileId == kCommonProfileId)
        {
            if (tagNum < 65536)
            {
                Write8(p, TLVTagControl::CommonProfile_2Bytes | elemType);
                LittleEndian::Write16(p, static_cast<uint16_t>(tagNum));
            }
            else
            {
                Write8(p, TLVTagControl::CommonProfile_4Bytes | elemType);
                LittleEndian::Write32(p, tagNum);
            }
        }
        else if (profileId == ImplicitProfileId)
        {
            if (tagNum < 65536)
            {
                Write8(p, TLVTagControl::ImplicitProfile_2Bytes | elemType);
                LittleEndian::Write16(p, static_cast<uint16_t>(tagNum));
            }
            else
            {
                Write8(p, TLVTagControl::ImplicitProfile_4Bytes | elemType);
                LittleEndian::Write32(p, tagNum);
            }
        }
        else
        {
            uint16_t vendorId   = static_cast<uint16_t>(profileId >> 16);
            uint16_t profileNum = static_cast<uint16_t>(profileId);

            if (tagNum < 65536)
            {
                Write8(p, TLVTagControl::FullyQualified_6Bytes | elemType);
                LittleEndian::Write16(p, vendorId);
                LittleEndian::Write16(p, profileNum);
                LittleEndian::Write16(p, static_cast<uint16_t>(tagNum));
            }
            else
            {
                Write8(p, TLVTagControl::FullyQualified_8Bytes | elemType);
                LittleEndian::Write16(p, vendorId);
                LittleEndian::Write16(p, profileNum);
                LittleEndian::Write32(p, tagNum);
            }
        }
    }

    switch (GetTLVFieldSize(elemType))
    {
    case kTLVFieldSize_0Byte:
        break;
    case kTLVFieldSize_1Byte:
        Write8(p, static_cast<uint8_t>(lenOrVal));
        break;
    case kTLVFieldSize_2Byte:
        LittleEndian::Write16(p, static_cast<uint16_t>(lenOrVal));
        break;
    case kTLVFieldSize_4Byte:
        LittleEndian::Write32(p, static_cast<uint32_t>(lenOrVal));
        break;
    case kTLVFieldSize_8Byte:
        LittleEndian::Write64(p, lenOrVal);
        break;
    }

    if ((mRemainingLen >= sizeof(stagingBuf)) && (mMaxLen >= sizeof(stagingBuf)))
    {
        uint32_t len = static_cast<uint32_t>(p - mWritePoint);
        mWritePoint  = p;
        mRemainingLen -= len;
        mLenWritten += len;
        return CHIP_NO_ERROR;
    }
    return WriteData(stagingBuf, static_cast<uint32_t>(p - stagingBuf));
}

CHIP_ERROR TLVWriter::WriteElementWithData(TLVType type, Tag tag, const uint8_t * data, uint32_t dataLen)
{
    if (static_cast<uint64_t>(type) & kTLVTypeSizeMask)
    {
        // We won't be able to recover this type properly!
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLVFieldSize lenFieldSize;

    if (dataLen <= UINT8_MAX)
        lenFieldSize = kTLVFieldSize_1Byte;
    else if (dataLen <= UINT16_MAX)
        lenFieldSize = kTLVFieldSize_2Byte;
    else
        lenFieldSize = kTLVFieldSize_4Byte;

    CHIP_ERROR err = WriteElementHead(static_cast<TLVElementType>(static_cast<uint8_t>(type) | static_cast<uint8_t>(lenFieldSize)),
                                      tag, dataLen);
    if (err != CHIP_NO_ERROR)
        return err;

    return WriteData(data, dataLen);
}

CHIP_ERROR TLVWriter::WriteData(const uint8_t * p, uint32_t len)
{
    VerifyOrReturnError((mLenWritten + len) <= mMaxLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    while (len > 0)
    {
        if (mRemainingLen == 0)
        {
            VerifyOrReturnError(mBackingStore != nullptr, CHIP_ERROR_NO_MEMORY);

            VerifyOrReturnError(CanCastTo<uint32_t>(mWritePoint - mBufStart), CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(mBackingStore->FinalizeBuffer(*this, mBufStart, static_cast<uint32_t>(mWritePoint - mBufStart)));

            ReturnErrorOnFailure(mBackingStore->GetNewBuffer(*this, mBufStart, mRemainingLen));

            mWritePoint = mBufStart;

            if (mRemainingLen > (mMaxLen - mLenWritten))
                mRemainingLen = (mMaxLen - mLenWritten);
        }

        uint32_t writeLen = len;
        if (writeLen > mRemainingLen)
            writeLen = mRemainingLen;

        memmove(mWritePoint, p, writeLen);
        mWritePoint += writeLen;
        mRemainingLen -= writeLen;
        mLenWritten += writeLen;
        p += writeLen;
        len -= writeLen;
    }

    return CHIP_NO_ERROR;
}

} // namespace TLV
} // namespace chip
