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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace List {

/**
 * @brief Deserialize a bytespan from buffer
 *
 * @param [in] buffer     A buffer containing serialized ByteSpan
 * @param [in] bufferLen  The buffer size
 * @param [in] span       The deserialized bytespan
 *
 */
CHIP_ERROR ReadByteSpan(const uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span);

/**
 * @brief Serialize a bytespan to buffer
 *
 * @param [in] buffer     A buffer containing serialized ByteSpan
 * @param [in] bufferLen  The buffer size
 * @param [in] span       The bytespan to serialize
 *
 */
CHIP_ERROR WriteByteSpan(uint8_t * buffer, uint16_t bufferLen, chip::ByteSpan * span);

/**
 * @brief Returns the byte offset of a serialized bytespan into a buffer of serialized bytespans.
 *
 * @param [in] buffer     A buffer containing serialized ByteSpan
 * @param [in] bufferLen  The buffer size
 * @param [in] index      The index of the serialized ByteSpan to retrieve
 *
 * @return offset on success. This value can be added to buffer to obtain a useful first argument to ReadByteSpan/WriteByteSpan
 *         0 on error or if the serialized bytespan is not found
 */
uint16_t GetByteSpanOffsetFromIndex(const uint8_t * buffer, uint16_t bufferLen, uint16_t index);

} // namespace List
} // namespace app
} // namespace chip
