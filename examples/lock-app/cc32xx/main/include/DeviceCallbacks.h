/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <CHIPDeviceManager.h>
#include <platform/CHIPDeviceLayer.h>

class DeviceCallbacks : public chip::DeviceManager::CHIPDeviceManagerCallbacks
{
public:
    void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
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
