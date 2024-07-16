/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/Constants.h>

#include <lib/support/Span.h>

#include "ota-provider-delegate.h"
#include "ota-provider.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using chip::app::Clusters::OTAProviderDelegate;
using Protocols::InteractionModel::Status;

// MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT is only defined if the
// cluster is actually enabled in the ZAP config.  To allow operation in setups
// where that's not the case (and custom dispatch is used), define it here as
// needed.
#ifndef MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT
#define MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT 0
#endif // MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT

static constexpr size_t kOtaProviderDelegateTableSize =
    MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kOtaProviderDelegateTableSize <= kEmberInvalidEndpointIndex, "OtaProvider Delegate table size error");

namespace {
constexpr size_t kLocationLen          = 2;   // The expected length of the location parameter in QueryImage
constexpr size_t kMaxMetadataLen       = 512; // The maximum length of Metadata in any OTA Provider command
constexpr size_t kUpdateTokenMaxLength = 32;  // The expected length of the Update Token parameter used in multiple commands
constexpr size_t kUpdateTokenMinLength = 8;   // The expected length of the Update Token parameter used in multiple commands

OTAProviderDelegate * gDelegateTable[kOtaProviderDelegateTableSize] = { nullptr };

OTAProviderDelegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, OtaSoftwareUpdateProvider::Id,
                                                       MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kOtaProviderDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool SendStatusIfDelegateNull(app::CommandHandler * commandObj, const app::ConcreteCommandPath & path)
{
    if (GetDelegate(path.mEndpointId) == nullptr)
    {
        ChipLogError(Zcl, "No OTAProviderDelegate set for ep:%u", path.mEndpointId);
        commandObj->AddStatus(path, Status::UnsupportedCommand);
        return true;
    }
    return false;
}
} // namespace

/**
 * @brief OTA Software Update Provider Cluster ApplyUpdateRequest Command callback
 *
 * @note It is the application's reponsibility to send the ApplyUpdateRequestResponse command after this is handled.
 */
bool emberAfOtaSoftwareUpdateProviderClusterApplyUpdateRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::ApplyUpdateRequest::DecodableType & commandData)
{
    auto & updateToken             = commandData.updateToken;
    EndpointId endpoint            = commandPath.mEndpointId;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    ChipLogProgress(Zcl, "OTA Provider received ApplyUpdateRequest");
    ChipLogDetail(Zcl, "  Update Token: %u", static_cast<unsigned int>(commandData.updateToken.size()));
    ChipLogDetail(Zcl, "  New Version: %" PRIu32, commandData.newVersion);

    if (SendStatusIfDelegateNull(commandObj, commandPath))
    {
        return true;
    }

    if (updateToken.size() > kUpdateTokenMaxLength || updateToken.size() < kUpdateTokenMinLength)
    {
        ChipLogError(Zcl, "expected size %u for UpdateToken, got %u", static_cast<unsigned int>(kUpdateTokenMaxLength),
                     static_cast<unsigned int>(updateToken.size()));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    delegate->HandleApplyUpdateRequest(commandObj, commandPath, commandData);

    return true;
}

/**
 * @brief OTA Software Update Provider Cluster NotifyUpdateApplied Command callback
 */
bool emberAfOtaSoftwareUpdateProviderClusterNotifyUpdateAppliedCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::NotifyUpdateApplied::DecodableType & commandData)
{
    auto & updateToken             = commandData.updateToken;
    EndpointId endpoint            = commandPath.mEndpointId;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    ChipLogProgress(Zcl, "OTA Provider received NotifyUpdateApplied");
    ChipLogDetail(Zcl, "  Update Token: %u", static_cast<unsigned int>(commandData.updateToken.size()));
    ChipLogDetail(Zcl, "  Software Version: %" PRIu32, commandData.softwareVersion);

    if (SendStatusIfDelegateNull(commandObj, commandPath))
    {
        return true;
    }

    if (updateToken.size() > kUpdateTokenMaxLength || updateToken.size() < kUpdateTokenMinLength)
    {
        ChipLogError(Zcl, "expected size %u for UpdateToken, got %u", static_cast<unsigned int>(kUpdateTokenMaxLength),
                     static_cast<unsigned int>(updateToken.size()));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    delegate->HandleNotifyUpdateApplied(commandObj, commandPath, commandData);

    return true;
}

/**
 * @brief OTA Software Update Provider Cluster QueryImage Command callback
 */
bool emberAfOtaSoftwareUpdateProviderClusterQueryImageCallback(app::CommandHandler * commandObj,
                                                               const app::ConcreteCommandPath & commandPath,
                                                               const Commands::QueryImage::DecodableType & commandData)
{
    auto & vendorId            = commandData.vendorID;
    auto & productId           = commandData.productID;
    auto & hardwareVersion     = commandData.hardwareVersion;
    auto & softwareVersion     = commandData.softwareVersion;
    auto & protocolsSupported  = commandData.protocolsSupported;
    auto & location            = commandData.location;
    auto & requestorCanConsent = commandData.requestorCanConsent;
    auto & metadataForProvider = commandData.metadataForProvider;

    (void) vendorId;
    (void) productId;
    (void) softwareVersion;

    EndpointId endpoint            = commandPath.mEndpointId;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    if (SendStatusIfDelegateNull(commandObj, commandPath))
    {
        return true;
    };

    ChipLogProgress(Zcl, "OTA Provider received QueryImage");
    ChipLogDetail(Zcl, "  VendorID: 0x%x", vendorId);
    ChipLogDetail(Zcl, "  ProductID: %u", productId);
    ChipLogDetail(Zcl, "  SoftwareVersion: %" PRIu32, softwareVersion);
    ChipLogDetail(Zcl, "  ProtocolsSupported: [");
    auto protocolIter = protocolsSupported.begin();
    while (protocolIter.Next())
    {
        ChipLogDetail(Zcl, "    %u", to_underlying(protocolIter.GetValue()));
    }
    ChipLogDetail(Zcl, "  ]");
    if (hardwareVersion.HasValue())
    {
        ChipLogDetail(Zcl, "  HardwareVersion: %u", hardwareVersion.Value());
    }
    if (location.HasValue())
    {
        ChipLogDetail(Zcl, "  Location: %.*s", static_cast<int>(location.Value().size()), location.Value().data());
    }
    if (requestorCanConsent.HasValue())
    {
        ChipLogDetail(Zcl, "  RequestorCanConsent: %u", requestorCanConsent.Value());
    }
    if (metadataForProvider.HasValue())
    {
        ChipLogDetail(Zcl, "  MetadataForProvider: %u", static_cast<unsigned int>(metadataForProvider.Value().size()));
    }

    if (location.HasValue() && location.Value().size() != kLocationLen)
    {
        ChipLogError(Zcl, "location param length %u != expected length %u", static_cast<unsigned int>(location.Value().size()),
                     static_cast<unsigned int>(kLocationLen));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    if (metadataForProvider.HasValue() && metadataForProvider.Value().size() > kMaxMetadataLen)
    {
        ChipLogError(Zcl, "metadata size %u exceeds max %u", static_cast<unsigned int>(metadataForProvider.Value().size()),
                     static_cast<unsigned int>(kMaxMetadataLen));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    delegate->HandleQueryImage(commandObj, commandPath, commandData);

    return true;
}

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(EndpointId endpoint, OTAProviderDelegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, OtaSoftwareUpdateProvider::Id,
                                                       MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kOtaProviderDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace OTAProvider
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterOtaSoftwareUpdateProviderPluginServerInitCallback() {}
