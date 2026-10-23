/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <type_traits>

#include <lib/support/Span.h>

namespace chip {

/**
 *  @brief
 *    A fixed-capacity, size-tracked buffer container.
 *
 *  This provides a lightweight container abstraction for the common
 *  pattern of a fixed-size backing array paired with a logical length
 *  (that is, count or size). The container never allocates, never
 *  grows beyond its compile-time capacity, and supports efficient
 *  assignment/appending from raw memory.
 *
 *  The container models a "logical (used) prefix" of the backing
 *  storage:
 *
 *    * The backing store is always @p N elements.
 *    * The logical size is tracked in @p mSize and is always in the
 *      range [0, N].
 *    * Iteration and comparisons operate on the logical (used)
 *      portion.
 *
 *  Operations that may fail (e.g. assign/append/resize) return @c
 *  bool and do not modify the logical size on failure.
 *
 *  @tparam T
 *    The element type stored in the backing array.
 *
 *  @tparam N
 *    The fixed element capacity of the buffer.
 *
 *  @tparam SizeT
 *    An integral type used to represent the logical size. This may be
 *    selected smaller than @c std::size_t (e.g. @c uint8_t) to reduce
 *    object footprint.
 *
 */
template <typename T, std::size_t N, typename SizeT = std::size_t>
class FixedBuffer
{
    static_assert(N > 0, "FixedBuffer capacity must be > 0");
    static_assert(std::is_unsigned<SizeT>::value, "FixedBuffer size type must be an unsigned type");

public:
    using value_type             = T;
    using size_type              = SizeT;
    using difference_type        = std::ptrdiff_t;
    using reference              = value_type &;
    using const_reference        = const value_type &;
    using pointer                = value_type *;
    using const_pointer          = const value_type *;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /**
     *  Mutable and immutable views over the fixed buffer logical
     *  (used) prefix size.
     */
    using span_type       = Span<value_type>;
    using const_span_type = Span<const value_type>;

    /**
     *  Mutable and immutable views over the fixed buffer capacity
     *  size.
     */
    using fixed_span_type       = FixedSpan<value_type, N>;
    using const_fixed_span_type = FixedSpan<const value_type, N>;

public:
    // Construction

    FixedBuffer(void) noexcept : mData{}, mSize(0) {}
    FixedBuffer(const std::initializer_list<value_type> & inData) noexcept : mData{}, mSize(0)
    {
        // assign will return false if @a inData.begin() is null, @a
        // inData.size() is zero (0) or greater than the number of
        // allowed elements, or if @a inData.size() exceeds the
        // maximum representable size for this buffer's @c size_type.
        //
        // Since this is a constructor and is marked 'noexcept', there
        // is no error status that can be returned or thrown;
        // likewise, to avoid propagating project-internal assertion
        // macros into integrating projects or products, we elide the
        // return status.

        (void) assign(inData.begin(), inData.size());
    }

    FixedBuffer(const value_type * inData, const std::size_t & inSize) noexcept : mData{}, mSize(0)
    {
        // assign will return false if @a inData is null, @a inSzie is
        // zero (0) or greater than the number of allowed elements, or
        // if @a inSize exceeds the maximum representable size for
        // this buffer's @c size_type.
        //
        // Since this is a constructor and is marked 'noexcept', there
        // is no error status that can be returned or thrown;
        // likewise, to avoid propagating project-internal assertion
        // macros into integrating projects or products, we elide the
        // return status.

        (void) assign(inData, inSize);
    }

    FixedBuffer(const FixedBuffer & inFixedBuffer) noexcept : mData{}, mSize(0)
    {
        // assign will return false if @a inFixedBuffer.begin() is null, @a
        // inFixedBuffer.size() is zero (0) or greater than the number of
        // allowed elements, or if @a inFixedBuffer.size() exceeds the
        // maximum representable size for this buffer's @c size_type.
        //
        // Since this is a constructor and is marked 'noexcept', there
        // is no error status that can be returned or thrown;
        // likewise, to avoid propagating project-internal assertion
        // macros into integrating projects or products, we elide the
        // return status.

        (void) assign(inFixedBuffer.data(), static_cast<std::size_t>(inFixedBuffer.size()));
    }

    FixedBuffer(const const_span_type & inSpan) noexcept : mData{}, mSize(0)
    {
        // assign will return false if @a inSpan.data() is null and @a
        // inSpan.size() != 0, if @a inSpan.size() > capacity(), or if
        // @a inSpan.size() exceeds the maximum representable size for
        // this buffer's @c size_type.
        //
        // Since this is a constructor and is marked 'noexcept', there
        // is no error status that can be returned or thrown;
        // likewise, to avoid propagating project-internal assertion
        // macros into integrating projects or products, we elide the
        // return status.

        (void) assign(inSpan);
    }

