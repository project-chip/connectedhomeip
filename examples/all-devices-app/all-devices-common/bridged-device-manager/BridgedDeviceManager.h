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

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace chip::app {

/**
 * @brief Manages bridged device additoin and deletion at runtime.
 */
class BridgedDeviceManager
{
public:
    // A unique identifier for a device interface managed by the BridgedDeviceManager.
    struct DeviceInterfaceId
    {
        DeviceInterfaceId();
        DeviceInterfaceId(uint16_t id) : mId(id) {}
        operator uint16_t() const { return mId; }

    private:
        uint16_t mId;
        inline static uint16_t nextId = 1;
    };

    virtual ~BridgedDeviceManager();

    /**
     * @brief Creates a new device instance of the specified type.
     *
     * @param deviceName The name of the device type to create (e.g., "electrical-sensor").
     * @param nodeLabel The label for the new device.
     *
     * If deviceName is "bridged-node", a BridgedNode device SHOULD be created.
     * If deviceName is "aggregator", an Aggregator device SHOULD be created.
     */
    virtual DeviceRegistrationEntry CreateDevice(const std::string & deviceName, const std::string & nodeLabel) = 0;
    BridgedDeviceManager(CodeDrivenDataModelProvider & provider, EndpointIdAllocator & endpointIdAllocator);

    /**
     * @brief Initializes the default aggregator device.
     *
     * If this function is not called, there will not be a default aggregator device.
     * This means that a valid aggregator endpoint must be specified when calling `AddBridgedDevice`, or the operation will fail.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR InitializeDefaultAggregator();

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
     * @param deviceName The name of the device type to create (e.g., "electrical-sensor").
     * @param composition The composition of the device. The parentId will be overridden.
     * @param aggregatorEndpointId The endpoint ID of the aggregator to use. If kInvalidEndpointId, the default aggregator will be used.
     * @param nodeLabel The label for the new device.
     *
     * @return An optional DeviceInterfaceId of the newly added device. If the operation fails, returns std::nullopt.
     *
     * This DeviceInterfaceId can be used to retrieve or remove the device later.
     */
    std::optional<DeviceInterfaceId> AddBridgedDevice(const std::string & deviceName, EndpointComposition composition = {},
                                                      EndpointId aggregatorEndpointId = kInvalidEndpointId,
                                                      const std::string & nodeLabel   = "");

    DeviceInterface * GetDevice(DeviceInterfaceId deviceInterfaceId);
    void RemoveDevice(DeviceInterfaceId deviceInterfaceId);
    void RemoveAllDevices();

private:
    struct DeviceStorage
    {
        DeviceInterfaceId id;
        DeviceRegistrationEntry deviceEntry;
        DeviceRegistrationEntry parentBridgedNodeDeviceEntry;
    };
    std::vector<DeviceStorage>::iterator GetDeviceStorageIterator(DeviceInterfaceId deviceInterfaceId);
    bool IsValidAggregatorEndpoint(EndpointId endpointId);

    std::vector<DeviceStorage> mDeviceEntries;
    CodeDrivenDataModelProvider & mProvider;
    EndpointIdAllocator & mEndpointIdAllocator;
    DeviceRegistrationEntry mDefaultAggregatorEntry;
};

} // namespace chip::app
