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

#include <app/util/af-types.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/DataModelTypes.h>

void emberAfInit();

/**
 * Retrieves the difference between the two passed values.
 * This function assumes that the two values have the same endianness.
 * On platforms that support 64-bit primitive types, this function will work
 * on data sizes up to 8 bytes. Otherwise, it will only work on data sizes of
 * up to 4 bytes.
 */
EmberAfDifferenceType emberAfGetDifference(uint8_t * pData, EmberAfDifferenceType value, uint8_t dataSize);

/* @brief returns true if the attribute is known to be volatile (i.e. RAM
 * storage).
 */
bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);
