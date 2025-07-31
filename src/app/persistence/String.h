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

#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/PascalString.h>
#include <lib/support/Span.h>

#include <cstddef>
#include <cstdint>

namespace chip::app::Storage {

namespace Internal {

/// Represents a string that can be read/written to storage.
///
/// This is currently implemented for SHORT strings only (< 255 bytes)
/// that are NOT nullable
class ShortString
{
public:
    /// Use the input buffer as a short pascal string.
    ///
    /// The input buffer is assumed to have an extra 1 byte for c_str to work.
    ///
    /// This class is considered Internal, use chip::app::Storage::String<MAX_LENGTH> in application code.
    ShortString(char * buffer, size_t buffer_size) : mBuffer(buffer), mPascalSize(static_cast<uint8_t>(buffer_size - 1))
    {
        // for a buffer to be usable we need 1 byte for size, 1 byte for content and 1 byte for null terminator.
        // Strings without any size make no sense
        VerifyOrDie(buffer_size >= 3);
        VerifyOrDie(buffer_size < 256);
    }

    /// Returns the content as a character span
    CharSpan Content() const { return AsPascal().Content(); }

    /// Returns the content as a null terminated string
    const char * c_str() const { return Content().data(); /* ALWAYS null terminated*/ }

    /// sets the internal value of the string to the given value.
    /// If the set fails, the value is set to empty string and false is returned.
    bool SetContent(CharSpan value);

    friend class ShortStringIO;

private:
    char * mBuffer;
    uint8_t mPascalSize;

    ShortPascalString AsPascal() { return { mBuffer, mPascalSize }; }
    ShortConstPascalString AsPascal() const { return { mBuffer, mPascalSize }; }

    /// Places a null terminator after the pascal string content, so that c_str() works
    void NullTerminate() { mBuffer[AsPascal().ContentWithLenPrefix().size()] = 0; }
};

/// Provides internal access and processing to a ShortString class.
///
/// Meant for internal use only, this is NOT public API outside the SDK code itself.
class ShortStringIO
{
public:
    ShortStringIO(ShortString & value) : mValue(value) {}
    MutableByteSpan ReadBuffer()
    {
        return { reinterpret_cast<uint8_t *>(mValue.mBuffer), static_cast<size_t>(mValue.mPascalSize) };
    }

    /// Method to be called once data has been read into ReadBuffer
    ///
    /// Validates that the read span is valid, sets the value to empty string on failure.
    bool FinalizeRead(MutableByteSpan actuallyRead);

    ByteSpan ContentWithPrefix() const { return mValue.AsPascal().ContentWithLenPrefix(); }

private:
    ShortString & mValue;
};

} // namespace Internal

template <size_t N, typename = std::enable_if_t<(N < 255)>>
class String : public Internal::ShortString
{
public:
    String() : Internal::ShortString(mBuffer, sizeof(mBuffer)) { SetContent(""_span); }

    // internal shortstring is self-referencing pointers. That cannot be copied, so we assume no copy for now
    // These could be implemented, however for now we assume people should just use the underlying Span() to set
    // the values.
    String(String &&)                  = delete;
    String & operator=(String &&)      = delete;
    String(const String &)             = delete;
    String & operator=(const String &) = delete;

private:
    //   - 1 byte to null-terminate to allow for a c_str() implementation
    //     as that seems very convenient.
    char mBuffer[ShortPascalString::BufferSizeFor(N) + 1] = { 0 };
};

} // namespace chip::app::Storage
