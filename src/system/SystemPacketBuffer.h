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

#ifndef SYSTEMPACKETBUFFER_H
#define SYSTEMPACKETBUFFER_H

// Include configuration header
#include <system/SystemConfig.h>

// Include dependent headers
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
 *      PacketBuffer objects are reference-counted, and the prevailing usage mode within chip is "fire-and-forget".  As the packet
 *      (and its underlying PacketBuffer object) is dispatched through various protocol layers, the successful upcall or downcall
 *      between layers implies ownership transfer, and the callee is responsible for freeing the buffer.  On failure of a
 *      cross-layer call, the responsibilty for freeing the buffer rests with the caller.
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

    void AddToEnd(PacketBuffer * aPacket);
    PacketBuffer * DetachTail();
    void CompactHead();
    PacketBuffer * Consume(uint16_t aConsumeLength);
    void ConsumeHead(uint16_t aConsumeLength);
    bool EnsureReservedSize(uint16_t aReservedSize);
    bool AlignPayload(uint16_t aAlignBytes);

    void AddRef();

    static PacketBuffer * NewWithAvailableSize(uint16_t aAvailableSize);
    static PacketBuffer * NewWithAvailableSize(uint16_t aReservedSize, uint16_t aAvailableSize);

    static PacketBuffer * New();
    static PacketBuffer * New(uint16_t aReservedSize);

    static PacketBuffer * RightSize(PacketBuffer * aPacket);

    static void Free(PacketBuffer * aPacket);
    static PacketBuffer * FreeHead(PacketBuffer * aPacket);

private:
#if !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
    static PacketBuffer * sFreeList;

    static PacketBuffer * BuildFreeList();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

    void Clear();
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

#endif // defined(SYSTEMPACKETBUFFER_H)
