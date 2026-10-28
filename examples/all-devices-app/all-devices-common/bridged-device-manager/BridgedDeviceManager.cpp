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

namespace {

// RAII helper to unregister an entry in case of errors.
class RAIIDeviceEntryRollbackOnFailure
{
public:
    RAIIDeviceEntryRollbackOnFailure(DeviceRegistrationEntry & entry, CodeDrivenDataModelProvider & provider) :
    mEntry(entry), mProvider(provider)
    {}

    ~RAIIDeviceEntryRollbackOnFailure()
    {
        if (mUnregister)
        {
            mEntry.Unregister(mProvider);
        }
        if (mReset)
        {
            mEntry = DeviceRegistrationEntry{ nullptr, nullptr, nullptr };
        }
    }

    void UnregisterOnDestruction() { mUnregister = true; }
    void ResetOnDestruction() { mReset = true; }
    void CancelRollback() { mUnregister = false; mReset = false; }
private:
    DeviceRegistrationEntry & mEntry;
    CodeDrivenDataModelProvider & mProvider;
    bool mUnregister = false;
    bool mReset = false;
};

} // namespace

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
    VerifyOrReturnError(mDefaultAggregatorEntry.device == nullptr, CHIP_ERROR_ALREADY_INITIALIZED);
    mDefaultAggregatorEntry = CreateDevice("aggregator", "Default aggregator for bridged devices");
    if (mDefaultAggregatorEntry.device == nullptr)
    {
        ChipLogError(AppServer, "Failed to create a default aggregator device");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    RAIIDeviceEntryRollbackOnFailure rollback{mDefaultAggregatorEntry, mProvider};
    rollback.ResetOnDestruction();
    CHIP_ERROR err = mDefaultAggregatorEntry.Register(mEndpointIdAllocator, mProvider, {});
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register the default aggregator device: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    rollback.UnregisterOnDestruction();
    if (!InterfaceHasDeviceType(*mDefaultAggregatorEntry.device, Device::Type::kAggregator))
    {
        ChipLogError(AppServer, "The default aggregator device is not a valid aggregator device");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    rollback.CancelRollback();
    return CHIP_NO_ERROR;
}

std::optional<BridgedDeviceManager::DeviceInterfaceId> BridgedDeviceManager::AddBridgedDevice(const std::string & deviceName,
                                                                                              EndpointComposition composition,
                                                                                              EndpointId aggregatorEndpointId,
                                                                                              const std::string & nodeLabel,
                                                                                              const std::string & bridgedNodeLabel)
{
    if (deviceName == "bridged-node" || deviceName == "aggregator")
    {
        ChipLogError(AppServer, "Creation of `%s` bridged device is not allowed", deviceName.c_str());
        return std::nullopt;
    }

    auto bridgedNodeEntry = CreateDevice("bridged-node", bridgedNodeLabel);
    if (bridgedNodeEntry.device == nullptr)
    {
        ChipLogError(AppServer, "Failed to create bridged node for device %s", deviceName.c_str());
        return std::nullopt;
    }
    auto deviceEntry = CreateDevice(deviceName, nodeLabel);
    if (deviceEntry.device == nullptr)
    {
        ChipLogError(AppServer, "Failed to create device %s", deviceName.c_str());
        return std::nullopt;
    }

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
        // Initialize the default aggregator lazily.
        if (mDefaultAggregatorEntry.device == nullptr)
        {
            CHIP_ERROR err = InitializeDefaultAggregator();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to initialize the default aggregator: %" CHIP_ERROR_FORMAT, err.Format());
                return std::nullopt;
            }
        }
        auto defaultAggregator = static_cast<SingleEndpoint *>(mDefaultAggregatorEntry.device.get());
        aggregatorEndpointId   = defaultAggregator->GetEndpointId();
    }

    EndpointComposition bridgedNodeComposition{ aggregatorEndpointId, DataModel::EndpointCompositionPattern::kFullFamily };
    CHIP_ERROR err = bridgedNodeEntry.Register(mEndpointIdAllocator, mProvider, bridgedNodeComposition);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register bridged node for device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(),
                     err.Format());
        return std::nullopt;
    }

    RAIIDeviceEntryRollbackOnFailure rollback{bridgedNodeEntry, mProvider};
    rollback.UnregisterOnDestruction();

    if (!InterfaceHasDeviceType(*bridgedNodeEntry.device, Device::Type::kBridgedNode))
    {
        ChipLogError(AppServer, "The bridged node for device %s is not a valid bridged node device", deviceName.c_str());
        return std::nullopt;
    }

    composition.parentId = static_cast<SingleEndpoint *>(bridgedNodeEntry.device.get())->GetEndpointId();
    err                  = deviceEntry.Register(mEndpointIdAllocator, mProvider, composition);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register device %s: %" CHIP_ERROR_FORMAT, deviceName.c_str(), err.Format());
        return std::nullopt;
    }

    mDeviceEntries.push_back(DeviceStorage{ DeviceInterfaceId(), std::move(deviceEntry), std::move(bridgedNodeEntry) });
    rollback.CancelRollback();
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
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    ReturnValueOnFailure(deviceInterface.DeviceTypes(deviceTypesList), false);

    auto deviceTypes = deviceTypesList.TakeBuffer();
    return std::find(deviceTypes.begin(), deviceTypes.end(), deviceType) != deviceTypes.end();
}

} // namespace chip::app
