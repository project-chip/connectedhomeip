/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceManager {

/**
 * @brief
 *   This class provides a skeleton for all the callback functions. The functions will be
 *   called by other objects within the CHIP stack for specific events.
 *   Applications interested in receiving specific callbacks can specialize this class and handle
 *   these events in their implementation of this class.
 */
class DeviceManagerCallbacks
{
public:
    /**
     * @brief
     *   Called after an attribute has been changed
     *
     * @param endpoint           endpoint id
     * @param clusterID          cluster id
     * @param attributeId        attribute id that was changed
     * @param type               attribute type
     * @param size               size of the attribute
     * @param value              pointer to the new value
     */
    virtual void PostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t type,
                                             uint16_t size, uint8_t * value){};

    /**
     * @brief
     *  Called before an attribute is changed
     *
     * @param endpoint           endpoint id
     * @param clusterID          cluster id
     * @param attributeId        attribute id that was changed
     * @param type               attribute type
     * @param size               size of the attribute
     * @param value              pointer to the new value
     */
    Protocols::InteractionModel::Status PreAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId,
                                                                   AttributeId attributeId, uint8_t type, uint16_t size,
                                                                   uint8_t * value)
    {
        return Protocols::InteractionModel::Status::Success;
    };

    virtual ~DeviceManagerCallbacks(){};
};

} // namespace DeviceManager
} // namespace chip
