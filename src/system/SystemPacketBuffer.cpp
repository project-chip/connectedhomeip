/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      the chip::System::PacketBuffer class, which provides the
 *      mechanisms for manipulating packets of octet-serialized
 *      data.
 */
// Include standard C library limit macros
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

// Include module header
#include <system/SystemPacketBuffer.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemMutex.h>
#include <system/SystemStats.h>

#include <stdint.h>

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace System {

//
// Pool allocation for PacketBuffer objects (toll-free bridged with LwIP pbuf allocator if CHIP_SYSTEM_CONFIG_USE_LWIP)
//
#if !CHIP_SYSTEM_CONFIG_USE_LWIP
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

static BufferPoolElement sBufferPool[CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC];

PacketBuffer * PacketBuffer::sFreeList = PacketBuffer::BuildFreeList();

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
static Mutex sBufferPoolMutex;

#define LOCK_BUF_POOL()                                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        sBufferPoolMutex.Lock();                                                                                                   \
    } while (0)
#define UNLOCK_BUF_POOL()                                                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        sBufferPoolMutex.Unlock();                                                                                                 \
    } while (0)
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING

#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

#ifndef LOCK_BUF_POOL
#define LOCK_BUF_POOL()                                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (0)
#endif // !defined(LOCK_BUF_POOL)

#ifndef UNLOCK_BUF_POOL
#define UNLOCK_BUF_POOL()                                                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (0)
#endif // !defined(UNLOCK_BUF_POOL)

#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP

void PacketBuffer::SetStart(uint8_t * aNewStart)
{
    uint8_t * const kStart = reinterpret_cast<uint8_t *>(this) + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
    uint8_t * const kEnd   = kStart + this->AllocSize();

    if (aNewStart < kStart)
        aNewStart = kStart;
    else if (aNewStart > kEnd)
        aNewStart = kEnd;

    ptrdiff_t lDelta = aNewStart - static_cast<uint8_t *>(this->payload);
    if (lDelta > this->len)
        lDelta = this->len;

    this->len     = static_cast<uint16_t>(static_cast<ptrdiff_t>(this->len) - lDelta);
    this->tot_len = static_cast<uint16_t>(static_cast<ptrdiff_t>(this->tot_len) - lDelta);
    this->payload = aNewStart;
}

void PacketBuffer::SetDataLength(uint16_t aNewLen, PacketBuffer * aChainHead)
{
    const uint16_t kMaxDataLen = this->MaxDataLength();

    if (aNewLen > kMaxDataLen)
        aNewLen = kMaxDataLen;

    ptrdiff_t lDelta = static_cast<ptrdiff_t>(aNewLen) - static_cast<ptrdiff_t>(this->len);

    this->len     = aNewLen;
    this->tot_len = static_cast<uint16_t>(this->tot_len + lDelta);

    while (aChainHead != nullptr && aChainHead != this)
    {
        aChainHead->tot_len = static_cast<uint16_t>(aChainHead->tot_len + lDelta);
        aChainHead          = static_cast<PacketBuffer *>(aChainHead->next);
    }
}

uint16_t PacketBuffer::MaxDataLength() const
{
    const uint8_t * const kStart = reinterpret_cast<const uint8_t *>(this) + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
    const ptrdiff_t kDelta       = static_cast<uint8_t *>(this->payload) - kStart;
    return static_cast<uint16_t>(this->AllocSize() - kDelta);
}

uint16_t PacketBuffer::AvailableDataLength() const
{
    return static_cast<uint16_t>(this->MaxDataLength() - this->DataLength());
}

uint16_t PacketBuffer::ReservedSize() const
{
    // Cast to size_t is safe because this->payload always points to "after"
    // this.
    const size_t kDelta = static_cast<size_t>(static_cast<uint8_t *>(this->payload) - reinterpret_cast<const uint8_t *>(this));
    return static_cast<uint16_t>(kDelta - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE);
}

