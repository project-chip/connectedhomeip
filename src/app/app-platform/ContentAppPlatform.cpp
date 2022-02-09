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
 * @file Contains functions relating to Content App platform of the Video Player.
 */

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>
#include <zap-generated/CHIPClusters.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;
using ApplicationStatusEnum   = app::Clusters::ApplicationBasic::ApplicationStatusEnum;
using GetSetupPINResponseType = app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type;

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeReadCallback endpoint %d ", endpointIndex);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ret = app->HandleReadAttribute(clusterId, attributeMetadata->attributeId, buffer, maxReadLength);
    }

    return ret;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback endpoint %d ", endpointIndex);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ret = app->HandleWriteAttribute(clusterId, attributeMetadata->attributeId, buffer);
    }

    return ret;
}

namespace chip {
namespace AppPlatform {

EndpointId ContentAppPlatform::AddContentApp(ContentApp * app, EmberAfEndpointType * ep, uint16_t deviceType,
                                             const Span<DataVersion> & dataVersionStorage)
{
    CatalogVendorApp vendorApp = app->GetApplicationBasicDelegate()->GetCatalogVendorApp();

    ChipLogProgress(DeviceLayer, "Adding ContentApp with appid %s ", vendorApp.applicationId);
    uint8_t index = 0;
    // check if already loaded
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] == app)
        {
            ChipLogProgress(DeviceLayer, "Already added");
            return app->GetEndpointId();
        }
        index++;
    }

    index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (nullptr == mContentApps[index])
        {
            mContentApps[index] = app;
            EmberAfStatus ret;
            while (1)
            {
                ret = emberAfSetDynamicEndpoint(index, mCurrentEndpointId, ep, deviceType, DEVICE_VERSION_DEFAULT,
                                                dataVersionStorage);
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added ContentApp %s to dynamic endpoint %d (index=%d)", vendorApp.applicationId,
                                    mCurrentEndpointId, index);
                    app->SetEndpointId(mCurrentEndpointId);
                    return app->GetEndpointId();
                }
                else if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    ChipLogProgress(DeviceLayer, "Adding ContentApp error=%d", ret);
                    return kNoCurrentEndpointId;
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
    return kNoCurrentEndpointId;
}

EndpointId ContentAppPlatform::RemoveContentApp(ContentApp * app)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] == app)
        {
            EndpointId curEndpoint = app->GetEndpointId();
            EndpointId ep          = emberAfClearDynamicEndpoint(index);
            mContentApps[index]    = nullptr;
            ChipLogProgress(DeviceLayer, "Removed device %d from dynamic endpoint %d (index=%d)",
                            app->GetApplicationBasicDelegate()->HandleGetVendorId(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            if (curEndpoint == mCurrentAppEndpointId)
            {
                mCurrentAppEndpointId = kNoCurrentEndpointId;
            }
            return curEndpoint;
        }
        index++;
    }
    return kNoCurrentEndpointId;
}

void ContentAppPlatform::SetupAppPlatform()
{
    ChipLogProgress(DeviceLayer, "AppPlatform::SetupAppPlatform()");

    // Clear out the device database
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        mContentApps[index] = nullptr;
        index++;
    }

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    mFirstDynamicEndpointId = static_cast<EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    mCurrentEndpointId = mFirstDynamicEndpointId;

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

ContentApp * ContentAppPlatform::GetContentAppInternal(const CatalogVendorApp & vendorApp)
{
    if (vendorApp.catalogVendorId != mContentAppFactory->GetPlatformCatalogVendorId())
    {
        return nullptr;
    }
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ContentApp * app = mContentApps[index];
        if (app != nullptr && app->GetApplicationBasicDelegate()->GetCatalogVendorApp()->Matches(vendorApp))
        {
            return app;
        }
        index++;
    }
    return nullptr;
}

ContentApp * ContentAppPlatform::LoadContentAppInternal(const CatalogVendorApp & vendorApp)
{
    ContentApp * app = GetContentAppInternal(vendorApp);
    if (app != nullptr)
    {
        return app;
    }
    if (mContentAppFactory != nullptr)
    {
        return mContentAppFactory->LoadContentApp(vendorApp);
    }
    return nullptr;
}

ContentApp * ContentAppPlatform::LoadContentAppByClient(uint16_t vendorId, uint16_t productId)
{
    ChipLogProgress(DeviceLayer, "GetLoadContentAppByVendorId() - vendorId %d, productId %d", vendorId, productId);

    CatalogVendorApp vendorApp;
    CHIP_ERROR err = mContentAppFactory->LookupCatalogVendorApp(vendorId, productId, &vendorApp);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "GetLoadContentAppByVendorId() - failed to find an app for vendorId %d, productId %d",
                        vendorId, productId);
        return nullptr;
    }
    return LoadContentAppInternal(&vendorApp);
}

ContentApp * ContentAppPlatform::LoadContentApp(const CatalogVendorApp & vendorApp)
{
    if (vendorApp.catalogVendorId == mContentAppFactory->GetPlatformCatalogVendorId())
    {
        return LoadContentAppInternal(vendorApp);
    }
    CatalogVendorApp destinationApp;
    CHIP_ERROR err = mContentAppFactory->ConvertToPlatformCatalogVendorApp(vendorApp, &destinationApp);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "GetLoadContentApp() - failed to find an app for catalog vendorId %d, appId %s",
                        vendorApp.catalogVendorId, vendorApp.applicationId);
        return nullptr;
    }
    return LoadContentAppInternal(&destinationApp);
}

