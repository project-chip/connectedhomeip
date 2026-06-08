/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace chip {
namespace app {
namespace Storage {

/// Describes how to set/get the length of pascal strings
/// given a PREFIX_LEN in bytes.
template <size_t PREFIX_LEN>
struct PascalPrefixOperations;

template <>
struct PascalPrefixOperations<1>
{
    using LengthType                           = uint8_t;
    static constexpr LengthType kInvalidLength = 0xFF;

    static LengthType GetContentLength(const uint8_t * buffer) { return *buffer; }
    static void SetContentLength(uint8_t * buffer, LengthType size) { *buffer = static_cast<uint8_t>(size); }

    // Casts for chars as well
    static LengthType GetContentLength(const char * buffer) { return GetContentLength(Uint8::from_const_char(buffer)); }
    static void SetContentLength(char * buffer, LengthType size) { SetContentLength(Uint8::from_char(buffer), size); }
};

template <>
struct PascalPrefixOperations<2>
{
    using LengthType                           = uint16_t;
    static constexpr LengthType kInvalidLength = 0xFFFF;

    static LengthType GetContentLength(const uint8_t * buffer) { return Encoding::LittleEndian::Get16(buffer); }
    static void SetContentLength(uint8_t * buffer, LengthType size) { Encoding::LittleEndian::Put16(buffer, size); }

    // Casts for chars as well
    static LengthType GetContentLength(const char * buffer) { return GetContentLength(Uint8::from_const_char(buffer)); }
    static void SetContentLength(char * buffer, LengthType size) { SetContentLength(Uint8::from_char(buffer), size); }
};

/// Interprets a byte buffer as a pascal buffer:
///   - a prefix that specifies the length of the data
///   - the following characters that contain the data
///
/// Parameters:
///   T - the underlying data type, generally uint8_t for bytes or char for strings
///   PREFIX_LEN - the size of the pascal prefix (generally 1 or 2 bytes)
template <typename T, uint8_t PREFIX_LEN>
class PascalBuffer
{
public:
    using LengthType                           = typename PascalPrefixOperations<PREFIX_LEN>::LengthType;
    using ValueType                            = Span<const T>;
    static constexpr LengthType kInvalidLength = PascalPrefixOperations<PREFIX_LEN>::kInvalidLength;

    /// How many bytes of buffer are needed to store a max `charCount` sized buffer.
    static constexpr size_t BufferSizeFor(size_t charCount) { return PREFIX_LEN + charCount; }

    static_assert(sizeof(T) == 1);

    PascalBuffer(PascalBuffer &&)      = default;
    PascalBuffer(const PascalBuffer &) = default;

    /// Interprets the given data as the pascal string content.
    ///
    /// Note that this references the given data in place. Data lifetime must
    /// exceed the PascalBuffer lifetime.
    template <size_t N>
    PascalBuffer(T (&data)[N]) : mData(data), mMaxSize(N - PREFIX_LEN)
    {
        static_assert(N >= PREFIX_LEN);
        static_assert(N <= kInvalidLength);
    }

    /// Uses an existing buffer for the pascal string, represented as a span.
    ///
    /// The size of the span includes the prefix and must be at least PREFIX_LEN (this is NOT checked).
    ///
    /// Note that this references the given buffer in place. The buffer that the span
    /// points to must have a lifetime that exceeds the PascalBuffer lifetime.
    PascalBuffer(Span<T> data) : mData(data.data()), mMaxSize(static_cast<LengthType>(data.size() - PREFIX_LEN)) {}

    /// Returns the content of the pascal string.
    /// Uses the prefix size information
    Span<const T> Content() const { return { mData + PREFIX_LEN, GetContentLength() }; }

    /// Accesses the "PASCAL" string (i.e. valid data including the string prefix)
    ///
    /// Use this to serialize the data. Specifically to recover the original string from
    /// persistent storage one can do one of two things:
    ///   - persist pascalString.ContentWithLenPrefix (will include the data WITH the size prefix AND the prefix
    ///     will correctly identify NULL strings via a kInvalidLength marker)
    ///     - read via pascalString.RawFullBuffer
    ///   - persist pascalString.Content (will NOT include data size and NULL strings will be the same as
    ///     empty strings) and consider if IsNull requires special handling
    ///     - read into a temporary buffer and set value via pascalString.SetValue()/SetNull()
    ///     - OR read into (RawFullBuffer().data() + PREFIX_LEN) and call SetContentLength() or SetNull if applicable
    ByteSpan ContentWithLenPrefix() const
    {
        return { reinterpret_cast<const uint8_t *>(mData), static_cast<size_t>(GetContentLength() + PREFIX_LEN) };
    }

    /// Access to the full buffer. does NOT take into account current size
    /// and includes the "size prefix"
    MutableByteSpan RawFullBuffer()
    {
        static_assert(!std::is_const_v<T>, "Cannot mutate a const pascal string");
        return { reinterpret_cast<uint8_t *>(mData), static_cast<size_t>(mMaxSize + PREFIX_LEN) };
    }

    LengthType GetContentLength() const
    {
        const LengthType length = PascalPrefixOperations<PREFIX_LEN>::GetContentLength(mData);
        if (length == kInvalidLength)
        {
            return 0;
        }
        return std::min<LengthType>(mMaxSize, length);
    }

    // Returns true if the length was valid and could be set
    bool SetContentLength(LengthType len)
    {
        static_assert(!std::is_const_v<T>, "Cannot mutate a const pascal string");
        if (len != kInvalidLength)
        {
            VerifyOrReturnError(len <= mMaxSize, false);
        }
        PascalPrefixOperations<PREFIX_LEN>::SetContentLength(mData, len);
        return true;
    }
    void SetNull() { (void) SetContentLength(kInvalidLength); }
    bool IsNull() const { return PascalPrefixOperations<PREFIX_LEN>::GetContentLength(mData) == kInvalidLength; }

    // Returns true if the length of the input buffer fit in the
    // pascal buffer (and could be set)
    bool SetValue(Span<const T> value)
    {
        static_assert(!std::is_const_v<T>, "Cannot mutate a const pascal string");
        VerifyOrReturnValue(value.size() < kInvalidLength, false);
        VerifyOrReturnValue(SetContentLength(static_cast<LengthType>(value.size())), false);
        memcpy(mData + PREFIX_LEN, value.data(), value.size());
        return true;
    }

    /// Checks if the given span is a valid Pascal string: i.e. its size prefix
    /// is either Invalid (i.e. null marker) or it has a size that fits in the buffer
    static bool IsValid(ByteSpan span)
    {
        VerifyOrReturnValue(span.size() >= PREFIX_LEN, false);
        LengthType len = PascalPrefixOperations<PREFIX_LEN>::GetContentLength(span.data());
        return len == kInvalidLength || (static_cast<size_t>(len + PREFIX_LEN) <= span.size());
    }

private:
    T * mData;
    const LengthType mMaxSize;
};

using ShortPascalString = PascalBuffer<char, 1>;
using ShortPascalBytes  = PascalBuffer<uint8_t, 1>;
using LongPascalString  = PascalBuffer<char, 2>;
using LongPascalBytes   = PascalBuffer<uint8_t, 2>;

// same as the pascal strings, except the data is const and cannot
// be changed. Useful to get the content of data
using ShortConstPascalString = PascalBuffer<const char, 1>;
using ShortConstPascalBytes  = PascalBuffer<const uint8_t, 1>;
using LongConstPascalString  = PascalBuffer<const char, 2>;
using LongConstPascalBytes   = PascalBuffer<const uint8_t, 2>;

} // namespace Storage
} // namespace app
} // namespace chip
