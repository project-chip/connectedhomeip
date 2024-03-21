/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    All rights reserved.
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
 * @file CommonDeviceCallbacks.h
 *
 * Common Implementations for the DeviceManager callbacks for all applications
 *
 **/

#pragma once

#include "CHIPDeviceManager.h"
#include <app/util/af-types.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace NXP {
namespace App {
class CommonDeviceCallbacks : public chip::DeviceManager::CHIPDeviceManagerCallbacks
{
public:
    virtual void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    virtual void OnWiFiConnectivityChange(const chip::DeviceLayer::ChipDeviceEvent * event);
    virtual void OnInternetConnectivityChange(const chip::DeviceLayer::ChipDeviceEvent * event);
    virtual void OnSessionEstablished(const chip::DeviceLayer::ChipDeviceEvent * event);
    virtual void OnInterfaceIpAddressChanged(const chip::DeviceLayer::ChipDeviceEvent * event);
#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    virtual void OnComissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event);
#endif
};

class DeviceCallbacksDelegate
{
public:
    static DeviceCallbacksDelegate & Instance()
    {
        static DeviceCallbacksDelegate instance;
        return instance;
    }
    DeviceCallbacksDelegate * mDelegate = nullptr;
    void SetAppDelegate(DeviceCallbacksDelegate * delegate) { mDelegate = delegate; }
    DeviceCallbacksDelegate * GetAppDelegate() { return mDelegate; }
};

/**
 * Returns the application-specific implementation of the CommonDeviceCallbacks object.
 *
 * Applications can use this to gain access to features of the CommonDeviceCallbacks.
 */
extern chip::DeviceManager::CHIPDeviceManagerCallbacks & GetDeviceCallbacks();
} // namespace App
} // namespace NXP
} // namespace chip
