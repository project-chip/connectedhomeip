/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
