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

namespace chip {

/**
 * @brief A wrapper class for holding objects and its length, without the ownership of it.
 * We can use C++20 std::span once we support it, the data() and size() come from C++20 std::span.
 */
template <class T>
class Span
{
public:
    constexpr Span() : mDataBuf(nullptr), mDataLen(0) {}
    constexpr Span(const T * databuf, size_t datalen) : mDataBuf(databuf), mDataLen(datalen) {}
    template <size_t N>
    constexpr explicit Span(const T (&databuf)[N]) : Span(databuf, N)
    {}

    const T * data() const { return mDataBuf; }
    size_t size() const { return mDataLen; }

private:
    const T * mDataBuf;
    size_t mDataLen;
};

using ByteSpan = Span<uint8_t>;

} // namespace chip
