/*
 *
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
 *      This file defines the member functions and private data for
 *      the nl::Weave::System::PacketBuffer class, which provides the
 *      mechanisms for manipulating packets of octet-serialized
 *      data.
 */

// Include standard C library limit macros
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>

// Include module header
#include <SystemLayer/SystemPacketBuffer.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <SystemLayer/SystemMutex.h>
#include <SystemLayer/SystemFaultInjection.h>

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#include <lwip/pbuf.h>
#include <lwip/mem.h>
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#include <Weave/Support/logging/WeaveLogging.h>
#include <Weave/Support/crypto/WeaveCrypto.h>
#include <Weave/Support/CodeUtils.h>

#include <SystemLayer/SystemStats.h>

namespace nl {
namespace Weave {
namespace System {

//
// Pool allocation for PacketBuffer objects (toll-free bridged with LwIP pbuf allocator if WEAVE_SYSTEM_CONFIG_USE_LWIP)
//
#if !WEAVE_SYSTEM_CONFIG_USE_LWIP
#if WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

static BufferPoolElement sBufferPool[WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC];

PacketBuffer* PacketBuffer::sFreeList = PacketBuffer::BuildFreeList();

#if !WEAVE_SYSTEM_CONFIG_NO_LOCKING
static Mutex sBufferPoolMutex;

#define LOCK_BUF_POOL()     do { sBufferPoolMutex.Lock(); } while (0)
#define UNLOCK_BUF_POOL()   do { sBufferPoolMutex.Unlock(); } while (0)
#endif // !WEAVE_SYSTEM_CONFIG_NO_LOCKING

#endif // WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

#ifndef LOCK_BUF_POOL
#define LOCK_BUF_POOL()     do { } while (0)
#endif // !defined(LOCK_BUF_POOL)

#ifndef UNLOCK_BUF_POOL
#define UNLOCK_BUF_POOL()   do { } while (0)
#endif // !defined(UNLOCK_BUF_POOL)

#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP

/**
 * Get pointer to start of data in buffer.
 *
 *  @return pointer to the start of data.
 */
uint8_t* PacketBuffer::Start() const
{
    return static_cast<uint8_t*>(this->payload);
}

/**
 *  Set the start data in buffer, adjusting length and total length accordingly.
 *
 *  @note The data within the buffer is not moved, only accounting information is changed.  The function is commonly used to either
 *  strip or prepend protocol headers in a zero-copy way.
 *
 *  @note This call should not be used on any buffer that is not the head of a buffer chain, as it only alters the current buffer.
 *
 *  @param[in] aNewStart - A pointer to where the new payload should start.  newStart will be adjusted internally to fall within
 *      the boundaries of the first buffer in the PacketBuffer chain.
 */
void PacketBuffer::SetStart(uint8_t* aNewStart)
{
    uint8_t* const kStart = reinterpret_cast<uint8_t*>(this) + WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE;
    uint8_t* const kEnd = kStart + this->AllocSize();

    if (aNewStart < kStart)
        aNewStart = kStart;
    else if (aNewStart > kEnd)
        aNewStart = kEnd;

    ptrdiff_t lDelta = aNewStart - static_cast<uint8_t*>(this->payload);
    if (lDelta > this->len)
        lDelta = this->len;

    this->len = static_cast<uint16_t>(static_cast<ptrdiff_t>(this->len) - lDelta);
    this->tot_len = static_cast<uint16_t>(static_cast<ptrdiff_t>(this->tot_len) - lDelta);
    this->payload = aNewStart;
}

/**
 * Get length, in bytes, of data in packet buffer.
 *
 *  @return length, in bytes (current payload length).
 */
uint16_t PacketBuffer::DataLength() const
{
    return this->len;
}

/**
 * Set length, in bytes, of data in buffer, adjusting total length accordingly.
 *
 *  The function sets the length, in bytes, of the data in the buffer, adjusting the total length appropriately.  When the buffer
 *  is not the head of the buffer chain (common case: the caller adds data to the last buffer in the PacketBuffer chain prior to
 *  calling higher layers), the aChainHead __must__ be passed in to properly adjust the total lengths of each buffer ahead of the
 *  current buffer.
 *
 *  @param[in] aNewLen - new length, in bytes, of this buffer.
 *
 *  @param[inout] aChainHead - the head of the buffer chain the current buffer belongs to.  May be NULL if the current buffer is
 *      the head of the buffer chain.
 */
void PacketBuffer::SetDataLength(uint16_t aNewLen, PacketBuffer* aChainHead)
{
    const uint16_t kMaxDataLen = this->MaxDataLength();

    if (aNewLen > kMaxDataLen)
        aNewLen = kMaxDataLen;

    ptrdiff_t lDelta = static_cast<ptrdiff_t>(aNewLen) - static_cast<ptrdiff_t>(this->len);

    this->len = aNewLen;
    this->tot_len = (uint16_t) (this->tot_len + lDelta);

    while (aChainHead != NULL && aChainHead != this)
    {
        aChainHead->tot_len = static_cast<uint16_t>(aChainHead->tot_len + lDelta);
        aChainHead = static_cast<PacketBuffer*>(aChainHead->next);
    }
}

/**
 * Get total length of packet data in the buffer chain.
 *
 * @return total length, in octets.
 */
uint16_t PacketBuffer::TotalLength() const
{
    return this->tot_len;
}

/**
 * Get the maximum amount, in bytes, of data that will fit in the buffer given the current start position and buffer size.
 *
 * @return number of bytes that fits in the buffer given the current start position.
 */
uint16_t PacketBuffer::MaxDataLength() const
{
    const uint8_t* const kStart = reinterpret_cast<const uint8_t*>(this) + WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE;
    const ptrdiff_t kDelta = static_cast<uint8_t*>(this->payload) - kStart;
    return static_cast<uint16_t>(this->AllocSize() - kDelta);
}

/**
 * Get the number of bytes of data that can be added to the current buffer given the current start position and data length.
 *
 * @return the length, in bytes, of data that will fit in the current buffer given the current start position and data length.
 */
uint16_t PacketBuffer::AvailableDataLength() const
{
    return this->MaxDataLength() - this->len;
}

/**
 * Get the number of bytes within the current buffer between the start of the buffer and the current data start position.
 *
 * @return the amount, in bytes, of space between the start of the buffer and the current data start position.
 */
uint16_t PacketBuffer::ReservedSize() const
{
    const ptrdiff_t kDelta = static_cast<uint8_t*>(this->payload) - reinterpret_cast<const uint8_t*>(this);
    return static_cast<uint16_t>(kDelta - WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE);
}

/**
 *
 * Add the given packet buffer to the end of the buffer chain, adjusting the total length of each buffer in the chain accordingly.
 *
 *  @note The current packet buffer must be the head of the buffer chain for the lengths to be adjusted properly. The caller MUST
 *  NOT reference the given packet buffer afterwards.
 *
 * @param[in] aPacket - the packet buffer to be added to the end of the current chain.
 */
void PacketBuffer::AddToEnd(PacketBuffer* aPacket)
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    pbuf_cat(this, aPacket);
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
    PacketBuffer* lCursor = this;

