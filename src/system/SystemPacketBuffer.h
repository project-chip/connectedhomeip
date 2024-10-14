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
#include <system/SystemPacketBufferInternal.h>

// Include dependent headers
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemAlignSize.h>
#include <system/SystemError.h>

#include <stddef.h>
#include <utility>

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
    size_t tot_len;
    size_t len;
    uint16_t ref;
#if CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
    size_t alloc_size;
#endif
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
 *      for protocol headers at each layer of a configurable communication stack.  For details, see `PacketBufferHandle::New()`
 *      as well as LwIP documentation.
 *
 *      PacketBuffer objects are reference-counted, and normally held and used through a PacketBufferHandle that owns one of the
 *      counted references. When a PacketBufferHandle goes out of scope, its reference is released. To take ownership, a function
 *      takes a PacketBufferHandle by value. To borrow ownership, a function takes a `const PacketBufferHandle &`.
 *
 *      New objects of PacketBuffer class are initialized at the beginning of an allocation of memory obtained from the underlying
 *      environment, e.g. from LwIP pbuf target pools, from the standard C library heap, from an internal buffer pool. In the
 *      simple pool case, the size of the data buffer is PacketBuffer::kBlockSize.
 *
 *      PacketBuffer objects may be chained to accommodate larger payloads.  Chaining, however, is not transparent, and users of the
 *      class must explicitly decide to support chaining.  Examples of classes written with chaining support are as follows:
 *
 *          @ref chip::TLVReader
 *          @ref chip::TLVWriter
 *
 * ### PacketBuffer format
 *
 * <pre>
 *           ┌────────────────────────────────────┐
 *           │       ┌────────────────────┐       │
 *           │       │                    │◁──────┴───────▷│
 *  ┏━━━━━━━━┿━━━━━━━┿━┳━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━┓
 *  ┃ pbuf len payload ┃ reserve          ┃ data           ┃ unused                  ┃
 *  ┗━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━┛
 *  │                  │← ReservedSize() →│← DataLength() →│← AvailableDataLength() →│
 *  │                  │                  │← MaxDataLength() → · · · · · · · · · · ·→│
 *  │                  │                  Start()                                    │
 *  │← kStructureSize →│← AllocSize() → · · · · · · · · · · · · · · · · · · · · · · →│
 * </pre>
 *
 */
class DLL_EXPORT PacketBuffer : private pbuf
{
private:
    // The effective size of the packet buffer structure.
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static constexpr size_t kStructureSize = LWIP_MEM_ALIGN_SIZE(sizeof(struct ::pbuf));
#else  // CHIP_SYSTEM_CONFIG_USE_LWIP
    static constexpr size_t kStructureSize         = CHIP_SYSTEM_ALIGN_SIZE(sizeof(::chip::System::pbuf), 4u);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

public:
    /**
     * The maximum size of a regular buffer an application can allocate with no protocol header reserve.
     */
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    static constexpr size_t kMaxSizeWithoutReserve = LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE);
#else
    static constexpr size_t kMaxSizeWithoutReserve = CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX;
