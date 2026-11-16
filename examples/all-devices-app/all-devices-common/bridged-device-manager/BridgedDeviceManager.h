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
#include <device-factory/DeviceRegistrationEntry.h>
#include <device/api/Interface.h>
#include <device/api/allocator/EndpointIdAllocator.h>
#include <device/types/aggregator/Aggregator.h>
#include <device/types/bridged-node/BridgedNode.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace chip::app {

/**
 * @brief Manages bridged device addition and deletion at runtime.
 */
class BridgedDeviceManager
{
public:
    // A unique identifier for a device interface managed by the BridgedDeviceManager.
    using DeviceInterfaceId = uint16_t;

    virtual ~BridgedDeviceManager();

    /**
     * @brief Creates a new device instance of the specified type.
     *
     * @param deviceName The name of the device type to create (e.g., "electrical-sensor").
     * @param nodeLabel The label for the new device.
     */
    virtual DeviceRegistrationEntry CreateDevice(const std::string & deviceName, const std::string & nodeLabel) = 0;

    /**
     * @brief Creates a new bridged-node device instance.
     *
     * @param nodeLabel The label for the new device.
     */
    virtual DeviceRegistrationEntryOfConcreteType<BridgedNode> CreateBridgedNode(const std::string & nodeLabel) = 0;

    /**
     * @brief Creates a new aggregator device instance.
     *
     * @param nodeLabel The label for the new device.
     */
    virtual DeviceRegistrationEntryOfConcreteType<Aggregator> CreateAggregator() = 0;

    BridgedDeviceManager(CodeDrivenDataModelProvider & provider, EndpointIdAllocator & endpointIdAllocator);

    /**
     * @brief Adds a new bridged device to the DataModelProvider.
     *
     * This will create a new device with the hierarchy:
     *
     *       Aggregator (specified or default)
     *                     |
     *       Bridged Node (created automatically)
     *                     |
     *       New Device (created based on deviceName)
     *
     * @param deviceName The name of the device type to create (e.g., "electrical-sensor"). The names "bridged-node" and
     * "aggregator" will be rejected.
     * @param composition The composition of the device. The parentId will be overridden.
     * @param aggregatorEndpointId The endpoint ID of the aggregator to use. If kInvalidEndpointId, the default aggregator will be
     * used (it also will be lazily initialized if needed).
     * @param nodeLabel The label for the new device.
     *
     * @return An optional DeviceInterfaceId of the newly added device. If the operation fails, returns std::nullopt.
     *
     * This DeviceInterfaceId can be used to retrieve or remove the device later.
     */
    std::optional<DeviceInterfaceId> AddBridgedDevice(const std::string & deviceName, EndpointComposition composition = {},
                                                      EndpointId aggregatorEndpointId      = kInvalidEndpointId,
                                                      const std::string & nodeLabel        = "",
                                                      const std::string & bridgedNodeLabel = "");

    // The pointer returned by GetDevice is valid until RemoveDevice is called for the same deviceInterfaceId.
    // Returns nullptr if no device is found for the given deviceInterfaceId.
    DeviceInterface * GetDevice(DeviceInterfaceId deviceInterfaceId);
    void RemoveDevice(DeviceInterfaceId deviceInterfaceId);
    void RemoveAllDevices();

private:
    struct DeviceStorage
    {
        DeviceInterfaceId id;
        DeviceRegistrationEntry deviceEntry;
        DeviceRegistrationEntryOfConcreteType<BridgedNode> parentBridgedNodeDeviceEntry;
    };

    std::vector<DeviceStorage>::iterator GetDeviceStorageIterator(DeviceInterfaceId deviceInterfaceId);
    CHIP_ERROR InitializeDefaultAggregator();
    bool IsValidAggregatorEndpoint(EndpointId endpointId);

    std::vector<DeviceStorage> mDeviceEntries;
    CodeDrivenDataModelProvider & mProvider;
    EndpointIdAllocator & mEndpointIdAllocator;
    DeviceRegistrationEntryOfConcreteType<Aggregator> mDefaultAggregatorEntry;

    // id counter for created devices
    DeviceInterfaceId mNextDeviceId = 0;
};

} // namespace chip::app
