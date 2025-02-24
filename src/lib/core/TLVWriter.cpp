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
#include <lib/core/TLVWriter.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLVBackingStore.h>
#include <lib/core/TLVCommon.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <lib/support/logging/Constants.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <lib/support/utf8.h>
#include <system/SystemConfig.h>

// Doxygen is confused by the __attribute__ annotation
#ifndef DOXYGEN
#define NO_INLINE __attribute__((noinline))
#endif // DOXYGEN

// You can enable this block manually to abort on usage of uninitialized writers in
// your codebase. There are no such usages in the SDK (outside of tests).
#if 0
#define ABORT_ON_UNINITIALIZED_IF_ENABLED() VerifyOrDie(IsInitialized() == true)
#else
#define ABORT_ON_UNINITIALIZED_IF_ENABLED()                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (0)
#endif

namespace chip {
namespace TLV {

using namespace chip::Encoding;

TLVWriter::TLVWriter() :
    ImplicitProfileId(kProfileIdNotSpecified), AppData(nullptr), mBackingStore(nullptr), mBufStart(nullptr), mWritePoint(nullptr),
    mRemainingLen(0), mLenWritten(0), mMaxLen(0), mReservedSize(0), mContainerType(kTLVType_NotSpecified), mInitializationCookie(0),
    mContainerOpen(false), mCloseContainerReserved(true)
{}

NO_INLINE void TLVWriter::Init(uint8_t * buf, size_t maxLen)
{
    // TODO: Maybe we can just make mMaxLen, mLenWritten, mRemainingLen size_t instead?
    uint32_t actualMaxLen = maxLen > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(maxLen);

    // TODO(#30825): Need to ensure a single init path for this complex data.
    mInitializationCookie = 0;
    mBackingStore         = nullptr;
    mBufStart             = buf;
    mWritePoint           = buf;
    mRemainingLen         = actualMaxLen;
    mLenWritten           = 0;
    mMaxLen               = actualMaxLen;
    mContainerType        = kTLVType_NotSpecified;
    mReservedSize         = 0;
    SetContainerOpen(false);
    SetCloseContainerReserved(true);

    ImplicitProfileId     = kProfileIdNotSpecified;
    mInitializationCookie = kExpectedInitializationCookie;
}

CHIP_ERROR TLVWriter::Init(TLVBackingStore & backingStore, uint32_t maxLen /* = UINT32_MAX */)
{
    // TODO(#30825): Need to ensure a single init path for this complex data.
    Init(nullptr, maxLen);
    mInitializationCookie = 0;

    mBackingStore  = &backingStore;
    mBufStart      = nullptr;
    mRemainingLen  = 0;
    CHIP_ERROR err = mBackingStore->OnInit(*this, mBufStart, mRemainingLen);
    if (err != CHIP_NO_ERROR)
        return err;

    VerifyOrReturnError(mBufStart != nullptr, CHIP_ERROR_INTERNAL);
    mWritePoint           = mBufStart;
    mInitializationCookie = kExpectedInitializationCookie;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::Finalize()
{
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (IsContainerOpen())
        return CHIP_ERROR_TLV_CONTAINER_OPEN;
    if (mBackingStore != nullptr)
        err = mBackingStore->FinalizeBuffer(*this, mBufStart, static_cast<uint32_t>(mWritePoint - mBufStart));

        // TODO(#30825) The following should be safe, but in some cases (without mBackingStore), there are incremental writes that
        // start failing.
#if 0
    if (err == CHIP_NO_ERROR)
        mInitializationCookie = 0;
#endif

    return err;
}

CHIP_ERROR TLVWriter::ReserveBuffer(uint32_t aBufferSize)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRemainingLen >= aBufferSize, CHIP_ERROR_NO_MEMORY);

