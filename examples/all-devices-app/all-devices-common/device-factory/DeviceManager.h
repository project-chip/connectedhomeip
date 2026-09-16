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
#include <unordered_map>
#include <vector>
namespace chip::app {

class DeviceManager
{
public:
    struct DeviceId
    {
        DeviceId() : value(sNextValue++) {}
        explicit DeviceId(uint16_t value) : value(value) {}

        bool operator==(const DeviceId & other) const { return value == other.value; }

        uint16_t value;

    private:
        inline static uint16_t sNextValue = 0;
    };

private:
    struct DeviceWithStateOwning
    {
        std::string name;
        std::unique_ptr<DeviceInterface> device;
        bool isRegistered;
    };

public:
    struct DeviceWithState {
        const std::string & name;
        DeviceInterface & device;
        bool isRegistered;
    };

    DeviceManager(DeviceFactory & deviceFactory, CodeDrivenDataModelProvider & provider) : mDeviceFactory(deviceFactory), mProvider(provider) {};
    void SetEndpointIdAllocator(EndpointIdAllocator * endpointIdAllocator) { mEndpointIdAllocator = endpointIdAllocator; }
    EndpointIdAllocator * GetEndpointIdAllocator() { return mEndpointIdAllocator; }
    std::optional<DeviceId> CreateDevice(const std::string & deviceName, const std::string & nodeLabel = "")
    {
        auto device = mDeviceFactory.Create(deviceName, nodeLabel);
        if (device == nullptr)
        {
            return std::nullopt;
        }
        DeviceId deviceId;
        mConstructedDevices.emplace(deviceId, DeviceWithStateOwning{ deviceName, std::move(device), false });
        return deviceId;
    };
    CHIP_ERROR RegisterDevice(DeviceId deviceId, EndpointIdAllocator & endpointIdAllocator, EndpointComposition composition = {})
    {
        auto it = mConstructedDevices.find(deviceId);
        VerifyOrReturnError(it != mConstructedDevices.end(), CHIP_ERROR_NOT_FOUND);
        auto & deviceWithState = it->second;
        VerifyOrReturnError(!deviceWithState.isRegistered, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(deviceWithState.device->Register(endpointIdAllocator, mProvider, composition));
        deviceWithState.isRegistered = true;
        return CHIP_NO_ERROR;
    };
    std::optional<DeviceId> CreateAndRegisterDevice(const std::string & deviceName, EndpointIdAllocator & endpointIdAllocator,
                                                    const std::string & nodeLabel = "", EndpointComposition composition = {})
    {
        auto deviceId = CreateDevice(deviceName, nodeLabel);
        if (!deviceId.has_value())
        {
            ChipLogError(AppServer, "Failed to create device %s", deviceName.c_str());
            return std::nullopt;
        }

        CHIP_ERROR err = RegisterDevice(deviceId.value(), endpointIdAllocator, composition);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to register device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(), err.Format());
            return std::nullopt;
        }

        return deviceId;
    };
    std::optional<DeviceWithState> GetDevice(DeviceId deviceId)
    {
        auto it = mConstructedDevices.find(deviceId);
        if (it != mConstructedDevices.end())
        {
            return DeviceWithState{ it->second.name, *it->second.device, it->second.isRegistered };
        }
        return std::nullopt;
    };
    std::vector<DeviceInterface *> GetRegisteredDevices() const
    {
        std::vector<DeviceInterface*> registeredDevices;
        for (auto & [deviceName, deviceWithState] : mConstructedDevices)
        {
            if (deviceWithState.isRegistered)
            {
                registeredDevices.push_back(deviceWithState.device.get());
            }
        }
        return registeredDevices;
    };
    void UnregisterDevice(DeviceId deviceId)
    {
        auto it = mConstructedDevices.find(deviceId);
        VerifyOrReturn(it != mConstructedDevices.end());
        auto & deviceWithState = it->second;
        VerifyOrReturn(deviceWithState.isRegistered);
        deviceWithState.device->Unregister(mProvider);
        deviceWithState.isRegistered = false;
    };
    void UnregisterAndDestroyDevice(DeviceId deviceId)
    {
        UnregisterDevice(deviceId);
        auto it = mConstructedDevices.find(deviceId);
        if (it != mConstructedDevices.end())
        {
            mConstructedDevices.erase(it);
        }
    };
    void UnregisterAllDevices()
    {
        for (auto & [deviceName, deviceWithState] : mConstructedDevices)
        {
            if (deviceWithState.isRegistered)
            {
                deviceWithState.device->Unregister(mProvider);
                deviceWithState.isRegistered = false;
            }
        }
    };
    void UnregisterAndDestroyAllDevices()
    {
        UnregisterAllDevices();
        mConstructedDevices.clear();
    };

private:
    struct DeviceIdHash
    {
        size_t operator()(const DeviceId & deviceId) const { return std::hash<uint16_t>{}(deviceId.value); }
    };

    std::unordered_map<DeviceId, DeviceWithStateOwning, DeviceIdHash> mConstructedDevices;
    DeviceFactory & mDeviceFactory;
    CodeDrivenDataModelProvider & mProvider;
    EndpointIdAllocator * mEndpointIdAllocator = nullptr;
};

} // namespace chip::app