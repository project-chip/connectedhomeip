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

#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {
namespace DeviceLayer {

struct OTAImageContentHeader
{
    static constexpr size_t kMaxFiles = 3;

    enum class FileId : uint32_t
    {
        kMcuboot    = 0,
        kAppMcuboot = 1,
        kNetMcuboot = 2,
    };

    struct FileInfo
    {
        FileId mFileId;
        uint32_t mFileSize;
    };

    FileInfo mFiles[kMaxFiles];
};

class OTAImageContentHeaderParser
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
     * @brief Decode nRF Connect Matter OTA image content header
     *
     * The method takes subsequent chunks of the Matter OTA image file and decodes the header when
     * enough data has been provided. If more image chunks are needed, CHIP_ERROR_BUFFER_TOO_SMALL
     * error is returned. Other error codes indicate that the header is invalid.
     *
     * @param buffer Byte span containing a subsequent Matter OTA image chunk. When the method
     *               returns CHIP_NO_ERROR, the byte span is used to return a remaining part
     *               of the chunk, not used by the header.
     * @param header Structure to store results of the operation.
     *
     * @retval CHIP_NO_ERROR                        Header has been decoded successfully.
     * @retval CHIP_ERROR_BUFFER_TOO_SMALL          Provided buffers are insufficient to decode the
     *                                              header. A user is expected call the method again
     *                                              when the next image chunk is available.
     * @retval Error code                           Encoded header is invalid.
     */
    CHIP_ERROR AccumulateAndDecode(ByteSpan & buffer, OTAImageContentHeader & header);

private:
    enum State
    {
        kNotInitialized,
        kInitialized,
        kTlv
    };

    void Append(ByteSpan & buffer, uint32_t numBytes);
    CHIP_ERROR DecodeFixed();
    CHIP_ERROR DecodeTlv(OTAImageContentHeader & header);

    State mState;
    uint32_t mBufferOffset;
    uint32_t mHeaderTlvSize;
    Platform::ScopedMemoryBuffer<uint8_t> mBuffer;
};

} // namespace DeviceLayer
} // namespace chip
