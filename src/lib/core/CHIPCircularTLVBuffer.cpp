/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements the circular buffer for TLV
 *      elements. When used as the backing store for the TLVReader and
 *      TLVWriter, those classes will work with the wraparound of data
 *      within the buffer.  Additionally, the TLVWriter will be able
 *      to continually add top-level TLV elements by evicting
 *      pre-existing elements.
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <core/CHIPCircularTLVBuffer.h>

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>

#include <support/CodeUtils.h>

#include <stdint.h>

namespace chip {
namespace TLV {

using namespace chip::Encoding;

/**
 * @brief
 *   CHIPCircularTLVBuffer constructor
 *
 * @param[in] inBuffer       A pointer to the backing store for the queue
 *
 * @param[in] inBufferLength Length, in bytes, of the backing store
 *
 * @param[in] inHead         Initial point for the head.  The @a inHead pointer is must fall within the backing store for the
 * circular buffer, i.e. within @a inBuffer and &(@a inBuffer[@a inBufferLength])
 */
CHIPCircularTLVBuffer::CHIPCircularTLVBuffer(uint8_t * inBuffer, size_t inBufferLength, uint8_t * inHead)
{
    mQueue       = inBuffer;
    mQueueSize   = inBufferLength;
    mQueueLength = 0;
    mQueueHead   = inHead;

    mProcessEvictedElement = nullptr;
    mAppData               = nullptr;

    // use common as opposed to unspecified, s.t. the reader that
    // skips over the elements does not complain about implicit
    // profile tags.
    mImplicitProfileId = kCommonProfileId;
}

/**
 * @brief
 *   CHIPCircularTLVBuffer constructor
 *
 * @param[in] inBuffer       A pointer to the backing store for the queue
 *
 * @param[in] inBufferLength Length, in bytes, of the backing store
 */
CHIPCircularTLVBuffer::CHIPCircularTLVBuffer(uint8_t * inBuffer, size_t inBufferLength)
{
    mQueue       = inBuffer;
    mQueueSize   = inBufferLength;
    mQueueLength = 0;
    mQueueHead   = mQueue;

    mProcessEvictedElement = nullptr;
    mAppData               = nullptr;

    // use common as opposed to unspecified, s.t. the reader that
    // skips over the elements does not complain about implicit
    // profile tags.
    mImplicitProfileId = kCommonProfileId;
}

/**
 * @brief
 *   Evicts the oldest top-level TLV element in the CHIPCircularTLVBuffer
 *
 * This function removes the oldest top level TLV element in the
 * buffer.  The function will call the callback registered at
 * #mProcessEvictedElement to process the element prior to removal.
 * If the callback returns anything but #CHIP_NO_ERROR, the element
 * is not removed.  Similarly, if any other error occurs -- no
 * elements within the buffer, etc -- the underlying
 * #CHIPCircularTLVBuffer remains unchanged.
 *
 *  @retval #CHIP_NO_ERROR On success.
 *
 *  @retval other          On any other error returned either by the callback
 *                         or by the TLVReader.
 *
 */
CHIP_ERROR CHIPCircularTLVBuffer::EvictHead()
{
    CircularTLVReader reader;
    uint8_t * newHead;
    size_t newLen;
    CHIP_ERROR err;

    // find the boundaries of an event to throw away
    reader.Init(this);
    reader.ImplicitProfileId = mImplicitProfileId;

    // position the reader on the first element
    err = reader.Next();
    SuccessOrExit(err);

    // skip to the next element
    err = reader.Skip();
    SuccessOrExit(err);

    // record the state of the queue post-call
    newLen  = mQueueLength - (reader.GetLengthRead());
    newHead = const_cast<uint8_t *>(reader.GetReadPoint());

    // if a custom handler is installed, give it a chance to
    // process the element before we evict it from the buffer.
    if (mProcessEvictedElement != nullptr)
    {
        // Reinitialize the reader
        reader.Init(this);
        reader.ImplicitProfileId = mImplicitProfileId;

        err = mProcessEvictedElement(*this, mAppData, reader);
        SuccessOrExit(err);
    }

    // update queue state
    mQueueLength = newLen;
    mQueueHead   = newHead;

exit:
    return err;
}

/**
 * @brief
 *   Get additional space for the TLVWriter.  In actuality, the
 *   function evicts an element from the circular buffer, and adjusts
 *   the head of this buffer queue
 *
 * @param[in,out] ioWriter  TLVWriter calling this function
 *
 * @param[out] outBufStart The pointer to the new buffer
 *
 * @param[out] outBufLen   The available length for writing
 *
 * @retval #CHIP_NO_ERROR On success.
 *
 * @retval other           If the function was unable to elide a complete
 *                         top-level TLV element.
 */

CHIP_ERROR CHIPCircularTLVBuffer::GetNewBuffer(TLVWriter & ioWriter, uint8_t *& outBufStart, uint32_t & outBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t * tail = QueueTail();

    if (mQueueLength >= mQueueSize)
    {
        // Queue is out of space, need to evict an element
        err = EvictHead();
        SuccessOrExit(err);
    }

    // set the output values, returned buffer must be contiguous
    outBufStart = tail;

    if (tail >= mQueueHead)
    {
        outBufLen = mQueueSize - (tail - mQueue);
    }
    else
    {
        outBufLen = mQueueHead - tail;
    }

exit:
    return err;
}

/**
 * @brief
 *   FinalizeBuffer adjust the `CHIPCircularTLVBuffer` state on
 *   completion of output from the TLVWriter.  This function affects
 *   the position of the queue tail.
 *
 * @param[in,out] ioWriter TLVWriter calling this function
 *
 * @param[in] inBufStart pointer to the start of data (from `TLVWriter`
 *                       perspective)
 *
 * @param[in] inBufLen   length of data in the buffer pointed to by
 *                       `inbufStart`
 *
 * @retval #CHIP_NO_ERROR Unconditionally.
 */

CHIP_ERROR CHIPCircularTLVBuffer::FinalizeBuffer(TLVWriter & ioWriter, uint8_t * inBufStart, uint32_t inBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t * tail = inBufStart + inBufLen;
    if (inBufLen)
    {
        if (tail <= mQueueHead)
        {
            mQueueLength = mQueueSize + (tail - mQueueHead);
        }
        else
        {
            mQueueLength = tail - mQueueHead;
        }
    }
    return err;
}

/**
 * @brief
 *   Get additional space for the TLVReader.
 *
 *  The storage provided by the CHIPCircularTLVBuffer may be
 *  wraparound within the buffer.  This function provides us with an
 *  ability to match the buffering of the circular buffer to the
 *  TLVReader constraints.  The reader will read at most `mQueueSize`
 *  bytes from the buffer.
 *
 * @param[in] ioReader         TLVReader calling this function.
 *
 * @param[in,out] outBufStart  The reference to the data buffer.  On
 *                             return, it is set to a value within this
 *                             buffer.
 *
 * @param[out] outBufLen       On return, set to the number of continuous
 *                             bytes that could be read out of the buffer.
 *
 * @retval #CHIP_NO_ERROR      Succeeds unconditionally.
 */
CHIP_ERROR CHIPCircularTLVBuffer::GetNextBuffer(TLVReader & ioReader, const uint8_t *& outBufStart, uint32_t & outBufLen)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    uint8_t * tail              = QueueTail();
    const uint8_t * readerStart = outBufStart;

