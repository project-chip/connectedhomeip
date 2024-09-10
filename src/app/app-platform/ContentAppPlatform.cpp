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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <app/util/endpoint-config-api.h>
#include <controller/CHIPCluster.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/StatusCode.h>

#include <string>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;
using namespace chip::Access;
using ApplicationStatusEnum   = app::Clusters::ApplicationBasic::ApplicationStatusEnum;
using GetSetupPINResponseType = app::Clusters::AccountLogin::Commands::GetSetupPINResponse::Type;
using chip::Protocols::InteractionModel::Status;

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                            const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                            uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogDetail(DeviceLayer, "emberAfExternalAttributeReadCallback endpoint %d ", endpointIndex);

    Status ret = Status::Failure;

    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ret = app->HandleReadAttribute(clusterId, attributeMetadata->attributeId, buffer, maxReadLength);
    }
    else
    {
        ret = AppPlatformExternalAttributeReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
    }

    return ret;
}

Status emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                             const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogDetail(DeviceLayer, "emberAfExternalAttributeWriteCallback endpoint %d ", endpointIndex);

    Status ret = Status::Failure;

    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ret = app->HandleWriteAttribute(clusterId, attributeMetadata->attributeId, buffer);
    }
    else
    {
        ret = AppPlatformExternalAttributeWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
    }

    return ret;
}

namespace chip {
namespace AppPlatform {

Status __attribute__((weak)) AppPlatformExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                                      const EmberAfAttributeMetadata * attributeMetadata,
                                                                      uint8_t * buffer, uint16_t maxReadLength)
{
    return (Status::Failure);
}

Status __attribute__((weak))
AppPlatformExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                          const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    return (Status::Failure);
}

EndpointId ContentAppPlatform::AddContentApp(ContentApp * app, EmberAfEndpointType * ep,
                                             const Span<DataVersion> & dataVersionStorage,
                                             const Span<const EmberAfDeviceType> & deviceTypeList)
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
            // already added, return endpointId of already added endpoint.
            // desired endpointId does not have any impact
            return app->GetEndpointId();
        }
        index++;
    }

    index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] != nullptr)
        {
            index++;
            continue;
        }
        CHIP_ERROR err;
        EndpointId initEndpointId = mCurrentEndpointId;

        do
        {
            err = emberAfSetDynamicEndpoint(index, mCurrentEndpointId, ep, dataVersionStorage, deviceTypeList);
            if (err == CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "Added ContentApp %s to dynamic endpoint %d (index=%d)", vendorApp.applicationId,
                                mCurrentEndpointId, index);
                app->SetEndpointId(mCurrentEndpointId);
                mContentApps[index] = app;
                IncrementCurrentEndpointID();
                return app->GetEndpointId();
            }
            else if (err != CHIP_ERROR_ENDPOINT_EXISTS)
            {
                ChipLogError(DeviceLayer, "Adding ContentApp error=%" CHIP_ERROR_FORMAT, err.Format());
                return kNoCurrentEndpointId;
            }
            IncrementCurrentEndpointID();
        } while (initEndpointId != mCurrentEndpointId);
        ChipLogError(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
        return kNoCurrentEndpointId;
    }
    ChipLogError(DeviceLayer, "Failed to add dynamic endpoint: max endpoint count reached!");
    return kNoCurrentEndpointId;
}

EndpointId ContentAppPlatform::AddContentApp(ContentApp * app, EmberAfEndpointType * ep,
                                             const Span<DataVersion> & dataVersionStorage,
                                             const Span<const EmberAfDeviceType> & deviceTypeList, EndpointId desiredEndpointId)
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
            // already added, return endpointId of already added endpoint.
            // desired endpointId does not have any impact
            return app->GetEndpointId();
        }
        index++;
    }

    if (desiredEndpointId < FIXED_ENDPOINT_COUNT ||
        emberAfGetDynamicIndexFromEndpoint(desiredEndpointId) != kEmberInvalidEndpointIndex)
    {
        // invalid desiredEndpointId
        ChipLogError(DeviceLayer, "Failed to add dynamic endpoint: desired endpointID is invalid!");
        return kNoCurrentEndpointId;
    }

    index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] != nullptr)
        {
            index++;
            continue;
        }
        CHIP_ERROR err = emberAfSetDynamicEndpoint(index, desiredEndpointId, ep, dataVersionStorage, deviceTypeList);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Adding ContentApp error : %" CHIP_ERROR_FORMAT, err.Format());
            return kNoCurrentEndpointId;
        }
        ChipLogProgress(DeviceLayer, "Added ContentApp %s to dynamic endpoint %d (index=%d)", vendorApp.applicationId,
                        desiredEndpointId, index);
        app->SetEndpointId(desiredEndpointId);
        mContentApps[index] = app;
        return app->GetEndpointId();
    }
    ChipLogError(DeviceLayer, "Failed to add dynamic endpoint: max endpoint count reached!");
    return kNoCurrentEndpointId;
}

