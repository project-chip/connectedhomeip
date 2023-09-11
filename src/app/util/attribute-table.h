/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/af.h>

#define ZCL_NULL_ATTRIBUTE_TABLE_INDEX 0xFFFF

// Remote devices writing attributes of local device
EmberAfStatus emberAfWriteAttributeExternal(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                            uint8_t * dataPtr, EmberAfAttributeType dataType);

EmberAfStatus emAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t * data,
                                 EmberAfAttributeType dataType, bool overrideReadOnlyAndDataType, bool justTest);

EmberAfStatus emAfReadAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                uint8_t * dataPtr, uint16_t readLength, EmberAfAttributeType * dataType);
