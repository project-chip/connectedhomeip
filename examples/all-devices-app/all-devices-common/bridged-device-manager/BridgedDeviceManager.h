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
#include <device-factory/DeviceRegistrationEntry.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace chip::app {

class BridgedDeviceManager
{
public:
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
    virtual DeviceRegistrationEntry CreateDevice(const std::string & deviceName, const std::string & nodeLabel) = 0;
    BridgedDeviceManager(CodeDrivenDataModelProvider & provider, EndpointIdAllocator & endpointIdAllocator);
    CHIP_ERROR InitializeDefaultAggregator();
    std::optional<DeviceInterfaceId> AddBridgedDevice(const std::string & deviceName, EndpointComposition composition = {},
                                                      EndpointId aggregatorEndpointId = kInvalidEndpointId,
                                                      const std::string & nodeLabel = "");
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
