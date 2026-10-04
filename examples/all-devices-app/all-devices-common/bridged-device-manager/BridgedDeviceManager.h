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
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/Interface.h>
#include <device/api/allocator/EndpointIdAllocator.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace chip::app {

class BridgedDeviceManager
{
public:
    struct DeviceId
    {
        DeviceId()
        {
            VerifyOrDie(nextId != 0); // Ensure we didn't overflow.
            mId = nextId++;
        }
        DeviceId(uint16_t id) : mId(id) {}
        operator uint16_t() const { return mId; }
    private:
        uint16_t mId;
        inline static uint16_t nextId = 1;
    };
private:
    struct DeviceStorage
    {
        DeviceId id;
        DeviceRegistrationEntry deviceEntry;
        DeviceRegistrationEntry parentBridgedNodeDeviceEntry;
    };
public:
    virtual ~BridgedDeviceManager()
    {
        RemoveAllDevices();
        if (mDefaultAggregatorEntry.device != nullptr)
        {
            mDefaultAggregatorEntry.device->Unregister(mProvider);
        }
    }
    virtual DeviceRegistrationEntry CreateDevice(const std::string & deviceName, const std::string & nodeLabel) = 0;
    BridgedDeviceManager(CodeDrivenDataModelProvider & provider, EndpointIdAllocator & endpointIdAllocator) : mProvider(provider), mEndpointIdAllocator(endpointIdAllocator), mDefaultAggregatorEntry(CreateDevice("aggregator", "Default aggregator for bridged devices"))
    {
        if (mDefaultAggregatorEntry.device == nullptr)
        {
            ChipLogError(AppServer, "Failed to create a default aggregator device");
            return;
        }
        CHIP_ERROR err = mDefaultAggregatorEntry.device->Register(mEndpointIdAllocator, mProvider, {});
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to register the default aggregator device: %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
        if (mDefaultAggregatorEntry.onDeviceRegistered)
        {
            mDefaultAggregatorEntry.onDeviceRegistered();
        }
    }
    std::optional<DeviceId> AddBridgedDevice(const std::string & deviceName, EndpointComposition composition = {}, EndpointId aggregatorEndpointId = kInvalidEndpointId, const std::string & nodeLabel = "")
    {
        if (aggregatorEndpointId != kInvalidEndpointId)
        {
            if (!IsValidAggregatorEndpoint(aggregatorEndpointId))
            {
                ChipLogError(AppServer, "No aggregator device type on endpoint ID: %u", aggregatorEndpointId);
                return std::nullopt;
            }
        }
        else
        {
            auto defaultAggregator = mDefaultAggregatorEntry.device.get();
            if (defaultAggregator == nullptr)
            {
                ChipLogError(AppServer, "No default aggregator available");
                return std::nullopt;
            }
            aggregatorEndpointId = defaultAggregator->GetEndpointId();
        }
        auto bridgedNodeEntry = CreateDevice("bridged-node", "");
        if (bridgedNodeEntry.device == nullptr)
        {
            ChipLogError(AppServer, "Failed to create bridged node for device %s", deviceName.c_str());
            return std::nullopt;
        }
        EndpointComposition bridgedNodeComposition{aggregatorEndpointId, EndpointCompositionPattern::kFullFamily};
        CHIP_ERROR err = bridgedNodeEntry.device->Register(mEndpointIdAllocator, mProvider, bridgedNodeComposition);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to register bridged node for device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(), err.Format());
            return std::nullopt;
        }
        auto deviceEntry = CreateDevice(deviceName, nodeLabel);
        if (deviceEntry.device == nullptr)
        {
            ChipLogError(AppServer, "Failed to create device %s", deviceName.c_str());
            return std::nullopt;
        }

        err = deviceEntry.device->Register(mEndpointIdAllocator, mProvider, composition);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to register device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(), err.Format());
            return std::nullopt;
        }
        if (deviceEntry.onDeviceRegistered)
        {
            deviceEntry.onDeviceRegistered();
        }
        return mDeviceEntries.emplace_back(DeviceId(), deviceEntry, bridgedNodeEntry).id;
    };
    DeviceInterface * GetDevice(DeviceId deviceId)
    {
        auto it = GetDeviceStorageIterator(deviceId);
        if (it != mDeviceEntries.end())
        {
            return it->deviceEntry.device.get();
        }
        return nullptr;
    };
    void RemoveDevice(DeviceId deviceId)
    {
        auto it = GetDeviceStorageIterator(deviceId);
        VerifyOrReturn(it != mDeviceEntries.end());
        it->deviceEntry.device->Unregister(mProvider);
        it->parentBridgedNodeDeviceEntry.device->Unregister(mProvider);
        mDeviceEntries.erase(it);
    }
    void RemoveAllDevices()
    {
        for (auto & device : mDeviceEntries)
        {
            device.deviceEntry.device->Unregister(mProvider);
            device.parentBridgedNodeDeviceEntry.device->Unregister(mProvider);
        }
        mDeviceEntries.clear();
    };

private:
    auto GetDeviceStorageIterator(DeviceId deviceId)
    {
        return std::find_if(mDeviceEntries.begin(), mDeviceEntries.end(),
                            [deviceId](const auto & device) { return device.id == deviceId; });
    };

    bool IsValidAggregatorEndpoint(EndpointId endpointId)
    {
        ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> endpointsList;
        ReturnOnFailure(mProvider.DeviceTypes(endpointId, endpointsList));

        return std::any_of(endpointsList.begin(), endpointsList.end(),
                           [](const auto & deviceTypeEntry) { return deviceTypeEntry.type == Device::Type::kAggregator; });
    };

    std::vector<DeviceStorage> mDeviceEntries;
    CodeDrivenDataModelProvider & mProvider;
    EndpointIdAllocator & mEndpointIdAllocator;
    DeviceRegistrationEntry mDefaultAggregatorEntry;
};

} // namespace chip::app
