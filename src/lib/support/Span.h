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

#include <array>
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
    using pointer   = T *;
    using reference = T &;

    constexpr Span() : mDataBuf(nullptr), mDataLen(0) {}

    // Note: VerifyOrDie cannot be used inside a constexpr function, because it uses
    // "static" on some platforms (e.g. when CHIP_PW_TOKENIZER_LOGGING is true)
    // and that's not allowed in constexpr functions.

    Span(pointer databuf, size_t datalen) : mDataBuf(databuf), mDataLen(datalen)
    {
        VerifyOrDie(databuf != nullptr || datalen == 0); // not constexpr on some platforms
    }

    // A Span can only point to null if it is empty (size == 0). The default constructor
    // should be used to construct empty Spans. All other cases involving null are invalid.
    Span(std::nullptr_t null, size_t size) = delete;

    template <class U, size_t N, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr explicit Span(U (&databuf)[N]) : mDataBuf(databuf), mDataLen(N)
    {}

    template <class U, size_t N, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr Span(std::array<U, N> & arr) : mDataBuf(arr.data()), mDataLen(N)
    {}

    template <size_t N>
    constexpr Span & operator=(T (&databuf)[N])
    {
        mDataBuf = databuf;
        mDataLen = N;
        return (*this);
    }

    // Allow implicit construction from a Span over a type that matches our
    // type's size, if a pointer to the other type can be treated as a pointer
    // to our type (e.g. other type is same as ours, or is a same-size
    // subclass).  The size check is really important to make sure we don't get
    // confused about where our object boundaries are.
    template <class U, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr Span(const Span<U> & other) : Span(other.data(), other.size())
    {}

    // Allow implicit construction from a FixedSpan over a type that matches our
    // type's size, if a pointer to the other type can be treated as a pointer
    // to our type (e.g. other type is same as ours, or is a same-size
    // subclass).  The size check is really important to make sure we don't get
    // confused about where our object boundaries are.
    template <class U, size_t N, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr inline Span(const FixedSpan<U, N> & other);

    constexpr pointer data() const { return mDataBuf; }
    constexpr size_t size() const { return mDataLen; }
    constexpr bool empty() const { return size() == 0; }
    constexpr pointer begin() const { return data(); }
    constexpr pointer end() const { return data() + size(); }

    // Element accessors, matching the std::span API.
    reference operator[](size_t index) const
    {
        VerifyOrDie(index < size()); // not constexpr on some platforms
        return data()[index];
    }
    reference front() const { return (*this)[0]; }
    reference back() const { return (*this)[size() - 1]; }

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

    Span SubSpan(size_t offset) const
    {
        VerifyOrDie(offset <= mDataLen);
        return Span(mDataBuf + offset, mDataLen - offset);
    }

    // Allow reducing the size of a span.
    void reduce_size(size_t new_size)
    {
        VerifyOrDie(new_size <= size());
        mDataLen = new_size;
    }

    // Allow creating ByteSpans and CharSpans from ZCL octet strings, so we
    // don't have to reinvent it various places.
    template <class U,
              typename = std::enable_if_t<std::is_same<uint8_t, std::remove_const_t<U>>::value &&
                                          (std::is_same<const uint8_t, T>::value || std::is_same<const char, T>::value)>>
    static Span fromZclString(U * bytes)
    {
        size_t length = bytes[0];
        // Treat 0xFF (aka "null string") as zero-length.
        if (length == 0xFF)
        {
            length = 0;
        }
        // Need reinterpret_cast if we're a CharSpan.
        return Span(reinterpret_cast<T *>(&bytes[1]), length);
    }

    // Allow creating CharSpans from a character string.
    template <class U, typename = std::enable_if_t<std::is_same<T, const U>::value && std::is_same<const char, T>::value>>
    static Span fromCharString(U * chars)
    {
        return Span(chars, strlen(chars));
    }

    // operator== explicitly not implemented on Span, because its meaning
    // (equality of data, or pointing to the same buffer and same length) is
    // ambiguous.  Use data_equal if testing for equality of data.
    template <typename U>
    bool operator==(const Span<U> & other) const = delete;

private:
    pointer mDataBuf;
    size_t mDataLen;
};

namespace detail {

// To make FixedSpan (specifically various FixedByteSpan types) default constructible
// without creating a weird "empty() == true but size() != 0" state, we need an
// appropriate sized array of zeroes. With a naive definition like
//      template <class T, size_t N> constexpr T kZero[N] {};
// we would end up with separate zero arrays for each size, and might also accidentally
// increase the read-only data size of the binary by a large amount. Instead, we define
// a per-type limit for the zero array, FixedSpan won't be default constructible for
// T / N combinations that exceed the limit. The default limit is 0.
template <class T>
struct zero_limit : std::integral_constant<size_t, 0>
{
};

// FixedByteSpan types up to N=65 currently need to be default-constructible.
template <>
struct zero_limit<uint8_t> : std::integral_constant<size_t, 65>
{
};

template <class T>
inline constexpr T kZeroes[zero_limit<T>::value]{};

template <class T, size_t N>
constexpr T const * shared_zeroes()
{
    static_assert(N <= zero_limit<typename std::remove_const<T>::type>::value, "N exceeds zero_limit<T>");
    return kZeroes<typename std::remove_const<T>::type>;
}

} // namespace detail

/**
 * Similar to a Span but with a fixed size.
 */
