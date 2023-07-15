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

} // namespace chip
