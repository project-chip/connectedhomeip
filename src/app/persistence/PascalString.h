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
#include <lib/support/Span.h>

#include <algorithm>
#include <cstdint>

namespace chip {
namespace app {
namespace Storage {

/// Interprets a byte buffer as a pascal string with a 1-byte prefix.
///
/// 0xFF is a marker of "INVALID/NULL" string.
template <typename T>
class ShortPascalString
{
public:
    static constexpr uint8_t kInvalidLength = 0xFF;

    ShortPascalString(ShortPascalString &&)      = default;
    ShortPascalString(const ShortPascalString &) = default;

    template <size_t N>
    ShortPascalString(T (&data)[N]) : mData(data), mMaxSize(N - 1)
    {
        static_assert(N > 0);
        static_assert(N <= 0xFF);
    }

    /// Returns the content of the pascal string.
    /// Uses the prefix size information
    Span<T> Content() { return { mData + 1, GetLength() }; }
    Span<const T> Content() const { return { mData + 1, GetLength() }; }

    /// Accesses the "PASCAL" string (i.e. valid data includint the string perfix)
    Span<const T> PascalContent() const { return { mData, static_cast<size_t>(GetLength() + 1) }; }

    /// Access to the full buffer. does NOT take into account current size
    /// and includes the "size prefix"
    Span<T> Buffer() { return { mData, static_cast<size_t>(mMaxSize + 1) }; }

    uint8_t GetLength() const
    {
        const uint8_t length = reinterpret_cast<const uint8_t *>(mData)[0];
        if (length == kInvalidLength)
        {
            return 0;
        }
        return std::min(mMaxSize, length);
    }
    bool SetLength(uint8_t len)
    {
        if (len != kInvalidLength)
        {
            VerifyOrReturnError(len <= mMaxSize, false);
        }
        mData[0] = static_cast<T>(len);
        return true;
    }
    void SetNull() { (void) SetLength(kInvalidLength); }

    bool SetValue(Span<const T> value)
    {
        if (!SetLength(value.size()))
        {
            return false;
        }
        memcpy(mData + 1, value.data(), value.size());
        return true;
    }

private:
    T * mData;
    const uint8_t mMaxSize;
};

/// Interprets a byte buffer as a pascal string with a 2-byte prefix.
///
/// The prefix is stored in LittleEndian (to be compatible with ember logic)
/// 0xFFFF is a marker of "INVALID/NULL" string.
template <typename T>
class LongPascalString
{
public:
    static constexpr uint16_t kInvalidLength = 0xFFFF;

    LongPascalString(LongPascalString &&)      = default;
    LongPascalString(const LongPascalString &) = default;

    template <size_t N>
    LongPascalString(T (&data)[N]) : mData(data), mMaxSize(N - 2)
    {
        static_assert(N > 0);
        static_assert(N <= 0xFF);
    }

    /// Returns the content of the pascal string.
    /// Uses the prefix size information
    Span<T> Content() { return { mData + 2, GetLength() }; }
    Span<const T> Content() const { return { mData + 2, GetLength() }; }

    /// Accesses the "PASCAL" string (i.e. valid data includint the string perfix)
    Span<const T> PascalContent() const { return { mData, static_cast<size_t>(GetLength() + 2) }; }

    /// Access to the full buffer. does NOT take into account current size
    /// and includes the "size prefix"
    Span<T> Buffer() { return { mData, static_cast<size_t>(mMaxSize + 2) }; }

    uint16_t GetLength() const
    {
        uint16_t length = Encoding::LittleEndian::Get16(reinterpret_cast<const uint8_t *>(mData));
        if (length == kInvalidLength)
        {
            return 0;
        }
        return std::min(mMaxSize, length);
    }

    bool SetLength(uint16_t len)
    {
        if (len != kInvalidLength)
        {
            VerifyOrReturnError(len <= mMaxSize, false);
        }
        auto p = reinterpret_cast<uint8_t*>(mData);
        Encoding::LittleEndian::Put16(p, len);
        return true;
    }
    void SetNull() { (void) SetLength(kInvalidLength); }

    bool SetValue(Span<const T> value)
    {
        if (!SetLength(value.size()))
        {
            return false;
        }
        memcpy(mData + 2, value.data(), value.size());
        return true;
    }

private:
    T * mData;
    const uint16_t mMaxSize;
    static_assert(sizeof(mMaxSize) == 2);
};

} // namespace Storage
} // namespace app
} // namespace chip
