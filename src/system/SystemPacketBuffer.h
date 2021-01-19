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

class PacketBufferTest;

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
 *      PacketBuffer objects are reference-counted, and normally held and used through a PacketBufferHandle that owns one of the
 *      counted references. When a PacketBufferHandle goes out of scope, its reference is released. To take ownership, a function
 *      takes a PacketBufferHandle by value. To borrow ownership, a function takes a `const PacketBufferHandle &`.
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
    /**
     * Return the size of the allocation including the reserved and payload data spaces but not including space
     * allocated for the PacketBuffer structure.
     *
     *  @note    The allocation size is equal to or greater than the \c aAllocSize parameter to the \c Create method).
     *
     *  @return     size of the allocation
     */
    uint16_t AllocSize() const;

    /**
     * Get a pointer to the start of data in a buffer.
     *
     *  @return pointer to the start of data.
     */
    uint8_t * Start() const { return static_cast<uint8_t *>(this->payload); }

    /**
     *  Set the the start of data in a buffer, adjusting length and total length accordingly.
     *
     *  @note The data within the buffer is not moved, only accounting information is changed.  The function is commonly used to
     *      either strip or prepend protocol headers in a zero-copy way.
     *
     *  @note This call should not be used on any buffer that is not the head of a buffer chain, as it only alters the current
     *      buffer.
     *
     *  @param[in] aNewStart - A pointer to where the new payload should start.  newStart will be adjusted internally to fall within
     *      the boundaries of the first buffer in the PacketBuffer chain.
     */
    void SetStart(uint8_t * aNewStart);

    /**
     * Get the length, in bytes, of data in a packet buffer.
     *
     *  @return length, in bytes (current payload length).
     */
    uint16_t DataLength() const { return this->len; }

    /**
     * Set the length, in bytes, of data in a packet buffer, adjusting total length accordingly.
     *
     *  The function sets the length, in bytes, of the data in the buffer, adjusting the total length appropriately. When the buffer
     *  is not the head of the buffer chain (common case: the caller adds data to the last buffer in the PacketBuffer chain prior to
     *  calling higher layers), the aChainHead __must__ be passed in to properly adjust the total lengths of each buffer ahead of
     *  the current buffer.
     *
     *  @param[in] aNewLen - new length, in bytes, of this buffer.
     *
     *  @param[in,out] aChainHead - the head of the buffer chain the current buffer belongs to.  May be \c nullptr if the current
     *      buffer is the head of the buffer chain.
     */
    void SetDataLength(uint16_t aNewLen, const PacketBufferHandle & aChainHead);
    void SetDataLength(uint16_t aNewLen) { SetDataLength(aNewLen, nullptr); }

    /**
     * Get the total length of packet data in the buffer chain.
     *
     *  @return total length, in octets.
     */
    uint16_t TotalLength() const { return this->tot_len; }

    /**
     * Get the maximum amount, in bytes, of data that will fit in the buffer given the current start position and buffer size.
     *
     *  @return number of bytes that fits in the buffer given the current start position.
     */
    uint16_t MaxDataLength() const;

    /**
     * Get the number of bytes of data that can be added to the current buffer given the current start position and data length.
     *
     *  @return the length, in bytes, of data that will fit in the current buffer given the current start position and data length.
     */
    uint16_t AvailableDataLength() const;

    /**
     * Get the number of bytes within the current buffer between the start of the buffer and the current data start position.
     *
     *  @return the amount, in bytes, of space between the start of the buffer and the current data start position.
     */
    uint16_t ReservedSize() const;

    /**
     * Determine whether there are any additional buffers chained to the current buffer.
     *
     *  @return \c true if there is a chained buffer.
     */
    bool HasChainedBuffer() const { return ChainedBuffer() != nullptr; }

    /**
     * Add the given packet buffer to the end of the buffer chain, adjusting the total length of each buffer in the chain
     * accordingly.
     *
     *  @note The current packet buffer must be the head of the buffer chain for the lengths to be adjusted properly.
     *
     *  @note Ownership is transferred from the argument to the `next` link at the end of the current chain.
     *
     *  @param[in] aPacket - the packet buffer to be added to the end of the current chain.
     */
    void AddToEnd(PacketBufferHandle && aPacket);

    /**
     * Move data from subsequent buffers in the chain into the current buffer until it is full.
     *
     *  Only the current buffer is compacted: the data within the current buffer is moved to the front of the buffer, eliminating
     *  any reserved space. The remaining available space is filled with data moved from subsequent buffers in the chain, until the
     *  current buffer is full. If a subsequent buffer in the chain is moved into the current buffer in its entirety, it is removed
     *  from the chain and freed. The method takes no parameters, returns no results and cannot fail.
     */
    void CompactHead();

    /**
     * Adjust the current buffer to indicate the amount of data consumed.
     *
     *  Advance the data start position in the current buffer by the specified amount, in bytes, up to the length of data in the
     *  buffer. Decrease the length and total length by the amount consumed.
     *
     *  @param[in] aConsumeLength - number of bytes to consume from the current buffer.
     */
    void ConsumeHead(uint16_t aConsumeLength);

    /**
     * Ensure the buffer has at least the specified amount of reserved space.
     *
     *  Ensure the buffer has at least the specified amount of reserved space, moving the data in the buffer forward to make room if
     *  necessary.
     *
     *  @param[in] aReservedSize - number of bytes desired for the headers.
     *
     *  @return \c true if the requested reserved size is available, \c false if there's not enough room in the buffer.
     */
    bool EnsureReservedSize(uint16_t aReservedSize);

    /**
     * Align the buffer payload on the specified bytes boundary.
     *
     *  Moving the payload in the buffer forward if necessary.
     *
     *  @param[in] aAlignBytes - specifies number of bytes alignment for the payload start pointer.
     *
     *  @return \c true if alignment is successful, \c false if there's not enough room in the buffer.
     */
    bool AlignPayload(uint16_t aAlignBytes);

    /**
     * Return the next buffer in a buffer chain.
     *
     *  If there is no next buffer, the handle will have \c IsNull() \c true.
     *
     *  @return a handle to the next buffer in the buffer chain.
     */
    CHECK_RETURN_VALUE PacketBufferHandle Next();

    /**
     * Return the last buffer in a buffer chain.
     *
     *  @return a handle to the last buffer in the buffer chain.
     */
    CHECK_RETURN_VALUE PacketBufferHandle Last();

    /**
     * Allocates a PacketBuffer object with at least \c aReservedSize bytes reserved in the payload for headers, and at least
     *  \c aAllocSize bytes of space for additional data after the initial cursor pointer.
     *
     *  @param[in]  aReservedSize   Number of octets to reserve behind the cursor.
     *  @param[in]  aAvailableSize  Number of octets to allocate after the cursor.
     *
     *  @return     On success, a pointer to the PacketBuffer in the allocated block. On fail, \c nullptr.
     */
    static PacketBufferHandle NewWithAvailableSize(uint16_t aReservedSize, uint16_t aAvailableSize);

    /**
     * Allocates a PacketBuffer with default reserved size (#CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) in the payload for headers,
     * and at least \c aAllocSize bytes of space for additional data after the initial cursor pointer.
     *
     * This usage is most appropriate when allocating a PacketBuffer for an application-layer message.
     *
     *  @param[in]  aAvailableSize  Number of octets to allocate after the cursor.
     *
     *  @return     On success, a pointer to the PacketBuffer in the allocated block. On fail, \c nullptr.
     */
    static PacketBufferHandle NewWithAvailableSize(uint16_t aAvailableSize);

    /**
     * Allocates a single PacketBuffer of maximum total size with a specific header reserve size.
     *
     *  The parameter passed in is the size reserved prior to the payload to accomodate packet headers from different stack layers,
     *  __not__ the overall size of the buffer to allocate. The size of the buffer #CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX
     *  and not, specified in the call.
     *
     *  `PacketBuffer::New(0)` : when called in this fashion, the buffer will be returned without any header reserved, consequently
     *  the entire payload is usable by the caller. This pattern is particularly useful at the lower layers of networking stacks,
     *  in cases where the user knows the payload will be copied out into the final message with appropriate header reserves or in
     *  creating PacketBuffer that are appended to a chain of PacketBuffer via `PacketBuffer::AddToEnd()`.
     *
     *  @param[in] aReservedSize  amount of header space to reserve.
     *
     *  @return On success, a pointer to the PacketBuffer, on failure \c nullptr.
     */
    static PacketBufferHandle New(uint16_t aReservedSize);

    /**
     * Allocates a single PacketBuffer of default max size (#CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX) with default reserved
     * size (#CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) in the payload.
     *
     * The reserved size (#CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE) is large enough to hold transport layer headers as well as
     * headers required by \c chipMessageLayer and \c chipExchangeLayer.
     */
    static PacketBufferHandle New();

