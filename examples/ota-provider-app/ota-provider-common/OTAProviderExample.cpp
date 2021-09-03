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

#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandPathParams.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <app/util/af.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/RandUtils.h>
#include <protocols/secure_channel/PASESession.h> // For chip::kTestDeviceNodeId

#include <string.h>

using chip::ByteSpan;
using chip::Span;
using chip::app::CommandPathFlags;
using chip::app::CommandPathParams;
using chip::app::clusters::OTAProviderDelegate;
using chip::TLV::ContextTag;
using chip::TLV::TLVWriter;

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
        buf[i] = chip::GetRandU8();
    }
}

bool GenerateBdxUri(const Span<char> & fileDesignator, Span<char> outUri, size_t availableSize)
{
    static constexpr char bdxPrefix[] = "bdx://";
    chip::NodeId nodeId               = chip::kTestDeviceNodeId; // TODO: read this dynamically
    size_t nodeIdHexStrLen            = sizeof(nodeId) * 2;
    size_t expectedLength             = strlen(bdxPrefix) + nodeIdHexStrLen + fileDesignator.size();

    if (expectedLength >= availableSize)
    {
        return false;
    }

    size_t written = static_cast<size_t>(snprintf(outUri.data(), availableSize, "%s" ChipLogFormatX64 "%s", bdxPrefix,
                                                  ChipLogValueX64(nodeId), fileDesignator.data()));

    return expectedLength == written;
}

OTAProviderExample::OTAProviderExample()
{
    memset(mOTAFilePath, 0, kFilepathBufLen);
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

EmberAfStatus OTAProviderExample::HandleQueryImage(chip::app::CommandHandler * commandObj, uint16_t vendorId, uint16_t productId,
                                                   uint16_t imageType, uint16_t hardwareVersion, uint32_t currentVersion,
                                                   uint8_t protocolsSupported, const chip::ByteSpan & location,
                                                   bool clientCanConsent, const chip::ByteSpan & metadataForServer)
{
    // TODO: add confiuration for returning BUSY status

    EmberAfOTAQueryStatus queryStatus =
        (strlen(mOTAFilePath) ? EMBER_ZCL_OTA_QUERY_STATUS_UPDATE_AVAILABLE : EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE);
    uint32_t delayedActionTimeSec = 0;
    uint32_t softwareVersion      = currentVersion + 1; // This implementation will always indicate that an update is available
                                                        // (if the user provides a file).
    bool userConsentNeeded               = false;
    uint8_t updateToken[kUpdateTokenLen] = { 0 };
    char strBuf[kUpdateTokenStrLen]      = { 0 };
    char uriBuf[kUriMaxLen]              = { 0 };

    GenerateUpdateToken(updateToken, kUpdateTokenLen);
    GetUpdateTokenString(ByteSpan(updateToken), strBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "generated updateToken: %s", strBuf);

    if (strlen(mOTAFilePath))
    {
        // Only doing BDX transport for now
        GenerateBdxUri(Span<char>(mOTAFilePath, strlen(mOTAFilePath)), Span<char>(uriBuf, 0), kUriMaxLen);
        ChipLogDetail(SoftwareUpdate, "generated URI: %s", uriBuf);
    }

    CommandPathParams cmdParams = { emberAfCurrentEndpoint(), 0 /* mGroupId */, ZCL_OTA_PROVIDER_CLUSTER_ID,
                                    ZCL_QUERY_IMAGE_RESPONSE_COMMAND_ID, (CommandPathFlags::kEndpointIdValid) };
    TLVWriter * writer          = nullptr;
    uint8_t tagNum              = 0;
    VerifyOrReturnError((commandObj->PrepareCommand(cmdParams) == CHIP_NO_ERROR), EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->Put(ContextTag(tagNum++), queryStatus) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->Put(ContextTag(tagNum++), delayedActionTimeSec) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->PutString(ContextTag(tagNum++), uriBuf) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->Put(ContextTag(tagNum++), softwareVersion) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->PutBytes(ContextTag(tagNum++), updateToken, kUpdateTokenLen) == CHIP_NO_ERROR,
                        EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->Put(ContextTag(tagNum++), userConsentNeeded) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->PutBytes(ContextTag(tagNum++), updateToken, kUpdateTokenLen) == CHIP_NO_ERROR,
                        EMBER_ZCL_STATUS_FAILURE); // metadata
    VerifyOrReturnError((commandObj->FinishCommand() == CHIP_NO_ERROR), EMBER_ZCL_STATUS_FAILURE);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus OTAProviderExample::HandleApplyUpdateRequest(chip::app::CommandHandler * commandObj,
                                                           const chip::ByteSpan & updateToken, uint32_t newVersion)
{
    // TODO: handle multiple transfers by tracking updateTokens
    // TODO: add configuration for sending different updateAction and delayedActionTime values

    EmberAfOTAApplyUpdateAction updateAction = EMBER_ZCL_OTA_APPLY_UPDATE_ACTION_PROCEED; // For now, just allow any update request
    uint32_t delayedActionTimeSec            = 0;
    char tokenBuf[kUpdateTokenStrLen]        = { 0 };

    GetUpdateTokenString(updateToken, tokenBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "%s: token: %s, version: %" PRIu32, __FUNCTION__, tokenBuf, newVersion);

    VerifyOrReturnError(commandObj != nullptr, EMBER_ZCL_STATUS_INVALID_VALUE);

    CommandPathParams cmdParams = { emberAfCurrentEndpoint(), 0 /* mGroupId */, ZCL_OTA_PROVIDER_CLUSTER_ID,
                                    ZCL_APPLY_UPDATE_REQUEST_RESPONSE_COMMAND_ID, (CommandPathFlags::kEndpointIdValid) };
    TLVWriter * writer          = nullptr;

    VerifyOrReturnError((commandObj->PrepareCommand(cmdParams) == CHIP_NO_ERROR), EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->Put(ContextTag(0), updateAction) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError(writer->Put(ContextTag(1), delayedActionTimeSec) == CHIP_NO_ERROR, EMBER_ZCL_STATUS_FAILURE);
    VerifyOrReturnError((commandObj->FinishCommand() == CHIP_NO_ERROR), EMBER_ZCL_STATUS_FAILURE);

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus OTAProviderExample::HandleNotifyUpdateApplied(const chip::ByteSpan & updateToken, uint32_t currentVersion)
{
    char tokenBuf[kUpdateTokenStrLen] = { 0 };

    GetUpdateTokenString(updateToken, tokenBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "%s: token: %s, version: %" PRIu32, __FUNCTION__, tokenBuf, currentVersion);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);

    return EMBER_ZCL_STATUS_SUCCESS;
}
