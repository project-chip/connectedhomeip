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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

#include <lib/support/Span.h>

#include "ota-provider-delegate.h"
#include "ota-provider.h"

using namespace chip;
using chip::app::clusters::OTAProviderDelegate;

namespace {
constexpr uint8_t kLocationParamLength   = 2;   // The expected length of the Location parameter in QueryImage
constexpr size_t kMaxMetadataLen         = 512; // The maximum length of Metadata in any OTA Provider command
constexpr size_t kUpdateTokenParamLength = 32;  // The expected length of the Update Token parameter used in multiple commands

OTAProviderDelegate * gDelegateTable[EMBER_AF_OTA_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

OTAProviderDelegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_OTA_PROVIDER_CLUSTER_ID);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool SendStatusIfDelegateNull(EndpointId endpoint)
{
    if (GetDelegate(endpoint) == nullptr)
    {
        ChipLogError(Zcl, "No OTAProviderDelegate set for ep:%" PRIu16, endpoint);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_COMMAND);
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

bool emberAfOtaSoftwareUpdateProviderClusterApplyUpdateRequestCallback(EndpointId endpoint, app::CommandHandler * commandObj,
                                                                       ByteSpan updateToken, uint32_t newVersion)
{
    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    ChipLogDetail(Zcl, "OTA Provider received ApplyUpdateRequest");

    if (SendStatusIfDelegateNull(endpoint))
    {
        return true;
    }

    if (updateToken.size() != kUpdateTokenParamLength)
    {
        ChipLogError(Zcl, "expected size %zu for UpdateToken, got %zu", kUpdateTokenParamLength, updateToken.size());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    status = delegate->HandleApplyUpdateRequest(commandObj, updateToken, newVersion);
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
 * @param currentVersion The current SoftwareVersion value. Should match the SoftwarVersion attribute in the
 *                       OTA Requestor's Basic Information Cluster.
 */

bool emberAfOtaSoftwareUpdateProviderClusterNotifyUpdateAppliedCallback(EndpointId endpoint, app::CommandHandler * commandObj,
                                                                        ByteSpan updateToken, uint32_t currentVersion)
{
    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    ChipLogDetail(Zcl, "OTA Provider received NotifyUpdateUpplied");

    if (SendStatusIfDelegateNull(endpoint))
    {
        return true;
    }

    if (updateToken.size() != kUpdateTokenParamLength)
    {
        ChipLogError(Zcl, "expected size %zu for UpdateToken, got %zu", kUpdateTokenParamLength, updateToken.size());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    status = delegate->HandleNotifyUpdateApplied(updateToken, currentVersion);
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
 * @param currentVersion The current version running on the OTA Requestor. Should match the SoftwareVersion attribute of the
 * Client's Basic Information Cluster.
 * @param protocolsSupported A list of OTADownloadProtocol enum values indicating download protocols supported by the OTA Requestor
 *                           (max length 8 entries).
 * @param location Optional, 2 chars. If present, it should match the Location value in the Client's Basic Information Cluster.
 * @param clientCanConsent Optional. May be set by an OTA Requestor which is capable of obtaining user consent for OTA application.
 * @param metadataForProvider Optional, max 512 octets. A TLV-encoded Vendor-specific payload.
 */

bool emberAfOtaSoftwareUpdateProviderClusterQueryImageCallback(EndpointId endpoint, app::CommandHandler * commandObj,
                                                               uint16_t vendorId, uint16_t productId, uint16_t imageType,
                                                               uint16_t hardwareVersion, uint32_t currentVersion,
                                                               /* TODO(#8605): change this to list */ uint8_t protocolsSupported,
                                                               uint8_t * location, bool clientCanConsent,
                                                               ByteSpan metadataForProvider)
{
    EmberAfStatus status           = EMBER_ZCL_STATUS_SUCCESS;
    OTAProviderDelegate * delegate = GetDelegate(endpoint);

    if (SendStatusIfDelegateNull(endpoint))
    {
        return true;
    };

    ChipLogDetail(Zcl, "OTA Provider received QueryImage");

    // TODO: (#7112) change location size checking once CHAR_STRING is supported
    const uint8_t locationLen = emberAfStringLength(location);
    if (locationLen != kLocationParamLength)
    {
        ChipLogError(Zcl, "expected location length %" PRIu8 ", got %" PRIu8, locationLen, kLocationParamLength);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }
    else if (metadataForProvider.size() > kMaxMetadataLen)
    {
        ChipLogError(Zcl, "metadata size %zu exceeds max %zu", metadataForProvider.size(), kMaxMetadataLen);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    ByteSpan locationSpan(location, locationLen);

    status = delegate->HandleQueryImage(commandObj, vendorId, productId, imageType, hardwareVersion, currentVersion,
                                        protocolsSupported, locationSpan, clientCanConsent, metadataForProvider);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

namespace chip {
namespace app {
namespace clusters {

void OTAProvider::SetDelegate(EndpointId endpoint, OTAProviderDelegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_OTA_PROVIDER_CLUSTER_ID);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace clusters
} // namespace app
} // namespace chip
