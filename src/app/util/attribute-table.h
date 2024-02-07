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

/**
 * Write an attribute for a request arriving from external sources.
 *
 * This will check attribute writeability and that
 * the provided data type matches the expected data type.
 */
EmberAfStatus emAfWriteAttributeExternal(chip::EndpointId endpoint, chip::ClusterId cluster, chip::AttributeId attributeID,
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
                                 EmberAfAttributeType dataType, bool overrideReadOnlyAndDataType);