    // Destruction

    ~FixedBuffer(void) noexcept = default;

    // Assignment

    FixedBuffer & operator=(const std::initializer_list<value_type> & inData) noexcept
    {
        // assign will return false if @a inData.begin() is null, @a
        // inData.size() is zero (0) or greater than the number of
        // allowed elements, or if @a inData.size() exceeds the
        // maximum representable size for this buffer's @c size_type.
        //
        // Since this is an assignment operator and is marked
        // 'noexcept', there is no error status that can be returned
        // or thrown; likewise, to avoid propagating project-internal
        // assertion macros into integrating projects or products, we
        // elide the return status.

        (void) assign(inData.begin(), inData.size());

        return *this;
    }

    FixedBuffer & operator=(const FixedBuffer & inFixedBuffer) noexcept
    {
        if (this != &inFixedBuffer)
        {
            // assign will return false if @a inFixedBuffer.begin() is
            // null, @a inFixedBuffer.size() is zero (0) or greater
            // than the number of allowed elements, or if @a
            // inFixedBuffer.size() exceeds the maximum representable
            // size for this buffer's @c size_type.
            //
            // Since this is an assignment operator and is marked
            // 'noexcept', there is no error status that can be
            // returned or thrown; likewise, to avoid propagating
            // project-internal assertion macros into integrating
            // projects or products, we elide the return status.

            (void) assign(inFixedBuffer.data(), static_cast<std::size_t>(inFixedBuffer.size()));
        }

        return *this;
    }

    FixedBuffer & operator=(const const_span_type & inSpan) noexcept
    {
        // assign will return false if @a inSpan.data() is null and @a
        // inSpan.size() != 0, if @a inSpan.size() > capacity(), or if
        // @a inSpan.size() exceeds the maximum representable size for
        // this buffer's @c size_type.
        //
        // Since this is an assignment operator and is marked
        // 'noexcept', there is no error status that can be returned
        // or thrown; likewise, to avoid propagating project-internal
        // assertion macros into integrating projects or products, we
        // elide the return status.

        (void) assign(inSpan);

        return *this;
    }

    // Introspection

    bool empty(void) const noexcept { return (mSize == 0); }

    bool full(void) const noexcept { return (mSize == capacity()); }

    // Equality

    bool operator==(const FixedBuffer & inFixedBuffer) const noexcept
    {
        if (mSize != inFixedBuffer.mSize)
        {
            return false;
        }
        else
        {
            const std::size_t lSize = static_cast<std::size_t>(mSize);

            if (lSize == 0)
            {
                return true;
            }
            else if constexpr (std::is_trivially_copyable<value_type>::value)
            {
                return memcmp(mData.data(), inFixedBuffer.mData.data(), lSize * sizeof(value_type)) == 0;
            }
            else
            {
                return std::equal(begin(), end(), inFixedBuffer.begin());
            }
        }
    }

    bool operator==(const const_span_type & inSpan) const noexcept
    {
        if (static_cast<std::size_t>(mSize) != inSpan.size())
        {
            return false;
        }
        else
        {
            const std::size_t lSize = inSpan.size();

            if (lSize == 0)
            {
                return true;
            }
            else if constexpr (std::is_trivially_copyable<value_type>::value)
            {
                return memcmp(mData.data(), inSpan.data(), lSize * sizeof(value_type)) == 0;
            }
            else
            {
                return std::equal(begin(), end(), inSpan.begin());
            }
        }
    }

    bool operator!=(const FixedBuffer & inFixedBuffer) const noexcept { return !(*this == inFixedBuffer); }
    bool operator!=(const const_span_type & inSpan) const noexcept { return !(*this == inSpan); }

    // Observation

    pointer at_ptr(const std::size_t & inPosition) noexcept
    {
        return (inPosition < static_cast<std::size_t>(mSize)) ? (mData.data() + inPosition) : nullptr;
    }

    const_pointer at_ptr(const std::size_t & inPosition) const noexcept
    {
        return (inPosition < static_cast<std::size_t>(mSize)) ? (mData.data() + inPosition) : nullptr;
    }

    size_type available(void) const noexcept { return capacity() - mSize; }

    static constexpr size_type capacity(void) noexcept { return static_cast<size_type>(N); }

    pointer data(void) noexcept { return mData.data(); }

    const_pointer data(void) const noexcept { return mData.data(); }

    reference operator[](const std::size_t & inPosition) noexcept { return mData[inPosition]; }

    const_reference operator[](const std::size_t & inPosition) const noexcept { return mData[inPosition]; }

    size_type size(void) const noexcept { return mSize; }

    span_type span(void) noexcept { return span_type(data(), static_cast<std::size_t>(size())); }
    const_span_type span(void) const noexcept { return const_span_type(data(), static_cast<std::size_t>(size())); }
    fixed_span_type fixed_span(void) noexcept { return fixed_span_type(mData); }
    const_fixed_span_type fixed_span(void) const noexcept { return const_fixed_span_type(mData); }

