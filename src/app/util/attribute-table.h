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

#ifndef ZCL_UTIL_ATTRIBUTE_TABLE_H
#define ZCL_UTIL_ATTRIBUTE_TABLE_H

#include "af.h"

#define ZCL_NULL_ATTRIBUTE_TABLE_INDEX 0xFFFF

// Remote devices writing attributes of local device
EmberAfStatus emberAfWriteAttributeExternal(uint8_t endpoint, EmberAfClusterId cluster, EmberAfAttributeId attributeID,
                                            uint8_t mask, uint16_t manufacturerCode, uint8_t * dataPtr,
                                            EmberAfAttributeType dataType);

void emberAfRetrieveAttributeAndCraftResponse(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attrId, uint8_t mask,
                                              uint16_t manufacturerCode, uint16_t readLength);
EmberAfStatus emberAfAppendAttributeReportFields(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId,
                                                 uint8_t mask, uint8_t * buffer, uint8_t bufLen, uint8_t * bufIndex);
void emberAfPrintAttributeTable(void);

bool emberAfReadSequentialAttributesAddToResponse(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId startAttributeId,
                                                  uint8_t mask, uint16_t manufacturerCode, uint8_t maxAttributeIds,
                                                  bool includeAccessControl);

EmberAfStatus emAfWriteAttribute(uint8_t endpoint, EmberAfClusterId cluster, EmberAfAttributeId attributeID, uint8_t mask,
                                 uint16_t manufacturerCode, uint8_t * data, EmberAfAttributeType dataType,
                                 bool overrideReadOnlyAndDataType, bool justTest);

EmberAfStatus emAfReadAttribute(uint8_t endpoint, EmberAfClusterId cluster, EmberAfAttributeId attributeID, uint8_t mask,
                                uint16_t manufacturerCode, uint8_t * dataPtr, uint16_t readLength, EmberAfAttributeType * dataType);

#endif // ZCL_UTIL_ATTRIBUTE_TABLE_H
