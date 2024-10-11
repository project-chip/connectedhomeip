/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <cstddef>
#include <cstdint>

/*
 * @brief Function that determines the length of a zigbee Cluster Library string
 *   (where the first byte is assumed to be the length).
 */
uint8_t emberAfStringLength(const uint8_t * buffer);
/*
 * @brief Function that determines the length of a zigbee Cluster Library long string.
 *   (where the first two bytes are assumed to be the length).
 */
uint16_t emberAfLongStringLength(const uint8_t * buffer);

/*
 * @brief Function that copies a ZCL string type into a buffer.  The size
 * parameter should indicate the maximum number of characters to copy to the
 * destination buffer not including the length byte.
 */
void emberAfCopyString(uint8_t * dest, const uint8_t * src, size_t size);

/*
 * @brief Function that copies a ZCL long string into a buffer.  The size
 * parameter should indicate the maximum number of characters to copy to the
 * destination buffer not including the length bytes.
 */
void emberAfCopyLongString(uint8_t * dest, const uint8_t * src, size_t size);
