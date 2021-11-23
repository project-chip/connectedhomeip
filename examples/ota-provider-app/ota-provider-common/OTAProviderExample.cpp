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

#include <ota-provider-common/OTAProviderExample.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <app/util/af.h>
#include <crypto/RandUtils.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMemString.h>
#include <protocols/secure_channel/PASESession.h> // For chip::kTestDeviceNodeId

#include <string.h>

using chip::ByteSpan;
using chip::CharSpan;
using chip::Optional;
using chip::Span;
using chip::app::Clusters::OTAProviderDelegate;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

constexpr uint8_t kUpdateTokenLen    = 32;                      // must be between 8 and 32
constexpr uint8_t kUpdateTokenStrLen = kUpdateTokenLen * 2 + 1; // Hex string needs 2 hex chars for every byte
constexpr size_t kUriMaxLen          = 256;

void GetUpdateTokenString(const chip::ByteSpan & token, char * buf, size_t bufSize)
{
    const uint8_t * tokenData = static_cast<const uint8_t *>(token.data());
    size_t minLength          = chip::min(token.size(), bufSize);
    for (size_t i = 0; i < (minLength / 2) - 1; ++i)
    {
        snprintf(&buf[i * 2], bufSize, "%02X", tokenData[i]);
    }
}

void GenerateUpdateToken(uint8_t * buf, size_t bufSize)
{
    for (size_t i = 0; i < bufSize; ++i)
    {
        buf[i] = chip::Crypto::GetRandU8();
    }
}

bool GenerateBdxUri(const Span<char> & fileDesignator, Span<char> outUri, size_t availableSize)
{
    static constexpr char bdxPrefix[] = "bdx://";
    chip::NodeId nodeId               = chip::kTestDeviceNodeId; // TODO: read this dynamically
    size_t nodeIdHexStrLen            = sizeof(nodeId) * 2;
    size_t expectedLength             = strlen(bdxPrefix) + nodeIdHexStrLen + 1 + fileDesignator.size();

    if (expectedLength >= availableSize)
    {
        return false;
    }

    size_t written = static_cast<size_t>(snprintf(outUri.data(), availableSize, "%s" ChipLogFormatX64 "/%s", bdxPrefix,
                                                  ChipLogValueX64(nodeId), fileDesignator.data()));

    return expectedLength == written;
}

OTAProviderExample::OTAProviderExample()
{
    memset(mOTAFilePath, 0, kFilepathBufLen);
    mQueryImageBehavior   = kRespondWithNotAvailable;
    mDelayedActionTimeSec = 0;
}

void OTAProviderExample::SetOTAFilePath(const char * path)
{
    if (path != nullptr)
    {
        chip::Platform::CopyString(mOTAFilePath, path);
    }
    else
    {
        memset(mOTAFilePath, 0, kFilepathBufLen);
    }
}

EmberAfStatus OTAProviderExample::HandleQueryImage(chip::app::CommandHandler * commandObj,
                                                   const chip::app::ConcreteCommandPath & commandPath,
                                                   const QueryImage::DecodableType & commandData)
{
    // TODO: add confiuration for returning BUSY status

    EmberAfOTAQueryStatus queryStatus = EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE;
    uint32_t newSoftwareVersion = commandData.softwareVersion + 1; // This implementation will always indicate that an update is
                                                                   // available (if the user provides a file).
    constexpr char kExampleSoftwareString[] = "Example-Image-V0.1";
    bool userConsentNeeded                  = false;
    uint8_t updateToken[kUpdateTokenLen]    = { 0 };
    char strBuf[kUpdateTokenStrLen]         = { 0 };
    char uriBuf[kUriMaxLen]                 = { 0 };

    GenerateUpdateToken(updateToken, kUpdateTokenLen);
    GetUpdateTokenString(ByteSpan(updateToken), strBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "generated updateToken: %s", strBuf);

    if (strlen(mOTAFilePath))
    {
        // Only doing BDX transport for now
        GenerateBdxUri(Span<char>(mOTAFilePath, strlen(mOTAFilePath)), Span<char>(uriBuf, 0), kUriMaxLen);
        ChipLogDetail(SoftwareUpdate, "generated URI: %s", uriBuf);
    }

    // Set Status for the Query Image Response
    switch (mQueryImageBehavior)
    {
    case kRespondWithUpdateAvailable: {
        if (strlen(mOTAFilePath) != 0)
        {
            queryStatus = EMBER_ZCL_OTA_QUERY_STATUS_UPDATE_AVAILABLE;
        }
        else
        {
            queryStatus = EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE;
            ChipLogError(SoftwareUpdate, "No OTA file configured on the Provider");
        }
        break;
    }
    case kRespondWithBusy: {
        queryStatus = EMBER_ZCL_OTA_QUERY_STATUS_BUSY;
        break;
    }
    case kRespondWithNotAvailable: {
        queryStatus = EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE;
        break;
    }
    default:
        queryStatus = EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE;
    }

    QueryImageResponse::Type response;
    response.status = queryStatus;
    response.delayedActionTime.Emplace(mDelayedActionTimeSec);
    response.imageURI.Emplace(chip::CharSpan(uriBuf, strlen(uriBuf)));
    response.softwareVersion.Emplace(newSoftwareVersion);
    response.softwareVersionString.Emplace(chip::CharSpan(kExampleSoftwareString, strlen(kExampleSoftwareString)));
    response.updateToken.Emplace(chip::ByteSpan(updateToken));
    response.userConsentNeeded.Emplace(userConsentNeeded);
    // Could also just not send metadataForRequestor at all.
    response.metadataForRequestor.Emplace(chip::ByteSpan());

    VerifyOrReturnError(commandObj->AddResponseData(commandPath, response) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus OTAProviderExample::HandleApplyUpdateRequest(chip::app::CommandHandler * commandObj,
                                                           const chip::app::ConcreteCommandPath & commandPath,
                                                           const ApplyUpdateRequest::DecodableType & commandData)
{
    // TODO: handle multiple transfers by tracking updateTokens

    EmberAfOTAApplyUpdateAction updateAction = EMBER_ZCL_OTA_APPLY_UPDATE_ACTION_PROCEED; // For now, just allow any update request
    char tokenBuf[kUpdateTokenStrLen]        = { 0 };

    GetUpdateTokenString(commandData.updateToken, tokenBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "%s: token: %s, version: %" PRIu32, __FUNCTION__, tokenBuf, commandData.newVersion);

    VerifyOrReturnError(commandObj != nullptr, EMBER_ZCL_STATUS_INVALID_VALUE);

    ApplyUpdateResponse::Type response;
    response.action            = updateAction;
    response.delayedActionTime = mDelayedActionTimeSec;
    VerifyOrReturnError(commandObj->AddResponseData(commandPath, response) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus OTAProviderExample::HandleNotifyUpdateApplied(chip::app::CommandHandler * commandObj,
                                                            const chip::app::ConcreteCommandPath & commandPath,
                                                            const NotifyUpdateApplied::DecodableType & commandData)
{
    char tokenBuf[kUpdateTokenStrLen] = { 0 };

    GetUpdateTokenString(commandData.updateToken, tokenBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "%s: token: %s, version: %" PRIu32, __FUNCTION__, tokenBuf, commandData.softwareVersion);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);

    return EMBER_ZCL_STATUS_SUCCESS;
}
