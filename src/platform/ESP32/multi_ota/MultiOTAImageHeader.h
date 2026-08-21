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
    uint32_t imageId; // identifies which sub-processor handles this binary
    uint32_t version; // expected installed version of this binary
    uint32_t offset;  // byte offset of binary data from payload start
    uint32_t length;  // exact byte count of the binary
    // SHA-256 of this binary as sent over the wire. For an encrypted or delta image, that is the
    // encrypted/patch bytes — not the final decrypted or rebuilt image.
    uint8_t sha256[Crypto::kSHA256_Hash_Length];
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
     * @brief Decodes a MultiOTAImageHeader from the OTA payload.
     *
     * Call this method with successive payload chunks until the header is fully
     * decoded. If more data is required, it returns CHIP_ERROR_BUFFER_TOO_SMALL
     * and the caller is expected to call the method again with the next chunk.
     * Other error codes indicate that the header is invalid.
     *
     * @param buffer Payload chunk.
     * @param header Receives the decoded header.
     * @return CHIP_NO_ERROR if the header is decoded successfully, CHIP_ERROR_BUFFER_TOO_SMALL if more data is required,
     *         CHIP_ERROR_INVALID_FILE_IDENTIFIER if the header identifier is invalid,
     *         CHIP_ERROR_INVALID_ARGUMENT if the header is structurally invalid,
     *         or other error code if the header is otherwise invalid.
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