    // Mutation

    /**
     *  @brief
     *    Append span to the fixed buffer.
     *
     *  This attempts to append @a inSpan to the fixed buffer,
     *  starting at the tail.
     *
     *  @param[in]  inSpan
     *    A reference to the immutable span to append to the fixed
     *    buffer.
     *
     *  @returns
     *    True if the @a inSpan was successfully appended to this
     *    fixed buffer; otherwise, false if @a inSpan.data() was null,
     *    @a inSpan.size() was zero (0) or greater than the number of
     *    available elements, or if the current size plus @a
     *    inSpan.size() would exceed the maximum representable size
     *    for this buffer's @c size_type.
     *
     */
    bool append(const const_span_type & inSpan) noexcept { return append(inSpan.data(), inSpan.size()); }

    /**
     *  @brief
     *    Append elements to the fixed buffer.
     *
     *  This attempts to append @a inSize elements at @a inData to the
     *  fixed buffer, starting at the tail.
     *
     *  @param[in]  inData
     *    A pointer to the immutable elements to append to the fixed
     *    buffer.
     *
     *  @param[in]  inSize
     *    A reference to the immutable number of elements in @a inData
     *    to append to the fixed buffer.
     *
     *  @returns
     *    True if the @a inSize elements at @a inData were
     *    successfully appended to this fixed buffer; otherwise, false
     *    if @a inData was null, @a inSize was zero (0) or greater
     *    than the number of available elements, or if the current
     *    size plus @a inSize would exceed the maximum representable
     *    size for this buffer's @c size_type.
     *
     */
    bool append(const value_type * inData, const std::size_t & inSize) noexcept
    {
        const std::size_t lCurSize = static_cast<std::size_t>(mSize);
        const std::size_t lNewSize = lCurSize + inSize;

        if (inData == nullptr && inSize != 0)
        {
            return false;
        }

        if (inSize > (N - lCurSize))
        {
            return false;
        }

        if (lNewSize > static_cast<std::size_t>(std::numeric_limits<size_type>::max()))
        {
            return false;
        }

        if (inSize > 0)
        {
            memmove(mData.data() + lCurSize, inData, inSize * sizeof(value_type));
        }

        mSize = static_cast<size_type>(lNewSize);

        return true;
    }

    /**
     *  @brief
     *    Assign span to the fixed buffer.
     *
     *  This attempts to assign @a inSpan to the fixed buffer,
     *  starting at the head.
     *
     *  @param[in]  inSpan
     *    A reference to the immutable span to assign to the fixed
     *    buffer.
     *
     *  @returns
     *    True if the @a inSpan was successfully assigned to this
     *    fixed buffer; otherwise, false if @a inSpan.data() was null,
     *    @a inSpan.size() was zero (0) or greater than the number of
     *    allowed elements, or if @a inSpan.size() exceeds the maximum
     *    representable size for this buffer's @c size_type.
     *
     */
    bool assign(const const_span_type & inSpan) noexcept { return assign(inSpan.data(), inSpan.size()); }

    /**
     *  @brief
     *    Assign elements to the fixed buffer.
     *
     *  This attempts to assign @a inSize elements at @a inData to the
     *  fixed buffer, starting at the head.
     *
     *  @param[in]  inData
     *    A pointer to the immutable elements to assign to the fixed
     *    buffer.
     *
     *  @param[in]  inSize
     *    A reference to the immutable number of elements in @a inData
     *    to assign to the fixed buffer.
     *
     *  @returns
     *    True if the @a inSize elements at @a inData were
     *    successfully assigned to this fixed buffer; otherwise, false
     *    if @a inData was null, @a inSize was zero (0) or greater
     *    than the number of allowed elements, or if @a inSize exceeds
     *    the maximum representable size for this buffer's @c
     *    size_type.
     *
     */
    bool assign(const value_type * inData, const std::size_t & inSize) noexcept
    {
        if (inData == nullptr && inSize != 0)
        {
            return false;
        }

        if (inSize > N)
        {
            return false;
        }

        if (inSize > static_cast<std::size_t>(std::numeric_limits<size_type>::max()))
        {
            return false;
        }

        if (inSize > 0)
        {
            memmove(mData.data(), inData, inSize * sizeof(value_type));
        }

        mSize = static_cast<size_type>(inSize);

        return true;
    }

    /**
     *  @brief
     *    Remove all elements from the fixed buffer.
     *
     *  This removes all elements from the fixed buffer, resetting the
     *  size to zero (0) and invalidating all pointers, references,
     *  and iterators to the fixed buffer contents.
     *
     *  @sa fill
     *  @sa reset
     *
     */
    void clear(void) noexcept { mSize = 0; }