ContentApp * ContentAppPlatform::GetContentApp(const CatalogVendorApp & vendorApp)
{
    if (vendorApp.catalogVendorId == mContentAppFactory->GetPlatformCatalogVendorId())
    {
        return GetContentAppInternal(vendorApp);
    }
    CatalogVendorApp destinationApp;
    CHIP_ERROR err = mContentAppFactory->ConvertToPlatformCatalogVendorApp(vendorApp, &destinationApp);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "GetContentApp() - failed to find an app for catalog vendorId %d, appId %s",
                        vendorApp.catalogVendorId, vendorApp.applicationId);
        return nullptr;
    }
    return GetContentAppInternal(&destinationApp);
}

ContentApp * ContentAppPlatform::GetContentApp(EndpointId id)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        ContentApp * app = mContentApps[index];
        if (app != nullptr && app->GetEndpointId() == id)
        {
            return app;
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "GetContentAppByEndpointId() - endpoint %d not found ", id);
    return nullptr;
}

void ContentAppPlatform::SetCurrentApp(ContentApp * app)
{
    if (!HasCurrentApp())
    {
        mCurrentAppEndpointId = app->GetEndpointId();
        app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kActiveVisibleFocus);
        return;
    }

    // if this is the current app, then no action
    if (mCurrentAppEndpointId == app->GetEndpointId())
    {
        ChipLogProgress(DeviceLayer, "AppPlatform::SetCurrentApp already current app");
        app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kActiveVisibleFocus);
        return;
    }
    // if there is another current app, then need to hide it
    ContentApp * previousApp = GetContentApp(mCurrentAppEndpointId);
    if (previousApp == nullptr)
    {
        ChipLogProgress(DeviceLayer, "AppPlatform::SetCurrentApp current app not found");
        mCurrentAppEndpointId = app->GetEndpointId();
        app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kActiveVisibleFocus);
        return;
    }

    ChipLogProgress(DeviceLayer, "AppPlatform::SetCurrentApp has a current app");
    // make sure to mark previousApp as hidden
    previousApp->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kActiveHidden);

    mCurrentAppEndpointId = app->GetEndpointId();
    app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kActiveVisibleFocus);
    return;
} // namespace AppPlatform

bool ContentAppPlatform::IsCurrentApp(ContentApp * app)
{
    if (HasCurrentApp())
    {
        return false;
    }
    if (mCurrentAppEndpointId != app->GetEndpointId())
    {
        return false;
    }
    if (app != GetContentApp(mCurrentAppEndpointId))
    {
        // current app us not there, fix our state and exit
        ChipLogProgress(DeviceLayer, "AppPlatform::IsCurrentApp current app not found");
        mCurrentAppEndpointId = kNoCurrentEndpointId;
        return false;
    }
    return true;
}

void ContentAppPlatform::UnsetIfCurrentApp(ContentApp * app)
{
    if (IsCurrentApp(app))
    {
        ChipLogProgress(DeviceLayer, "UnsetIfCurrentApp setting to no current app");
        mCurrentAppEndpointId = kNoCurrentEndpointId;
        app->GetApplicationBasicDelegate()->SetApplicationStatus(ApplicationStatusEnum::kActiveHidden);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "UnsetIfCurrentApp not current app");
    }
}

uint32_t ContentAppPlatform::GetPincodeFromContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId)
{
    ContentApp * app = LoadContentAppByClient(vendorId, productId);
    if (app == nullptr)
    {
        ChipLogProgress(DeviceLayer, "no app found for vendor id=%d \r\n", vendorId);
        return 0;
    }

    if (app->GetAccountLoginDelegate() == nullptr)
    {
        ChipLogProgress(DeviceLayer, "no AccountLogin cluster for app with vendor id=%d \r\n", vendorId);
        return 0;
    }

    static const size_t kSetupPINSize = 12;
    char mSetupPIN[kSetupPINSize];

    app->GetAccountLoginDelegate()->GetSetupPin(mSetupPIN, kSetupPINSize, rotatingId);
    std::string pinString(mSetupPIN);

    char * eptr;
    return (uint32_t) strtol(pinString.c_str(), &eptr, 10);
}

CHIP_ERROR ContentAppPlatform::CreateBindingWithCallback(OperationalDeviceProxy * device, chip::EndpointId deviceEndpointId,
                                                         chip::NodeId bindingNodeId, chip::GroupId bindingGroupId,
                                                         chip::EndpointId bindingEndpointId, chip::ClusterId bindingClusterId,
                                                         CommandResponseSuccessCallback<app::DataModel::NullObjectType> successCb,
                                                         CommandResponseFailureCallback failureCb)
{
    chip::Controller::BindingCluster cluster;
    cluster.Associate(device, deviceEndpointId);

    Binding::Commands::Bind::Type request;
    request.nodeId     = bindingNodeId;
    request.groupId    = bindingGroupId;
    request.endpointId = bindingEndpointId;
    request.clusterId  = bindingClusterId;
    ReturnErrorOnFailure(cluster.InvokeCommand(request, this, successCb, failureCb));

    ChipLogDetail(Controller, "CreateBindingWithCallback: Sent Bind command request, waiting for response");
    return CHIP_NO_ERROR;
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
