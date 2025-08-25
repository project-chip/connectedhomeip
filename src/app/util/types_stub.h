/**
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

#pragma once

#include <string.h> // For mem* functions.

#include <optional>

#include <app/util/basic-types.h>
#include <lib/core/NodeId.h>

static_assert(sizeof(chip::NodeId) == sizeof(uint64_t), "Unexpected node if size");

/**
 * @brief Size of EUI64 (an IEEE address) in bytes (8).
 */
#define EUI64_SIZE 8

/**
 * @brief EUI 64-bit ID (an IEEE address).
 */
typedef uint8_t EmberEUI64[EUI64_SIZE];

/**
 * @brief 16-bit ZigBee network address.
 */
typedef uint16_t EmberNodeId;

/**
 * @brief 802.15.4 PAN ID.
 */
typedef uint16_t EmberPanId;

/**
 * @brief Function pointer for timer callback
 */
typedef void (*TimerCallback)(chip::EndpointId);

/** @brief The control structure for events.
 *
 * It holds the callback and its parameters.
 */
typedef struct
{
    /* Callback information */
    TimerCallback callback;
    chip::EndpointId endpoint;

} EmberEventControl;

/**
 * @brief Returns the value of the bitmask \c bits within
 * the register or byte \c reg.
 */
#define READBITS(reg, bits) ((reg) & (bits))

#define MILLISECOND_TICKS_PER_SECOND 1000
#define MILLISECOND_TICKS_PER_DECISECOND (MILLISECOND_TICKS_PER_SECOND / 10)
