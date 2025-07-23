/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <cctype>
#include <nlassert.h>

#include "BufferWriter.h"

namespace chip {

/// Build a c-style string out of distinct parts
class StringBuilderBase
{
public:
    StringBuilderBase(char * buffer, size_t size) : mWriter(reinterpret_cast<uint8_t *>(buffer), size - 1)
    {
        nlASSERT(size > 0);
        buffer[0] = 0; // make c-str work by default
    }

    /// Append a null terminated string
    StringBuilderBase & Add(const char * s)
    {
        mWriter.Put(s);
        NullTerminate();
        return *this;
    }

    /// Append an integer value
    StringBuilderBase & Add(int value)
    {
        char buff[32];
        snprintf(buff, sizeof(buff), "%d", value);
        buff[sizeof(buff) - 1] = 0;
        return Add(buff);
    }

    /// Append an uint8_t value
    StringBuilderBase & Add(uint8_t value)
    {
        uint8_t actual = std::isprint(value) ? value : '.';
        mWriter.Put(actual);
        NullTerminate();
        return *this;
    }

    /// Append a memory block
    StringBuilderBase & Add(const void * data, size_t size)
    {
        mWriter.Put(data, size);
        NullTerminate();
        return *this;
    }

    /// Did all the values fit?
    bool Fit() const { return mWriter.Fit(); }

    /// Was nothing written yet?
    bool Empty() const { return mWriter.Needed() == 0; }

    /// Write a formatted string to the stringbuilder
    StringBuilderBase & AddFormat(const char * format, ...) ENFORCE_FORMAT(2, 3);

    /// For strings we often want to know when they were truncated. If the underlying writer did
    /// not fit, this replaces the last 3 characters with "."
    StringBuilderBase & AddMarkerIfOverflow();

    StringBuilderBase & Reset()
    {
        mWriter.Reset();
        NullTerminate();
        return *this;
    }

    /// access the underlying value
    const char * c_str() const { return reinterpret_cast<const char *>(mWriter.Buffer()); }

protected:
    /// Number of bytes actually needed
    size_t Needed() const { return mWriter.Needed(); }

    /// Size of the output buffer
    size_t Size() const { return mWriter.Size(); }

private:
    Encoding::BufferWriter mWriter;

    void NullTerminate()
    {
        if (mWriter.Fit())
        {
            mWriter.Buffer()[mWriter.Needed()] = 0;
        }
        else
        {
            mWriter.Buffer()[mWriter.Size()] = 0;
        }
    }
};

/// A preallocated sized string builder
template <size_t kSize>
class StringBuilder : public StringBuilderBase
{
public:
    StringBuilder() : StringBuilderBase(mBuffer, kSize) {}

    StringBuilder(const char * data, size_t size, bool add_marker_if_overflow = true) : StringBuilder()
    {
        Add(data, size);

        if (add_marker_if_overflow)
        {
            AddMarkerIfOverflow();
        }

        size_t length = Fit() ? Needed() : Size();
        for (size_t i = 0; i < length; ++i)
        {
            if (mBuffer[i] == '\0')
            {
                mBuffer[i] = '.';
            }
        }
    }

    StringBuilder(const CharSpan & span, bool add_marker_if_overflow = true) :
        StringBuilder(span.data(), span.size(), add_marker_if_overflow)
    {}

    StringBuilder(const uint8_t * data, size_t size, bool add_marker_if_overflow = true) : StringBuilder()
    {
        Add(data, size);

        if (add_marker_if_overflow)
        {
            AddMarkerIfOverflow();
        }

        size_t length = Fit() ? Needed() : Size();
        for (size_t i = 0; i < length; ++i)
        {
            if (!std::isprint(mBuffer[i]))
            {
                mBuffer[i] = '.';
            }
        }
    }

    StringBuilder(const ByteSpan & span, bool add_marker_if_overflow = true) :
        StringBuilder(span.data(), span.size(), add_marker_if_overflow)
    {}

private:
    char mBuffer[kSize];
};

/// Default buffer size is 257 to accommodate values with size up to 256
/// If the buffer size is not enough the value will be truncated and an overflow marker will be added
/// TODO Note that this buffer size will be used always by default even for much smaller values
/// TODO Preferably the buffer size should be the one appropriate for each value
using NullTerminated = StringBuilder<257>;

} // namespace chip
