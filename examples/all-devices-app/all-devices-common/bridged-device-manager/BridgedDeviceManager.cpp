/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "BridgedDeviceManager.h"

#include <device/types/aggregator/Aggregator.h>
#include <device/types/bridged-node/BridgedNode.h>
#include <devices/Types.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>
#include <utility>

namespace chip::app {

BridgedDeviceManager::DeviceInterfaceId::DeviceInterfaceId()
{
    VerifyOrDie(nextId != 0);
    mId = nextId++;
}

BridgedDeviceManager::BridgedDeviceManager(CodeDrivenDataModelProvider & provider, EndpointIdAllocator & endpointIdAllocator) :
    mProvider(provider), mEndpointIdAllocator(endpointIdAllocator)
{}

BridgedDeviceManager::~BridgedDeviceManager()
{
    RemoveAllDevices();
    if (mDefaultAggregatorEntry.device != nullptr)
    {
        mDefaultAggregatorEntry.Unregister(mProvider);
    }
}

CHIP_ERROR BridgedDeviceManager::InitializeDefaultAggregator()
{
    mDefaultAggregatorEntry = CreateDevice("aggregator", "Default aggregator for bridged devices");
    if (mDefaultAggregatorEntry.device == nullptr)
    {
        ChipLogError(AppServer, "Failed to create a default aggregator device");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (!InterfaceHasDeviceType(*mDefaultAggregatorEntry.device, Device::Type::kAggregator))
    {
        ChipLogError(AppServer, "Failed to create a valid default aggregator device");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    CHIP_ERROR err = mDefaultAggregatorEntry.Register(mEndpointIdAllocator, mProvider, {});
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register the default aggregator device: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

std::optional<BridgedDeviceManager::DeviceInterfaceId> BridgedDeviceManager::AddBridgedDevice(const std::string & deviceName,
                                                                                              EndpointComposition composition,
                                                                                              EndpointId aggregatorEndpointId,
                                                                                              const std::string & nodeLabel)
{
    if (aggregatorEndpointId != kInvalidEndpointId)
    {
        if (!IsValidAggregatorEndpoint(aggregatorEndpointId))
        {
            ChipLogError(AppServer, "No aggregator device type found on endpoint ID: %u", aggregatorEndpointId);
            return std::nullopt;
        }
    }
    else
    {
        auto defaultAggregator = static_cast<SingleEndpoint *>(mDefaultAggregatorEntry.device.get());
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
    if (!InterfaceHasDeviceType(*bridgedNodeEntry.device, Device::Type::kBridgedNode))
    {
        ChipLogError(AppServer, "Failed to create a valid bridged node for device %s", deviceName.c_str());
        return std::nullopt;
    }
    EndpointComposition bridgedNodeComposition{ aggregatorEndpointId, DataModel::EndpointCompositionPattern::kFullFamily };
    CHIP_ERROR err = bridgedNodeEntry.device->Register(mEndpointIdAllocator, mProvider, bridgedNodeComposition);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register bridged node for device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(),
                     err.Format());
        return std::nullopt;
    }
    auto deviceEntry = CreateDevice(deviceName, nodeLabel);
    if (deviceEntry.device == nullptr)
    {
        ChipLogError(AppServer, "Failed to create device %s", deviceName.c_str());
        bridgedNodeEntry.device->Unregister(mProvider);
        return std::nullopt;
    }
    composition.parentId = static_cast<SingleEndpoint *>(bridgedNodeEntry.device.get())->GetEndpointId();
    err                  = deviceEntry.device->Register(mEndpointIdAllocator, mProvider, composition);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(), err.Format());
        bridgedNodeEntry.device->Unregister(mProvider);
        return std::nullopt;
    }
    if (bridgedNodeEntry.afterDeviceRegistration)
    {
        bridgedNodeEntry.afterDeviceRegistration();
    }
    if (deviceEntry.afterDeviceRegistration)
    {
        deviceEntry.afterDeviceRegistration();
    }
    mDeviceEntries.push_back(DeviceStorage(DeviceInterfaceId(), std::move(deviceEntry), std::move(bridgedNodeEntry)));
    return mDeviceEntries.back().id;
}

DeviceInterface * BridgedDeviceManager::GetDevice(DeviceInterfaceId deviceInterfaceId)
{
    auto it = GetDeviceStorageIterator(deviceInterfaceId);
    if (it != mDeviceEntries.end())
    {
        return it->deviceEntry.device.get();
    }
    return nullptr;
}

void BridgedDeviceManager::RemoveDevice(DeviceInterfaceId deviceInterfaceId)
{
    auto it = GetDeviceStorageIterator(deviceInterfaceId);
    VerifyOrReturn(it != mDeviceEntries.end());
    it->deviceEntry.Unregister(mProvider);
    it->parentBridgedNodeDeviceEntry.Unregister(mProvider);
    mDeviceEntries.erase(it);
}

void BridgedDeviceManager::RemoveAllDevices()
{
    for (auto & device : mDeviceEntries)
    {
        device.deviceEntry.Unregister(mProvider);
        device.parentBridgedNodeDeviceEntry.Unregister(mProvider);
    }
    mDeviceEntries.clear();
}

std::vector<BridgedDeviceManager::DeviceStorage>::iterator
BridgedDeviceManager::GetDeviceStorageIterator(DeviceInterfaceId deviceInterfaceId)
{
    return std::find_if(mDeviceEntries.begin(), mDeviceEntries.end(),
                        [deviceInterfaceId](const auto & device) { return device.id == deviceInterfaceId; });
}

bool BridgedDeviceManager::IsValidAggregatorEndpoint(EndpointId endpointId)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    ReturnValueOnFailure(mProvider.DeviceTypes(endpointId, deviceTypesList), false);

    auto deviceTypes = deviceTypesList.TakeBuffer();
    return std::any_of(deviceTypes.begin(), deviceTypes.end(),
                       [](const auto & deviceTypeEntry) { return deviceTypeEntry == Device::Type::kAggregator; });
}

bool BridgedDeviceManager::InterfaceHasDeviceType(DeviceInterface & deviceInterface, const DataModel::DeviceTypeEntry & deviceType)
{
    return std::find(deviceInterface.GetDeviceTypes().begin(), deviceInterface.GetDeviceTypes().end(),
                     deviceType) != deviceInterface.GetDeviceTypes().end();
}

} // namespace chip::app
