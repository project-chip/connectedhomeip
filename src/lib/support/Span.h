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

#include <lib/support/CodeUtils.h>

namespace chip {

template <class T, size_t N>
class FixedSpan;

/**
 * @brief A wrapper class for holding objects and its length, without the ownership of it.
 * We can use C++20 std::span once we support it, the data() and size() come from C++20 std::span.
 */
template <class T>
class Span
{
public:
    using pointer       = T *;
    using const_pointer = const T *;

    constexpr Span() : mDataBuf(nullptr), mDataLen(0) {}
    constexpr Span(pointer databuf, size_t datalen) : mDataBuf(databuf), mDataLen(datalen) {}
    template <size_t N>
    constexpr explicit Span(T (&databuf)[N]) : Span(databuf, N)
    {}

    // Allow implicit construction from a Span over a type that matches our
    // type, up to const-ness.
    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    constexpr Span(const Span<U> & other) : Span(other.data(), other.size())
    {}

    // Allow implicit construction from a FixedSpan over a type that matches our
    // type, up to const-ness.
    template <class U, size_t N, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    constexpr inline Span(const FixedSpan<U, N> & other);

    constexpr pointer data() const { return mDataBuf; }
    constexpr size_t size() const { return mDataLen; }
    constexpr bool empty() const { return size() == 0; }
    constexpr const_pointer begin() const { return data(); }
    constexpr const_pointer end() const { return data() + size(); }
    constexpr pointer begin() { return data(); }
    constexpr pointer end() { return data() + size(); }

    // Allow data_equal for spans that are over the same type up to const-ness.
    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const Span<U> & other) const
    {
        return (size() == other.size()) && (empty() || (memcmp(data(), other.data(), size() * sizeof(T)) == 0));
    }
    template <class U, size_t N, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    inline bool data_equal(const FixedSpan<U, N> & other) const;

    Span SubSpan(size_t offset, size_t length) const
    {
        VerifyOrDie(offset <= mDataLen);
        VerifyOrDie(length <= mDataLen - offset);
        return Span(mDataBuf + offset, length);
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
    using pointer       = T *;
    using const_pointer = const T *;

    constexpr FixedSpan() : mDataBuf(nullptr) {}

    // We want to allow construction from things that look like T*, but we want
    // to make construction from an array use the constructor that asserts the
    // array is big enough.  This requires that both constructors be templates
    // (because otherwise the non-template would be favored by overload
    // resolution, since due to decay to pointer it matches just as well as the
    // template).
    //
    // To do that we have a template constructor enabled only when the type
    // passed to it is a pointer type, and that pointer is to a type that
    // matches T up to const-ness.  Importantly, we do NOT want to allow
    // subclasses of T here, because they would have a different size and our
    // Span would not work right.
    template <
        class U,
        typename = std::enable_if_t<std::is_pointer<U>::value &&
                                    std::is_same<std::remove_const_t<T>, std::remove_const_t<std::remove_pointer_t<U>>>::value>>
    constexpr explicit FixedSpan(U databuf) : mDataBuf(databuf)
    {}
    template <class U, size_t M, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    constexpr explicit FixedSpan(U (&databuf)[M]) : mDataBuf(databuf)
    {
        static_assert(M >= N, "Passed-in buffer too small for FixedSpan");
    }

    // Allow implicit construction from a FixedSpan of sufficient size over a
    // type that matches our type, up to const-ness.
    template <class U, size_t M, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    constexpr FixedSpan(FixedSpan<U, M> const & other) : mDataBuf(other.data())
    {
        static_assert(M >= N, "Passed-in FixedSpan is smaller than we are");
    }

    constexpr pointer data() const { return mDataBuf; }
    constexpr size_t size() const { return N; }
    constexpr bool empty() const { return data() == nullptr; }

    constexpr pointer begin() { return mDataBuf; }
    constexpr pointer end() { return mDataBuf + N; }
    constexpr const_pointer begin() const { return mDataBuf; }
    constexpr const_pointer end() const { return mDataBuf + N; }

    // Allow data_equal for spans that are over the same type up to const-ness.
    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const FixedSpan<U, N> & other) const
    {
        return (empty() && other.empty()) ||
            (!empty() && !other.empty() && (memcmp(data(), other.data(), size() * sizeof(T)) == 0));
    }

    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const Span<U> & other) const
    {
        return (size() == other.size() && (empty() || (memcmp(data(), other.data(), size() * sizeof(T)) == 0)));
    }

private:
    pointer mDataBuf;
};

template <class T>
template <class U, size_t N, typename>
constexpr Span<T>::Span(const FixedSpan<U, N> & other) : Span(other.data(), other.size())
{}

template <class T>
template <class U, size_t N, typename>
inline bool Span<T>::data_equal(const FixedSpan<U, N> & other) const
{
    return (size() == other.size()) && (empty() || (memcmp(data(), other.data(), size() * sizeof(T)) == 0));
}

/**
 * @brief Returns true if the `span` could be used to access some data,
 *        false otherwise.
 * @param[in] span The Span to validate.
 */
template <typename T>
inline bool IsSpanUsable(const Span<T> & span)
{
    return (span.data() != nullptr) && (span.size() > 0);
}

/**
 * @brief Returns true if the `span` could be used to access some data,
 *        false otherwise.
 * @param[in] span The FixedSpan to validate.
 */
template <typename T, size_t N>
inline bool IsSpanUsable(const FixedSpan<T, N> & span)
{
    return (span.data() != nullptr);
}

using ByteSpan        = Span<const uint8_t>;
using MutableByteSpan = Span<uint8_t>;
template <size_t N>
using FixedByteSpan = FixedSpan<const uint8_t, N>;

using MutableCharSpan = Span<char>;

} // namespace chip
