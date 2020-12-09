/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the chip::System::PacketBuffer class,
 *      which provides the mechanisms for manipulating packets of *
 *      octet-serialized data.
 */

#pragma once

// Include configuration header
#include <system/SystemConfig.h>

// Include dependent headers
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <system/SystemAlignSize.h>
#include <system/SystemError.h>

#include <stddef.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/mem.h>
#include <lwip/memp.h>
#include <lwip/pbuf.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace System {

class PacketBufferHandle;

#if !CHIP_SYSTEM_CONFIG_USE_LWIP
struct pbuf
{
    struct pbuf * next;
    void * payload;
    uint16_t tot_len;
    uint16_t len;
    uint16_t ref;
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    uint16_t alloc_size;
#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
};
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP

/**    @class PacketBuffer
 *
 *     @brief
 *      The packet buffer class is the core structure used for manipulating packets of octet-serialized data, usually in the
 *      context of a data communications network, like Bluetooth or the Internet protocol.
 *
 *      In LwIP-based environments, this class is built on top of the pbuf structure defined in that library. In the absence of
 *      LwIP, chip provides either a malloc-based implementation, or a pool-based implementation that closely approximates the
 *      memory challenges of deeply embedded devices.
 *
 *      The PacketBuffer class, like many similar structures used in layered network stacks, provide a mechanism to reserve space
 *      for protocol headers at each layer of a configurable communication stack.  For details, see `PacketBuffer::New()` as well
 *      as LwIP documentation.
 *
 *      FIXME: CHIP is in the middle of a transition to use PacketBufferHandle to manage PacketBuffer ownership, so the following
 *      paragraph is neither entirely correct nor entirely wrong.
 *
 *        PacketBuffer objects are reference-counted, and the prevailing usage mode within chip is "fire-and-forget".  As the packet
 *        (and its underlying PacketBuffer object) is dispatched through various protocol layers, the successful upcall or downcall
 *        between layers implies ownership transfer, and the callee is responsible for freeing the buffer.  On failure of a
 *        cross-layer call, the responsibilty for freeing the buffer rests with the caller.
 *
 *      The end goal is:
 *
 *        PacketBuffer objects are reference-counted, and held through `PacketBufferHandle`s. When a PacketBufferHandle goes out
 *        of scope, its reference is released. To transfer ownership, a function takes a PacketBufferHandle by value. To borrow
 *        ownership, a function takes a `const PacketBufferHandle &`.
 *
 *      New objects of PacketBuffer class are initialized at the beginning of an allocation of memory obtained from the underlying
 *      environment, e.g. from LwIP pbuf target pools, from the standard C library heap, from an internal buffer pool. In the
 *      simple case, the size of the data buffer is #CHIP_SYSTEM_PACKETBUFFER_SIZE. A composer is provided that permits usage of
 *      data buffers of other sizes.
 *
 *      PacketBuffer objects may be chained to accomodate larger payloads.  Chaining, however, is not transparent, and users of the
 *      class must explicitly decide to support chaining.  Examples of classes written with chaining support are as follows:
 *
 *          @ref chip::chipTLVReader
 *          @ref chip::chipTLVWriter
 *
 */
class DLL_EXPORT PacketBuffer : private pbuf
{
public:
    uint16_t AllocSize() const;

    uint8_t * Start() const;
    void SetStart(uint8_t * aNewStart);

    uint16_t DataLength() const;
    void SetDataLength(uint16_t aNewLen, PacketBuffer * aChainHead = nullptr);

    uint16_t TotalLength() const;

    uint16_t MaxDataLength() const;
    uint16_t AvailableDataLength() const;

    uint16_t ReservedSize() const;

    PacketBuffer * Next() const;