    while (true)
    {
        lCursor->tot_len += aPacket->tot_len;
        if (lCursor->next == NULL)
        {
            lCursor->next = aPacket;
            break;
        }

        lCursor = static_cast<PacketBuffer*>(lCursor->next);
    }
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 *  Detach the current buffer from its chain and return a pointer to the remaining buffers.  The current buffer must be the head of
 *  the chain.
 *
 *  @return the tail of the current buffer chain or NULL if the current buffer is the only buffer in the chain.
 */
PacketBuffer* PacketBuffer::DetachTail()
{
    PacketBuffer& lReturn = *static_cast<PacketBuffer*>(this->next);

    this->next = NULL;
    this->tot_len = this->len;

    return &lReturn;
}

/**
 * Move data from subsequent buffers in the chain into the current buffer until it is full.
 *
 *  Only the current buffer is compacted: the data within the current buffer is moved to the front of the buffer, eliminating any
 *  reserved space.  The remaining available space is filled with data moved from subsequent buffers in the chain, until the
 *  current buffer is full.  If a subsequent buffer in the chain is moved into the current buffer in its entirety, it is removed
 *  from the chain and freed.  The method takes no parameters, returns no results and cannot fail.
 */
void PacketBuffer::CompactHead()
{
    uint8_t* const kStart = reinterpret_cast<uint8_t*>(this) + WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE;

    if (this->payload != kStart)
    {
        memmove(kStart, this->payload, this->len);
        this->payload = kStart;
    }

    uint16_t lAvailLength = this->AvailableDataLength();

    while (lAvailLength > 0 && this->next != NULL)
    {
        PacketBuffer& lNextPacket = *static_cast<PacketBuffer*>(this->next);
        VerifyOrDieWithMsg(lNextPacket.ref == 1, WeaveSystemLayer, "next buffer %p is not exclusive to this chain", &lNextPacket);

        uint16_t lMoveLength = lNextPacket.len;
        if (lMoveLength > lAvailLength)
            lMoveLength = lAvailLength;

        memcpy(static_cast<uint8_t*>(this->payload) + this->len, lNextPacket.payload, lMoveLength);

        lNextPacket.payload = (uint8_t *) lNextPacket.payload + lMoveLength;
        this->len += lMoveLength;
        lAvailLength -= lMoveLength;
        lNextPacket.len -= lMoveLength;
        lNextPacket.tot_len -= lMoveLength;

        if (lNextPacket.len == 0)
            this->next = this->FreeHead(&lNextPacket);
    }
}

/**
 * Adjust the current buffer to indicate the amount of data consumed.
 *
 *  Advance the data start position in the current buffer by the specified amount, in bytes, up to the length of data in the
 *  buffer. Decrease the length and total length by the amount consumed.
 *
 *  @param[in] aConsumeLen - number of bytes to consume from the current buffer.
 */
void PacketBuffer::ConsumeHead(uint16_t aConsumeLength)
{
    if (aConsumeLength > this->len)
        aConsumeLength = this->len;
    this->payload = static_cast<uint8_t*>(this->payload) + aConsumeLength;
    this->len -= aConsumeLength;
    this->tot_len -= aConsumeLength;
}

/**
 * Consume data in a chain of buffers.
 *
 *  Consume data in a chain of buffers starting with the current buffer and proceeding through the remaining buffers in the chain.
 *  Each buffer that is completely consumed is freed and the function returns the first buffer (if any) containing the remaining
 *  data. The current buffer must be the head of the buffer chain.
 *
 *  @param[in] aConsumeLength - number of bytes to consume from the current chain.
 *
 *  @return the first buffer from the current chain that contains any remaining data.  If no data remains, a NULL is returned.
 */
PacketBuffer* PacketBuffer::Consume(uint16_t aConsumeLength)
{
    PacketBuffer* lPacket = this;

    while (lPacket != NULL && aConsumeLength > 0)
    {
        const uint16_t kLength = lPacket->DataLength();

        if (aConsumeLength >= kLength)
        {
            lPacket = PacketBuffer::FreeHead(lPacket);
            aConsumeLength -= kLength;
        }
        else
        {
            lPacket->ConsumeHead(aConsumeLength);
            break;
        }
    }

    return lPacket;
}

/**
 * Ensure the buffer has at least the specified amount of reserved space.
 *
 *  Ensure the buffer has at least the specified amount of reserved space moving the data in the buffer forward to make room if
 *  necessary.
 *
 *  @param[in] aReservedSize - number of bytes desired for the headers.
 *
 *  @return \c true if the requested reserved size is available, \c false if there's not enough room in the buffer.
 */
bool PacketBuffer::EnsureReservedSize(uint16_t aReservedSize)
{
    const uint16_t kCurrentReservedSize = this->ReservedSize();
    if (aReservedSize <= kCurrentReservedSize)
        return true;

    if ((aReservedSize + this->len) > this->AllocSize())
        return false;

    const uint16_t kMoveLength = aReservedSize - kCurrentReservedSize;
    memmove(static_cast<uint8_t*>(this->payload) + kMoveLength, this->payload, this->len);
    payload = static_cast<uint8_t*>(this->payload) + kMoveLength;

    return true;
}

/**
 * Align the buffer payload on the specified bytes boundary.
 *
 *  Moving the payload in the buffer forward if necessary.
 *
 *  @param[in] aAlignBytes - specifies number of bytes alignment for the payload start pointer.
 *
 *  @return \c true if alignment is successful, \c false if there's not enough room in the buffer.
 */
bool PacketBuffer::AlignPayload(uint16_t aAlignBytes)
{
    if (aAlignBytes == 0)
        return false;

    const uint16_t kPayloadOffset = reinterpret_cast<uintptr_t>(this->payload) % aAlignBytes;

    if (kPayloadOffset == 0)
        return true;

    const uint16_t kPayloadShift = aAlignBytes - kPayloadOffset;

    return (this->EnsureReservedSize(this->ReservedSize() + kPayloadShift));
}

/**
 * Get pointer to next buffer in chain.
 *
 *  @return a pointer to the next buffer in the chain.  \c NULL is returned when there is no buffers in the chain.
 */
PacketBuffer* PacketBuffer::Next() const
{
    return static_cast<PacketBuffer*>(this->next);
}

/**
 * Increment the reference count of the current buffer.
 */
void PacketBuffer::AddRef()
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    pbuf_ref(this);
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
    LOCK_BUF_POOL();
    ++this->ref;
    UNLOCK_BUF_POOL();
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * Allocates a PacketBuffer object with at least \c aReservedSize bytes reserved in the payload for headers, and at least
 *  \c aAllocSize bytes of space for additional data after the initial cursor pointer.
 *
 *  @param[in]  aReservedSize   Number of octets to reserve behind the cursor.
 *  @param[in]  aAvailableSize  Number of octets to allocate after the cursor.
 *
 *  @return     On success, a pointer to the PacketBuffer in the allocated block. On fail, \c NULL.
 */
PacketBuffer* PacketBuffer::NewWithAvailableSize(uint16_t aReservedSize, size_t aAvailableSize)
{
    const size_t lReservedSize = static_cast<size_t>(aReservedSize);
    const size_t lAllocSize = lReservedSize + aAvailableSize;
    const size_t lBlockSize = WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE + lAllocSize;
    PacketBuffer* lPacket;

    WEAVE_SYSTEM_FAULT_INJECT(FaultInjection::kFault_PacketBufferNew, return NULL);

    if (lAllocSize > WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX)
    {
        WeaveLogError(WeaveSystemLayer, "PacketBuffer: allocation too large.");
        return NULL;
    }

#if WEAVE_SYSTEM_CONFIG_USE_LWIP

    lPacket = static_cast<PacketBuffer*>(pbuf_alloc(PBUF_RAW, lBlockSize, PBUF_POOL));

    SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();

#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
#if WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

    static_cast<void>(lBlockSize);

    LOCK_BUF_POOL();

    lPacket = sFreeList;
    if (lPacket != NULL)
    {
        sFreeList = static_cast<PacketBuffer*>(lPacket->next);
        SYSTEM_STATS_INCREMENT(nl::Weave::System::Stats::kSystemLayer_NumPacketBufs);
    }

    UNLOCK_BUF_POOL();

#else // !WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

    lPacket = reinterpret_cast<PacketBuffer*>(malloc(lBlockSize));
    SYSTEM_STATS_INCREMENT(nl::Weave::System::Stats::kSystemLayer_NumPacketBufs);

#endif // !WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP

    if (lPacket == NULL)
    {
        WeaveLogError(WeaveSystemLayer, "PacketBuffer: pool EMPTY.");
        return NULL;
    }

    lPacket->payload = reinterpret_cast<uint8_t*>(lPacket) + WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE + lReservedSize;
    lPacket->len = lPacket->tot_len = 0;
    lPacket->next = NULL;
    lPacket->ref = 1;
#if WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    lPacket->alloc_size = lAllocSize;
#endif // WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0

    return lPacket;
}

/**
 * Allocates a PacketBuffer with default reserved size (#WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) in the payload for headers,
 * and at least \c aAllocSize bytes of space for additional data after the initial cursor pointer.
 *
 * This usage is most appropriate when allocating a PacketBuffer for an application-layer message.
 *
 *  @param[in]  aAvailableSize  Number of octets to allocate after the cursor.
 *
 *  @return     On success, a pointer to the PacketBuffer in the allocated block. On fail, \c NULL. *
 */
PacketBuffer* PacketBuffer::NewWithAvailableSize(size_t aAvailableSize)
{
    return PacketBuffer::NewWithAvailableSize(WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE, aAvailableSize);
}

/**
 * Allocates a single PacketBuffer of maximum total size with a specific header reserve size.
 *
 *  The parameter passed in is the size reserved prior to the payload to accomodate packet headers from different stack layers,
 *  __not__ the overall size of the buffer to allocate. The size of the buffer #WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
 *  and not, specified in the call.
 *
 * - `PacketBuffer::New(0)` : when called in this fashion, the buffer will be returned without any header reserved, consequently
 *      the entire payload is usable by the caller. This pattern is particularly useful at the lower layers of networking stacks,
 *      in cases where the user knows the payload will be copied out into the final message with appropriate header reserves or in
 *      creating PacketBuffer that are appended to a chain of PacketBuffer via `PacketBuffer::AddToEnd()`.
 *
 *  @param[in] aReservedSize  amount of header space to reserve.
 *
 *  @return On success, a pointer to the PacketBuffer, on failure \c NULL.
 */
PacketBuffer* PacketBuffer::New(uint16_t aReservedSize)
{
    const size_t lReservedSize = static_cast<size_t>(aReservedSize);

    const size_t lAvailableSize =
        lReservedSize < WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX ? WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX - lReservedSize : 0;

    return PacketBuffer::NewWithAvailableSize(aReservedSize, lAvailableSize);
}

/**
 * Allocates a single PacketBuffer of default max size (#WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX) with default reserved size
 * (#WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) in the payload.
 *
 * The reserved size (#WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) is large enough to hold transport layer headers as well as headers required by
 * \c WeaveMessageLayer and \c WeaveExchangeLayer.
 */
PacketBuffer* PacketBuffer::New(void)
{
    return PacketBuffer::New(WEAVE_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);
}

/**
 * Free all packet buffers in a chain.
 *
 *  Decrement the reference count to all the buffers in the current chain. If the reference count reaches 0, the respective buffers
 *  are freed or returned to allocation pools as appropriate. As a rule, users should treat this method as an equivalent of
 *  `free()` function and not use the argument after the call.
 *
 *  @param[in] aPacket - packet buffer to be freed.
 */
void PacketBuffer::Free(PacketBuffer* aPacket)
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP

