/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "attribute-list-byte-span.h"

#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace List {

// The first 2 bytes of a OCTET_STRING/CHAR_STRING field specifies the length in little-endian.
constexpr uint16_t kByteSpanSizeLengthInBytes = 2;

static CHIP_ERROR ReadByteSpanSize(const uint8_t * buffer, uint16_t bufferLen, uint16_t * size)
{
    ReturnErrorCodeIf(bufferLen < kByteSpanSizeLengthInBytes, CHIP_ERROR_INVALID_ARGUMENT);

    chip::Encoding::LittleEndian::Reader reader(buffer, kByteSpanSizeLengthInBytes);
    ReturnErrorOnFailure(reader.Read16(size).StatusCode());
    return CHIP_NO_ERROR;
}

static CHIP_ERROR WriteByteSpanSize(uint8_t * buffer, uint16_t bufferLen, uint16_t size)
{
    ReturnErrorCodeIf(bufferLen < kByteSpanSizeLengthInBytes, CHIP_ERROR_BUFFER_TOO_SMALL);

    chip::Encoding::LittleEndian::BufferWriter writer(buffer, kByteSpanSizeLengthInBytes);
    ReturnErrorCodeIf(!writer.Put16(size).Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadByteSpan(const uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span)
{
    uint16_t size = 0;
    ReturnErrorOnFailure(ReadByteSpanSize(buffer, bufferLen, &size));

    ReturnErrorCodeIf(size > bufferLen - kByteSpanSizeLengthInBytes, CHIP_ERROR_BUFFER_TOO_SMALL);
    *span = chip::ByteSpan(buffer + kByteSpanSizeLengthInBytes, size);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteByteSpan(uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span)
{
    uint16_t size = static_cast<uint16_t>(span->size());
    ReturnErrorOnFailure(WriteByteSpanSize(buffer, bufferLen, size));

    ReturnErrorCodeIf(size > bufferLen - kByteSpanSizeLengthInBytes, CHIP_ERROR_BUFFER_TOO_SMALL);
    memmove(buffer + kByteSpanSizeLengthInBytes, span->data(), size);

    return CHIP_NO_ERROR;
}

uint16_t GetByteSpanOffsetFromIndex(const uint8_t * buffer, uint16_t bufferLen, uint16_t index)
{
    chip::Encoding::LittleEndian::Reader reader(buffer, bufferLen);

    uint16_t entryCount = 0;
    ReturnErrorCodeIf(CHIP_NO_ERROR != reader.Read16(&entryCount).StatusCode(), 0);

    for (uint16_t i = 0; i < index; i++)
    {
        uint16_t entrySize = 0;
        ReturnErrorCodeIf(CHIP_NO_ERROR != reader.Read16(&entrySize).StatusCode(), 0);
        ReturnErrorCodeIf(!reader.HasAtLeast(entrySize), 0);
        reader.Skip(entrySize);
    }

    return static_cast<uint16_t>(reader.OctetsRead());
}

} // namespace List
} // namespace app
} // namespace chip
