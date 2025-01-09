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
#include <stddef.h>
#include <type_traits>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace DataModel {

template <typename T>
class MetadataList
{
public:
    using SpanType = Span<const T>;

    MetadataList() {}
    MetadataList(const MetadataList &)                   = delete;
    MetadataList & operator=(const MetadataList & other) = delete;

    MetadataList(MetadataList && other) { *this = std::move(other); }

    MetadataList & operator=(MetadataList && other)
    {
        if (this != &other)
        {
            this->mBackingStorage.Free();

            this->mBackingStorage = std::move(other.mBackingStorage);
            this->mSize           = other.mSize;
            this->mIsImmutable    = other.mIsImmutable;
            this->mSpan           = other.mSpan;

            other.mIsImmutable = true;
            other.mSize        = 0;
            other.mSpan        = Span<T>{};
        }
        return *this;
    }

    const T & operator[](size_t index)
    {
        VerifyOrDie((mSize > 0) && (index < mSize));

        if (this->mBackingStorage)
        {
            return Span<const T>{ mBackingStorage.Get(), mSize }[index];
        }
        else
        {
            return mSpan[index];
        }
    }

    template <size_t N>
    static MetadataList FromArray(const std::array<T, N> & arr)
    {
        MetadataList<T> list;

        if (list.reserve(N) != CHIP_NO_ERROR)
        {
            list.Invalidate();
            return list;
        }

        for (const auto & item : arr)
        {
            if (list.Append(item) != CHIP_NO_ERROR)
            {
                list.Invalidate();
                return list;
            }
        }

        list.mIsImmutable = true;
        return list;
    }

    template <size_t N>
    static MetadataList FromConstArray(const std::array<const T, N> & arr)
    {
        MetadataList<T> list;
        list.mSpan        = Span<const T>{ arr.data(), arr.size() };
        list.mSize        = arr.size();
        list.mIsImmutable = true;

        return list;
    }

    static MetadataList FromConstSpan(const Span<const T> & span)
    {
        MetadataList<T> list;
        list.mSpan        = span;
        list.mSize        = span.size();
        list.mIsImmutable = true;

        return list;
    }

    CHIP_ERROR reserve(size_t numElements)
    {
        VerifyOrReturnError(!mIsImmutable, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mSize == 0, CHIP_ERROR_INCORRECT_STATE);
        if (!mBackingStorage.Calloc(numElements))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        return CHIP_NO_ERROR;
    }

    size_t size() const { return mSize; }

    bool empty() const { return size() == 0u; }

    size_t capacity() const { return std::max(mSize, mBackingStorage.AllocatedSize()); }

    void Invalidate()
    {
        mBackingStorage.Free();
        mSize        = 0;
        mIsImmutable = true;
    }

    Span<const T> GetSpanValidForLifetime() const
    {
        static_assert(std::is_same<SpanType, Span<const T>>::value, "SpanType must remain Span<const T>");
        if (mBackingStorage)
        {
            mSpan = Span<const T>{ mBackingStorage.Get(), mSize };
        }

        mIsImmutable = true;
        return mSpan;
    }

    CHIP_ERROR Append(const T & value)
    {
        VerifyOrReturnError(!mIsImmutable, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mSize < mBackingStorage.AllocatedSize(), CHIP_ERROR_NO_MEMORY);

        mBackingStorage[mSize++] = value;
        return CHIP_NO_ERROR;
    }

protected:
    chip::Platform::ScopedMemoryBufferWithSize<T> mBackingStorage;
    mutable Span<const T> mSpan;
    size_t mSize = 0;

    // Set to true as soon as a span is obtained, since more writes may invalidate the span.
    mutable bool mIsImmutable = false;
};

} // namespace DataModel
} // namespace app
} // namespace chip