private:
#if !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
    static PacketBuffer * sFreeList;

    static PacketBuffer * BuildFreeList();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_PBUF_FROM_CUSTOM_POOLS
    static PacketBuffer * RightSize(PacketBuffer * aPacket);
#endif

    void AddRef();
    static void Free(PacketBuffer * aPacket);
    static PacketBuffer * FreeHead(PacketBuffer * aPacket);

    PacketBuffer * ChainedBuffer() const { return static_cast<PacketBuffer *>(this->next); }
    PacketBuffer * Consume(uint16_t aConsumeLength);
    void Clear();
    void SetDataLength(uint16_t aNewLen, PacketBuffer * aChainHead);

    friend class PacketBufferHandle;
    friend class ::PacketBufferTest;
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

/**
 * @class PacketBufferHandle
 *
 * @brief
 *  Tracks ownership of a PacketBuffer.
 *
 *  PacketBuffer objects are reference-counted, and normally held and used through a PacketBufferHandle that owns one of the
 *  counted references. When a PacketBufferHandle goes out of scope, its reference is released. To take ownership, a function
 *  takes a PacketBufferHandle by value. To borrow ownership, a function takes a `const PacketBufferHandle &`.
 */
class DLL_EXPORT PacketBufferHandle
{
public:
    /**
     * Construct an empty PacketBufferHandle.
     */
    PacketBufferHandle() : mBuffer(nullptr) {}
    PacketBufferHandle(decltype(nullptr)) : mBuffer(nullptr) {}

