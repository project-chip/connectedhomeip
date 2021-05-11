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
#include <app/util/af.h>
#include <gen/att-storage.h>
#include <gen/attribute-id.h>
#include <gen/attribute-type.h>
#include <gen/callback.h>
#include <gen/cluster-id.h>
#include <gen/command-id.h>
#include <gen/enums.h>

#include <support/Span.h>

#include "ota-server-delegate.h"
#include "ota-server.h"

using namespace chip;
using namespace chip::app::clusters;

namespace {
constexpr uint8_t kLocationParamLength   = 2;   // The expected length of the Location parameter in QueryImage
constexpr size_t kMaxMetadataLen         = 512; // The maximum length of Metadata in any OTA Server command
constexpr size_t kUpdateTokenParamLength = 32;  // The expected length of the Update Token parameter used in multiple commands

chip::app::clusters::OTAServerDelegate * gServerDelegate = nullptr;

bool SendStatusIfDelegateNull()
{
    if (gServerDelegate == nullptr)
    {
        ChipLogError(Zcl, "No OTAServerDelegate set");
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

bool emberAfOtaSoftwareUpdateServerClusterApplyUpdateRequestCallback(app::Command * commandObj, chip::ByteSpan updateToken,
                                                                     uint32_t newVersion)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    ChipLogDetail(Zcl, "OTA Server received ApplyUpdateRequest");

    if (updateToken.size() != kUpdateTokenParamLength)
    {
        // TODO: correct error?
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    }

    if (SendStatusIfDelegateNull())
        return true;
    status = gServerDelegate->HandleApplyUpdateRequest(updateToken, newVersion);
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

bool emberAfOtaSoftwareUpdateServerClusterNotifyUpdateAppliedCallback(app::Command * commandObj, chip::ByteSpan updateToken,
                                                                      uint32_t currentVersion)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    ChipLogDetail(Zcl, "OTA Server received NotifyUpdateUpplied");

    if (updateToken.size() != kUpdateTokenParamLength)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    }

    if (SendStatusIfDelegateNull())
        return true;
    status = gServerDelegate->HandleNotifyUpdateApplied(updateToken, currentVersion);
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
    app::Command * commandObj, uint16_t vendorId, uint16_t productId, uint16_t imageType, uint16_t hardwareVersion,
    uint32_t currentVersion,
    /* TYPE WARNING: array array defaults to */ uint8_t * protocolsSupported, uint8_t * location, uint8_t clientCanConsent,
    chip::ByteSpan metadataForServer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    ChipLogDetail(Zcl, "OTA Server received QueryImage");

    const uint8_t locationLen = emberAfStringLength(location);
    if ((locationLen != kLocationParamLength) || (metadataForServer.size() > kMaxMetadataLen))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    }

    chip::ByteSpan locationSpan(location, locationLen);

    if (SendStatusIfDelegateNull())
        return true;
    status = gServerDelegate->HandleQueryImage(vendorId, productId, imageType, hardwareVersion, currentVersion, protocolsSupported,
                                               locationSpan, clientCanConsent, metadataForServer);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(status);
    }

    return true;
}

void chip::app::clusters::OTAServer::SetDelegate(OTAServerDelegate * delegate)
{
    gServerDelegate = delegate;
}