#endif

    /**
     * The number of bytes to reserve in a network packet buffer to contain all the possible protocol encapsulation headers
     * before the application data.
     */
    static constexpr uint16_t kDefaultHeaderReserve = CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE;

    /**
     * The maximum size of a regular buffer an application can allocate with the default protocol header reserve.
     */
    static constexpr size_t kMaxSize = kMaxSizeWithoutReserve - kDefaultHeaderReserve;

    /**
     * The maximum size of a large buffer(> IPv6 MTU) that an application can allocate with no protocol header reserve.
     */
    static constexpr size_t kLargeBufMaxSizeWithoutReserve = CHIP_SYSTEM_CONFIG_MAX_LARGE_BUFFER_SIZE_BYTES;

    /**
     * The maximum size of a large buffer(> IPv6 MTU) that an application can allocate with the default protocol header reserve.
     */
    static constexpr size_t kLargeBufMaxSize = kLargeBufMaxSizeWithoutReserve - kDefaultHeaderReserve;

    /**
     * Unified constant(both regular and large buffers) for the maximum size that an application can allocate with no
     * protocol header reserve.
     */
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    static constexpr size_t kMaxAllocSize = kLargeBufMaxSizeWithoutReserve;
#else
    static constexpr size_t kMaxAllocSize          = kMaxSizeWithoutReserve;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    /**
     * Return the size of the allocation including the reserved and payload data spaces but not including space
     * allocated for the PacketBuffer structure.
     *
     *  @note    The allocation size is equal to or greater than the \c aAllocSize parameter to the \c Create method).
     *
     *  @return     size of the allocation
     */
    size_t AllocSize() const
    {
#if CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_STANDARD_POOL || CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL
        return kMaxSizeWithoutReserve;
#elif CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_HEAP
        return this->alloc_size;
#elif CHIP_SYSTEM_PACKETBUFFER_FROM_LWIP_CUSTOM_POOL
        // Temporary workaround for custom pbufs by assuming size to be PBUF_POOL_BUFSIZE
        if (this->flags & PBUF_FLAG_IS_CUSTOM)
            return LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE) - kStructureSize;
        else
            return LWIP_MEM_ALIGN_SIZE(memp_sizes[this->pool]) - kStructureSize;
#else
#error "Unimplemented PacketBuffer storage case"
#endif
    }

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
    size_t DataLength() const { return this->len; }

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
    void SetDataLength(size_t aNewLen, const PacketBufferHandle & aChainHead);
    void SetDataLength(size_t aNewLen) { SetDataLength(aNewLen, nullptr); }

    /**
     * Get the total length of packet data in the buffer chain.
     *
     *  @return total length, in octets.
     */
    size_t TotalLength() const { return this->tot_len; }

    /**
     * Get the maximum amount, in bytes, of data that will fit in the buffer given the current start position and buffer size.
     *
     *  @return number of bytes that fits in the buffer given the current start position.
     */
    size_t MaxDataLength() const;

    /**
     * Get the number of bytes of data that can be added to the current buffer given the current start position and data length.
     *
     *  @return the length, in bytes, of data that will fit in the current buffer given the current start position and data length.
     */
    size_t AvailableDataLength() const;

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
    void ConsumeHead(size_t aConsumeLength);

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
    CHECK_RETURN_VALUE bool EnsureReservedSize(uint16_t aReservedSize);

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
     * Copies data from the payloads of a chain of packet buffers until a given amount of data has been copied.
     *
     * @param[in]  buf             Destination buffer; must be at least @a length bytes.
     * @param[in]  length          Destination buffer length.
     *
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL If the total length of the payloads in the chain is less than the requested @a length.
     * @retval #CHIP_ERROR_INTERNAL         In case of an inconsistency in the buffer chain.
     * @retval #CHIP_NO_ERROR               If the requested payload has been copied.
     */
    CHIP_ERROR Read(uint8_t * buf, size_t length) const;
    template <size_t N>
    inline CHIP_ERROR Read(uint8_t (&buf)[N]) const
    {
        return Read(buf, N);
    }

    /**
     * Perform an implementation-defined check on the validity of a PacketBuffer pointer.
     *
     * Unless enabled by #CHIP_CONFIG_MEMORY_DEBUG_CHECKS == 1, this function does nothing.
     *
     * When enabled, it performs an implementation- and configuration-defined check on
     * the validity of the packet buffer. It MAY log an error and/or abort the program
     * if the packet buffer or the implementation-defined memory management system is in
     * a faulty state. (Some configurations may not actually perform any check.)
     *
     * @note  A null pointer is not considered faulty.
     *
     *  @param[in] buffer - the packet buffer to check.
     */
    static void Check(const PacketBuffer * buffer)
    {
#if CHIP_SYSTEM_PACKETBUFFER_HAS_CHECK
        InternalCheck(buffer);
#endif
    }

