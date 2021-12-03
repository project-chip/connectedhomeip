/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @file Contains shell commands for a commissionee (eg. end device) related to commissioning.
 */

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/Command.h>
#include <app/util/ContentAppPlatform.h>
#include <cstdio>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::AppPlatform;

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength, int32_t index)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeReadCallback endpoint %d ", endpointIndex);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    ContentApp * app = chip::AppPlatform::AppPlatform::GetInstance().GetContentAppByEndpointId(endpoint);
    if (app != NULL)
    {
        ret = app->HandleReadAttribute(clusterId, attributeMetadata->attributeId, buffer, maxReadLength);
    }

    return ret;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                    uint8_t * buffer, int32_t index)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback endpoint %d ", endpointIndex);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    ContentApp * app = chip::AppPlatform::AppPlatform::GetInstance().GetContentAppByEndpointId(endpoint);
    if (app != NULL)
    {
        ret = app->HandleWriteAttribute(clusterId, attributeMetadata->attributeId, buffer);
    }

    return ret;
}

namespace chip {
namespace AppPlatform {

int AppPlatform::AddContentApp(ContentApp * app, EmberAfEndpointType * ep, uint16_t deviceType)
{
    ChipLogProgress(DeviceLayer, "Adding device %s ", app->GetApplicationBasic()->GetApplicationName());
    uint8_t index = 0;
    // check if already loaded
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] == app)
        {
            ChipLogProgress(DeviceLayer, "Already added");
            return index;
        }
        index++;
    }

    index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (NULL == mContentApps[index])
        {
            mContentApps[index] = app;
            EmberAfStatus ret;
            while (1)
            {
                ret = emberAfSetDynamicEndpoint(index, mCurrentEndpointId, ep, deviceType, DEVICE_VERSION_DEFAULT);
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)",
                                    app->GetApplicationBasic()->GetApplicationName(), mCurrentEndpointId, index);
                    app->SetEndpointId(mCurrentEndpointId);
                    return index;
                }
                else if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    ChipLogProgress(DeviceLayer, "Adding device error=%d", ret);
                    return -1;
                }
                // Handle wrap condition
                if (++mCurrentEndpointId < mFirstDynamicEndpointId)
                {
                    mCurrentEndpointId = mFirstDynamicEndpointId;
                }
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int AppPlatform::RemoveContentApp(ContentApp * app)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] == app)
        {
            EndpointId ep       = emberAfClearDynamicEndpoint(index);
            mContentApps[index] = NULL;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)",
                            app->GetApplicationBasic()->GetApplicationName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return index;
        }
        index++;
    }
    return -1;
}

void AppPlatform::SetupAppPlatform()
{
    ChipLogProgress(DeviceLayer, "AppPlatform::SetupAppPlatform()");

    // Clear out the device database
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        mContentApps[index] = NULL;
        index++;
    }

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    mFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    mCurrentEndpointId = mFirstDynamicEndpointId;

    {
        for (int i = 0; i < emberAfFixedEndpointCount(); i++)
        {
            ChipLogProgress(DeviceLayer, "endpoint index=%d, id=%d", i,
                            static_cast<chip::EndpointId>(static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(i)))));
        }
    }

    if (mCurrentEndpointId < emberAfFixedEndpointCount())
    {
        mCurrentEndpointId = emberAfFixedEndpointCount();
    }

    ChipLogProgress(DeviceLayer, "emberAfFixedEndpointCount()=%d mCurrentEndpointId=%d", emberAfFixedEndpointCount(),
                    mCurrentEndpointId);

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    // emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
}

void AppPlatform::UnloadContentAppByVendorId(uint16_t vendorId)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ContentApp * app = mContentApps[index];
        if (app != NULL && app->GetApplicationBasic()->GetVendorId() == vendorId)
        {
            EndpointId ep       = emberAfClearDynamicEndpoint(index);
            mContentApps[index] = NULL;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)",
                            app->GetApplicationBasic()->GetApplicationName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return;
        }
        index++;
    }
    return;
}

ContentApp * AppPlatform::GetLoadContentAppByVendorId(uint16_t vendorId)
{
    ChipLogProgress(DeviceLayer, "GetLoadContentAppByVendorId() - vendorId %d ", vendorId);
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ContentApp * app = mContentApps[index];
        if (app != NULL && app->GetApplicationBasic()->GetVendorId() == vendorId)
        {
            return app;
        }
        index++;
    }
    if (mContentAppFactory != NULL)
    {
        return mContentAppFactory->LoadContentAppByVendorId(vendorId);
    }
    return NULL;
}

ContentApp * AppPlatform::GetContentAppByEndpointId(chip::EndpointId id)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ContentApp * app = mContentApps[index];
        if (app != NULL && app->GetEndpointId() == id)
        {
            return app;
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "GetContentAppByEndpointId() - endpoint %d not found ", id);
    return NULL;
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