    /**
     *  @brief
     *    Assigns the specified value to all elements in the fixed
     *    buffer.
     *
     *  This assigns the specified value to all elements in the fixed
     *  buffer and sets the size to the fixed buffer capacity. Callers
     *  looking to do the same but @b without setting the size to
     *  capacity and instead setting the size to zero (0) should use
     *  #reset.
     *
     *  @param[in]  inValue
     *    A reference to the immutable value to assign to the elements
     *    of the fixed buffer.
     *
     *  @sa clear
     *  @sa reset
     *
     */
    void fill(const_reference inValue) noexcept
    {
        mData.fill(inValue);

        mSize = capacity();
    }

    /**
     *  @brief
     *    Assigns the default value to all elements in the fixed
     *    buffer.
     *
     *  This assigns the default value for @c value_type to all
     *  elements in the fixed buffer and sets the size to zero
     *  (0). Callers looking to do the same but @b without setting the
     *  size to zero (0) and instead setting the size to capacity
     *  should use #fill.
     *
     *  @sa clear
     *  @sa fill
     *
     */
    void reset(void) noexcept { reset(value_type{}); }

    /**
     *  @brief
     *    Assigns the specified value to all elements in the fixed
     *    buffer.
     *
     *  This assigns the specified value to all elements in the fixed
     *  buffer and sets the size to zero (0). Callers
     *  looking to do the same but @b without setting the size to
     *  zero (0) and instead setting the size to capacity should use
     *  #fill.
     *
     *  @param[in]  inValue
     *    A reference to the immutable value to assign to the elements
     *    of the fixed buffer.
     *
     *  @sa clear
     *  @sa fill
     *
     */
    void reset(const_reference inValue) noexcept
    {
        mData.fill(inValue);
        mSize = 0;
    }

    /**
     *  @brief
     *    Resize the fixed buffer.
     *
     *  This attempts to resize the fixed buffer to @a inSize.
     *
     *  @note
     *    No attempt is made to fill or otherwise invalidate elements
     *    between the tail of the former size and that of @a inSize.
     *
     *  @param[in]  inSize
     *    A reference to the immutable size, in number of elements, to
     *    set the fixed buffer to.
     *
     *  @returns
     *    True if the fixed buffer was resized to @a inSize elements;
     *    otherwise, false if @a inSize was greater than the number of
     *    allowed elements or if @a inSize exceeds the maximum
     *    representable size for this buffer's @c size_type.
     *
     *  @sa append
     *  @sa assign
     *  @sa available
     *  @sa capacity
     *  @sa clear
     *  @sa fill
     *  @sa reset
     *  @sa size
     *
     */
    bool resize(const std::size_t & inSize) noexcept
    {
        if (inSize > N)
        {
            return false;
        }

        if (inSize > static_cast<std::size_t>(std::numeric_limits<size_type>::max()))
        {
            return false;
        }

        mSize = static_cast<size_type>(inSize);

        return true;
    }

    // Iteration

    iterator begin(void) noexcept { return data(); }
    const_iterator begin(void) const noexcept { return data(); }
    const_iterator cbegin(void) const noexcept { return data(); }
    iterator end(void) noexcept { return data() + static_cast<std::size_t>(mSize); }
    const_iterator end(void) const noexcept { return data() + static_cast<std::size_t>(mSize); }
    const_iterator cend(void) const noexcept { return data() + static_cast<std::size_t>(mSize); }

    reverse_iterator rbegin(void) noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin(void) const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin(void) const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend(void) noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend(void) const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend(void) const noexcept { return const_reverse_iterator(begin()); }

private:
    std::array<value_type, N> mData;
    size_type mSize;
};

/**
 *  @brief
 *    A fixed-capacity, size-tracked buffer of bytes.
 *
 *  This specializes and extends @c FixedBuffer as a fixed-capacity,
 *  size-tracked buffer of bytes.
 *
 *  @tparam N
 *    The fixed byte capacity of the buffer.
 *
 *  @tparam SizeT
 *    An integral type used to represent the logical size.
 *
 */
template <std::size_t N, typename SizeT = std::size_t>
using FixedByteBuffer = FixedBuffer<std::uint8_t, N, SizeT>;

/**
 *  @brief
 *    A fixed-capacity, size-tracked buffer of characters.
 *
 *  This specializes and extends @c FixedBuffer as a fixed-capacity,
 *  size-tracked buffer of characters.
 *
 *  @tparam N
 *    The fixed character capacity of the buffer.
 *
 *  @tparam SizeT
 *    An integral type used to represent the logical size.
 *
 */
template <std::size_t N, typename SizeT = std::size_t>
using FixedCharBuffer = FixedBuffer<char, N, SizeT>;

} // namespace chip