    /**
     * Construct a PacketBufferHandle that takes ownership of a PacketBuffer from another.
     */
    PacketBufferHandle(PacketBufferHandle && aOther)
    {
        mBuffer        = aOther.mBuffer;
        aOther.mBuffer = nullptr;
    }

    ~PacketBufferHandle() { *this = nullptr; }

    /**
     * Take ownership of a PacketBuffer from another PacketBufferHandle, freeing any existing owned buffer.
     */
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

    /**
     * Free any buffer owned by this handle.
     */
    PacketBufferHandle & operator=(decltype(nullptr))
    {
        if (mBuffer != nullptr)
        {
            PacketBuffer::Free(mBuffer);
        }
        mBuffer = nullptr;
        return *this;
    }

    /**
     * Get a new handle to an existing buffer.
     *
     * @return a PacketBufferHandle that shares ownership with this.
     */
    PacketBufferHandle Retain() const
    {
        mBuffer->AddRef();
        return PacketBufferHandle(mBuffer);
    }

    /**
     * Access a PackerBuffer's public methods.
     */
    PacketBuffer * operator->() const { return mBuffer; }

    /**
     * Test whether this PacketBufferHandle is empty, or owns a PacketBuffer.
     *
     * @return \c true if this PacketBufferHandle is empty; return \c false if it owns a PacketBuffer.
     */
    bool IsNull() const { return mBuffer == nullptr; }

    /**
     *  Detach and return the head of a buffer chain while updating this handle to point to the remaining buffers.
     *  The current buffer must be the head of the chain.
     *
     *  This PacketBufferHandle now holds the ownership formerly held by the head of the chain.
     *  The returned PacketBufferHandle holds the ownership formerly held by this.
     *
     *  @return the detached buffer formerly at the head of the buffer chain.
     */
    CHECK_RETURN_VALUE PacketBufferHandle PopHead();

    /**
     * Free the first buffer in a chain.
     *
     *  @note When the buffer chain is referenced by multiple handles, `FreeHead()` will detach the head, but will not forcibly
     *  deallocate the head buffer.
     */
    void FreeHead()
    {
        // `PacketBuffer::FreeHead()` frees the current head; this takes ownership from the `next` link.
        mBuffer = PacketBuffer::FreeHead(mBuffer);
    }

    /**
     * Consume data in a chain of buffers.
     *
     *  Consume data in a chain of buffers starting with the current buffer and proceeding through the remaining buffers in the
     *  chain. Each buffer that is completely consumed is freed and the handle holds the first buffer (if any) containing the
     *  remaining data. The current buffer must be the head of the buffer chain.
     *
     *  @param[in] aConsumeLength - number of bytes to consume from the current chain.
     */
    void Consume(uint16_t aConsumeLength) { mBuffer = mBuffer->Consume(aConsumeLength); }

