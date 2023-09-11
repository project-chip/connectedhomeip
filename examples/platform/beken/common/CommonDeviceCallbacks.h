/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/util/af-types.h>
#include <common/CHIPDeviceManager.h>
#include <platform/CHIPDeviceLayer.h>

class CommonDeviceCallbacks : public chip::DeviceManager::CHIPDeviceManagerCallbacks
{
public:
    void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg) override;

private:
    void OnInternetConnectivityChange(const chip::DeviceLayer::ChipDeviceEvent * event);
};

class DeviceCallbacksDelegate
{
public:
    static DeviceCallbacksDelegate & Instance()
    {
        static DeviceCallbacksDelegate instance;
        return instance;
    }
    virtual void OnIPv4ConnectivityEstablished() {}
    virtual void OnIPv4ConnectivityLost() {}
    void SetAppDelegate(DeviceCallbacksDelegate * delegate) { mDelegate = delegate; }
    DeviceCallbacksDelegate * GetAppDelegate() { return mDelegate; }

private:
    DeviceCallbacksDelegate * mDelegate = nullptr;
};