    if (aPacket != NULL)
    {
        pbuf_free(aPacket);

        SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();
    }

#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP

    LOCK_BUF_POOL();

    while (aPacket != NULL)
    {
        PacketBuffer* lNextPacket = static_cast<PacketBuffer*>(aPacket->next);

        VerifyOrDieWithMsg(aPacket->ref > 0, WeaveSystemLayer, "SystemPacketBuffer::Free: aPacket->ref = 0");

        aPacket->ref--;
        if (aPacket->ref == 0)
        {
            SYSTEM_STATS_DECREMENT(nl::Weave::System::Stats::kSystemLayer_NumPacketBufs);
            aPacket->Clear();
#if WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
            aPacket->next = sFreeList;
            sFreeList = aPacket;
#else // !WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
            free(aPacket);
#endif // !WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
            aPacket = lNextPacket;
        }
        else
        {
            aPacket = NULL;
        }
    }

    UNLOCK_BUF_POOL();

#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP
}

/**
 * Clear content of the packet buffer.
 *
 * This method is called by Free(), before the buffer is released to the free buffer pool.
 */
void PacketBuffer::Clear(void)
{
    nl::Weave::Crypto::ClearSecretData(reinterpret_cast<uint8_t*>(this) + WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE, this->AllocSize());
    tot_len = 0;
    len = 0;
#if WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    alloc_size = 0;
#endif // WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
}

/**
 * Free the first buffer in a chain, returning a pointer to the remaining buffers.
 `*
 *  @note When the buffer chain is referenced by multiple callers, `FreeHead()` will detach the head, but will not forcibly
 *  deallocate the head buffer.
 *
 *  @param[in] aPacket - buffer chain.
 *
 *  @return packet buffer chain consisting of the tail of the input buffer (may be \c NULL).
 */
PacketBuffer* PacketBuffer::FreeHead(PacketBuffer* aPacket)
{
    PacketBuffer* lNextPacket = static_cast<PacketBuffer*>(aPacket->next);
    aPacket->next = NULL;
    PacketBuffer::Free(aPacket);
    return lNextPacket;
}

/**
 * Copy the given buffer to a right-sized buffer if applicable.
 * This function is a no-op for sockets.
 *
 *  @param[in] aPacket - buffer or buffer chain.
 *
 *  @return new packet buffer or the original buffer
 */
PacketBuffer* PacketBuffer::RightSize(PacketBuffer *aPacket)
{
    PacketBuffer *lNewPacket = aPacket;
#if WEAVE_SYSTEM_CONFIG_USE_LWIP && LWIP_PBUF_FROM_CUSTOM_POOLS
    lNewPacket =  static_cast<PacketBuffer *>(pbuf_rightsize((struct pbuf *)aPacket, -1));
    if (lNewPacket != aPacket)
    {
        SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();

        WeaveLogProgress(WeaveSystemLayer, "PacketBuffer: RightSize Copied");
    }
#endif
    return lNewPacket;
}

#if !WEAVE_SYSTEM_CONFIG_USE_LWIP && WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

PacketBuffer* PacketBuffer::BuildFreeList()
{
    PacketBuffer* lHead = NULL;

    for (int i = 0; i < WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC; i++)
    {
        PacketBuffer* lCursor = &sBufferPool[i].Header;
        lCursor->next = lHead;
        lCursor->ref = 0;
        lHead = lCursor;
    }

    Mutex::Init(sBufferPoolMutex);

    return lHead;
}

#endif //  !WEAVE_SYSTEM_CONFIG_USE_LWIP && WEAVE_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

} // namespace System
} // namespace Weave
} // namespace nl
