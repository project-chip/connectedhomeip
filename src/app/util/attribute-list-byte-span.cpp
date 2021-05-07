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

#include <support/BufferReader.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>

namespace chip {
namespace app {
namespace List {

// The first 2 bytes specify the number of entries. A value of 0xFFFF means the list in invalid
// and data is undefined.
constexpr uint16_t kSizeLengthInBytes = 2u;

// The first 2 bytes of a OCTET_STRING/CHAR_STRING field specifies the length.
constexpr uint16_t kByteSpanSizeLengthInBytes = 2;

CHIP_ERROR ReadByteSpan(const uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span)
{
    ReturnErrorCodeIf(bufferLen < kByteSpanSizeLengthInBytes, CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t size = 0;
    ReturnErrorOnFailure(ReadByteSpanSize(buffer, &size));

    *span = chip::ByteSpan(buffer + kByteSpanSizeLengthInBytes, size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteByteSpan(uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span)
{
    ReturnErrorCodeIf(bufferLen < (kByteSpanSizeLengthInBytes + span->size()), CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t size = static_cast<uint16_t>(span->size());
    ReturnErrorOnFailure(WriteByteSpanSize(buffer, size));

    memmove(buffer + kByteSpanSizeLengthInBytes, span->data(), size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadByteSpanSize(const uint8_t * buffer, uint16_t * size)
{
    chip::Encoding::LittleEndian::Reader reader(buffer, kByteSpanSizeLengthInBytes);
    return reader.Read16(size).StatusCode();
}

CHIP_ERROR WriteByteSpanSize(uint8_t * buffer, uint16_t size)
{
    chip::Encoding::LittleEndian::BufferWriter writer(buffer, kByteSpanSizeLengthInBytes);
    writer.Put16(size);
    return CHIP_NO_ERROR;
}

uint16_t GetByteSpanOffsetFromIndex(const uint8_t * buffer, uint16_t bufferLen, uint16_t index)
{
    uint16_t entryOffset = kSizeLengthInBytes;
    for (uint16_t i = 0; i < index; i++)
    {
        uint16_t entrySize = 0;
        if (CHIP_NO_ERROR != ReadByteSpanSize(buffer + entryOffset, &entrySize))
        {
            return 0;
        }

        entryOffset = entryOffset + kByteSpanSizeLengthInBytes + entrySize;
        if (entryOffset > bufferLen)
        {
            return 0;
        }
    }
    return entryOffset;
}

} // namespace List
} // namespace app
} // namespace chip
