/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *  @file
 *      This file defines the circular buffer for TLV
 *      elements. When used as the backing store for the TLVReader and
 *      TLVWriter, those classes will work with the wraparound of data
 *      within the buffer.  Additionally, the TLVWriter will be able
 *      to continually add top-level TLV elements by evicting
 *      pre-existing elements.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/TLVBackingStore.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>
#include <lib/support/DLLUtil.h>

#include <stdint.h>
#include <stdlib.h>

namespace chip {
namespace TLV {

/**
 * @class TLVCircularBuffer
 *
 * @brief
 *    TLVCircularBuffer provides circular storage for the
 *    chip::TLV::TLVWriter and chip::TLVTLVReader.  chip::TLV::TLVWriter is able to write an
 *    unbounded number of TLV entries to the TLVCircularBuffer
 *    as long as each individual TLV entry fits entirely within the
 *    provided storage.  The chip::TLV::TLVReader will read at most the size of
 *    the buffer, but will accommodate the wraparound within the
 *    buffer.
 *
 */
class DLL_EXPORT TLVCircularBuffer : public chip::TLV::TLVBackingStore
{
public:
    TLVCircularBuffer(uint8_t * inBuffer, uint32_t inBufferLength);
    TLVCircularBuffer(uint8_t * inBuffer, uint32_t inBufferLength, uint8_t * inHead);

    void Init(uint8_t * inBuffer, uint32_t inBufferLength);
    inline uint8_t * QueueHead() const { return mQueueHead; }
    inline uint8_t * QueueTail() const { return mQueue + ((static_cast<size_t>(mQueueHead - mQueue) + mQueueLength) % mQueueSize); }
    inline uint32_t DataLength() const { return mQueueLength; }
    inline uint32_t AvailableDataLength() const { return mQueueSize - mQueueLength; }
    inline uint32_t GetTotalDataLength() const { return mQueueSize; }
    inline uint8_t * GetQueue() const { return mQueue; }

    CHIP_ERROR EvictHead();

    // chip::TLV::TLVBackingStore overrides:
    CHIP_ERROR OnInit(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) override;
    CHIP_ERROR GetNextBuffer(TLVReader & ioReader, const uint8_t *& outBufStart, uint32_t & outBufLen) override;
    CHIP_ERROR OnInit(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) override;
    CHIP_ERROR GetNewBuffer(TLVWriter & ioWriter, uint8_t *& outBufStart, uint32_t & outBufLen) override;
    CHIP_ERROR FinalizeBuffer(TLVWriter & ioWriter, uint8_t * inBufStart, uint32_t inBufLen) override;

    /**
     *  @typedef CHIP_ERROR (*ProcessEvictedElementFunct)(TLVCircularBuffer &inBuffer, void * inAppData, TLVReader &inReader)
     *
     *  A function that is called to process a TLV element prior to it
     *  being evicted from the chip::TLV::TLVCircularBuffer
     *
     *  Functions of this type are used to process a TLV element about
     *  to be evicted from the buffer.  The function will be given a
     *  chip::TLV::TLVReader positioned on the element about to be deleted, as
     *  well as void * context where the user may have provided
     *  additional environment for the callback.  If the function
     *  processed the element successfully, it must return
     *  #CHIP_NO_ERROR ; this signifies to the TLVCircularBuffer
     *  that the element may be safely evicted.  Any other return
     *  value is treated as an error and will prevent the
     *  #TLVCircularBuffer from evicting the element under
     *  consideration.
     *
     *  Note: This callback may be used to force
     *  TLVCircularBuffer to not evict the element.  This may be
     *  useful in a number of circumstances, when it is desired to
     *  have an underlying circular buffer, but not to override any
     *  elements within it.
     *
     *  @param[in] inBuffer  A reference to the buffer from which the
     *                       eviction takes place
     *
     *  @param[in] inAppData A pointer to the user-provided structure
     *                       containing additional context for this
     *                       callback
     *
     *  @param[in] inReader  A TLVReader positioned at the element to
     *                       be evicted.
     *
     *  @retval #CHIP_NO_ERROR On success. Element will be evicted.
     *
     *  @retval other        An error has occurred during the event
     *                       processing.  The element stays in the
     *                       buffer.  The write function that
     *                       triggered this element eviction will
     *                       fail.
     */
    typedef CHIP_ERROR (*ProcessEvictedElementFunct)(TLVCircularBuffer & inBuffer, void * inAppData, TLVReader & inReader);

    uint32_t mImplicitProfileId;
    void * mAppData; /**< An optional, user supplied context to be used with the callback processing the evicted element. */
    ProcessEvictedElementFunct
        mProcessEvictedElement; /**< An optional, user-supplied callback that processes the element prior to evicting it from the
                                   circular buffer.  See the ProcessEvictedElementFunct type definition on additional information on
                                   implementing the mProcessEvictedElement function. */

protected:
    /**
     * @brief
     *   returns the actual state of what our current available buffer space is
     *
     * @param[out] outBufStart The pointer to the current buffer
     *
     * @param[out] outBufLen   The available length for writing
     */
    void GetCurrentWritableBuffer(uint8_t *& outBufStart, uint32_t & outBufLen) const;

private:
    uint8_t * mQueue;
    uint32_t mQueueSize;
    uint8_t * mQueueHead;
    uint32_t mQueueLength;
};

class DLL_EXPORT CircularTLVReader : public TLVReader
{
public:
    /**
     * @brief
     *   Initializes a TLVReader object to read from a single TLVCircularBuffer
     *
     * Parsing begins at the start of the buffer (obtained by the
     * buffer->Start() position) and continues until the end of the buffer
     * Parsing may wraparound within the buffer (on any element).  At most
     * buffer->GetQueueSize() bytes are read out.
     *
     * @param[in]    buf   A pointer to a fully initialized TLVCircularBuffer
     *
     */
    void Init(TLVCircularBuffer & buf) { TLVReader::Init(buf, buf.DataLength()); }
};

class DLL_EXPORT CircularTLVWriter : public TLVWriter
{
public:
    /**
     * @brief
     *   Initializes a TLVWriter object to write from a single TLVCircularBuffer
     *
     * Writing begins at the last byte of the buffer.  The number of bytes
     * to be written is not constrained by the underlying circular buffer:
     * writing new elements to the buffer will kick out previous elements
     * as long as an individual top-level TLV structure fits within the
     * buffer.  For example, writing a 7-byte top-level boolean TLV into a
     * 7 byte buffer will work indefinitely, but writing an 8-byte TLV
     * structure will result in an error.
     *
     * @param[in]    buf   A pointer to a fully initialized TLVCircularBuffer
     *
     */
    void Init(TLVCircularBuffer & buf) { TLVWriter::Init(buf, UINT32_MAX); }
};

} // namespace TLV
} // namespace chip
