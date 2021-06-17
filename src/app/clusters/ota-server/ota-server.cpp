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

#include <app/Command.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>
#include <gen/callback.h>

#include <support/Span.h>

#include "ota-server-delegate.h"
#include "ota-server.h"

using chip::app::clusters::OTAServerDelegate;

namespace {
constexpr uint8_t kLocationParamLength   = 2;   // The expected length of the Location parameter in QueryImage
constexpr size_t kMaxMetadataLen         = 512; // The maximum length of Metadata in any OTA Server command
constexpr size_t kUpdateTokenParamLength = 32;  // The expected length of the Update Token parameter used in multiple commands

OTAServerDelegate * gDelegateTable[EMBER_AF_OTA_SERVER_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

OTAServerDelegate * GetDelegate(chip::EndpointId endpoint)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_OTA_SERVER_CLUSTER_ID);
    return (ep == 0xFF ? NULL : gDelegateTable[ep]);
}

bool SendStatusIfDelegateNull(chip::EndpointId endpointId)
{
    if (GetDelegate(endpointId) == nullptr)
    {
        ChipLogError(Zcl, "No OTAServerDelegate set for ep:%" PRIu8, endpointId);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_COMMAND);
        return true;
    }
    return false;
}
} // namespace

/**
 * @brief OTA Software Update Server Cluster ApplyUpdateRequest Command callback
 *
 * @note It is the application's reponsibility to send the ApplyUpdateRequestResponse command after this is handled.
 *
 * @param updateToken Identifier for the Software Image to be applied. Should be 32 octets long.
 * @param newVersion The SoftwareVersion value of the new Software Image that the client is ready to apply.
 */

bool emberAfOtaSoftwareUpdateServerClusterApplyUpdateRequestCallback(chip::app::Command * commandObj, chip::ByteSpan updateToken,
                                                                     uint32_t newVersion)
{
    EmberAfStatus status         = EMBER_ZCL_STATUS_SUCCESS;
    chip::EndpointId endpointId  = emberAfCurrentEndpoint();
    OTAServerDelegate * delegate = GetDelegate(endpointId);

    ChipLogDetail(Zcl, "OTA Server received ApplyUpdateRequest");

    if (SendStatusIfDelegateNull(endpointId))
    {
        return true;
    }

    if (updateToken.size() != kUpdateTokenParamLength)
    {
        ChipLogError(Zcl, "expected size %zu for UpdateToken, got %zu", kUpdateTokenParamLength, updateToken.size());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    status = delegate->HandleApplyUpdateRequest(updateToken, newVersion);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

/**
 * @brief OTA Software Update Server Cluster NotifyUpdateApplied Command callback
 *
 *
 * @param updateToken Identifier for the Software Image that was applied. Should be 32 octets long.
 * @param currentVersion The current SoftwareVersion value. Should match the SoftwarVersion attribute in the
 *                       OTA Client's Basic Information Cluster.
 */

bool emberAfOtaSoftwareUpdateServerClusterNotifyUpdateAppliedCallback(chip::app::Command * commandObj, chip::ByteSpan updateToken,
                                                                      uint32_t currentVersion)
{
    EmberAfStatus status         = EMBER_ZCL_STATUS_SUCCESS;
    chip::EndpointId endpointId  = emberAfCurrentEndpoint();
    OTAServerDelegate * delegate = GetDelegate(endpointId);

    ChipLogDetail(Zcl, "OTA Server received NotifyUpdateUpplied");

    if (SendStatusIfDelegateNull(endpointId))
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
 * @brief OTA Software Update Server Cluster QueryImage Command callback
 *
 * @param vendorId The Vendor ID applying to the OTA Client’s Node. Should match the value in the Basic Information Cluster.
 * @param productId The Product ID applying to the OTA Client’s Node. Should match the value in the Basic Information Cluster.
 * @param imageType A Vendor-specific numerical value that may help an OTA Server select the correct payload.
 * @param hardwareVersion The OTA Client’s hardware version. Should match the HardwareVersion attribute of the Client's Basic
 *                        Information Cluster.
 * @param currentVersion The current version running on the OTA Client. Should match the SoftwareVersion attribute of the Client's
 *                       Basic Information Cluster.
 * @param protocolsSupported A list of OTADownloadProtocol enum values indicating download protocols supported by the OTA Client
 *                           (max length 8 entries).
 * @param location Optional, 2 chars. If present, it should match the Location value in the Client's Basic Information Cluster.
 * @param clientCanConsent Optional. May be set by an OTA Client which is capable of obtaining user consent for OTA application.
 * @param metadataForServer Optional, max 512 octets. A TLV-encoded Vendor-specific payload.
 */

bool emberAfOtaSoftwareUpdateServerClusterQueryImageCallback(
    chip::app::Command * commandObj, uint16_t vendorId, uint16_t productId, uint16_t imageType, uint16_t hardwareVersion,
    uint32_t currentVersion,
    /* TYPE WARNING: array array defaults to */ uint8_t * protocolsSupported, uint8_t * location, uint8_t clientCanConsent,
    chip::ByteSpan metadataForServer)
{
    EmberAfStatus status         = EMBER_ZCL_STATUS_SUCCESS;
    chip::EndpointId endpointId  = emberAfCurrentEndpoint();
    OTAServerDelegate * delegate = GetDelegate(endpointId);

    if (SendStatusIfDelegateNull(endpointId))
    {
        return true;
    };

    ChipLogDetail(Zcl, "OTA Server received QueryImage");

    // TODO: (#7112) change location size checking once CHAR_STRING is supported
    const uint8_t locationLen = emberAfStringLength(location);
    if (locationLen != kLocationParamLength)
    {
        ChipLogError(Zcl, "expected location length %" PRIu8 ", got %" PRIu8, locationLen, kLocationParamLength);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }
    else if (metadataForServer.size() > kMaxMetadataLen)
    {
        ChipLogError(Zcl, "metadata size %zu exceeds max %zu", metadataForServer.size(), kMaxMetadataLen);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    chip::ByteSpan locationSpan(location, locationLen);

    status = delegate->HandleQueryImage(vendorId, productId, imageType, hardwareVersion, currentVersion, protocolsSupported,
                                        locationSpan, clientCanConsent, metadataForServer);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

namespace chip {
namespace app {
namespace clusters {

void OTAServer::SetDelegate(chip::EndpointId endpointId, OTAServerDelegate * delegate)
{
    uint8_t ep = emberAfFindClusterServerEndpointIndex(endpointId, ZCL_OTA_SERVER_CLUSTER_ID);
    if (ep != 0xFF)
    {
        gDelegateTable[ep] = delegate;
    }
}

} // namespace clusters
} // namespace app
} // namespace chip
