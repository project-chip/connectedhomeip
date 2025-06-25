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

    /// Append an unsigned char
    StringBuilderBase & Add(unsigned char value)
    {
        char buff[32];
        snprintf(buff, sizeof(buff), "%c", value);
        buff[sizeof(buff) - 1] = 0;
        return Add(buff);
    }

    /// did all the values fit?
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

/// a preallocated sized string builder
template <size_t kSize>
class StringBuilder : public StringBuilderBase
{
public:
    StringBuilder() : StringBuilderBase(mBuffer, kSize) {}

private:
    char mBuffer[kSize];
};

/// Build a c-style string of a char* and length that can be used inplace
/// Default buffer size is 256
template <size_t N = 256>
StringBuilder<N> StringOf(const char * data, size_t length)
{
    chip::StringBuilder<N> builder;
    builder.AddFormat("%.*s", static_cast<int>(length), data);
    return builder;
}

/// Build a c-style string of an unsigned char* and length that can be used inplace
/// Only printable characters will be added
/// Default buffer size is 256
template <size_t N = 256>
StringBuilder<N> StringOf(const unsigned char * data, size_t length)
{
    chip::StringBuilder<N> builder;
    for (size_t i = 0; i < length; ++i)
    {
        if (std::isprint(*(data + i)))
        {
            builder.Add(*(data + i));
        }
    }
    return builder;
}

/// Build a c-style string of a CharSpan that can be used inplace
/// Default buffer size is 256
template <size_t N = 256>
StringBuilder<N> StringOf(const CharSpan & span)
{
    chip::StringBuilder<N> builder;
    builder.AddFormat("%.*s", static_cast<int>(span.size()), span.data());
    return builder;
}

} // namespace chip