void PacketBuffer::AddToEnd(PacketBufferHandle && aPacketHandle)
{
    // Ownership of aPacketHandle's buffer is transferred to the end of the chain.
    PacketBuffer * aPacket = std::move(aPacketHandle).UnsafeRelease();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    pbuf_cat(this, aPacket);
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
    PacketBuffer * lCursor = this;

    while (true)
    {
        lCursor->tot_len = static_cast<uint16_t>(lCursor->tot_len + aPacket->tot_len);
        if (lCursor->next == nullptr)
        {
            lCursor->next = aPacket;
            break;
        }

        lCursor = static_cast<PacketBuffer *>(lCursor->next);
    }
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

void PacketBuffer::CompactHead()
{
    uint8_t * const kStart = reinterpret_cast<uint8_t *>(this) + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;

    if (this->payload != kStart)
    {
        memmove(kStart, this->payload, this->len);
        this->payload = kStart;
    }

    uint16_t lAvailLength = this->AvailableDataLength();

    while (lAvailLength > 0 && this->next != nullptr)
    {
        PacketBuffer & lNextPacket = *static_cast<PacketBuffer *>(this->next);
        VerifyOrDieWithMsg(lNextPacket.ref == 1, chipSystemLayer, "next buffer %p is not exclusive to this chain", &lNextPacket);

        uint16_t lMoveLength = lNextPacket.len;
        if (lMoveLength > lAvailLength)
            lMoveLength = lAvailLength;

        memcpy(static_cast<uint8_t *>(this->payload) + this->len, lNextPacket.payload, lMoveLength);

        lNextPacket.payload = static_cast<uint8_t *>(lNextPacket.payload) + lMoveLength;
        this->len           = static_cast<uint16_t>(this->len + lMoveLength);
        lAvailLength        = static_cast<uint16_t>(lAvailLength - lMoveLength);
        lNextPacket.len     = static_cast<uint16_t>(lNextPacket.len - lMoveLength);
        lNextPacket.tot_len = static_cast<uint16_t>(lNextPacket.tot_len - lMoveLength);

        if (lNextPacket.len == 0)
            this->next = this->FreeHead(&lNextPacket);
    }
}

void PacketBuffer::ConsumeHead(uint16_t aConsumeLength)
{
    if (aConsumeLength > this->len)
        aConsumeLength = this->len;
    this->payload = static_cast<uint8_t *>(this->payload) + aConsumeLength;
    this->len     = static_cast<uint16_t>(this->len - aConsumeLength);
    this->tot_len = static_cast<uint16_t>(this->tot_len - aConsumeLength);
}

/**
 * Consume data in a chain of buffers.
 *
 *  Consume data in a chain of buffers starting with the current buffer and proceeding through the remaining buffers in the
 * chain. Each buffer that is completely consumed is freed and the function returns the first buffer (if any) containing the
 * remaining data. The current buffer must be the head of the buffer chain.
 *
 *  @param[in] aConsumeLength - number of bytes to consume from the current chain.
 *
 *  @return the first buffer from the current chain that contains any remaining data.  If no data remains, nullptr is returned.
 */
PacketBuffer * PacketBuffer::Consume(uint16_t aConsumeLength)
{
    PacketBuffer * lPacket = this;

    while (lPacket != nullptr && aConsumeLength > 0)
    {
        const uint16_t kLength = lPacket->DataLength();

        if (aConsumeLength >= kLength)
        {
            lPacket        = PacketBuffer::FreeHead(lPacket);
            aConsumeLength = static_cast<uint16_t>(aConsumeLength - kLength);
        }
        else
        {
            lPacket->ConsumeHead(aConsumeLength);
            break;
        }
    }

    return lPacket;
}

bool PacketBuffer::EnsureReservedSize(uint16_t aReservedSize)
{
    const uint16_t kCurrentReservedSize = this->ReservedSize();
    if (aReservedSize <= kCurrentReservedSize)
        return true;

    if ((aReservedSize + this->len) > this->AllocSize())
        return false;

    // Cast is safe because aReservedSize > kCurrentReservedSize.
    const uint16_t kMoveLength = static_cast<uint16_t>(aReservedSize - kCurrentReservedSize);
    memmove(static_cast<uint8_t *>(this->payload) + kMoveLength, this->payload, this->len);
    payload = static_cast<uint8_t *>(this->payload) + kMoveLength;

    return true;
}

bool PacketBuffer::AlignPayload(uint16_t aAlignBytes)
{
    if (aAlignBytes == 0)
        return false;

    const uint16_t kPayloadOffset = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(this->payload) % aAlignBytes);

    if (kPayloadOffset == 0)
        return true;

    // Cast is safe because by construction kPayloadOffset < aAlignBytes.
    const uint16_t kPayloadShift = static_cast<uint16_t>(aAlignBytes - kPayloadOffset);

    if (!CanCastTo<uint16_t>(this->ReservedSize() + kPayloadShift))
    {
        return false;
    }

    return (this->EnsureReservedSize(static_cast<uint16_t>(this->ReservedSize() + kPayloadShift)));
}

