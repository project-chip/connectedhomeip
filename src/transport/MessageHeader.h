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

/**
 * @file  This file defines the chip::MessageHeader class.
 */

#ifndef MESSAGEHEADER_H_
#define MESSAGEHEADER_H_

#include <cstddef>
#include <cstdint>

#include <core/CHIPError.h>
#include <core/Optional.h>

namespace chip {

/** Handles encoding/decoding of CHIP message headers */
class MessageHeader
{
public:
    const Optional<uint64_t> & GetSourceNodeId() const { return mSourceNodeId; }
    const Optional<uint64_t> & GetDestinationNodeId() const { return mDestinationNodeId; }
    uint32_t GetMessageId() const { return mMessageId; }

    MessageHeader & SetMessageId(uint32_t id)
    {
        mMessageId = id;

        return *this;
    }

    MessageHeader & SetSourceNodeId(uint64_t id)
    {
        mSourceNodeId.SetValue(id);

        return *this;
    }

    MessageHeader & ClearSourceNodeId()
    {
        mSourceNodeId.ClearValue();

        return *this;
    }

    MessageHeader & SetDestinationNodeId(uint64_t id)
    {
        mDestinationNodeId.SetValue(id);

        return *this;
    }

    MessageHeader & ClearDestinationNodeId()
    {
        mDestinationNodeId.ClearValue();

        return *this;
    }

    /**
     * A call to `Encode` will require at least this many bytes on the current
     * object to be successful.
     *
     * @return the number of bytes needed in a buffer to be able to Encode.
     */
    size_t EncodeSizeBytes() const;

    /**
     * Decodes a header from the given buffer.
     *
     * @param data - the buffer to read from
     * @param size - bytes available in the buffer
     * @param decode_size - number of bytes read from the buffer to decode the
     *                      object
     */
    CHIP_ERROR Decode(const uint8_t * data, size_t size, size_t * decode_size);

    /**
     * Encodes a header into the given buffer.
     *
     * @param data - the buffer to write to
     * @param size - space available in the buffer (in bytes)
     * @param decode_size - number of bytes written to the buffer.
     */
    CHIP_ERROR Encode(uint8_t * data, size_t size, size_t * encode_size);

private:
    /// Represents the current encode/decode header version
    static constexpr int kHeaderVersion = 2;

    /// Value expected to be incremented for each message sent.
    uint32_t mMessageId = 0;

    /// What node the message originated from
    Optional<uint64_t> mSourceNodeId;

    /// Intended recipient of the message.
    Optional<uint64_t> mDestinationNodeId;
};

} // namespace chip

#endif // MESSAGEHEADER_H_
