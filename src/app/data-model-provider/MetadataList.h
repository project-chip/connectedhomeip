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

#include <algorithm>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

// essentially a `void *` untyped metadata list base,
// so that actual functionality does not template-explode
class GenericMetadataList
{
public:
    GenericMetadataList(size_t elementSize) : mElementSize(elementSize) {}
    ~GenericMetadataList();

    GenericMetadataList(const GenericMetadataList &)                   = delete;
    GenericMetadataList & operator=(const GenericMetadataList & other) = delete;

    GenericMetadataList(GenericMetadataList && other) { *this = std::move(other); }

    GenericMetadataList & operator=(GenericMetadataList && other);
    const void * operator[](size_t index) const;

    CHIP_ERROR reserve(size_t numElements);
    void Invalidate();

    size_t elementCount() const { return mElementCount; }
    size_t capacity() const { return mCapacity; }
    bool empty() const { return mElementCount == 0; }

protected:
    /// Copy over the data from the given buffer
    CHIP_ERROR CopyExistingBuffer(const void * buffer, size_t elements);

    /// use existing buffer AS IS
    void AquireExistingBuffer(const void * buffer, size_t elements);

    CHIP_ERROR AppendRaw(const void * buffer);
    const void * RawBuffer() const { return mBuffer; }

    /// Marks a list as immutable and immutability is aquired
    /// during const access (so this is const)
    void SetImmutable() const { mIsImmutable = true; }

private:
    bool mAllocated = false;

    // buffer may point to both allocated or re-used (e.g. from const arrays) buffers.
    // buffer is assumed allocated if mAllocated is true.
    uint8_t * mBuffer = nullptr;
    size_t mElementSize;
    size_t mElementCount = 0;
    size_t mCapacity     = 0;

    // Set to true as soon as a span is obtained, since more writes may invalidate the span.
    mutable bool mIsImmutable = false;
};

template <typename T>
class MetadataList : public GenericMetadataList
{
public:
    using SpanType = Span<const T>;

    MetadataList() : GenericMetadataList(sizeof(T)) {}
    MetadataList(const MetadataList &)                   = delete;
    MetadataList & operator=(const MetadataList & other) = delete;

    MetadataList(MetadataList && other) : GenericMetadataList(sizeof(T)) { *this = std::move(other); }

    MetadataList & operator=(MetadataList && other)
    {
        *((GenericMetadataList *) this) = std::move(other);
        return *this;
    }

    const T & operator[](size_t index) { return *static_cast<const T *>(GenericMetadataList::operator[](index)); }

    template <size_t N>
    static MetadataList FromArray(const std::array<T, N> & arr)
    {
        MetadataList<T> list;

        if (list.CopyExistingBuffer(arr.data(), arr.size()) != CHIP_NO_ERROR)
        {
            list.Invalidate();
        }

        return list;
    }

    template <size_t N>
    static MetadataList FromConstArray(const std::array<const T, N> & arr)
    {
        MetadataList<T> list;
        list.AquireExistingBuffer(arr.data(), arr.size());
        return list;
    }

    static MetadataList FromConstSpan(const Span<const T> & span)
    {
        MetadataList<T> list;
        list.AquireExistingBuffer(span.data(), span.size());
        return list;
    }

    size_t size() const { return elementCount(); }

    Span<const T> GetSpanValidForLifetime() const
    {
        SetImmutable();
        return Span<const T>(static_cast<const T *>(RawBuffer()), elementCount());
    }

    CHIP_ERROR Append(const T & value) { return AppendRaw(&value); }
};

} // namespace DataModel
} // namespace app
} // namespace chip
