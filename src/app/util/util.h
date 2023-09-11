/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

#include <app/util/af-types.h>
#include <app/util/endpoint-config-api.h>

// Cluster name structure
typedef struct
{
    chip::ClusterId id;
    const char * name;
} EmberAfClusterName;

extern const EmberAfClusterName zclClusterNames[];

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

/* @brief returns true if the attribute is known to be volatile (i.e. RAM
 * storage).
 */
bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);
