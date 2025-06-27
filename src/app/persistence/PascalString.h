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

    static LengthType GetLength(const uint8_t * buffer) { return *buffer; }
    static void SetLength(uint8_t * buffer, LengthType size) { *buffer = static_cast<uint8_t>(size); }

    // Casts for chars as well
    static LengthType GetLength(const char * buffer) { return GetLength(Uint8::from_const_char(buffer)); }
    static void SetLength(char * buffer, LengthType size) { SetLength(Uint8::from_char(buffer), size); }
};

template <>
struct PascalPrefixOperations<2>
{
    using LengthType                           = uint16_t;
    static constexpr LengthType kInvalidLength = 0xFFFF;

    static LengthType GetLength(const uint8_t * buffer) { return Encoding::LittleEndian::Get16(buffer); }
    static void SetLength(uint8_t * buffer, LengthType size) { Encoding::LittleEndian::Put16(buffer, size); }

    // Casts for chars as well
    static LengthType GetLength(const char * buffer) { return GetLength(Uint8::from_const_char(buffer)); }
    static void SetLength(char * buffer, LengthType size) { SetLength(Uint8::from_char(buffer), size); }
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
    static constexpr LengthType kInvalidLength = PascalPrefixOperations<PREFIX_LEN>::kInvalidLength;

    static_assert(sizeof(T) == 1);

    PascalBuffer(PascalBuffer &&)      = default;
    PascalBuffer(const PascalBuffer &) = default;

    template <size_t N>
    PascalBuffer(T (&data)[N]) : mData(data), mMaxSize(N - PREFIX_LEN)
    {
        static_assert(N >= PREFIX_LEN);
        static_assert(N <= kInvalidLength);
    }

    /// Returns the content of the pascal string.
    /// Uses the prefix size information
    Span<T> Content() { return { mData + PREFIX_LEN, GetLength() }; }
    Span<const T> Content() const { return { mData + PREFIX_LEN, GetLength() }; }

    /// Accesses the "PASCAL" string (i.e. valid data including the string prefix)
    Span<const T> PascalContent() const { return { mData, static_cast<size_t>(GetLength() + PREFIX_LEN) }; }

    /// Access to the full buffer. does NOT take into account current size
    /// and includes the "size prefix"
    Span<T> Buffer() { return { mData, static_cast<size_t>(mMaxSize + PREFIX_LEN) }; }

    LengthType GetLength() const
    {
        const LengthType length = PascalPrefixOperations<PREFIX_LEN>::GetLength(mData);
        if (length == kInvalidLength)
        {
            return 0;
        }
        return std::min<LengthType>(mMaxSize, length);
    }

    // Returns true if the length was valid and could be set
    bool SetLength(LengthType len)
    {
        if (len != kInvalidLength)
        {
            VerifyOrReturnError(len <= mMaxSize, false);
        }
        PascalPrefixOperations<PREFIX_LEN>::SetLength(mData, len);
        return true;
    }
    void SetNull() { (void) SetLength(kInvalidLength); }
    bool IsNull() const { return PascalPrefixOperations<PREFIX_LEN>::GetLength(mData) == kInvalidLength; }

    // Returns true if the length of the input buffer fit in the
    // pascal buffer (and could be set)
    bool SetValue(Span<const T> value)
    {
        VerifyOrReturnValue(value.size() < kInvalidLength, false);
        VerifyOrReturnValue(SetLength(static_cast<LengthType>(value.size())), false);
        memcpy(mData + PREFIX_LEN, value.data(), value.size());
        return true;
    }

    /// Checks if the given span is a valid Pascal string: i.e. its size prefix
    /// is either Invalid (i.e. null marker) or it has a size that fits in the buffer
    static bool IsValid(Span<const T> span)
    {
        VerifyOrReturnValue(span.size() >= PREFIX_LEN, false);
        LengthType len = PascalPrefixOperations<PREFIX_LEN>::GetLength(span.data());
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

} // namespace Storage
} // namespace app
} // namespace chip