private:
    // Memory required for a maximum-size PacketBuffer.
    static constexpr uint16_t kBlockSize = PacketBuffer::kStructureSize + PacketBuffer::kMaxSizeWithoutReserve;

    // Note: this condition includes DOXYGEN to work around a Doxygen error. DOXYGEN is never defined in any actual build.
#if CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL || defined(DOXYGEN)
    typedef union
    {
        pbuf Header;
        uint8_t Block[PacketBuffer::kBlockSize];
    } BufferPoolElement;
    static BufferPoolElement sBufferPool[CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE];
    static PacketBuffer * sFreeList;
    static PacketBuffer * BuildFreeList();
#endif // CHIP_SYSTEM_PACKETBUFFER_FROM_CHIP_POOL || defined(DOXYGEN)

#if CHIP_SYSTEM_PACKETBUFFER_HAS_CHECK
    static void InternalCheck(const PacketBuffer * buffer);
#endif

    void AddRef();
    bool HasSoleOwnership() const { return (this->ref == 1); }
    static void Free(PacketBuffer * aPacket);
    static PacketBuffer * FreeHead(PacketBuffer * aPacket);

    PacketBuffer * ChainedBuffer() const { return static_cast<PacketBuffer *>(this->next); }
    PacketBuffer * Consume(size_t aConsumeLength);
    void Clear();
    void SetDataLength(size_t aNewLen, PacketBuffer * aChainHead);

    /**
     * Get a pointer to the start of the reserved space (which comes before the
     * payload).  The actual reserved space is the ReservedSize() bytes starting
     * at this pointer.
     */
    uint8_t * ReserveStart();
    const uint8_t * ReserveStart() const;

    friend class PacketBufferHandle;
    friend class TestSystemPacketBuffer;
};

