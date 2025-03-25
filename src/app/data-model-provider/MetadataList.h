/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

#include <cstdint>
#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

namespace detail {

class GenericAppendOnlyBuffer
{
public:
    GenericAppendOnlyBuffer(size_t elementSize) : mElementSize(elementSize) {}
    ~GenericAppendOnlyBuffer();

    GenericAppendOnlyBuffer(GenericAppendOnlyBuffer && other);
    GenericAppendOnlyBuffer & operator=(GenericAppendOnlyBuffer &&);

    GenericAppendOnlyBuffer()                                            = delete;
    GenericAppendOnlyBuffer & operator=(const GenericAppendOnlyBuffer &) = delete;

    /// Ensure that at least the specified number of elements
    /// can be appended to the internal buffer;
    ///
    /// This will cause the internal buffer to become an allocated buffer
    CHIP_ERROR EnsureAppendCapacity(size_t numElements);

    bool IsEmpty() const { return mElementCount == 0; }

    /// Number of elements stored in the object.
    size_t Size() const { return mElementCount; }

protected:
    /// Appends a single element of mElementSize size.
    ///
    /// ALWAYS COPIES the given element internally.
    /// Sufficient capacity MUST exist to append.
    CHIP_ERROR AppendSingleElementRaw(const void * buffer);

    /// Appends a list of elements from a raw array.
    ///
    /// This ALWAYS COPIES the elements internally.
    /// Additional capacity is AUTOMATICALLY ADDED.
    ///
    /// buffer MUST NOT point inside "own" buffer as mBuffer may be reallocated
    /// as part of the appending.
    CHIP_ERROR AppendElementArrayRaw(const void * __restrict__ buffer, size_t numElements);

    /// Appends a list of elements from a raw array.
    ///
    /// If the buffer contains no elements, this will just REFERENCE the given
    /// buffer, so its lifetime MUST be longer than the lifetime of this buffer and
    /// its usage.
    ///
    /// If the buffer already contains some elements, this will AUTOMATICALLY
    /// add additional capacity and COPY the elements at the end of the internal array.
    CHIP_ERROR ReferenceExistingElementArrayRaw(const void * buffer, size_t numElements);

    /// release ownership of any used buffer.
    ///
    /// Returns the current buffer details and releases ownership of it (clears internal state)
    void ReleaseBuffer(void *& buffer, size_t & size, bool & allocated);

private:
    const size_t mElementSize; // size of one element in the buffer
    uint8_t * mBuffer       = nullptr;
    size_t mElementCount    = 0;     // how many elements are stored in the class
    size_t mCapacity        = 0;     // how many elements can be stored in total in mBuffer
    bool mBufferIsAllocated = false; // if mBuffer is an allocated buffer
};

/// Represents a RAII instance owning a buffer.
///
/// It auto-frees the owned buffer on destruction via `Platform::MemoryFree`.
///
/// This class is designed to be a storage class for `GenericAppendOnlyBuffer` and
/// its subclasses (i.e. GenericAppendOnlyBuffer uses PlatformMemory and this class
/// does the same. They MUST be kept in sync.)
class ScopedBuffer
{
public:
    ScopedBuffer(void * buffer) : mBuffer(buffer) {}
    ~ScopedBuffer();

    ScopedBuffer(const ScopedBuffer &)             = delete;
    ScopedBuffer & operator=(const ScopedBuffer &) = delete;

    ScopedBuffer(ScopedBuffer && other) : mBuffer(other.mBuffer) { other.mBuffer = nullptr; }
    ScopedBuffer & operator=(ScopedBuffer && other);

private:
    void * mBuffer;
};

} // namespace detail

template <typename T>
class ReadOnlyBuffer : public Span<const T>, detail::ScopedBuffer
{
public:
    ReadOnlyBuffer() : ScopedBuffer(nullptr) {}
    ReadOnlyBuffer(const T * buffer, size_t size, bool allocated) :
        Span<const T>(buffer, size), ScopedBuffer(allocated ? const_cast<void *>(static_cast<const void *>(buffer)) : nullptr)
    {}
    ~ReadOnlyBuffer() = default;

    ReadOnlyBuffer & operator=(ReadOnlyBuffer && other)
    {
        *static_cast<Span<const T> *>(this) = other;
        *static_cast<ScopedBuffer *>(this)  = std::move(other);
        return *this;
    }
};

template <typename T>
class ListBuilder : public detail::GenericAppendOnlyBuffer
{
public:
    using SpanType = Span<const T>;

    // we do not call destructors, just malloc things.
    // Note that classes should also be trivially assignable (we do NOT call the assignment operator)
    // This makes this class somewhat dangerous...
    //
    // Note: ideally we would want `is_trivially_copyable_v` as well however our chip::Optional
    //       implementation does not actually report that.
    static_assert(std::is_trivially_destructible_v<T>);

    ListBuilder() : GenericAppendOnlyBuffer(sizeof(T)) {}

    ListBuilder(const ListBuilder &)                   = delete;
    ListBuilder & operator=(const ListBuilder & other) = delete;

    ListBuilder(ListBuilder && other) : GenericAppendOnlyBuffer{ std::move(other) } {}

    ListBuilder & operator=(ListBuilder && other)
    {
        detail::GenericAppendOnlyBuffer::operator=(std::move(other));
        return *this;
    }

    /// Reference methods attempt to reference the existing array IN PLACE
    /// so its lifetime is assumed to be longer than the usage of this list.
    CHIP_ERROR ReferenceExisting(SpanType span) { return ReferenceExistingElementArrayRaw(span.data(), span.size()); }

    /// Append always attempts to append/extend existing memory.
    ///
    /// Automatically attempts to allocate sufficient space to fulfill the element
    /// requirements.
    ///
    /// `span` MUST NOT point inside "own" buffer (and generally will not
    /// as this class does not expose buffer access except by releasing ownership
    /// via `Take`)
    CHIP_ERROR AppendElements(SpanType span) { return AppendElementArrayRaw(span.data(), span.size()); }

    /// Append a single element.
    /// Sufficent append capacity MUST exist.
    CHIP_ERROR Append(const T & value) { return AppendSingleElementRaw(&value); }

    /// Once a list is built, the data is taken as a scoped SPAN that owns its data
    /// and the original list is cleared
    ReadOnlyBuffer<T> TakeBuffer()
    {
        void * buffer;
        size_t size;
        bool allocated;
        ReleaseBuffer(buffer, size, allocated);

        return ReadOnlyBuffer<T>(static_cast<const T *>(buffer), size, allocated);
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
