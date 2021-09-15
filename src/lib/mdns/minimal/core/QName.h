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

#include <string.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferWriter.h>

#include <lib/mdns/minimal/core/BytesRange.h>
#include <lib/mdns/minimal/core/DnsHeader.h>

namespace mdns {
namespace Minimal {

/// A QName part is a null-terminated string
using QNamePart = const char *;

/// A list of QNames that is simple to pass around
///
/// As the struct may be copied, the lifetime of 'names' has to extend beyond
/// the objects that use this struct.
struct FullQName
{
    const QNamePart * names;
    size_t nameCount;

    FullQName() : names(nullptr), nameCount(0) {}
    FullQName(const FullQName &) = default;
    FullQName & operator=(const FullQName &) = default;

    template <size_t N>
    FullQName(const QNamePart (&data)[N]) : names(data), nameCount(N)
    {}

    void Output(chip::Encoding::BigEndian::BufferWriter & out) const
    {
        for (uint16_t i = 0; i < nameCount; i++)
        {

            out.Put8(static_cast<uint8_t>(strlen(names[i])));
            out.Put(names[i]);
        }
        out.Put8(0); // end of qnames
    }

    bool operator==(const FullQName & other) const;
    bool operator!=(const FullQName & other) const { return !(*this == other); }
};

/// A serialized QNAME is comprised of
///  - length-prefixed parts
///  - Ends in a 0-length item
///  - May contain pointers to previous data (for efficient transmission)
///
/// This class allows iterating over such parts while validating
/// that the parts are within a valid range
class SerializedQNameIterator
{
public:
    SerializedQNameIterator() : mLookBehindMax(0), mCurrentPosition(nullptr), mIsValid(false) {}
    SerializedQNameIterator(const SerializedQNameIterator &) = default;
    SerializedQNameIterator & operator=(const SerializedQNameIterator &) = default;

    SerializedQNameIterator(const BytesRange validData, const uint8_t * position) :
        mValidData(validData), mLookBehindMax(static_cast<size_t>(position - validData.Start())), mCurrentPosition(position)
    {}

    /// Advances to the next element in the sequence
    /// Returns true if new data was available
    bool Next();

    /// Find out if the data parsing is ok.
    /// If invalid data is encountered during a [Next] call, this will
    /// return false. Check this after Next returns false.
    bool IsValid() const { return mIsValid; }

    /// Valid IFF Next() returned true.
    /// Next has to be called after construction
    QNamePart Value() const { return mValue; }

    /// Get the end of the sequence *without* following any
    /// backwards pointers. Changes iterator state.
    ///
    /// returs nullptr on error (invalid data)
    const uint8_t * FindDataEnd();

    bool operator==(const FullQName & other) const;
    bool operator!=(const FullQName & other) const { return !(*this == other); }

    void Put(chip::Encoding::BigEndian::BufferWriter & out) const
    {
        SerializedQNameIterator copy = *this;
        while (copy.Next())
        {

            out.Put8(static_cast<uint8_t>(strlen(copy.Value())));
            out.Put(copy.Value());
        }
        out.Put8(0); // end of qnames
    }

private:
    static constexpr size_t kMaxValueSize = 63;
    static constexpr uint8_t kPtrMask     = 0xC0;

    BytesRange mValidData;
    size_t mLookBehindMax; // avoid loops by limiting lookbehind
    const uint8_t * mCurrentPosition;
    bool mIsValid = true;

    char mValue[kMaxValueSize + 1] = { 0 };

    // Advances to the next element in the sequence
    bool Next(bool followIndirectPointers);
};

} // namespace Minimal
} // namespace mdns
