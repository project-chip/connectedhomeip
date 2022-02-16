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
#include <app/util/af.h>
#include <platform/CHIPDeviceConfig.h>

#include <lib/support/Span.h>

#include "ota-provider-delegate.h"
#include "ota-provider.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using chip::app::Clusters::OTAProviderDelegate;

static constexpr size_t kOtaProviderDelegateTableSize =
    EMBER_AF_OTA_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

namespace {
constexpr size_t kLocationLen          = 2;   // The expected length of the location parameter in QueryImage
constexpr size_t kMaxMetadataLen       = 512; // The maximum length of Metadata in any OTA Provider command
constexpr size_t kUpdateTokenMaxLength = 32;  // The expected length of the Update Token parameter used in multiple commands
constexpr size_t kUpdateTokenMinLength = 8;   // The expected length of the Update Token parameter used in multiple commands

OTAProviderDelegate * gDelegateTable[kOtaProviderDelegateTableSize] = { nullptr };

OTAProviderDelegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, OtaSoftwareUpdateProvider::Id);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool SendStatusIfDelegateNull(EndpointId endpoint)
{
    if (GetDelegate(endpoint) == nullptr)
    {
        ChipLogError(Zcl, "No OTAProviderDelegate set for ep:%" PRIu16, endpoint);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);
        return true;
    }
    return false;
}
} // namespace

/**
 * @brief OTA Software Update Provider Cluster ApplyUpdateRequest Command callback
 *
 * @note It is the application's reponsibility to send the ApplyUpdateRequestResponse command after this is handled.
 *
 * @param updateToken Identifier for the Software Image to be applied. Should be 32 octets long.
 * @param newVersion The SoftwareVersion value of the new Software Image that the client is ready to apply.
 */

bool emberAfOtaSoftwareUpdateProviderClusterApplyUpdateRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::ApplyUpdateRequest::DecodableType & commandData)
{
    auto & updateToken = commandData.updateToken;

    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    ChipLogProgress(Zcl, "OTA Provider received ApplyUpdateRequest");
    ChipLogDetail(Zcl, "  Update Token: %zu", commandData.updateToken.size());
    ChipLogDetail(Zcl, "  New Version: %" PRIu32, commandData.newVersion);

    if (SendStatusIfDelegateNull(endpoint))
    {
        return true;
    }

    if (updateToken.size() > kUpdateTokenMaxLength || updateToken.size() < kUpdateTokenMinLength)
    {
        ChipLogError(Zcl, "expected size %zu for UpdateToken, got %zu", kUpdateTokenMaxLength, updateToken.size());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    status = delegate->HandleApplyUpdateRequest(commandObj, commandPath, commandData);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

/**
 * @brief OTA Software Update Provider Cluster NotifyUpdateApplied Command callback
 *
 *
 * @param updateToken Identifier for the Software Image that was applied. Should be 32 octets long.
 * @param softwareVersion The current SoftwareVersion value. Should match the SoftwarVersion attribute in the
 *                       OTA Requestor's Basic Information Cluster.
 */

bool emberAfOtaSoftwareUpdateProviderClusterNotifyUpdateAppliedCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::NotifyUpdateApplied::DecodableType & commandData)
{
    auto & updateToken = commandData.updateToken;

    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    ChipLogProgress(Zcl, "OTA Provider received NotifyUpdateApplied");
    ChipLogDetail(Zcl, "  Update Token: %zu", commandData.updateToken.size());
    ChipLogDetail(Zcl, "  Software Version: %" PRIu32, commandData.softwareVersion);

    if (SendStatusIfDelegateNull(endpoint))
    {
        return true;
    }

    if (updateToken.size() > kUpdateTokenMaxLength || updateToken.size() < kUpdateTokenMinLength)
    {
        ChipLogError(Zcl, "expected size %zu for UpdateToken, got %zu", kUpdateTokenMaxLength, updateToken.size());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    status = delegate->HandleNotifyUpdateApplied(commandObj, commandPath, commandData);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

/**
 * @brief OTA Software Update Provider Cluster QueryImage Command callback
 *
 * @param vendorId The Vendor ID applying to the OTA Requestor’s Node. Should match the value in the Basic Information Cluster.
 * @param productId The Product ID applying to the OTA Requestor’s Node. Should match the value in the Basic Information Cluster.
 * @param imageType A Vendor-specific numerical value that may help an OTA Provider select the correct payload.
 * @param hardwareVersion The OTA Requestor’s hardware version. Should match the HardwareVersion attribute of the Client's Basic
 *                        Information Cluster.
 * @param softwareVersion The current version running on the OTA Requestor. Should match the SoftwareVersion attribute of the
 * Client's Basic Information Cluster.
 * @param protocolsSupported A list of OTADownloadProtocol enum values indicating download protocols supported by the OTA Requestor
 *                           (max length 8 entries).
 * @param location Optional, 2 chars. If present, it should match the Location value in the Client's Basic Information Cluster.
 * @param clientCanConsent Optional. May be set by an OTA Requestor which is capable of obtaining user consent for OTA application.
 * @param metadataForProvider Optional, max 512 octets. A TLV-encoded Vendor-specific payload.
 */

bool emberAfOtaSoftwareUpdateProviderClusterQueryImageCallback(app::CommandHandler * commandObj,
                                                               const app::ConcreteCommandPath & commandPath,
                                                               const Commands::QueryImage::DecodableType & commandData)
{
    auto & vendorId            = commandData.vendorId;
    auto & productId           = commandData.productId;
    auto & hardwareVersion     = commandData.hardwareVersion;
    auto & softwareVersion     = commandData.softwareVersion;
    auto & protocolsSupported  = commandData.protocolsSupported;
    auto & location            = commandData.location;
    auto & requestorCanConsent = commandData.requestorCanConsent;
    auto & metadataForProvider = commandData.metadataForProvider;

    (void) vendorId;
    (void) productId;
    (void) softwareVersion;

    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    if (SendStatusIfDelegateNull(endpoint))
    {
        return true;
    };

    ChipLogProgress(Zcl, "OTA Provider received QueryImage");
    ChipLogDetail(Zcl, "  VendorID: 0x%" PRIx16, vendorId);
    ChipLogDetail(Zcl, "  ProductID: %" PRIu16, productId);
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
        ChipLogDetail(Zcl, "  HardwareVersion: %" PRIu16, hardwareVersion.Value());
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
        ChipLogDetail(Zcl, "  MetadataForProvider: %zu", metadataForProvider.Value().size());
    }

    if (location.HasValue() && location.Value().size() != kLocationLen)
    {
        ChipLogError(Zcl, "location param length %zu != expected length %zu", location.Value().size(), kLocationLen);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    if (metadataForProvider.HasValue() && metadataForProvider.Value().size() > kMaxMetadataLen)
    {
        ChipLogError(Zcl, "metadata size %zu exceeds max %zu", metadataForProvider.Value().size(), kMaxMetadataLen);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    status = delegate->HandleQueryImage(commandObj, commandPath, commandData);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(EndpointId endpoint, OTAProviderDelegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, OtaSoftwareUpdateProvider::Id);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace OTAProvider
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterOtaSoftwareUpdateProviderPluginServerInitCallback() {}