template <class T, size_t N>
class FixedSpan
{
public:
    using pointer   = T *;
    using reference = T &;

    // Creates a FixedSpan pointing to a sequence of zeroes.
    constexpr FixedSpan() : mDataBuf(detail::shared_zeroes<T, N>()) {}

    // We want to allow construction from things that look like T*, but we want
    // to make construction from an array use the constructor that asserts the
    // array is big enough.  This requires that both constructors be templates
    // (because otherwise the non-template would be favored by overload
    // resolution, since due to decay to pointer it matches just as well as the
    // template).
    //
    // To do that we have a template constructor enabled only when the type
    // passed to it is a pointer type, and that pointer is to a type that
    // matches T's size and can convert to T*.
    template <class U,
              typename = std::enable_if_t<std::is_pointer<U>::value && sizeof(std::remove_pointer_t<U>) == sizeof(T) &&
                                          std::is_convertible<U, T *>::value>>
    explicit FixedSpan(U databuf) : mDataBuf(databuf)
    {
        VerifyOrDie(databuf != nullptr || N == 0); // not constexpr on some platforms
    }

    // FixedSpan does not support an empty / null state.
    FixedSpan(std::nullptr_t null) = delete;

    template <class U, size_t M, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr explicit FixedSpan(U (&databuf)[M]) : mDataBuf(databuf)
    {
        static_assert(M >= N, "Passed-in buffer too small for FixedSpan");
    }

    template <class U, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr FixedSpan(std::array<U, N> & arr) : mDataBuf(arr.data())
    {}

    // Allow implicit construction from a FixedSpan of sufficient size over a
    // type that has the same size as ours, as long as the pointers are convertible.
    template <class U, size_t M, typename = std::enable_if_t<sizeof(U) == sizeof(T) && std::is_convertible<U *, T *>::value>>
    constexpr FixedSpan(FixedSpan<U, M> const & other) : mDataBuf(other.data())
    {
        static_assert(M >= N, "Passed-in FixedSpan is smaller than we are");
    }

    constexpr pointer data() const { return mDataBuf; }
    constexpr pointer begin() const { return mDataBuf; }
    constexpr pointer end() const { return mDataBuf + N; }

    // The size of a FixedSpan is always N. There is intentially no empty() method.
    static constexpr size_t size() { return N; }

    // Element accessors, matching the std::span API.
    // VerifyOrDie cannot be used inside a constexpr function, because it uses
    // "static" on some platforms (e.g. when CHIP_PW_TOKENIZER_LOGGING is true)
    // and that's not allowed in constexpr functions.
    reference operator[](size_t index) const
    {
        VerifyOrDie(index < N);
        return data()[index];
    }
    reference front() const { return (*this)[0]; }
    reference back() const { return (*this)[size() - 1]; }

    // Allow data_equal for spans that are over the same type up to const-ness.
    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const FixedSpan<U, N> & other) const
    {
        return (memcmp(data(), other.data(), N * sizeof(T)) == 0);
    }

    template <class U, typename = std::enable_if_t<std::is_same<std::remove_const_t<T>, std::remove_const_t<U>>::value>>
    bool data_equal(const Span<U> & other) const
    {
        return (N == other.size() && memcmp(data(), other.data(), N * sizeof(T)) == 0);
    }

    // operator== explicitly not implemented on FixedSpan, because its meaning
    // (equality of data, or pointing to the same buffer and same length) is
    // ambiguous.  Use data_equal if testing for equality of data.
    template <typename U>
    bool operator==(const Span<U> & other) const = delete;
    template <typename U, size_t M>
    bool operator==(const FixedSpan<U, M> & other) const = delete;

private:
    pointer mDataBuf;
};

template <class T>
template <class U, size_t N, typename>
constexpr Span<T>::Span(const FixedSpan<U, N> & other) : mDataBuf(other.data()), mDataLen(other.size())
{}

template <class T>
template <class U, size_t N, typename>
inline bool Span<T>::data_equal(const FixedSpan<U, N> & other) const
{
    return other.data_equal(*this);
}

template <typename T>
[[deprecated("Use !empty()")]] inline bool IsSpanUsable(const Span<T> & span)
{
    return !span.empty();
}

template <typename T, size_t N>
[[deprecated("FixedSpan is always usable / non-empty if N > 0")]] inline bool IsSpanUsable(const FixedSpan<T, N> & span)
{
    return N > 0;
}

using ByteSpan        = Span<const uint8_t>;
using MutableByteSpan = Span<uint8_t>;
template <size_t N>
using FixedByteSpan = FixedSpan<const uint8_t, N>;

using CharSpan        = Span<const char>;
using MutableCharSpan = Span<char>;

inline CHIP_ERROR CopySpanToMutableSpan(ByteSpan span_to_copy, MutableByteSpan & out_buf)
{
    VerifyOrReturnError(out_buf.size() >= span_to_copy.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(out_buf.data(), span_to_copy.data(), span_to_copy.size());
    out_buf.reduce_size(span_to_copy.size());

    return CHIP_NO_ERROR;
}

inline CHIP_ERROR CopyCharSpanToMutableCharSpan(CharSpan cspan_to_copy, MutableCharSpan & out_buf)
{
    VerifyOrReturnError(out_buf.size() >= cspan_to_copy.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(out_buf.data(), cspan_to_copy.data(), cspan_to_copy.size());
    out_buf.reduce_size(cspan_to_copy.size());

    return CHIP_NO_ERROR;
}

} // namespace chip
