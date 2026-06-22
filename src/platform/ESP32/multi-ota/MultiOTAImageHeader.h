/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>
#include <stdint.h>

namespace chip {

typedef uint32_t OTAProcessorTag;

/// Platform-defined application firmware image ID.
constexpr OTAProcessorTag kAppImageProcessorTag = 1;

constexpr uint32_t kMultiOTAImageFileIdentifier = 0x4D494F54u; // "MIOT"

/// Size of the MultiOTAImageHeader fixed preamble (magic + numImages + reserved)
constexpr uint32_t kFixedHeaderSize = 8;

/// Size of one SubImageHeader entry in the variable-length list
constexpr uint32_t kSubImageHeaderSize = 48;

/// Maximum number of SubImageHeader entries in a MultiOTAImageHeader
constexpr uint8_t kMaxNumImages = 255;

struct SubImageHeader
{
    uint32_t imageId;                            // identifies which sub-processor handles this binary
    uint32_t version;                            // expected installed version of this binary
    uint32_t offset;                             // byte offset of binary data from payload start
    uint32_t length;                             // exact byte count of the binary
    uint8_t sha256[Crypto::kSHA256_Hash_Length]; // mandatory SHA-256 digest of [offset, offset+length)
};

static_assert(sizeof(SubImageHeader) == 48, "SubImageHeader size must be 48 bytes");

// Fixed preamble (numImages) plus the variable-length list of SubImageHeader entries.
struct MultiOTAImageHeader
{
    uint8_t numImages;                    // number of SubImageHeader entries (1..kMaxNumImages)
    Span<const SubImageHeader> subImages; // points into the parser's buffer; valid only until Clear()
};

class MultiOTAImageHeaderParser
{
public:
    /**
     * @brief Prepare the parser for accepting Matter OTA image chunks.
     *
     * The method can be called many times to reset the parser state.
     */
    void Init();

    /**
     * @brief Clear all resources associated with the parser.
     */
    void Clear();

    /**
     * @brief Returns if the parser is ready to accept subsequent Matter OTA image chunks.
     */
    bool IsInitialized() const { return mState != State::kNotInitialized; }

    /**
     * @brief Returns if the header has been parsed.
     */
    bool IsHeaderParsed() const { return mHeaderParsed; }

    /**
     * @brief Decode the MultiOTAImageHeader.
     *
     * The MultiOTAImageHeader consists of a fixed preamble (magic, numImages, reserved) followed by
     * a variable-length list of numImages SubImageHeader entries. The method takes subsequent chunks
     * of the OTA payload (the bytes that follow the outer Matter OTA header) and decodes the header
     * once enough data has been provided. If more data is needed, CHIP_ERROR_BUFFER_TOO_SMALL is
     * returned and the caller is expected to call the method again with the next chunk. Other error
     * codes indicate that the header is invalid.
     *
     * @param buffer Byte span containing a subsequent payload chunk. When the method returns
     *               CHIP_NO_ERROR, the span is updated to reference the remaining bytes of the
     *               chunk that follow the header (i.e. the first sub-image's binary data).
     * @param header On success, receives the parsed header. header.subImages points into the
     *               parser's internal buffer and must not be referenced after Clear() is called.
     *
     * @retval CHIP_NO_ERROR                        Header has been decoded successfully.
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL          More payload bytes are needed to decode the
     *                                              header. Call the method again with the next chunk.
     * @retval CHIP_ERROR_INVALID_FILE_IDENTIFIER   Payload does not begin with the header
     *                                              identifier ("MIOT").
     * @retval CHIP_ERROR_INVALID_ARGUMENT          Header is structurally invalid (numImages == 0,
     *                                              reserved bytes nonzero, or an entry has
     *                                              imageId == 0 or length == 0).
     * @retval Error code                           Encoded header is otherwise invalid.
     */
    CHIP_ERROR AccumulateAndDecode(ByteSpan & buffer, MultiOTAImageHeader & header);

private:
    enum State
    {
        kNotInitialized,
        kInitialized,
        kSubImages,
    };

    void Append(ByteSpan & buffer, uint32_t numBytes);
    CHIP_ERROR DecodeMultiOTAImageHeader();
    CHIP_ERROR DecodeSubImageHeaders(MultiOTAImageHeader & header);

    State mState;
    uint32_t mBufferOffset;
    uint8_t mNumImages = 0;
    bool mHeaderParsed = false;
    Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};

} // namespace chip