/**
 * Increment the reference count of the current buffer.
 */
void PacketBuffer::AddRef()
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    pbuf_ref(this);
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
    LOCK_BUF_POOL();
    ++this->ref;
    UNLOCK_BUF_POOL();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

PacketBufferHandle PacketBuffer::NewWithAvailableSize(uint16_t aReservedSize, uint16_t aAvailableSize)
{
    // Adding three 16-bit-int sized numbers together will never overflow
    // assuming int is at least 32 bits.
    static_assert(INT_MAX >= INT32_MAX, "int is not big enough");
    static_assert(CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE < UINT16_MAX, "Check for overflow more carefully");
    static_assert(SIZE_MAX >= INT_MAX, "Our additions might not fit in size_t");
    static_assert(CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX <= UINT16_MAX, "PacketBuffer may have size not fitting uint16_t");

    const size_t lAllocSize = aReservedSize + aAvailableSize;
    const size_t lBlockSize = CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE + lAllocSize;
    PacketBuffer * lPacket;

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_PacketBufferNew, return PacketBufferHandle());

    if (lAllocSize > CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX || lBlockSize > UINT16_MAX)
    {
        ChipLogError(chipSystemLayer, "PacketBuffer: allocation too large.");
        return PacketBufferHandle();
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    lPacket = static_cast<PacketBuffer *>(pbuf_alloc(PBUF_RAW, static_cast<uint16_t>(lBlockSize), PBUF_POOL));

    SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();

#else // !CHIP_SYSTEM_CONFIG_USE_LWIP
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

    static_cast<void>(lBlockSize);

    LOCK_BUF_POOL();

    lPacket = sFreeList;
    if (lPacket != nullptr)
    {
        sFreeList = static_cast<PacketBuffer *>(lPacket->next);
        SYSTEM_STATS_INCREMENT(chip::System::Stats::kSystemLayer_NumPacketBufs);
    }

    UNLOCK_BUF_POOL();

#else // !CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

    lPacket = reinterpret_cast<PacketBuffer *>(chip::Platform::MemoryAlloc(lBlockSize));
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kSystemLayer_NumPacketBufs);

#endif // !CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP

    if (lPacket == nullptr)
    {
        ChipLogError(chipSystemLayer, "PacketBuffer: pool EMPTY.");
        return PacketBufferHandle();
    }

    lPacket->payload = reinterpret_cast<uint8_t *>(lPacket) + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE + aReservedSize;
    lPacket->len = lPacket->tot_len = 0;
    lPacket->next                   = nullptr;
    lPacket->ref                    = 1;
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    lPacket->alloc_size = static_cast<uint16_t>(lAllocSize);
#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0

    return PacketBufferHandle(lPacket);
}

PacketBufferHandle PacketBuffer::NewWithAvailableSize(uint16_t aAvailableSize)
{
    return PacketBuffer::NewWithAvailableSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE, aAvailableSize);
}

PacketBufferHandle PacketBuffer::New(uint16_t aReservedSize)
{
    static_assert(CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX <= UINT16_MAX, "Our available size won't fit in uint16_t");
    const uint16_t lAvailableSize = aReservedSize < CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
        ? static_cast<uint16_t>(CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX - aReservedSize)
        : 0;

    return PacketBuffer::NewWithAvailableSize(aReservedSize, lAvailableSize);
}