static_assert(sizeof(pbuf) == sizeof(PacketBuffer), "PacketBuffer must not have additional members");

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
     * Test whether this PacketBufferHandle is empty, or conversely owns a PacketBuffer.
     *
     * @return \c true if this PacketBufferHandle is empty; return \c false if it owns a PacketBuffer.
     */
    bool IsNull() const { return mBuffer == nullptr; }

    /**
     * Test whether the PacketBuffer owned by this PacketBufferHandle has unique ownership.
     *
     * @return \c true if the PacketBuffer owned by this PacketBufferHandle is solely owned; return \c false if
     * it has more than one ownership.
     */
    bool HasSoleOwnership() const { return mBuffer->HasSoleOwnership(); }

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
     * Add the given packet buffer to the end of the buffer chain, adjusting the total length of each buffer in the chain
     * accordingly.
     *
     *  @note The current packet buffer handle must either be the head of the buffer chain for the lengths to be adjusted properly,
     *        or be null (in which case it becomes the head).
     *
     *  @note Ownership is transferred from the argument to the `next` link at the end of the current chain,
     *        or to the handle if it's currently null.
     *
     *  @param[in] aPacket - the packet buffer to be added to the end of the current chain.
     */
    void AddToEnd(PacketBufferHandle && aPacket)
    {
        if (IsNull())
        {
            mBuffer         = aPacket.mBuffer;
            aPacket.mBuffer = nullptr;
        }
        else
        {
            mBuffer->AddToEnd(std::move(aPacket));
        }
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
    void Consume(size_t aConsumeLength) { mBuffer = mBuffer->Consume(aConsumeLength); }

    /**
     * Copy the given buffer to a right-sized buffer if applicable.
     *
     * Only operates on single buffers (for chains, use \c CompactHead() and RightSize the tail).
     * Requires that this handle be the only reference to the underlying buffer.
     */
    void RightSize()
    {
#if CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE
        InternalRightSize();
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
     *       and is responsible for managing it safely.
     *
     * @note The ref-qualifier `&&` requires the caller to use `std::move` to emphasize that ownership is
     *       moved out of this handle.
     */
    CHECK_RETURN_VALUE PacketBuffer * UnsafeRelease() &&
    {
        PacketBuffer::Check(mBuffer);
        PacketBuffer * buffer = mBuffer;
        mBuffer               = nullptr;
        return buffer;
    }

    /**
     * Allocates a packet buffer.
     *
     *  A packet buffer is conceptually divided into two parts:
     *  @li  Space reserved for network protocol headers. The size of this space normally defaults to a value determined
     *       by the network layer configuration, but can be given explicity by \c aReservedSize for special cases.
     *  @li  Space for application data. The minimum size of this space is given by \c aAvailableSize, and then \c Start()
     *       provides a pointer to the start of this space.
     *
     *  Fails and returns \c nullptr if no memory is available, or if the size requested is too large.
     *  When the sum of \a aAvailableSize and \a aReservedSize is no greater than \c PacketBuffer::kMaxSizeWithoutReserve,
     *  that is guaranteed not to be too large.
     *
     *  On success, it is guaranteed that \c AvailableDataSize() is no less than \a aAvailableSize.
     *
     *  @param[in]  aAvailableSize  Minimum number of octets to for application data (at `Start()`).
     *  @param[in]  aReservedSize   Number of octets to reserve for protocol headers (before `Start()`).
     *
     *  @return     On success, a PacketBufferHandle to the allocated buffer. On fail, \c nullptr.
     */
    static PacketBufferHandle New(size_t aAvailableSize, uint16_t aReservedSize = PacketBuffer::kDefaultHeaderReserve);

    /**
     * Allocates a packet buffer with initial contents.
     *
     *  @param[in]  aData           Initial buffer contents.
     *  @param[in]  aDataSize       Size of initial buffer contents.
     *  @param[in]  aAdditionalSize Size of additional application data space after the initial contents.
     *  @param[in]  aReservedSize   Number of octets to reserve for protocol headers.
     *
     *  @return     On success, a PacketBufferHandle to the allocated buffer. On fail, \c nullptr.
     */
    static PacketBufferHandle NewWithData(const void * aData, size_t aDataSize, size_t aAdditionalSize = 0,
                                          uint16_t aReservedSize = PacketBuffer::kDefaultHeaderReserve);

    /**
     * Creates a copy of a packet buffer (or chain).
     *
     * @returns empty handle on allocation failure. Otherwise, the returned buffer has the same sizes and contents as the original.
     */
    PacketBufferHandle CloneData() const;

    /**
     * Perform an implementation-defined check on the validity of a PacketBufferHandle.
     *
     * Unless enabled by #CHIP_CONFIG_MEMORY_DEBUG_CHECKS == 1, this function does nothing.
     *
     * When enabled, it performs an implementation- and configuration-defined check on
     * the validity of the packet buffer. It MAY log an error and/or abort the program
     * if the packet buffer or the implementation-defined memory management system is in
     * a faulty state. (Some configurations may not actually perform any check.)
     *
     * @note  A null handle is not considered faulty.
     */
    void Check() const
    {
#if CHIP_SYSTEM_PACKETBUFFER_HAS_CHECK
        PacketBuffer::Check(mBuffer);
#endif
    }

    bool operator==(const PacketBufferHandle & aOther) const { return mBuffer == aOther.mBuffer; }

protected:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // For use via LwIPPacketBufferView only.
    static struct pbuf * GetLwIPpbuf(const PacketBufferHandle & handle)
    {
        PacketBuffer::Check(handle.mBuffer);
        return static_cast<struct pbuf *>(handle.mBuffer);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

private:
    PacketBufferHandle(const PacketBufferHandle &)             = delete;
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
    PacketBuffer * GetNext() const { return static_cast<PacketBuffer *>(mBuffer->next); }

#if CHIP_SYSTEM_PACKETBUFFER_HAS_RIGHTSIZE
    void InternalRightSize();
#endif

    PacketBuffer * mBuffer;

    friend class PacketBuffer;
    friend class TestSystemPacketBuffer;
};

inline void PacketBuffer::SetDataLength(size_t aNewLen, const PacketBufferHandle & aChainHead)
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

namespace Encoding {

class PacketBufferWriterUtil
{
private:
    template <typename>
    friend class PacketBufferWriterBase;
    static System::PacketBufferHandle Finalize(BufferWriter & aBufferWriter, System::PacketBufferHandle & aPacket);
};

/**
 * BufferWriter backed by packet buffer.
 *
 * Typical use:
 *  @code
 *      PacketBufferWriter buf(maximumLength);
 *      if (buf.IsNull()) { return CHIP_ERROR_NO_MEMORY; }
 *      buf.Put(...);
 *      ...
 *      PacketBufferHandle handle = buf.Finalize();
 *      if (buf.IsNull()) { return CHIP_ERROR_BUFFER_TOO_SMALL; }
 *      // valid data
 *  @endcode
 */
template <class Writer>
class PacketBufferWriterBase : public Writer
{
public:
    /**
     * Constructs a BufferWriter that writes into a packet buffer, using all available space.
     *
     *  @param[in]  aPacket  A handle to PacketBuffer, to be used as backing store for the BufferWriter.
     */
    PacketBufferWriterBase(System::PacketBufferHandle && aPacket) :
        Writer(aPacket->Start() + aPacket->DataLength(), aPacket->AvailableDataLength())
    {
        mPacket = std::move(aPacket);
    }

    /**
     * Constructs a BufferWriter that writes into a packet buffer, using no more than the requested space.
     *
     *  @param[in]  aPacket A handle to PacketBuffer, to be used as backing store for the BufferWriter.
     *  @param[in]  aSize   Maximum number of octects to write into the packet buffer.
     */
    PacketBufferWriterBase(System::PacketBufferHandle && aPacket, size_t aSize) :
        Writer(aPacket->Start() + aPacket->DataLength(), chip::min(aSize, static_cast<size_t>(aPacket->AvailableDataLength())))
    {
        mPacket = std::move(aPacket);
    }

    /**
     * Test whether this PacketBufferWriter is null, or conversely owns a PacketBuffer.
     *
     * @retval true     The PacketBufferWriter is null; it does not own a PacketBuffer. This implies either that
     *                  construction failed, or that \c Finalize() has previously been called to release the buffer.
     * @retval false    The PacketBufferWriter owns a PacketBuffer, which can be written using BufferWriter \c Put() methods,
     *                  and (assuming no overflow) obtained by calling \c Finalize().
     */
    bool IsNull() const { return mPacket.IsNull(); }

    /**
     * Obtain the backing packet buffer, if it is valid.
     *
     *  If construction succeeded, \c Finalize() has not already been called, and \c BufferWriter::Fit() is true,
     *  the caller takes ownership of a buffer containing the desired data. Otherwise, the returned handle tests null,
     *  and any underlying storage has been released.
     *
     *  @return     A packet buffer handle.
     */
    System::PacketBufferHandle Finalize() { return PacketBufferWriterUtil::Finalize(*this, mPacket); }

private:
    System::PacketBufferHandle mPacket;
};

using PacketBufferWriter = PacketBufferWriterBase<chip::Encoding::BufferWriter>;

namespace LittleEndian {
using PacketBufferWriter = PacketBufferWriterBase<chip::Encoding::LittleEndian::BufferWriter>;
} // namespace LittleEndian

namespace BigEndian {
using PacketBufferWriter = PacketBufferWriterBase<chip::Encoding::BigEndian::BufferWriter>;
} // namespace BigEndian

} // namespace Encoding

} // namespace chip

#if CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {

namespace Inet {
class UDPEndPointImplLwIP;
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
    friend class Inet::UDPEndPointImplLwIP;
};

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
