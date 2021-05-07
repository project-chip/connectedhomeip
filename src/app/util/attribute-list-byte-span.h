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

#pragma once

#include <core/CHIPError.h>
#include <support/Span.h>

namespace chip {
namespace app {
namespace List {

CHIP_ERROR ReadByteSpan(const uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span);

CHIP_ERROR WriteByteSpan(uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span);

CHIP_ERROR ReadByteSpanSize(const uint8_t * buffer, uint16_t * size);

CHIP_ERROR WriteByteSpanSize(uint8_t * buffer, uint16_t size);

/**
 * @brief Returns the offset of a serialized bytespan N into a buffer of serialized bytespans.
 *
 * @param [in] buffer     A buffer containing serialized ByteSpan
 * @param [in] bufferLen  The buffer size
 * @param [in] index      The index of the serialized ByteSpan to retrieve
 *
 * @return offset on success
 *         0 on error or if the serialized bytespan is not found
 */
uint16_t GetByteSpanOffsetFromIndex(const uint8_t * buffer, uint16_t bufferLen, uint16_t index);

} // namespace List
} // namespace app
} // namespace chip
