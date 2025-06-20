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

#include <algorithm>

#include <app/ConcreteAttributePath.h>
#include <cstdint>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Storage {

/// Interprets a byte buffer as a pascal string with a 1-byte prefix.
template <typename T>
class ShortPascalString
{
public:
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
    Span<T> PascalContent() const { return { mData, static_cast<size_t>(GetLength() + 1) }; }

    /// Access to the full buffer. does NOT take into account current size
    /// and includes the "size prefix"
    Span<T> Buffer() { return { mData, mMaxSize + 1 }; }

    uint8_t GetLength() const { return std::min(mMaxSize, (reinterpret_cast<const uint8_t *>(mData))[0]); }
    bool SetLength(size_t len)
    {
        VerifyOrReturnError(len <= mMaxSize, false);
        mData[0] = static_cast<uint8_t>(len);
        return true;
    }

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
template <typename T>
class LongPascalString
{
public:
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
    Span<T> PascalContent() const { return { mData, static_cast<size_t>(GetLength() + 2) }; }

    /// Access to the full buffer. does NOT take into account current size
    /// and includes the "size prefix"
    Span<T> Buffer() { return { mData, mMaxSize + 2 }; }

    uint16_t GetLength() const
    {
        uint16_t len;
        memcpy(&len, mData, 2);
        return std::min(len, mMaxSize);
    }

    bool SetLength(size_t len)
    {
        VerifyOrReturnError(len <= mMaxSize, false);
        auto lenBytes = static_cast<uint16_t>(len);
        memcpy(mData, &lenBytes, 2);
        return true;
    }

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
