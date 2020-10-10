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
 *  @file
 *      This file defines the circular buffer for TLV
 *      elements. When used as the backing store for the TLVReader and
 *      TLVWriter, those classes will work with the wraparound of data
 *      within the buffer.  Additionally, the TLVWriter will be able
 *      to continually add top-level TLV elements by evicting
 *      pre-existing elements.
 */

#pragma once

#include <core/CHIPError.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVTags.h>
#include <core/CHIPTLVTypes.h>

#include <support/DLLUtil.h>

#include <stdlib.h>

namespace chip {
namespace TLV {

/**
 * @class CHIPCircularTLVBuffer
 *
 * @brief
 *    CHIPCircularTLVBuffer provides circular storage for the
 *    chip::TLV::TLVWriter and chip::TLVTLVReader.  chip::TLV::TLVWriter is able to write an
 *    unbounded number of TLV entries to the CHIPCircularTLVBuffer
 *    as long as each individual TLV entry fits entirely within the
 *    provided storage.  The chip::TLV::TLVReader will read at most the size of
 *    the buffer, but will accommodate the wraparound within the
 *    buffer.
 *
 */
class DLL_EXPORT CHIPCircularTLVBuffer
{
public:
    CHIPCircularTLVBuffer(uint8_t * inBuffer, size_t inBufferLength);
    CHIPCircularTLVBuffer(uint8_t * inBuffer, size_t inBufferLength, uint8_t * inHead);

    CHIP_ERROR GetNewBuffer(TLVWriter & ioWriter, uint8_t *& outBufStart, uint32_t & outBufLen);
    CHIP_ERROR FinalizeBuffer(TLVWriter & ioWriter, uint8_t * inBufStart, uint32_t inBufLen);
    CHIP_ERROR GetNextBuffer(TLVReader & ioReader, const uint8_t *& outBufStart, uint32_t & outBufLen);

    inline uint8_t * QueueHead() const { return mQueueHead; }
    inline uint8_t * QueueTail() const { return mQueue + (((mQueueHead - mQueue) + mQueueLength) % mQueueSize); }
    inline size_t DataLength() const { return mQueueLength; }
    inline size_t AvailableDataLength() const { return mQueueSize - mQueueLength; }
    inline size_t GetQueueSize() const { return mQueueSize; }
    inline uint8_t * GetQueue() const { return mQueue; }

    CHIP_ERROR EvictHead();

    static CHIP_ERROR GetNewBufferFunct(TLVWriter & ioWriter, uintptr_t & inBufHandle, uint8_t *& outBufStart,
                                        uint32_t & outBufLen);
    static CHIP_ERROR FinalizeBufferFunct(TLVWriter & ioWriter, uintptr_t inBufHandle, uint8_t * inBufStart, uint32_t inBufLen);
    static CHIP_ERROR GetNextBufferFunct(TLVReader & ioReader, uintptr_t & inBufHandle, const uint8_t *& outBufStart,
                                         uint32_t & outBufLen);

    /**
     *  @typedef CHIP_ERROR (*ProcessEvictedElementFunct)(CHIPCircularTLVBuffer &inBuffer, void * inAppData, TLVReader &inReader)
     *
     *  A function that is called to process a TLV element prior to it
     *  being evicted from the chip::TLV::CHIPCircularTLVBuffer
     *
     *  Functions of this type are used to process a TLV element about
     *  to be evicted from the buffer.  The function will be given a
     *  chip::TLV::TLVReader positioned on the element about to be deleted, as
     *  well as void * context where the user may have provided
     *  additional environment for the callback.  If the function
     *  processed the element successfully, it must return
     *  #CHIP_NO_ERROR ; this signifies to the CHIPCircularTLVBuffer
     *  that the element may be safely evicted.  Any other return
     *  value is treated as an error and will prevent the
     *  #CHIPCircularTLVBuffer from evicting the element under
     *  consideration.
     *
     *  Note: This callback may be used to force
     *  CHIPCircularTLVBuffer to not evict the element.  This may be
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
    typedef CHIP_ERROR (*ProcessEvictedElementFunct)(CHIPCircularTLVBuffer & inBuffer, void * inAppData, TLVReader & inReader);

    uint32_t mImplicitProfileId;
    void * mAppData; /**< An optional, user supplied context to be used with the callback processing the evicted element. */
    ProcessEvictedElementFunct
        mProcessEvictedElement; /**< An optional, user-supplied callback that processes the element prior to evicting it from the
                                   circular buffer.  See the ProcessEvictedElementFunct type definition on additional information on
                                   implementing the mProcessEvictedElement function. */

private:
    uint8_t * mQueue;
    size_t mQueueSize;
    uint8_t * mQueueHead;
    size_t mQueueLength;
};

class DLL_EXPORT CircularTLVReader : public TLVReader
{
public:
    void Init(CHIPCircularTLVBuffer * buf);
};

class DLL_EXPORT CircularTLVWriter : public TLVWriter
{
public:
    void Init(CHIPCircularTLVBuffer * buf);
};

} // namespace TLV
} // namespace chip
