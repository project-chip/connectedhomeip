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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief This file contains the definitions for the
 *attribute table, its sizes, count, and API.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "af.h"

#define ZCL_NULL_ATTRIBUTE_TABLE_INDEX 0xFFFF

// Remote devices writing attributes of local device
EmberAfStatus emberAfWriteAttributeExternal(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                            uint8_t mask, uint16_t manufacturerCode, uint8_t * dataPtr,
                                            EmberAfAttributeType dataType);

void emberAfRetrieveAttributeAndCraftResponse(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attrId,
                                              uint8_t mask, uint16_t manufacturerCode, uint16_t readLength);
EmberAfStatus emberAfAppendAttributeReportFields(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                 chip::AttributeId attributeId, uint8_t mask, uint8_t * buffer, uint8_t bufLen,
                                                 uint8_t * bufIndex);
void emberAfPrintAttributeTable(void);

bool emberAfReadSequentialAttributesAddToResponse(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                  chip::AttributeId startAttributeId, uint8_t mask, uint16_t manufacturerCode,
                                                  uint8_t maxAttributeIds, bool includeAccessControl);

EmberAfStatus emAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t mask,
                                 uint16_t manufacturerCode, uint8_t * data, EmberAfAttributeType dataType,
                                 bool overrideReadOnlyAndDataType, bool justTest);

EmberAfStatus emAfReadAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t mask,
                                uint16_t manufacturerCode, uint8_t * dataPtr, uint16_t readLength, EmberAfAttributeType * dataType);
