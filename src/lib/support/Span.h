/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <cstdint>
#include <cstdlib>
#include <string.h>
#include <type_traits>

#include <support/CodeUtils.h>

namespace chip {

/**
 * @brief A wrapper class for holding objects and its length, without the ownership of it.
 * We can use C++20 std::span once we support it, the data() and size() come from C++20 std::span.
 */
template <class T>
class Span
{
public:
    using pointer = T *;

    constexpr Span() : mDataBuf(nullptr), mDataLen(0) {}
    constexpr Span(pointer databuf, size_t datalen) : mDataBuf(databuf), mDataLen(datalen) {}
    template <size_t N>
    constexpr explicit Span(T (&databuf)[N]) : Span(databuf, N)
    {}

    constexpr pointer data() const { return mDataBuf; }
    size_t size() const { return mDataLen; }
    bool empty() const { return size() == 0; }

    // Allow data_equal for spans that are over the same type up to const-ness.
    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const Span<U> & other) const
    {
        return (size() == other.size()) && (empty() || (memcmp(data(), other.data(), size() * sizeof(T)) == 0));
    }

    Span SubSpan(size_t offset, size_t length) const
    {
        VerifyOrDie(offset <= mDataLen);
        VerifyOrDie(length <= mDataLen - offset);
        return Span(mDataBuf + offset, length);
    }

    // Allow converting a span with non-const T into a span with const T.
    template <class U = T>
    operator typename std::enable_if_t<!std::is_const<U>::value, Span<const T>>() const
    {
        return Span<const T>(data(), size());
    }

    // Allow reducing the size of a span.
    void reduce_size(size_t new_size)
    {
        VerifyOrDie(new_size <= size());
        mDataLen = new_size;
    }

private:
    pointer mDataBuf;
    size_t mDataLen;
};

template <class T, size_t N>
class FixedSpan
{
public:
    using pointer = T *;

    constexpr FixedSpan() : mDataBuf(nullptr) {}
    constexpr explicit FixedSpan(pointer databuf) : mDataBuf(databuf) {}

    constexpr pointer data() const { return mDataBuf; }
    size_t size() const { return N; }
    bool empty() const { return data() == nullptr; }

    // Allow data_equal for spans that are over the same type up to const-ness.
    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const FixedSpan<U, N> & other) const
    {
        return (empty() && other.empty()) ||
            (!empty() && !other.empty() && (memcmp(data(), other.data(), size() * sizeof(T)) == 0));
    }

    // Allow converting a span with non-const T into a span with const T.
    template <class U = T>
    operator typename std::enable_if_t<!std::is_const<U>::value, FixedSpan<const T, N>>() const
    {
        return FixedSpan<const T, N>(data());
    }

private:
    pointer mDataBuf;
};

using ByteSpan        = Span<const uint8_t>;
using MutableByteSpan = Span<uint8_t>;
template <size_t N>
using FixedByteSpan = FixedSpan<const uint8_t, N>;

} // namespace chip