    if (mBackingStore)
    {
        VerifyOrReturnError(mBackingStore->GetNewBufferWillAlwaysFail(), CHIP_ERROR_INCORRECT_STATE);
    }
    mReservedSize += aBufferSize;
    mRemainingLen -= aBufferSize;
    return CHIP_NO_ERROR;
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
    if (buf == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;
    if (mMaxLen == 0)
        return CHIP_ERROR_INCORRECT_STATE;

    // Calculate length with a hard limit to prevent unbounded reads.
    // Use mMaxLen instead of mRemainingLen to account for CircularTLVWriter.
    // Note: Overrun is still possible if buf is not null-terminated, and this
    // check cannot prevent all invalid memory reads.
    size_t len = strnlen(buf, mMaxLen);

    if (!CanCastTo<uint32_t>(len))
        return CHIP_ERROR_INVALID_ARGUMENT;

    uint32_t stringLen = static_cast<uint32_t>(len);

    // Null terminator was not found within the allocated space.
    if (stringLen == mMaxLen)
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    return PutString(tag, buf, stringLen);
}

CHIP_ERROR TLVWriter::PutString(Tag tag, const char * buf, uint32_t len)
{
#if CHIP_CONFIG_TLV_VALIDATE_CHAR_STRING_ON_WRITE
    // Spec requirement: A.11.2. UTF-8 and Octet Strings
    //
    // For UTF-8 strings, the value octets SHALL encode a valid
    // UTF-8 character (code points) sequence.
    //
    // Senders SHALL NOT include a terminating null character to
    // mark the end of a string.

    if (!Utf8::IsValid(CharSpan(buf, len)))
    {
        return CHIP_ERROR_INVALID_UTF8;
    }

    if ((len > 0) && (buf[len - 1] == 0))
    {
        return CHIP_ERROR_INVALID_TLV_CHAR_STRING;
    }
#endif // CHIP_CONFIG_TLV_VALIDATE_CHAR_STRING_ON_WRITE

    return WriteElementWithData(kTLVType_UTF8String, tag, reinterpret_cast<const uint8_t *>(buf), len);
}

CHIP_ERROR TLVWriter::PutString(Tag tag, CharSpan str)
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

void TLVWriter::TLVWriterPutcharCB(uint8_t c, void * appState)
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
#if !CONFIG_HAVE_VCBPRINTF
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

    // write length.
    err = WriteElementHead(
        static_cast<TLVElementType>(static_cast<uint8_t>(kTLVType_UTF8String) | static_cast<uint8_t>(lenFieldSize)), tag, dataLen);
    SuccessOrExit(err);

