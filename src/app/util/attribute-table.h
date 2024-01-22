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

#include <app/util/af.h>

#define ZCL_NULL_ATTRIBUTE_TABLE_INDEX 0xFFFF

// Remote devices writing attributes of local device
//
// Forwards to emberAfWriteAttribute after an extra check through
// a validation callback: emberAfAllowNetworkWriteAttributeCallback
//
// TODO: emberAfAllowNetworkWriteAttributeCallback seems never used or defined
//       in CHIP code, consider removing
EmberAfStatus emberAfWriteAttributeExternal(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                            uint8_t * dataPtr, EmberAfAttributeType dataType);

/**
 * @brief write an attribute, performing all the checks.
 *
 * This function will attempt to write the attribute value from
 * the provided pointer. This function will only check that the
 * attribute exists. If it does it will write the value into
 * the attribute table for the given attribute.
 *
 * This function will not check to see if the attribute is
 * writable since the read only / writable characteristic
 * of an attribute only pertains to external devices writing
 * over the air. Because this function is being called locally
 * it assumes that the device knows what it is doing and has permission
 * to perform the given operation.
 *
 * if true is passed in for overrideReadOnlyAndDataType then the data type is
 * not checked and the read-only flag is ignored. This mode is meant for
 * testing or setting the initial value of the attribute on the device.
 *
 * if true is passed for justTest, then the type is not written but all
 * checks are done to see if the type could be written
 * reads the attribute specified, returns false if the attribute is not in
 * the table or the data is too large, returns true and writes to dataPtr
 * if the attribute is supported and the readLength specified is less than
 * the length of the data.
 * this returns:
 * - EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT: if endpoint isn't supported by the device.
 * - EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER: if cluster isn't supported on the endpoint.
 * - EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE: if attribute isn't supported in the cluster.
 * - EMBER_ZCL_STATUS_INVALID_DATA_TYPE: if the data type passed in doesnt match the type
 *           stored in the attribute table
 * - EMBER_ZCL_STATUS_UNSUPPORTED_WRITE: if the attribute isnt writable
 * - EMBER_ZCL_STATUS_CONSTRAINT_ERROR: if the value is set out of the allowable range for
 *           the attribute
 * - EMBER_ZCL_STATUS_SUCCESS: if the attribute was found and successfully written
 */
EmberAfStatus emAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID, uint8_t * data,
                                 EmberAfAttributeType dataType, bool overrideReadOnlyAndDataType, bool justTest);

/**
 * @brief Read the attribute value, performing all the checks.
 *
 * This function will attempt to read the attribute and store it into the
 * pointer.
 *
 * dataPtr may be NULL, signifying that we don't need the value, just the status
 * (i.e. whether the attribute can be read).
 */
EmberAfStatus emAfReadAttribute(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
                                uint8_t * dataPtr, uint16_t readLength);