void ContentAppPlatform::IncrementCurrentEndpointID()
{
    // Handle wrap condition
    if (++mCurrentEndpointId < mFirstDynamicEndpointId)
    {
        mCurrentEndpointId = mFirstDynamicEndpointId;
    }
}

EndpointId ContentAppPlatform::RemoveContentApp(ContentApp * app)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mContentApps[index] == app)
        {
            EndpointId curEndpoint = app->GetEndpointId();
            // Silence complaints about unused ep when progress logging
            // disabled.
            /*[[maybe_unused]]*/ EndpointId ep = emberAfClearDynamicEndpoint(index);
            mContentApps[index]                = nullptr;
            ChipLogProgress(DeviceLayer, "Removed device %d from dynamic endpoint %d (index=%d)",
                            app->GetApplicationBasicDelegate()->HandleGetVendorId(), ep, index);
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
    ChipLogDetail(DeviceLayer, "AppPlatform::SetupAppPlatform()");

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

    ChipLogDetail(DeviceLayer, "emberAfFixedEndpointCount()=%d mCurrentEndpointId=%d", emberAfFixedEndpointCount(),
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
        ChipLogError(DeviceLayer, "GetLoadContentAppByVendorId() - failed to find an app for vendorId %d, productId %d", vendorId,
                     productId);
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
        ChipLogError(DeviceLayer, "GetLoadContentApp() - failed to find an app for catalog vendorId %d, appId %s",
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
        ChipLogError(DeviceLayer, "GetContentApp() - failed to find an app for catalog vendorId %d, appId %s",
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

// create a string key from vendorId and productId
std::string createKey(uint16_t vendorId, uint16_t productId)
{
    return std::to_string(vendorId) + ":" + std::to_string(productId);
}

void ContentAppPlatform::StoreNodeIdForContentApp(uint16_t vendorId, uint16_t productId, NodeId nodeId)
{
    std::string key = createKey(vendorId, productId);

    ChipLogProgress(DeviceLayer, "Stored node id: " ChipLogFormatX64 " for key: %s", ChipLogValueX64(nodeId), key.c_str());

    mConnectedContentAppNodeIds[key].insert(nodeId);
}

std::set<NodeId> ContentAppPlatform::GetNodeIdsForContentApp(uint16_t vendorId, uint16_t productId)
{
    std::string key = createKey(vendorId, productId);

    ChipLogProgress(DeviceLayer, "Retrieving node id for key: %s", key.c_str());

    auto it = mConnectedContentAppNodeIds.find(key);
    if (it != mConnectedContentAppNodeIds.end())
    {
        ChipLogProgress(DeviceLayer, "Found node id");
        return it->second;
    }

    ChipLogProgress(DeviceLayer, "Didn't find node id");
    // If key not found, return an empty set
    return {};
}

std::set<NodeId> ContentAppPlatform::GetNodeIdsForAllowedVendorId(uint16_t vendorId)
{
    std::set<NodeId> result;
    std::string vendorPrefix = std::to_string(vendorId) + ":";

    for (const auto & pair : mConnectedContentAppNodeIds)
    {
        const std::string & key = pair.first;
        if (key.find(vendorPrefix) == 0)
        { // Check if the key starts with the vendor prefix
            const std::set<NodeId> & nodeIds = pair.second;
            result.insert(nodeIds.begin(), nodeIds.end());
        }
    }

    return result;
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
    if (!HasCurrentApp())
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

bool ContentAppPlatform::HasTargetContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId,
                                             chip::Protocols::UserDirectedCommissioning::TargetAppInfo & info, uint32_t & passcode)
{
    // TODO: perform more complex search for matching apps
    ContentApp * app = LoadContentAppByClient(info.vendorId, info.productId);
    if (app == nullptr)
    {
        ChipLogProgress(DeviceLayer, "no app found for vendor id=%d \r\n", info.vendorId);
        return false;
    }

    if (app->GetApplicationBasicDelegate() == nullptr)
    {
        ChipLogProgress(DeviceLayer, "no ApplicationBasic cluster for app with vendor id=%d \r\n", info.vendorId);
        return false;
    }

    // first check if the vendor id matches the client
    bool allow = app->GetApplicationBasicDelegate()->HandleGetVendorId() == vendorId;
    if (!allow)
    {
        // if no match, then check allowed vendor list
        for (const auto & allowedVendor : app->GetApplicationBasicDelegate()->GetAllowedVendorList())
        {
            if (allowedVendor == vendorId)
            {
                allow = true;
                break;
            }
        }
        if (!allow)
        {
            ChipLogProgress(
                DeviceLayer,
                "no permission given by ApplicationBasic cluster on app with vendor id=%d to client with vendor id=%d\r\n",
                info.vendorId, vendorId);
            return false;
        }
    }

    if (app->GetAccountLoginDelegate() == nullptr)
    {
        ChipLogProgress(DeviceLayer, "no AccountLogin cluster for app with vendor id=%d \r\n", info.vendorId);
        return true;
    }

    if (!app->HasSupportedCluster(AccountLogin::Id))
    {
        ChipLogProgress(DeviceLayer, "AccountLogin cluster not supported for app with vendor id=%d \r\n", vendorId);
        return true;
    }

    static const size_t kSetupPasscodeSize = 12;
    char mSetupPasscode[kSetupPasscodeSize];

    app->GetAccountLoginDelegate()->GetSetupPin(mSetupPasscode, kSetupPasscodeSize, rotatingId);
    std::string passcodeString(mSetupPasscode);

    char * eptr;
    passcode = (uint32_t) strtol(passcodeString.c_str(), &eptr, 10);
    return true;
}

uint32_t ContentAppPlatform::GetPasscodeFromContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId)
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

    if (!app->HasSupportedCluster(AccountLogin::Id))
    {
        ChipLogProgress(DeviceLayer, "AccountLogin cluster not supported for app with vendor id=%d \r\n", vendorId);
        return 0;
    }

    static const size_t kSetupPasscodeSize = 12;
    char mSetupPasscode[kSetupPasscodeSize];

    app->GetAccountLoginDelegate()->GetSetupPin(mSetupPasscode, kSetupPasscodeSize, rotatingId);
    std::string passcodeString(mSetupPasscode);

    char * eptr;
    return (uint32_t) strtol(passcodeString.c_str(), &eptr, 10);
}

// Returns ACL entry with match subject or CHIP_ERROR_NOT_FOUND if no match is found
CHIP_ERROR ContentAppPlatform::GetACLEntryIndex(size_t * foundIndex, FabricIndex fabricIndex, NodeId subjectNodeId)
{
    size_t index = 0;
    if (Access::GetAccessControl().GetEntryCount(fabricIndex, index) == CHIP_NO_ERROR)
    {
        while (index)
        {
            Access::AccessControl::Entry entry;
            CHIP_ERROR err = Access::GetAccessControl().ReadEntry(fabricIndex, --index, entry);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogDetail(DeviceLayer, "ContentAppPlatform::GetACLEntryIndex error reading entry %d err %s",
                              static_cast<int>(index), ErrorStr(err));
            }
            else
            {
                size_t count;
                err = entry.GetSubjectCount(count);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogDetail(DeviceLayer,
                                  "ContentAppPlatform::GetACLEntryIndex error reading subject count for entry %d err %s",
                                  static_cast<int>(index), ErrorStr(err));
                    continue;
                }
                if (count)
                {
                    ChipLogDetail(DeviceLayer, "subjects: %u", static_cast<unsigned>(count));
                    for (size_t i = 0; i < count; ++i)
                    {
                        NodeId subject;
                        err = entry.GetSubject(i, subject);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogDetail(DeviceLayer,
                                          "ContentAppPlatform::GetACLEntryIndex error reading subject %i for entry %d err %s",
                                          static_cast<int>(i), static_cast<int>(index), ErrorStr(err));
                            continue;
                        }
                        if (subject == subjectNodeId)
                        {
                            ChipLogDetail(DeviceLayer, "ContentAppPlatform::GetACLEntryIndex found matching subject at index %d",
                                          static_cast<int>(index));
                            *foundIndex = index;
                            return CHIP_NO_ERROR;
                        }
                    }
                }
            }
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

// Add ACLs on this device for the given client,
// and create bindings on the given client so that it knows what it has access to.
CHIP_ERROR ContentAppPlatform::ManageClientAccess(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle,
                                                  uint16_t targetVendorId, uint16_t targetProductId, NodeId localNodeId,
                                                  CharSpan rotatingId, uint32_t passcode,
                                                  std::vector<Binding::Structs::TargetStruct::Type> bindings,
                                                  Controller::WriteResponseSuccessCallback successCb,
                                                  Controller::WriteResponseFailureCallback failureCb)
{
    VerifyOrReturnError(successCb != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(failureCb != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Access::Privilege vendorPrivilege = mContentAppFactory->GetVendorPrivilege(targetVendorId);

    NodeId subjectNodeId    = sessionHandle->GetPeer().GetNodeId();
    FabricIndex fabricIndex = sessionHandle->GetFabricIndex();

    // first, delete existing ACLs for this nodeId
    {
        size_t index;
        CHIP_ERROR err;
        while (CHIP_NO_ERROR == (err = GetACLEntryIndex(&index, fabricIndex, subjectNodeId)))
        {
            err = Access::GetAccessControl().DeleteEntry(nullptr, fabricIndex, index);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogDetail(DeviceLayer, "ContentAppPlatform::ManageClientAccess error entry %d err %s", static_cast<int>(index),
                              ErrorStr(err));
            }
        }
    }

    Access::AccessControl::Entry entry;
    ReturnErrorOnFailure(GetAccessControl().PrepareEntry(entry));
    ReturnErrorOnFailure(entry.SetAuthMode(Access::AuthMode::kCase));
    entry.SetFabricIndex(fabricIndex);
    ReturnErrorOnFailure(entry.SetPrivilege(vendorPrivilege));
    ReturnErrorOnFailure(entry.AddSubject(nullptr, subjectNodeId));

    /**
     * Here we are creating a single ACL entry containing:
     * a) selection of clusters on video player endpoint (8 targets)
     * b) speaker endpoint (1 target)
     * c) selection of content app endpoints (0 to many)
     * d) single subject which is the casting app
     * This organization was selected to make it easy to remove access (single ACL removal)
     *
     * We could have organized things differently, for example,
     * - a single ACL for (a) and (b) which is shared by many subjects
     * - a single ACL entry per subject for (c)
     *
     * We are also creating the following set of bindings on the remote device:
     * a) Video Player endpoint
     * b) Speaker endpoint
     * c) selection of content app endpoints (0 to many)
     * The purpose of the bindings is to inform the client of its access to
     * nodeId and endpoints on the app platform.
     */

    ChipLogProgress(Controller, "Create video player endpoint ACL and binding");
    {
        bool hasClusterAccess = false;
        if (vendorPrivilege == Access::Privilege::kAdminister)
        {
            ChipLogProgress(Controller, "ContentAppPlatform::ManageClientAccess Admin privilege granted");
            // a vendor with admin privilege gets access to all clusters on ep1
            Access::AccessControl::Entry::Target target = { .flags    = Access::AccessControl::Entry::Target::kEndpoint,
                                                            .endpoint = kLocalVideoPlayerEndpointId };
            ReturnErrorOnFailure(entry.AddTarget(nullptr, target));
            hasClusterAccess = true;
        }
        else
        {
            ChipLogProgress(Controller, "ContentAppPlatform::ManageClientAccess non-Admin privilege granted");
            // a vendor with non-admin privilege gets access to select clusters on ep1
            std::list<ClusterId> allowedClusterList = mContentAppFactory->GetAllowedClusterListForStaticEndpoint(
                kLocalVideoPlayerEndpointId, targetVendorId, targetProductId);

            for (const auto & clusterId : allowedClusterList)
            {
                Access::AccessControl::Entry::Target target = { .flags = Access::AccessControl::Entry::Target::kCluster |
                                                                    Access::AccessControl::Entry::Target::kEndpoint,
                                                                .cluster  = clusterId,
                                                                .endpoint = kLocalVideoPlayerEndpointId };
                ReturnErrorOnFailure(entry.AddTarget(nullptr, target));
                hasClusterAccess = true;
            }
        }

        if (hasClusterAccess)
        {
            ChipLogProgress(Controller, "ContentAppPlatform::ManageClientAccess adding a binding on ep1");
            bindings.push_back(Binding::Structs::TargetStruct::Type{
                .node        = MakeOptional(localNodeId),
                .group       = NullOptional,
                .endpoint    = MakeOptional(kLocalVideoPlayerEndpointId),
                .cluster     = NullOptional,
                .fabricIndex = kUndefinedFabricIndex,
            });
        }
    }

    ChipLogProgress(Controller, "Create speaker endpoint ACL and binding");
    {
        Access::AccessControl::Entry::Target target = { .flags    = Access::AccessControl::Entry::Target::kEndpoint,
                                                        .endpoint = kLocalSpeakerEndpointId };
        ReturnErrorOnFailure(entry.AddTarget(nullptr, target));

        bindings.push_back(Binding::Structs::TargetStruct::Type{
            .node        = MakeOptional(localNodeId),
            .group       = NullOptional,
            .endpoint    = MakeOptional(kLocalSpeakerEndpointId),
            .cluster     = NullOptional,
            .fabricIndex = kUndefinedFabricIndex,
        });
    }

    ChipLogProgress(Controller, "Create content app endpoints ACL and binding");
    {
        uint8_t index = 0;
        while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
        {
            ContentApp * app = mContentApps[index++];
            if (app == nullptr)
            {
                continue;
            }

            bool accessAllowed = false;
            for (const auto & allowedVendor : app->GetApplicationBasicDelegate()->GetAllowedVendorList())
            {
                if (allowedVendor == targetVendorId)
                {
                    Access::AccessControl::Entry::Target target = { .flags    = Access::AccessControl::Entry::Target::kEndpoint,
                                                                    .endpoint = app->GetEndpointId() };
                    ReturnErrorOnFailure(entry.AddTarget(nullptr, target));

                    bindings.push_back(Binding::Structs::TargetStruct::Type{
                        .node        = MakeOptional(localNodeId),
                        .group       = NullOptional,
                        .endpoint    = MakeOptional(app->GetEndpointId()),
                        .cluster     = NullOptional,
                        .fabricIndex = kUndefinedFabricIndex,
                    });

                    accessAllowed = true;
                }
            }
            if (accessAllowed)
            {
                // notify content app about this nodeId
                bool isNodeAdded = app->AddClientNode(subjectNodeId);

                if (isNodeAdded && rotatingId.size() != 0)
                {
                    // handle login
                    auto setupPIN             = std::to_string(passcode);
                    auto accountLoginDelegate = app->GetAccountLoginDelegate();
                    if (accountLoginDelegate != nullptr)
                    {
                        bool condition = accountLoginDelegate->HandleLogin(rotatingId, { setupPIN.data(), setupPIN.size() },
                                                                           MakeOptional(subjectNodeId));
                        ChipLogProgress(Controller, "AccountLogin::Login command sent and returned: %s",
                                        condition ? "success" : "failure");
                    }
                    else
                    {
                        ChipLogError(Controller, "AccountLoginDelegate not found for app");
                    }
                }
            }
        }
    }

    // TODO: add a subject description on the ACL
    ReturnErrorOnFailure(GetAccessControl().CreateEntry(nullptr, sessionHandle->GetFabricIndex(), nullptr, entry));

    ChipLogProgress(Controller, "Attempting to update Binding list");
    BindingListType bindingList(bindings.data(), bindings.size());

    Controller::ClusterBase cluster(exchangeMgr, sessionHandle, kTargetBindingClusterEndpointId);

    ReturnErrorOnFailure(
        cluster.WriteAttribute<Binding::Attributes::Binding::TypeInfo>(bindingList, nullptr, successCb, failureCb));

    ChipLogProgress(Controller, "Completed Bindings and ACLs");

    return CHIP_NO_ERROR;
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
