/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/Optional.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {

/// File signature (aka magic number) of a valid Matter OTA image
constexpr uint32_t kOTAImageFileIdentifier = 0x1BEEF11E;

enum class OTAImageDigestType : uint8_t
{
    kSha256     = 1,
    kSha256_128 = 2,
    kSha256_120 = 3,
    kSha256_96  = 4,
    kSha256_64  = 5,
    kSha256_32  = 6,
    kSha384     = 7,
    kSha512     = 8,
    kSha3_224   = 9,
    kSha3_256   = 10,
    kSha3_384   = 11,
    kSha3_512   = 12,
};

struct OTAImageHeader
{
    uint16_t mVendorId;
    uint16_t mProductId;
    uint32_t mSoftwareVersion;
    CharSpan mSoftwareVersionString;
    uint64_t mPayloadSize;
    Optional<uint32_t> mMinApplicableVersion;
    Optional<uint32_t> mMaxApplicableVersion;
    CharSpan mReleaseNotesURL;
    OTAImageDigestType mImageDigestType;
    ByteSpan mImageDigest;
};

class OTAImageHeaderParser
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
     * @brief Decode Matter OTA image header
     *
     * The method takes subsequent chunks of the Matter OTA image file and decodes the header when
     * enough data has been provided. If more image chunks are needed, CHIP_ERROR_BUFFER_TOO_SMALL
     * error is returned. Other error codes indicate that the header is invalid.
     *
     * @param buffer Byte span containing a subsequent Matter OTA image chunk. When the method
     *               returns CHIP_NO_ERROR, the byte span is used to return a remaining part
     *               of the chunk, not used by the header.
     * @param header Structure to store results of the operation. Note that the results must not be
     *               referenced after the parser is cleared since string members of the structure
     *               are only shallow-copied by the method.
     *
     * @retval CHIP_NO_ERROR                        Header has been decoded successfully.
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL          Provided buffers are insufficient to decode the
     *                                              header. A user is expected call the method again
     *                                              when the next image chunk is available.
     * @retval CHIP_ERROR_INVALID_FILE_IDENTIFIER   Not a Matter OTA image file.
     * @retval Error code                           Encoded header is invalid.
     */
    CHIP_ERROR AccumulateAndDecode(ByteSpan & buffer, OTAImageHeader & header);

private:
    enum State
    {
        kNotInitialized,
        kInitialized,
        kTlv
    };

    void Append(ByteSpan & buffer, uint32_t numBytes);
    CHIP_ERROR DecodeFixed();
    CHIP_ERROR DecodeTlv(OTAImageHeader & header);

    State mState;
    uint32_t mHeaderTlvSize;
    uint32_t mBufferOffset;
    Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};

} // namespace chip
