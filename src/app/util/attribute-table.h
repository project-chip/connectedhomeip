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

#include <core/CHIPCallback.h>
struct emberAfOnAttributeChangedFilter
{
    chip::EndpointId mEndpointId;
    chip::ClusterId mClusterId;
    chip::AttributeId mAttributeId;
    uint8_t mMask;

    bool matches(emberAfOnAttributeChangedFilter & other)
    {
        return (other.mEndpointId == mEndpointId) && (other.mClusterId == mClusterId) && (other.mAttributeId == mAttributeId) &&
            ((other.mMask & mMask) != 0);
    }
};

/**
 * @brief
 *  Register a callback to run when the attribute store (attribute table) is updated.  The Callback is run
 *    from the top of the main thread's stack via SystemLayer::ScheduleWork()
 *
 *  @param [in] cb        Callback to run when the attribute store is updated in a way that matches the filter
 *  @param [in] filter    A description of what kind of changes are of interest to the caller
 *
 * @note
 *   The Callback and the filter must remain valid memory until either the Callback is fired or canceled, i.e.
 *     the filter needs to outlast this registration
 *
 */
void emberAfOnPostAttributeChanged(emberAfOnAttributeChangedFilter & filter, chip::Callback::Callback<> & cb);
