/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *    @file
 *      This file contains definitions for the CHIP DeviceManager Interface
 *
 *      This object will co-ordinate multiple activities such as
 *      initialisation, rendezvous, session mgmt and other such
 *      activities within the CHIP stack. This is a singleton object.
 */

#pragma once

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#include <lib/support/DLLUtil.h>

#include <stdarg.h>
#include <stdlib.h>

#include <app/util/af-types.h>

namespace chip {
namespace DeviceManager {

/**
 * @brief
 *   This class provides a skeleton for all the callback functions. The functions will be
 *   called by other objects within the CHIP stack for specific events.
 *   Applications interested in receiving specific callbacks can specialize this class and handle
 *   these events in their implementation of this class.
 */
class CHIPDeviceManagerCallbacks
{
public:
    /**
     * @brief
     *   Called when CHIP Device events (PublicEventTypes) are triggered.
     *
     * @param event   ChipDeviceEvent that occurred
     * @param arg     arguments specific to the event, if any
     */
    virtual void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg) {}

    /**
     * @brief
     *   Called after an attribute has been changed
     *
     * @param endpoint           endpoint id
     * @param clusterID          cluster id
     * @param attributeId        attribute id that was changed
     * @param mask               mask of the attribute
     * @param manufacturerCode   manufacturer code
     * @param type               attribute type
     * @param size               size of the attribute
     * @param value              pointer to the new value
     */
    virtual void PostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                             uint8_t mask, uint8_t type, uint16_t size, uint8_t * value)
    {}
    virtual ~CHIPDeviceManagerCallbacks() {}
};

/**
 * @brief
 *   A common class that drives other components of the CHIP stack
 */
class DLL_EXPORT CHIPDeviceManager
{
public:
    CHIPDeviceManager(const CHIPDeviceManager &)  = delete;
    CHIPDeviceManager(const CHIPDeviceManager &&) = delete;
    CHIPDeviceManager & operator=(const CHIPDeviceManager &) = delete;

    static CHIPDeviceManager & GetInstance()
    {
        static CHIPDeviceManager instance;
        return instance;
    }

    /**
     * @brief
     *   Initialise CHIPDeviceManager
     *
     * @param cb Application's instance of the CHIPDeviceManagerCallbacks for consuming events
     */
    CHIP_ERROR Init(CHIPDeviceManagerCallbacks * cb);

    /**
     * @brief
     *   Fetch a pointer to the registered CHIPDeviceManagerCallbacks object.
     *
     */
    CHIPDeviceManagerCallbacks * GetCHIPDeviceManagerCallbacks() { return mCB; }

    /**
     * Use internally for registration of the ChipDeviceEvents
     */
    static void CommonDeviceEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

private:
    CHIPDeviceManagerCallbacks * mCB = nullptr;
    CHIPDeviceManager() {}
};

} // namespace DeviceManager
} // namespace chip
