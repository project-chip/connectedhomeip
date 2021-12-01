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

#include <lib/core/CHIPEncoding.h>
#include <lib/core/Optional.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/BufferWriter.h>

namespace mdns {
namespace Minimal {

/**
 * Handles writing into mdns packets.
 *
 * Generally the same as a binary data writer, but can handle qname writing with
 * compression.
 */
class RecordWriter
{
public:
    RecordWriter(const chip::Encoding::BigEndian::BufferWriter & output) : mOutput(output) { Reset(); }

    void Reset()
    {
        for (size_t i = 0; i < kMaxCachedReferences; i++)
        {
            mPreviousQNames[i] = kInvalidOffset;
        }
    }

    chip::Encoding::BigEndian::BufferWriter & Writer() { return mOutput; }

    /// Writes  the given qname into the underlying buffer, applying
    /// compression if possible
    void WriteQName(const FullQName & qname);

private:
    // How  many paths to remember as 'previously written'
    // and make use of them
    static constexpr size_t kMaxCachedReferences = 8;
    static constexpr uint16_t kInvalidOffset     = 0xFFFF;

    // Where the data is being outputted
    chip::Encoding::BigEndian::BufferWriter mOutput;
    uint16_t mPreviousQNames[kMaxCachedReferences];

    // Gets the iterator corresponding to the previous name
    // with the given index.
    //
    // Will return an iterator  that is not valid if
    // lookbehind index is not valid
    SerializedQNameIterator PreviousName(size_t index);
};

} // namespace Minimal
} // namespace mdns
