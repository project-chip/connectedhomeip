/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <cstddef>
#include <cstdint>

namespace mdns {
namespace Minimal {

/// Simple range of bytes with a start and an end
class BytesRange
{
public:
    BytesRange() {}
    BytesRange(const uint8_t * start, const uint8_t * end) : mStart(start), mEnd(end)
    {
        // negative ranges are not allowed
        if (mStart > mEnd)
        {
            mEnd = mStart;
        }
    }

    const uint8_t * Start() const { return mStart; }
    const uint8_t * End() const { return mEnd; }

    bool Contains(const uint8_t * p) const { return ((p >= mStart) && (p < mEnd)); }

    size_t Size() const { return static_cast<size_t>(mEnd - mStart); }

    inline static BytesRange BufferWithSize(const void * buff, size_t len)
    {
        return BytesRange(static_cast<const uint8_t *>(buff), static_cast<const uint8_t *>(buff) + len);
    }

private:
    const uint8_t * mStart = nullptr;
    const uint8_t * mEnd   = nullptr;
};

} // namespace Minimal

} // namespace mdns
