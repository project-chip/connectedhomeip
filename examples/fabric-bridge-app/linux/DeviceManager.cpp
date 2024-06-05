/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "DeviceManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/util.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>

#include <cstdio>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

class DeviceManager;

namespace {

constexpr uint8_t kMaxRetries = 10;
DeviceManager gDeviceManager;

} // namespace

DeviceManager::DeviceManager()
{
    memset(mDevices, 0, sizeof(mDevices));
    mFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    mCurrentEndpointId = mFirstDynamicEndpointId;
}

int DeviceManager::AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep,
                                     const chip::Span<const EmberAfDeviceType> & deviceTypeList,
                                     const chip::Span<chip::DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (nullptr == mDevices[index])
        {
            mDevices[index] = dev;
            CHIP_ERROR err;
            int retryCount = 0;
            while (retryCount < kMaxRetries)
            {
                DeviceLayer::StackLock lock;
                dev->SetEndpointId(mCurrentEndpointId);
                dev->SetParentEndpointId(parentEndpointId);
                err =
                    emberAfSetDynamicEndpoint(index, mCurrentEndpointId, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
                if (err == CHIP_NO_ERROR)
                {
                    ChipLogProgress(NotSpecified, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    mCurrentEndpointId, index);
                    return index;
                }
                if (err != CHIP_ERROR_ENDPOINT_EXISTS)
                {
                    return -1; // Return error as endpoint addition failed due to an error other than endpoint already exists
                }
                // Increment the endpoint ID and handle wrap condition
                if (++mCurrentEndpointId < mFirstDynamicEndpointId)
                {
                    mCurrentEndpointId = mFirstDynamicEndpointId;
                }
                retryCount++;
            }
            ChipLogError(NotSpecified, "Failed to add dynamic endpoint after %d retries", kMaxRetries);
            return -1; // Return error as all retries are exhausted
        }
        index++;
    }
    ChipLogProgress(NotSpecified, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int DeviceManager::RemoveDeviceEndpoint(Device * dev)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mDevices[index] == dev)
        {
            DeviceLayer::StackLock lock;
            // Silence complaints about unused ep when progress logging
            // disabled.
            [[maybe_unused]] EndpointId ep = emberAfClearDynamicEndpoint(index);
            mDevices[index]                = nullptr;
            ChipLogProgress(NotSpecified, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            return index;
        }
        index++;
    }
    return -1;
}

Device * DeviceManager::GetDevice(uint16_t index) const
{
    if (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        return mDevices[index];
    }
    return nullptr;
}

DeviceManager * GetDeviceManager()
{
    return &gDeviceManager;
}