    // The raw PacketBuffer version of AddToEnd() will be removed when conversion to PacketBufferHandle is complete.
    void AddToEnd_ForNow(PacketBuffer * aPacket);
    // The PacketBufferHandle's ownership is transferred to the `next` link at the end of the current chain.
    void AddToEnd(PacketBufferHandle aPacket);
    // The raw PacketBuffer version of DetachTail() will be removed when conversion to PacketBufferHandle is complete.
    PacketBuffer * DetachTail_ForNow();
    void CompactHead();
    PacketBuffer * Consume(uint16_t aConsumeLength);
    void ConsumeHead(uint16_t aConsumeLength);
    bool EnsureReservedSize(uint16_t aReservedSize);
    bool AlignPayload(uint16_t aAlignBytes);

    void AddRef();

    static PacketBufferHandle NewWithAvailableSize(uint16_t aAvailableSize);
    static PacketBufferHandle NewWithAvailableSize(uint16_t aReservedSize, uint16_t aAvailableSize);

    static PacketBufferHandle New();
    static PacketBufferHandle New(uint16_t aReservedSize);

    static PacketBuffer * RightSize(PacketBuffer * aPacket);

    static void Free(PacketBuffer * aPacket);
    // To be removed when conversion to PacketBufferHandle is complete:
    static PacketBuffer * FreeHead_ForNow(PacketBuffer * aPacket) { return FreeHead(aPacket); }

private:
#if !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
    static PacketBuffer * sFreeList;

    static PacketBuffer * BuildFreeList();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

    static PacketBuffer * FreeHead(PacketBuffer * aPacket);
    void Clear();
    friend class PacketBufferHandle;
};

} // namespace System
} // namespace chip

// Sizing definitions

/**
 * @def CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE
 *
 *  The effective size of the packet buffer structure.
 *
 *  TODO: This is an implementation details that does not need to be public and should be moved to the source file.
 */

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE LWIP_MEM_ALIGN_SIZE(sizeof(struct ::pbuf))
#else // CHIP_SYSTEM_CONFIG_USE_LWIP
#define CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE CHIP_SYSTEM_ALIGN_SIZE(sizeof(::chip::System::PacketBuffer), 4u)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * @def CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
 *
 *  See SystemConfig.h for full description. This is defined in here specifically for LwIP platform to preserve backwards
 *  compatibility.
 *
 *  TODO: This is an implementation details that does not need to be public and should be moved to the source file.
 *
 */
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX (LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE) - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * @def CHIP_SYSTEM_PACKETBUFFER_SIZE
 *
 *  The memory footprint of a PacketBuffer object, computed from max capacity size and the size of the packet buffer structure.
 *
 *  TODO: This is an implementation details that does not need to be public and should be moved to the source file.
 */

#define CHIP_SYSTEM_PACKETBUFFER_SIZE (CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE)

namespace chip {
namespace System {

//
// Pool allocation for PacketBuffer objects (toll-free bridged with LwIP pbuf allocator if CHIP_SYSTEM_CONFIG_USE_LWIP)
//
#if !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

typedef union
{
    PacketBuffer Header;
    uint8_t Block[CHIP_SYSTEM_PACKETBUFFER_SIZE];
} BufferPoolElement;

#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

/**
 * Return the size of the allocation including the reserved and payload data spaces but not including space
 * allocated for the PacketBuffer structure.
 *
 *  @note    The allocation size is equal or greater than \c aAllocSize paramater to \c Create method).
 *
 *  @return     size of the allocation
 */
inline uint16_t PacketBuffer::AllocSize() const
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if LWIP_PBUF_FROM_CUSTOM_POOLS
    // Temporary workaround for custom pbufs by assuming size to be PBUF_POOL_BUFSIZE
    if (this->flags & PBUF_FLAG_IS_CUSTOM)
        return LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE) - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
    else
        return LWIP_MEM_ALIGN_SIZE(memp_sizes[this->pool]) - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
#else  // !LWIP_PBUF_FROM_CUSTOM_POOLS
    return LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE) - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
