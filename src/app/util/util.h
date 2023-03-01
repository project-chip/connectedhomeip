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

#include <inttypes.h>

// Cluster name structure
typedef struct
{
    chip::ClusterId id;
    const char * name;
} EmberAfClusterName;

extern const EmberAfClusterName zclClusterNames[];

#include <app/util/af.h>

// EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH is defined in config.h
#define EMBER_AF_RESPONSE_BUFFER_LEN EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH

void emberAfInit();
uint16_t emberAfFindClusterNameIndex(chip::ClusterId cluster);

/**
 * Retrieves the difference between the two passed values.
 * This function assumes that the two values have the same endianness.
 * On platforms that support 64-bit primitive types, this function will work
 * on data sizes up to 8 bytes. Otherwise, it will only work on data sizes of
 * up to 4 bytes.
 */
EmberAfDifferenceType emberAfGetDifference(uint8_t * pData, EmberAfDifferenceType value, uint8_t dataSize);

/**
 * Retrieves an uint64_t from the given Zigbee payload. The integer retrieved
 * may be cast into an integer of the appropriate size depending on the
 * number of bytes requested from the message. In Zigbee, all integers are
 * passed over the air in LSB form. LSB to MSB conversion is
 * done within this function automatically before the integer is returned.
 *
 * Obviously (due to return value) this function can only handle
 * the retrieval of integers between 1 and 8 bytes in length.
 *
 */
uint64_t emberAfGetInt(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen, uint8_t bytes);

uint8_t * emberAfPutInt8uInResp(uint8_t value);
uint16_t * emberAfPutInt16uInResp(uint16_t value);
uint8_t * emberAfPutBlockInResp(const uint8_t * data, uint16_t length);
uint8_t * emberAfPutStringInResp(const uint8_t * buffer);
void emberAfPutInt16sInResp(int16_t value);

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

/* @brief returns true if the attribute is known to be volatile (i.e. RAM
 * storage).
 */
bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);
