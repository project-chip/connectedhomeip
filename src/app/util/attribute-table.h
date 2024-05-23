/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

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
 * This function also does NOT check that the input dataType matches the expected
 * data type (as Accessors.h/cpp have this correct by default).
 * TODO: this not checking seems off - what if this is run without Accessors.h ?
 */
chip::Protocols::InteractionModel::Status emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                chip::AttributeId attributeID, uint8_t * dataPtr,
                                                                EmberAfAttributeType dataType);

/**
 * A version of emberAfWriteAttribute that allows controlling when the attribute
 * should be marked dirty.  This is an overload, not an optional argument, to
 * reduce codesize at all the callsites that want to write without doing
 * anything special to control the dirty marking.
 */
chip::Protocols::InteractionModel::Status emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                chip::AttributeId attributeID, uint8_t * dataPtr,
                                                                EmberAfAttributeType dataType,
                                                                chip::app::MarkAttributeDirty markDirty);

/**
 * @brief Read the attribute value, performing all the checks.
 *
 * This function will attempt to read the attribute and store it into the
 * pointer.
 *
 * dataPtr may be NULL, signifying that we don't need the value, just the status
 * (i.e. whether the attribute can be read).
 */
chip::Protocols::InteractionModel::Status emberAfReadAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                               chip::AttributeId attributeID, uint8_t * dataPtr,
                                                               uint16_t readLength);