PacketBufferHandle PacketBuffer::New()
{
    return PacketBuffer::New(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);
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
void PacketBuffer::Free(PacketBuffer * aPacket)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP

    if (aPacket != nullptr)
    {
        pbuf_free(aPacket);

        SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();
    }

#else // !CHIP_SYSTEM_CONFIG_USE_LWIP

    LOCK_BUF_POOL();

    while (aPacket != nullptr)
    {
        PacketBuffer * lNextPacket = static_cast<PacketBuffer *>(aPacket->next);

        VerifyOrDieWithMsg(aPacket->ref > 0, chipSystemLayer, "SystemPacketBuffer::Free: aPacket->ref = 0");

        aPacket->ref--;
        if (aPacket->ref == 0)
        {
            SYSTEM_STATS_DECREMENT(chip::System::Stats::kSystemLayer_NumPacketBufs);
            aPacket->Clear();
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
            aPacket->next = sFreeList;
            sFreeList     = aPacket;
#else  // !CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
            chip::Platform::MemoryFree(aPacket);
#endif // !CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
            aPacket       = lNextPacket;
        }
        else
        {
            aPacket = nullptr;
        }
    }

    UNLOCK_BUF_POOL();

#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * Clear content of the packet buffer.
 *
 * This method is called by Free(), before the buffer is released to the free buffer pool.
 */
void PacketBuffer::Clear()
{
    tot_len = 0;
    len     = 0;
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    alloc_size = 0;
#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
}

/**
 * Free the first buffer in a chain, returning a pointer to the remaining buffers.
 `*
 *  @note When the buffer chain is referenced by multiple callers, `FreeHead()` will detach the head, but will not forcibly
 *  deallocate the head buffer.
 *
 *  @param[in] aPacket - buffer chain.
 *
 *  @return packet buffer chain consisting of the tail of the input buffer (may be \c nullptr).
 */
PacketBuffer * PacketBuffer::FreeHead(PacketBuffer * aPacket)
{
    PacketBuffer * lNextPacket = static_cast<PacketBuffer *>(aPacket->next);
    aPacket->next              = nullptr;
    PacketBuffer::Free(aPacket);
    return lNextPacket;
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_PBUF_FROM_CUSTOM_POOLS
PacketBuffer * PacketBuffer::RightSize(PacketBuffer * aPacket)
{
    PacketBuffer * lNewPacket = static_cast<PacketBuffer *>(pbuf_rightsize((struct pbuf *) aPacket, -1));
    if (lNewPacket != aPacket)
    {
        SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();

        ChipLogProgress(chipSystemLayer, "PacketBuffer: RightSize Copied");
    }
    return lNewPacket;
}
#endif

#if !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

PacketBuffer * PacketBuffer::BuildFreeList()
{
    PacketBuffer * lHead = nullptr;

    for (int i = 0; i < CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC; i++)
    {
        PacketBuffer * lCursor = &sBufferPool[i].Header;
        lCursor->next          = lHead;
        lCursor->ref           = 0;
        lHead                  = lCursor;
    }

    Mutex::Init(sBufferPoolMutex);

    return lHead;
}

#endif //  !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

PacketBufferHandle PacketBufferHandle::PopHead()
{
    PacketBuffer * head = mBuffer;

    // This takes ownership from the `next` link.
    mBuffer = static_cast<PacketBuffer *>(mBuffer->next);

    head->next    = nullptr;
    head->tot_len = head->len;

    // The returned handle takes ownership from this.
    return PacketBufferHandle(head);
}

PacketBufferHandle PacketBufferHandle::CloneData()
{
    if (!mBuffer->Next().IsNull())
    {
        // We do not clone an entire chain.
        return PacketBufferHandle();
    }

    PacketBufferHandle other = PacketBuffer::New();
    if (other.IsNull())
    {
        return other;
    }

    if (other->AvailableDataLength() < mBuffer->DataLength())
    {
        other = nullptr;
        return other;
    }

    memcpy(other->Start(), mBuffer->Start(), mBuffer->DataLength());
    other->SetDataLength(mBuffer->DataLength());

    return other;
}

} // namespace System
} // namespace chip
