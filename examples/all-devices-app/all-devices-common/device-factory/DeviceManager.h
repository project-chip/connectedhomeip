/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once
#include <device-factory/DeviceFactory.h>
#include <device/api/allocator/EndpointIdAllocator.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <string>
#include <optional>
#include <cstdint>
#include <functional>
#include <vector>
namespace chip::app {

template <typename DeviceFactoryT>
class DeviceManager
{
public:
    struct DeviceRef {
        const std::string & name;
        DeviceInterface & device;
        std::function<void()> & onDeviceRegistered;
        bool isBridged;
    };

private:
    struct DeviceStorage
    {
        std::string name;
        typename DeviceFactoryT::DeviceRegistrationEntry device;
        bool isBridged;

        DeviceStorage(std::string deviceName, typename DeviceFactoryT::DeviceRegistrationEntry && deviceEntry, bool bridged = false) :
            name(std::move(deviceName)), device(std::move(deviceEntry)), isBridged(bridged)
        {
            VerifyOrDie(device.device != nullptr);
        }

        DeviceRef GetDeviceRef()
        {
            return DeviceRef{ name, *device.device, device.onDeviceRegistered, isBridged };
        }
    };

public:
    DeviceManager(DeviceFactoryT & deviceFactory, CodeDrivenDataModelProvider & provider) : mDeviceFactory(deviceFactory), mProvider(provider) {};
    void SetEndpointIdAllocator(EndpointIdAllocator & endpointIdAllocator) { mEndpointIdAllocator = &endpointIdAllocator; }
    EndpointIdAllocator * GetEndpointIdAllocator() { return mEndpointIdAllocator; }
    std::optional<DeviceRef> AddDevice(const std::string & deviceName, const std::string & nodeLabel = "", EndpointComposition composition = {})
    {
        auto device = mDeviceFactory.Create(deviceName, nodeLabel);
        if (device.device == nullptr)
        {
            ChipLogError(AppServer, "Failed to create device %s", deviceName.c_str());
            return std::nullopt;
        }

        if (mEndpointIdAllocator == nullptr)
        {
            ChipLogError(AppServer, "EndpointIdAllocator is not set. Cannot register device %s", deviceName.c_str());
            return std::nullopt;
        }

        CHIP_ERROR err = device.device->Register(*mEndpointIdAllocator, mProvider, composition);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to register device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(), err.Format());
            return std::nullopt;
        }
        if (device.onDeviceRegistered)
        {
            device.onDeviceRegistered();
        }

        EndpointId parentEndpointId = composition.parentId;
        bool isBridged = false;
        if (parentEndpointId != kInvalidEndpointId)
        {
            auto parentDevice = GetDevice(parentEndpointId);
            isBridged = parentDevice.has_value() && parentDevice->isBridged;
        }
        isBridged |= (deviceName == "bridged-node");
        auto & deviceStorage = mDevices.emplace_back(DeviceStorage{ deviceName, std::move(device), isBridged });
        return deviceStorage.GetDeviceRef();
    };
    std::optional<DeviceRef> GetDevice(EndpointId endpointId)
    {
        auto it = GetDeviceStorageIterator(endpointId);
        if (it != mDevices.end())
        {
            return it->GetDeviceRef();
        }
        return std::nullopt;
    };
    std::vector<DeviceRef> GetAllDevices()
    {
        std::vector<DeviceRef> allDevices;
        allDevices.reserve(mDevices.size());
        for (auto & device : mDevices)
        {
            allDevices.push_back(device.GetDeviceRef());
        }
        return allDevices;
    };
    void RemoveDevice(EndpointId endpointId)
    {
        auto it = GetDeviceStorageIterator(endpointId);
        VerifyOrReturn(it != mDevices.end());
        it->device.device->Unregister(mProvider);
        mDevices.erase(it);
    }
    void RemoveAllDevices()
    {
        for (auto & deviceWithState : mDevices)
        {
            deviceWithState.device.device->Unregister(mProvider);
        }
        mDevices.clear();
    };

private:
    auto GetDeviceStorageIterator(EndpointId endpointId)
    {
        return find_if(mDevices.begin(), mDevices.end(),
                       [endpointId](const auto & device) { return device.device.device->GetEndpointId() == endpointId; });
    };

    std::vector<DeviceStorage> mDevices;
    DeviceFactoryT & mDeviceFactory;
    CodeDrivenDataModelProvider & mProvider;
    EndpointIdAllocator * mEndpointIdAllocator = nullptr;
};

} // namespace chip::app