#endif // !LWIP_PBUF_FROM_CUSTOM_POOLS
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    return this->alloc_size;
#else  // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC != 0
    extern BufferPoolElement gDummyBufferPoolElement;
    return sizeof(gDummyBufferPoolElement.Block) - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC != 0
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

} // namespace System
} // namespace chip

namespace chip {
namespace System {

/// Tracks ownership of a System::PacketBuffer.
class DLL_EXPORT PacketBufferHandle
{
public:
    PacketBufferHandle() : mBuffer(nullptr) {}
    PacketBufferHandle(decltype(nullptr)) : mBuffer(nullptr) {}

    PacketBufferHandle(PacketBufferHandle && aOther)
    {
        mBuffer        = aOther.mBuffer;
        aOther.mBuffer = nullptr;
    }

    ~PacketBufferHandle() { Adopt(nullptr); }

    PacketBufferHandle & operator=(PacketBufferHandle && aOther)
    {
        if (mBuffer != nullptr)
        {
            PacketBuffer::Free(mBuffer);
        }
        mBuffer        = aOther.mBuffer;
        aOther.mBuffer = nullptr;
        return *this;
    }
    PacketBufferHandle & operator=(decltype(nullptr))
    {
        Adopt(nullptr);
        return *this;
    }

    PacketBufferHandle Retain() const
    {
        mBuffer->AddRef();
        return PacketBufferHandle(mBuffer);
    }

    PacketBuffer * operator->() const { return mBuffer; }
    PacketBuffer & operator*() const { return *mBuffer; }

    // The caller's ownership is transferred to this.
    void Adopt(PacketBuffer * buffer)
    {
        if (mBuffer != nullptr)
        {
            PacketBuffer::Free(mBuffer);
        }
        mBuffer = buffer;
    }

    // The PacketBufferHandle's ownership is transferred to the caller.
    // This is intended to be used only to call functions that have not yet been converted; a permanent version may be created
    // if/when the need is clear. Most uses will be converted to take a `PacketBufferHandle` by value.
    CHECK_RETURN_VALUE PacketBuffer * Release_ForNow()
    {
        PacketBuffer * buffer = mBuffer;
        mBuffer               = nullptr;
        return buffer;
    }

    // The caller has access but no ownership.
    // This is intended to be used only to call functions that have not yet been converted to take a PacketBufferHandle;
    // a permanent version may be created if/when the need is clear. Most uses will be converted to take a
    // `const PacketBufferHandle &`.
    PacketBuffer * Get_ForNow() const { return mBuffer; }

    bool IsNull() const { return mBuffer == nullptr; }

    /**
     *  Detach and return the head of a buffer chain while updating this handle to point to the remaining buffers.
     *  The current buffer must be the head of the chain.
     *
     *  @return the detached buffer formerly at the head of the buffer chain.
     */
    PacketBufferHandle PopHead();

    /**
     * Free the first buffer in a chain.
     *
     *  @note When the buffer chain is referenced by multiple callers, `FreeHead()` will detach the head, but will not forcibly
     *  deallocate the head buffer.
     */
    void FreeHead()
    {
        // `PacketBuffer::FreeHead()` frees the current head; this takes ownership from the `next` link.
        mBuffer = PacketBuffer::FreeHead(mBuffer);
    }

    /**
     * Creates a copy of the data in this packet.
     *
     * Does NOT support chained buffers.
     *
     * @returns empty handle on allocation failure.
     */
    PacketBufferHandle Clone();

private:
    PacketBufferHandle(const PacketBufferHandle &) = delete;
    PacketBufferHandle & operator=(const PacketBufferHandle &) = delete;

    // The caller's ownership is transferred to this.
    explicit PacketBufferHandle(PacketBuffer * buffer) : mBuffer(buffer) {}

    PacketBuffer * mBuffer;
    friend class PacketBuffer;
};

} // namespace System
} // namespace chip