    /**
     * Copy the given buffer to a right-sized buffer if applicable.
     * This function is a no-op for sockets.
     *
     *  @param[in] aPacket - buffer or buffer chain.
     *
     *  @return new packet buffer or the original buffer
     */
    void RightSize()
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_PBUF_FROM_CUSTOM_POOLS
        mBuffer = PacketBuffer::RightSize(mBuffer);
#endif
    }

    /**
     * Get a new handle to a raw PacketBuffer pointer.
     *
     * @brief The caller's ownership is transferred to this.
     *
     * @note This should only be used in low-level code, e.g. to import buffers from LwIP or a similar stack.
     */
    static PacketBufferHandle Adopt(PacketBuffer * buffer) { return PacketBufferHandle(buffer); }
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static PacketBufferHandle Adopt(pbuf * buffer) { return Adopt(reinterpret_cast<PacketBuffer *>(buffer)); }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    /**
     * Advance this PacketBufferHandle to the next buffer in a chain.
     *
     *  @note This differs from `FreeHead()` in that it does not touch any content in the currently referenced packet buffer;
     *      it only changes which buffer this handle owns. (Note that this could result in the previous buffer being freed,
     *      if there is no other owner.) `Advance()` is designed to be used with an additional handle to traverse a buffer chain,
     *      whereas `FreeHead()` modifies a chain.
     */
    void Advance() { *this = Hold(mBuffer->ChainedBuffer()); }

    /**
     * Export a raw PacketBuffer pointer.
     *
     * @brief The PacketBufferHandle's ownership is transferred to the caller.
     *
     * @note This should only be used in low-level code. The caller owns one counted reference to the \c PacketBuffer
     *       and is reponsible for managing it safely.
     *
     * @note The ref-qualifier `&&` requires the caller to use `std::move` to emphasize that ownership is
     *       moved out of this handle.
     */
    CHECK_RETURN_VALUE PacketBuffer * UnsafeRelease() &&
    {
        PacketBuffer * buffer = mBuffer;
        mBuffer               = nullptr;
        return buffer;
    }

    /**
     * Creates a copy of the data in this packet.
     *
     * Does NOT support chained buffers.
     *
     * @returns empty handle on allocation failure.
     */
    PacketBufferHandle CloneData();

protected:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static struct pbuf * GetLwIPpbuf(const PacketBufferHandle & handle) { return static_cast<struct pbuf *>(handle.mBuffer); }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

private:
    PacketBufferHandle(const PacketBufferHandle &) = delete;
    PacketBufferHandle & operator=(const PacketBufferHandle &) = delete;

    // The caller's ownership is transferred to this.
    explicit PacketBufferHandle(PacketBuffer * buffer) : mBuffer(buffer) {}

    static PacketBufferHandle Hold(PacketBuffer * buffer)
    {
        if (buffer != nullptr)
        {
            buffer->AddRef();
        }
        return PacketBufferHandle(buffer);
    }

    PacketBuffer * Get() const { return mBuffer; }

    bool operator==(const PacketBufferHandle & aOther) { return mBuffer == aOther.mBuffer; }

    PacketBuffer * mBuffer;

    friend class PacketBuffer;
    friend class ::PacketBufferTest;
};

inline void PacketBuffer::SetDataLength(uint16_t aNewLen, const PacketBufferHandle & aChainHead)
{
    SetDataLength(aNewLen, aChainHead.mBuffer);
}

inline PacketBufferHandle PacketBuffer::Next()
{
    return PacketBufferHandle::Hold(ChainedBuffer());
}

inline PacketBufferHandle PacketBuffer::Last()
{
    PacketBuffer * p = this;
    while (p->HasChainedBuffer())
        p = p->ChainedBuffer();
    return PacketBufferHandle::Hold(p);
}

} // namespace System
} // namespace chip

#if CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {

namespace Inet {
class RawEndPoint;
class UDPEndPoint;
class IPEndPointBasis;
} // namespace Inet

namespace System {

/**
 * Provide low-level access to a raw `pbuf *`, limited to specific classes that interface with LwIP.
 */
class LwIPPacketBufferView : public PacketBufferHandle
{
private:
    /**
     * Borrow a raw LwIP `pbuf *`.
     *
     * @brief The caller has access but no ownership.
     *
     * @note This should be used ONLY by low-level code interfacing with LwIP.
     */
    static struct pbuf * UnsafeGetLwIPpbuf(const PacketBufferHandle & handle) { return PacketBufferHandle::GetLwIPpbuf(handle); }
    friend class Inet::RawEndPoint;
    friend class Inet::UDPEndPoint;
    friend class Inet::IPEndPointBasis;
};

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
