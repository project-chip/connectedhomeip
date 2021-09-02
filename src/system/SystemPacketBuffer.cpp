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
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
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

#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP
#include <lib/support/CHIPMem.h>
#endif

namespace chip {
namespace System {

#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_POOL
//
// Pool allocation for PacketBuffer objects.
//

PacketBuffer::BufferPoolElement PacketBuffer::sBufferPool[CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE];

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

PacketBuffer * PacketBuffer::BuildFreeList()
{
    pbuf * lHead = nullptr;

    for (int i = 0; i < CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE; i++)
    {
        pbuf * lCursor = &sBufferPool[i].Header;
        lCursor->next  = lHead;
        lCursor->ref   = 0;
        lHead          = lCursor;
    }

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    Mutex::Init(sBufferPoolMutex);
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING

    return static_cast<PacketBuffer *>(lHead);
}

#elif CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP
//
// Heap allocation for PacketBuffer objects.
//

#if CHIP_CONFIG_MEMORY_DEBUG_CHECKS
void PacketBuffer::InternalCheck(const PacketBuffer * buffer)
{
    if (buffer)
    {
        VerifyOrDieWithMsg(::chip::Platform::MemoryDebugCheckPointer(buffer, buffer->alloc_size + kStructureSize), chipSystemLayer,
                           "invalid packet buffer pointer");
        VerifyOrDieWithMsg(buffer->alloc_size >= buffer->ReservedSize() + buffer->len, chipSystemLayer,
                           "packet buffer overflow %u < %u+%u", buffer->alloc_size, buffer->ReservedSize(), buffer->len);
    }
}
#endif // CHIP_CONFIG_MEMORY_DEBUG_CHECKS

// Number of unused bytes below which \c RightSize() won't bother reallocating.
constexpr uint16_t kRightSizingThreshold = 16;

void PacketBufferHandle::InternalRightSize()
{
    // Require a single buffer with no other references.
    if ((mBuffer == nullptr) || mBuffer->HasChainedBuffer() || (mBuffer->ref != 1))
    {
        return;
    }

    // Reallocate only if enough space will be saved.
    uint8_t * const start   = reinterpret_cast<uint8_t *>(mBuffer) + PacketBuffer::kStructureSize;
    uint8_t * const payload = reinterpret_cast<uint8_t *>(mBuffer->payload);
    const uint16_t usedSize = static_cast<uint16_t>(payload - start + mBuffer->len);
    if (usedSize + kRightSizingThreshold > mBuffer->alloc_size)
    {
        return;
    }

    const size_t blockSize   = usedSize + PacketBuffer::kStructureSize;
    PacketBuffer * newBuffer = reinterpret_cast<PacketBuffer *>(chip::Platform::MemoryAlloc(blockSize));
    if (newBuffer == nullptr)
    {
        ChipLogError(chipSystemLayer, "PacketBuffer: pool EMPTY.");
        return;
    }

    uint8_t * const newStart = reinterpret_cast<uint8_t *>(newBuffer) + PacketBuffer::kStructureSize;
    newBuffer->next          = nullptr;
    newBuffer->payload       = newStart + (payload - start);
    newBuffer->tot_len       = mBuffer->tot_len;
    newBuffer->len           = mBuffer->len;
    newBuffer->ref           = 1;
    newBuffer->alloc_size    = static_cast<uint16_t>(usedSize);
    memcpy(reinterpret_cast<uint8_t *>(newBuffer) + PacketBuffer::kStructureSize, start, usedSize);

    PacketBuffer::Free(mBuffer);
    mBuffer = newBuffer;
}

#elif CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_LWIP_CUSTOM

void PacketBufferHandle::InternalRightSize()
{
    PacketBuffer * lNewPacket = static_cast<PacketBuffer *>(pbuf_rightsize((struct pbuf *) mBuffer, -1));
    if (lNewPacket != mBuffer)
    {
        mBuffer = lNewPacket;
        SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();
        ChipLogProgress(chipSystemLayer, "PacketBuffer: RightSize Copied");
    }
}

#endif // CHIP_SYSTEM_PACKETBUFFER_STORE

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

void PacketBuffer::SetStart(uint8_t * aNewStart)
{
    uint8_t * const kStart = reinterpret_cast<uint8_t *>(this) + kStructureSize;
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

    // SetDataLength is often called after a client finished writing to the buffer,
    // so it's a good time to check for possible corruption.
    Check(this);

    while (aChainHead != nullptr && aChainHead != this)
    {
        Check(aChainHead);
        aChainHead->tot_len = static_cast<uint16_t>(aChainHead->tot_len + lDelta);
        aChainHead          = aChainHead->ChainedBuffer();
    }
}

uint16_t PacketBuffer::MaxDataLength() const
{
    const uint8_t * const kStart = reinterpret_cast<const uint8_t *>(this) + kStructureSize;
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
    return static_cast<uint16_t>(kDelta - kStructureSize);
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
        uint16_t old_total_length = lCursor->tot_len;
        lCursor->tot_len          = static_cast<uint16_t>(lCursor->tot_len + aPacket->tot_len);
        VerifyOrDieWithMsg(lCursor->tot_len >= old_total_length, chipSystemLayer, "buffer chain too large");
        if (!lCursor->HasChainedBuffer())
        {
            lCursor->next = aPacket;
            break;
        }

        lCursor = lCursor->ChainedBuffer();
    }
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

void PacketBuffer::CompactHead()
{
    uint8_t * const kStart = reinterpret_cast<uint8_t *>(this) + kStructureSize;

    if (this->payload != kStart)
    {
        memmove(kStart, this->payload, this->len);
        this->payload = kStart;
    }

    uint16_t lAvailLength = this->AvailableDataLength();

    while (lAvailLength > 0 && HasChainedBuffer())
    {
        PacketBuffer & lNextPacket = *ChainedBuffer();
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

CHIP_ERROR PacketBuffer::Read(uint8_t * aDestination, size_t aReadLength) const
{
    const PacketBuffer * lPacket = this;

    if (aReadLength > TotalLength())
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    while (aReadLength > 0)
    {
        if (lPacket == nullptr)
        {
            // TotalLength() or an individual buffer's DataLength() must have been wrong.
            return CHIP_ERROR_INTERNAL;
        }
        size_t lToReadFromCurrentBuf = lPacket->DataLength();
        if (aReadLength < lToReadFromCurrentBuf)
        {
            lToReadFromCurrentBuf = aReadLength;
        }
        memcpy(aDestination, lPacket->Start(), lToReadFromCurrentBuf);
        aDestination += lToReadFromCurrentBuf;
        aReadLength -= lToReadFromCurrentBuf;
        lPacket = lPacket->ChainedBuffer();
    }
    return CHIP_NO_ERROR;
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

PacketBufferHandle PacketBufferHandle::New(size_t aAvailableSize, uint16_t aReservedSize)
{
    // Adding three 16-bit-int sized numbers together will never overflow
    // assuming int is at least 32 bits.
    static_assert(INT_MAX >= INT32_MAX, "int is not big enough");
    static_assert(PacketBuffer::kStructureSize == sizeof(PacketBuffer), "PacketBuffer size mismatch");
    static_assert(PacketBuffer::kStructureSize < UINT16_MAX, "Check for overflow more carefully");
    static_assert(SIZE_MAX >= INT_MAX, "Our additions might not fit in size_t");
    static_assert(PacketBuffer::kMaxSizeWithoutReserve <= UINT16_MAX, "PacketBuffer may have size not fitting uint16_t");

    // When `aAvailableSize` fits in uint16_t (as tested below) and size_t is at least 32 bits (as asserted above),
    // these additions will not overflow.
    const size_t lAllocSize = aReservedSize + aAvailableSize;
    const size_t lBlockSize = PacketBuffer::kStructureSize + lAllocSize;
    PacketBuffer * lPacket;

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_PacketBufferNew, return PacketBufferHandle());

    if (aAvailableSize > UINT16_MAX || lAllocSize > PacketBuffer::kMaxSizeWithoutReserve || lBlockSize > UINT16_MAX)
    {
        ChipLogError(chipSystemLayer, "PacketBuffer: allocation too large.");
        return PacketBufferHandle();
    }

#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_LWIP_POOL ||                                                  \
    CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_LWIP_CUSTOM

    lPacket = static_cast<PacketBuffer *>(pbuf_alloc(PBUF_RAW, static_cast<uint16_t>(lAllocSize), PBUF_POOL));

    SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();

#elif CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_POOL

    static_cast<void>(lBlockSize);

    LOCK_BUF_POOL();

    lPacket = PacketBuffer::sFreeList;
    if (lPacket != nullptr)
    {
        PacketBuffer::sFreeList = lPacket->ChainedBuffer();
        SYSTEM_STATS_INCREMENT(chip::System::Stats::kSystemLayer_NumPacketBufs);
    }

    UNLOCK_BUF_POOL();

#elif CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP

    lPacket = reinterpret_cast<PacketBuffer *>(chip::Platform::MemoryAlloc(lBlockSize));
    SYSTEM_STATS_INCREMENT(chip::System::Stats::kSystemLayer_NumPacketBufs);

#else
#error "Unimplemented CHIP_SYSTEM_PACKETBUFFER_STORE case"
#endif // CHIP_SYSTEM_PACKETBUFFER_STORE

    if (lPacket == nullptr)
    {
        ChipLogError(chipSystemLayer, "PacketBuffer: pool EMPTY.");
        return PacketBufferHandle();
    }

    lPacket->payload = reinterpret_cast<uint8_t *>(lPacket) + PacketBuffer::kStructureSize + aReservedSize;
    lPacket->len = lPacket->tot_len = 0;
    lPacket->next                   = nullptr;
    lPacket->ref                    = 1;
#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP
    lPacket->alloc_size = static_cast<uint16_t>(lAllocSize);
#endif

    return PacketBufferHandle(lPacket);
}

PacketBufferHandle PacketBufferHandle::NewWithData(const void * aData, size_t aDataSize, uint16_t aAdditionalSize,
                                                   uint16_t aReservedSize)
{
    if (aDataSize > UINT16_MAX)
    {
        ChipLogError(chipSystemLayer, "PacketBuffer: allocation too large.");
        return PacketBufferHandle();
    }
    // Since `aDataSize` fits in uint16_t, the sum `aDataSize + aAdditionalSize` will not overflow.
    // `New()` will only return a non-null buffer if the total allocation size does not overflow.
    PacketBufferHandle buffer = New(aDataSize + aAdditionalSize, aReservedSize);
    if (buffer.mBuffer != nullptr)
    {
        memcpy(buffer.mBuffer->payload, aData, aDataSize);
        buffer.mBuffer->len = buffer.mBuffer->tot_len = static_cast<uint16_t>(aDataSize);
    }
    return buffer;
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
#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_LWIP_POOL ||                                                  \
    CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_LWIP_CUSTOM

    if (aPacket != nullptr)
    {
        pbuf_free(aPacket);

        SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();
    }

#elif CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_POOL ||                                                \
    CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP

    LOCK_BUF_POOL();

    while (aPacket != nullptr)
    {
        PacketBuffer * lNextPacket = aPacket->ChainedBuffer();

        VerifyOrDieWithMsg(aPacket->ref > 0, chipSystemLayer, "SystemPacketBuffer::Free: aPacket->ref = 0");

        aPacket->ref--;
        if (aPacket->ref == 0)
        {
            SYSTEM_STATS_DECREMENT(chip::System::Stats::kSystemLayer_NumPacketBufs);
#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP
            ::chip::Platform::MemoryDebugCheckPointer(aPacket, aPacket->alloc_size + kStructureSize);
#endif
            aPacket->Clear();
#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_POOL
            aPacket->next = sFreeList;
            sFreeList     = aPacket;
#elif CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP
            chip::Platform::MemoryFree(aPacket);
#endif // CHIP_SYSTEM_PACKETBUFFER_STORE
            aPacket       = lNextPacket;
        }
        else
        {
            aPacket = nullptr;
        }
    }

    UNLOCK_BUF_POOL();

#else
#error "Unimplemented CHIP_SYSTEM_PACKETBUFFER_STORE case"
#endif // CHIP_SYSTEM_PACKETBUFFER_STORE
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
#if CHIP_SYSTEM_PACKETBUFFER_STORE == CHIP_SYSTEM_PACKETBUFFER_STORE_CHIP_HEAP
    alloc_size = 0;
#endif
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
    PacketBuffer * lNextPacket = aPacket->ChainedBuffer();
    aPacket->next              = nullptr;
    PacketBuffer::Free(aPacket);
    return lNextPacket;
}

PacketBufferHandle PacketBufferHandle::PopHead()
{
    PacketBuffer * head = mBuffer;

    // This takes ownership from the `next` link.
    mBuffer = mBuffer->ChainedBuffer();

    head->next    = nullptr;
    head->tot_len = head->len;

    // The returned handle takes ownership from this.
    return PacketBufferHandle(head);
}

PacketBufferHandle PacketBufferHandle::CloneData() const
{
    PacketBufferHandle cloneHead;

    for (PacketBuffer * original = mBuffer; original != nullptr; original = original->ChainedBuffer())
    {
        uint16_t originalDataSize     = original->MaxDataLength();
        uint16_t originalReservedSize = original->ReservedSize();

        if (originalDataSize + originalReservedSize > PacketBuffer::kMaxSizeWithoutReserve)
        {
            // The original memory allocation may have provided a larger block than requested (e.g. when using a shared pool),
            // and in particular may have provided a larger block than we are able to request from PackBufferHandle::New().
            // It is a genuine error if that extra space has been used.
            if (originalReservedSize + original->DataLength() > PacketBuffer::kMaxSizeWithoutReserve)
            {
                return PacketBufferHandle();
            }
            // Otherwise, reduce the requested data size. This subtraction can not underflow because the above test
            // guarantees originalReservedSize <= PacketBuffer::kMaxSizeWithoutReserve.
            originalDataSize = static_cast<uint16_t>(PacketBuffer::kMaxSizeWithoutReserve - originalReservedSize);
        }

        PacketBufferHandle clone = PacketBufferHandle::New(originalDataSize, originalReservedSize);
        if (clone.IsNull())
        {
            return PacketBufferHandle();
        }
        clone.mBuffer->tot_len = clone.mBuffer->len = original->len;
        memcpy(reinterpret_cast<uint8_t *>(clone.mBuffer) + PacketBuffer::kStructureSize,
               reinterpret_cast<uint8_t *>(original) + PacketBuffer::kStructureSize, originalDataSize + originalReservedSize);

        if (cloneHead.IsNull())
        {
            cloneHead = std::move(clone);
        }
        else
        {
            cloneHead->AddToEnd(std::move(clone));
        }
    }

    return cloneHead;
}

} // namespace System

namespace Encoding {

System::PacketBufferHandle PacketBufferWriterUtil::Finalize(BufferWriter & aBufferWriter, System::PacketBufferHandle & aPacket)
{
    if (!aPacket.IsNull() && aBufferWriter.Fit())
    {
        // Since mPacket was successfully allocated to hold the maximum length,
        // we know that the actual length fits in a uint16_t.
        aPacket->SetDataLength(static_cast<uint16_t>(aBufferWriter.Needed()));
    }
    else
    {
        aPacket = nullptr;
    }
    aBufferWriter = Encoding::BufferWriter(nullptr, 0);
    return std::move(aPacket);
}

} // namespace Encoding
} // namespace chip
