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

#include <app/ConcreteAttributePath.h>
#include <app/util/AttributesChangedListener.h>
#include <app/util/MarkAttributeDirty.h>
#include <app/util/attribute-metadata.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

struct EmberAfWriteDataInput
{
    // Where the input data originates from.
    //
    // NOTE: at this time there is no information regarding
    // input buffer size and it is assumed "correct" for the given data type.
    //
    // NOTE: technically this should be `const uint8_t*`, however ember internal logic uses a
    //       `emAfReadOrWriteAttribute` method and because of the duality of read/write it needs
    //       a non-const input. This is odd and should probably be fixed.
    uint8_t * dataPtr;

    // The data type that dataPtr points to
    EmberAfAttributeType dataType;

    // Controls when `changeListener` is called to flag an attribute dirty. It allows for things like:
    //     kIfChanged - only if the dataPtr contains a different value than what currenty exists
    //     kNo - never called
    //     kYes - always called
    chip::app::MarkAttributeDirty markDirty = chip::app::MarkAttributeDirty::kIfChanged;

    // Listener called when when the written data is consided changed/dirty.
    // This being called depends on settings of `markDirty` combined with the actual contents of dataPtr
    // vs the contents of the current attribute storage.
    chip::app::AttributesChangedListener * changeListener = nullptr;

    EmberAfWriteDataInput(uint8_t * data, EmberAfAttributeType type) : dataPtr(data), dataType(type) {}

    EmberAfWriteDataInput & SetMarkDirty(chip::app::MarkAttributeDirty value)
    {
        markDirty = value;
        return *this;
    }

    EmberAfWriteDataInput & SetChangeListener(chip::app::AttributesChangedListener * listener)
    {
        changeListener = listener;
        return *this;
    }
};

/**
 * @brief write an attribute, performing all the checks.
 *
 * TODO: what are "all the checks"? There are limitations below
 *       and generally input data pointer does not even have a size,
 *       hence data validity check capabilities seem limited.
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
chip::Protocols::InteractionModel::Status emberAfWriteAttribute(const chip::app::ConcreteAttributePath & path,
                                                                const EmberAfWriteDataInput & input);

/**
 * Override of emberAfWriteAttribute to reduce code size for calls that default
 * markDirty/changeListener and any other future customization calls.
 */
chip::Protocols::InteractionModel::Status emberAfWriteAttribute(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                chip::AttributeId attributeID, uint8_t * dataPtr,
                                                                EmberAfAttributeType dataType);

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