    if (readerStart == nullptr)
    {
        outBufStart = mQueueHead;

        if (outBufStart == mQueue + mQueueSize)
        {
            outBufStart = mQueue;
        }
    }
    else if (readerStart >= (mQueue + mQueueSize))
    {
        outBufStart = mQueue;
    }
    else
    {
        outBufLen = 0;
        return err;
    }

    if ((mQueueLength != 0) && (tail <= outBufStart))
    {
        // the buffer is non-empty and data wraps around the end
        // point.  The returned buffer conceptually spans from
        // outBufStart until the end of the underlying storage buffer
        // (i.e. mQueue+mQueueSize).  This case tail == outBufStart
        // indicates that the buffer is completely full
        outBufLen = (mQueue + mQueueSize) - outBufStart;
        if ((tail == outBufStart) && (readerStart != nullptr))
            outBufLen = 0;
    }
    else
    {
        // the buffer length is the distance between head and tail;
        // tail is either strictly larger or the buffer is empty
        outBufLen = tail - outBufStart;
    }
    return err;
}

/**
 * @brief
 *   A trampoline to fetch more space for the TLVWriter.
 *
 * @param[in,out] ioWriter TLVWriter calling this function
 *
 * @param[in,out] inBufHandle A handle to the `CircularTLVWriter` object
 *
 * @param[out] outBufStart The pointer to the new buffer
 *
 * @param[out] outBufLen   The available length for writing
 *
 * @retval #CHIP_NO_ERROR On success.
 *
 * @retval other           If the function was unable to elide a complete
 *                         top-level TLV element.
 */
CHIP_ERROR CHIPCircularTLVBuffer::GetNewBufferFunct(TLVWriter & ioWriter, uintptr_t & inBufHandle, uint8_t *& outBufStart,
                                                    uint32_t & outBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPCircularTLVBuffer * buf;

    VerifyOrExit(inBufHandle != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    buf = reinterpret_cast<CHIPCircularTLVBuffer *>(inBufHandle);

    err = buf->GetNewBuffer(ioWriter, outBufStart, outBufLen);

exit:
    return err;
}

/**
 * @brief
 *   A trampoline to CHIPCircularTLVBuffer::FinalizeBuffer
 *
 * @param[in,out] ioWriter TLVWriter calling this function
 *
 * @param[in,out] inBufHandle A handle to the `CircularTLVWriter` object
 *
 * @param[in] inBufStart pointer to the start of data (from `TLVWriter`
 *                       perspective)
 *
 * @param[in] inBufLen   length of data in the buffer pointed to by
 *                       `inbufStart`
 *
 * @retval #CHIP_NO_ERROR Unconditionally.
 */
CHIP_ERROR CHIPCircularTLVBuffer::FinalizeBufferFunct(TLVWriter & ioWriter, uintptr_t inBufHandle, uint8_t * inBufStart,
                                                      uint32_t inBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPCircularTLVBuffer * buf;

    VerifyOrExit(inBufHandle != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    buf = reinterpret_cast<CHIPCircularTLVBuffer *>(inBufHandle);

    err = buf->FinalizeBuffer(ioWriter, inBufStart, inBufLen);

exit:
    return err;
}

/**
 * @brief
 *   A trampoline to CHIPCircularTLVBuffer::GetNextBuffer
 *
 * @param[in,out] ioReader TLVReader calling this function
 *
 * @param[in,out] inBufHandle A handle to the `CircularTLVWriter` object
 *
 * @param[in,out] outBufStart  The reference to the data buffer.  On
 *                             return, it is set to a value within this
 *                             buffer.
 *
 * @param[out] outBufLen       On return, set to the number of continuous
 *                             bytes that could be read out of the buffer.
 *
 * @retval #CHIP_NO_ERROR      Succeeds unconditionally.
 */
CHIP_ERROR CHIPCircularTLVBuffer::GetNextBufferFunct(TLVReader & ioReader, uintptr_t & inBufHandle, const uint8_t *& outBufStart,
                                                     uint32_t & outBufLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPCircularTLVBuffer * buf;

    VerifyOrExit(inBufHandle != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    buf = reinterpret_cast<CHIPCircularTLVBuffer *>(inBufHandle);

    err = buf->GetNextBuffer(ioReader, outBufStart, outBufLen);

exit:
    return err;
}

/**
 * @brief
 *   Initializes a TLVWriter object to write from a single CHIPCircularTLVBuffer
 *
 * Writing begins at the last byte of the buffer.  The number of bytes
 * to be written is not constrained by the underlying circular buffer:
 * writing new elements to the buffer will kick out previous elements
 * as long as an individual top-level TLV structure fits within the
 * buffer.  For example, writing a 7-byte top-level boolean TLV into a
 * 7 byte buffer will work indefinitely, but writing an 8-byte TLV
 * structure will result in an error.
 *
 * @param[in]    buf   A pointer to a fully initialized CHIPCircularTLVBuffer
 *
 */
void CircularTLVWriter::Init(CHIPCircularTLVBuffer * buf)
{
    mBufHandle     = reinterpret_cast<uintptr_t>(buf);
    mLenWritten    = 0;
    mMaxLen        = UINT32_MAX;
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);
    SetCloseContainerReserved(false);

    ImplicitProfileId = kProfileIdNotSpecified;
    GetNewBuffer      = CHIPCircularTLVBuffer::GetNewBufferFunct;
    FinalizeBuffer    = CHIPCircularTLVBuffer::FinalizeBufferFunct;

    GetNewBuffer(*this, mBufHandle, mBufStart, mRemainingLen);
    mWritePoint = mBufStart;
}

/**
 * @brief
 *   Initializes a TLVReader object to read from a single CHIPCircularTLVBuffer
 *
 * Parsing begins at the start of the buffer (obtained by the
 * buffer->Start() position) and continues until the end of the buffer
 * Parsing may wraparound within the buffer (on any element).  At most
 * buffer->GetQueueSize() bytes are read out.
 *
 * @param[in]    buf   A pointer to a fully initialized CHIPCircularTLVBuffer
 *
 */
void CircularTLVReader::Init(CHIPCircularTLVBuffer * buf)
{
    uint32_t bufLen = 0;

    mBufHandle    = reinterpret_cast<uintptr_t>(buf);
    GetNextBuffer = CHIPCircularTLVBuffer::GetNextBufferFunct;
    mLenRead      = 0;
    mReadPoint    = nullptr;
    GetNextBuffer(*this, mBufHandle, mReadPoint, bufLen);
    mBufEnd        = mReadPoint + bufLen;
    mMaxLen        = buf->DataLength();
    mControlByte   = kTLVControlByte_NotSpecified;
    mElemTag       = AnonymousTag;
    mElemLenOrVal  = 0;
    mContainerType = kTLVType_NotSpecified;
    SetContainerOpen(false);
    ImplicitProfileId = kProfileIdNotSpecified;
    AppData           = nullptr;
}

} // namespace TLV
} // namespace chip