    VerifyOrExit((mLenWritten + dataLen) <= mMaxLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // write data
#if CONFIG_HAVE_VCBPRINTF

    va_copy(aq, ap);

    vcbprintf(TLVWriterPutcharCB, this, dataLen, fmt, aq);

    va_end(aq);
#else // CONFIG_HAVE_VCBPRINTF

    tmpBuf = static_cast<char *>(chip::Platform::MemoryAlloc(dataLen + 1));
    VerifyOrExit(tmpBuf != nullptr, err = CHIP_ERROR_NO_MEMORY);

    va_copy(aq, ap);

    vsnprintf(tmpBuf, dataLen + 1, fmt, aq);

    va_end(aq);

    err = WriteData(reinterpret_cast<uint8_t *>(tmpBuf), static_cast<uint32_t>(dataLen));
    chip::Platform::MemoryFree(tmpBuf);

#endif // CONFIG_HAVE_VCBPRINTF

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

const size_t kTLVCopyChunkSize = 16;

CHIP_ERROR TLVWriter::CopyElement(Tag tag, TLVReader & reader)
{
    TLVElementType elemType = reader.ElementType();
    uint64_t elemLenOrVal   = reader.mElemLenOrVal;
    TLVReader readerHelper; // used to figure out the length of the element and read data of the element
    uint32_t copyDataLen;
    uint8_t chunk[kTLVCopyChunkSize];

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
        uint32_t chunkSize = copyDataLen > kTLVCopyChunkSize ? kTLVCopyChunkSize : copyDataLen;
        ReturnErrorOnFailure(readerHelper.ReadData(chunk, chunkSize));

        ReturnErrorOnFailure(WriteData(chunk, chunkSize));

        copyDataLen -= chunkSize;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::OpenContainer(Tag tag, TLVType containerType, TLVWriter & containerWriter)
{
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
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

    // TODO(#30825): Clean-up this separate init path path.
    containerWriter.mBackingStore  = mBackingStore;
    containerWriter.mBufStart      = mBufStart;
    containerWriter.mWritePoint    = mWritePoint;
    containerWriter.mRemainingLen  = mRemainingLen;
    containerWriter.mLenWritten    = 0;
    containerWriter.mMaxLen        = mMaxLen - mLenWritten;
    containerWriter.mContainerType = containerType;
    containerWriter.SetContainerOpen(false);
    containerWriter.SetCloseContainerReserved(IsCloseContainerReserved());
    containerWriter.ImplicitProfileId     = ImplicitProfileId;
    containerWriter.mInitializationCookie = kExpectedInitializationCookie;

    SetContainerOpen(true);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::CloseContainer(TLVWriter & containerWriter)
{
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

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
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
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
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

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
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    TLVReader reader;

    reader.Init(encodedContainer, encodedContainerLen);

    ReturnErrorOnFailure(reader.Next());

    ReturnErrorOnFailure(PutPreEncodedContainer(tag, reader.GetType(), reader.GetReadPoint(), reader.GetRemainingLength()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVWriter::WriteElementHead(TLVElementType elemType, Tag tag, uint64_t lenOrVal)
{
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!IsContainerOpen(), CHIP_ERROR_TLV_CONTAINER_OPEN);

    uint8_t stagingBuf[17]; // 17 = 1 control byte + 8 tag bytes + 8 length/value bytes
    uint32_t tagNum = TagNumFromTag(tag);

    Encoding::LittleEndian::BufferWriter writer(stagingBuf, sizeof(stagingBuf));

    if (IsSpecialTag(tag))
    {
        if (tagNum <= Tag::kContextTagMaxNum)
        {
            if (mContainerType != kTLVType_Structure && mContainerType != kTLVType_List)
                return CHIP_ERROR_INVALID_TLV_TAG;

            writer.Put8(TLVTagControl::ContextSpecific | elemType);
            writer.Put8(static_cast<uint8_t>(tagNum));
        }
        else
        {
            if (elemType != TLVElementType::EndOfContainer && mContainerType != kTLVType_NotSpecified &&
                mContainerType != kTLVType_Array && mContainerType != kTLVType_List)
                return CHIP_ERROR_INVALID_TLV_TAG;

            writer.Put8(TLVTagControl::Anonymous | elemType);
        }
    }
    else
    {
        uint32_t profileId = ProfileIdFromTag(tag);

        if (mContainerType != kTLVType_NotSpecified && mContainerType != kTLVType_Structure && mContainerType != kTLVType_List)
            return CHIP_ERROR_INVALID_TLV_TAG;

        if (profileId == kCommonProfileId)
        {
            if (tagNum <= std::numeric_limits<uint16_t>::max())
            {
                writer.Put8(TLVTagControl::CommonProfile_2Bytes | elemType);
                writer.Put16(static_cast<uint16_t>(tagNum));
            }
            else
            {
                writer.Put8(TLVTagControl::CommonProfile_4Bytes | elemType);
                writer.Put32(tagNum);
            }
        }
        else if (profileId == ImplicitProfileId)
        {
            if (tagNum <= std::numeric_limits<uint16_t>::max())
            {
                writer.Put8(TLVTagControl::ImplicitProfile_2Bytes | elemType);
                writer.Put16(static_cast<uint16_t>(tagNum));
            }
            else
            {
                writer.Put8(TLVTagControl::ImplicitProfile_4Bytes | elemType);
                writer.Put32(tagNum);
            }
        }
        else
        {
            uint16_t vendorId   = static_cast<uint16_t>(profileId >> 16);
            uint16_t profileNum = static_cast<uint16_t>(profileId);

            if (tagNum <= std::numeric_limits<uint16_t>::max())
            {

                writer.Put8(TLVTagControl::FullyQualified_6Bytes | elemType);
                writer.Put16(vendorId);
                writer.Put16(profileNum);
                writer.Put16(static_cast<uint16_t>(tagNum));
            }
            else
            {
                writer.Put8(TLVTagControl::FullyQualified_8Bytes | elemType);
                writer.Put16(vendorId);
                writer.Put16(profileNum);
                writer.Put32(tagNum);
            }
        }
    }

    uint8_t lengthSize = TLVFieldSizeToBytes(GetTLVFieldSize(elemType));
    if (lengthSize > 0)
    {
        writer.EndianPut(lenOrVal, lengthSize);
    }

    size_t written = 0;
    VerifyOrDie(writer.Fit(written));
    return WriteData(stagingBuf, static_cast<uint32_t>(written));
}

CHIP_ERROR TLVWriter::WriteElementWithData(TLVType type, Tag tag, const uint8_t * data, uint32_t dataLen)
{
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
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
    ABORT_ON_UNINITIALIZED_IF_ENABLED();

    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError((mLenWritten + len) <= mMaxLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    while (len > 0)
    {
        if (mRemainingLen == 0)
        {
            VerifyOrReturnError(mBackingStore != nullptr, CHIP_ERROR_NO_MEMORY);

            VerifyOrReturnError(CanCastTo<uint32_t>(mWritePoint - mBufStart), CHIP_ERROR_INCORRECT_STATE);
            ReturnErrorOnFailure(mBackingStore->FinalizeBuffer(*this, mBufStart, static_cast<uint32_t>(mWritePoint - mBufStart)));

            ReturnErrorOnFailure(mBackingStore->GetNewBuffer(*this, mBufStart, mRemainingLen));
            VerifyOrReturnError(mRemainingLen > 0, CHIP_ERROR_NO_MEMORY);

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
