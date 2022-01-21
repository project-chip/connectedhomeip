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

#include <algorithm>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <credentials/FabricTable.h>
#include <crypto/RandUtils.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMemString.h>
#include <protocols/bdx/BdxUri.h>

#include <string.h>

using chip::BitFlags;
using chip::ByteSpan;
using chip::CharSpan;
using chip::FabricIndex;
using chip::FabricInfo;
using chip::MutableCharSpan;
using chip::NodeId;
using chip::Optional;
using chip::Server;
using chip::Span;
using chip::app::Clusters::OTAProviderDelegate;
using chip::bdx::TransferControlFlags;
using namespace chip;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

namespace {

// If we track user consent for individual node, we may not need to track states
enum UserConsentState
{
    // No pending user consent request or may have been rejected
    // QueryImage will obtain user consent if needed
    kIdle,
    // User consent request is pending, QueryImage will be responded as busy till timeout
    // No request for user consent will be raised
    kInProgress,
    // User consent is granted, Specific node will be responded with Image URL,
    // rest of the nodes will receive Busy. This state will be moved to kIdle after
    // responding with Image URL
    kGranted,
};

static UserConsentState mUserConsentStatus = UserConsentState::kIdle;
static EndpointId mUserConsentEndpoint     = kInvalidEndpointId;
static NodeId mUserConsentNodeId           = kUndefinedNodeId;

// Using 5 minutes as timeout for user consent expiration
constexpr chip::System::Clock::Timeout kDefaultUserConsentTimeout = chip::System::Clock::Seconds16(5 * 60);

static void OnUserConsentTimeout(System::Layer *, void * context)
{
    if (mUserConsentStatus != UserConsentState::kIdle)
    {
        ChipLogDetail(SoftwareUpdate, "User consent timeout expired");
        mUserConsentStatus   = UserConsentState::kIdle;
        mUserConsentEndpoint = kInvalidEndpointId;
        mUserConsentNodeId   = kUndefinedNodeId;
    }
}

static void ClearUserConsent()
{
    chip::DeviceLayer::SystemLayer().CancelTimer(OnUserConsentTimeout, nullptr);
    mUserConsentStatus   = UserConsentState::kIdle;
    mUserConsentEndpoint = kInvalidEndpointId;
    mUserConsentNodeId   = kUndefinedNodeId;
}

static void OnUserConsent(bool granted, EndpointId endpoint, NodeId nodeId)
{
    ChipLogDetail(SoftwareUpdate, "User consent %s for endpoint:%hu nodeid:%llu ", granted ? "granted" : "denied", endpoint,
                  nodeId);

    if (granted)
    {
        mUserConsentStatus   = UserConsentState::kGranted;
        mUserConsentEndpoint = endpoint;
        mUserConsentNodeId   = nodeId;
    }
    else
    {
        ClearUserConsent();
    }
}

} // namespace

constexpr uint8_t kUpdateTokenLen    = 32;                      // must be between 8 and 32
constexpr uint8_t kUpdateTokenStrLen = kUpdateTokenLen * 2 + 1; // Hex string needs 2 hex chars for every byte
constexpr size_t kUriMaxLen          = 256;

// Arbitrary BDX Transfer Params
constexpr uint32_t kMaxBdxBlockSize                 = 1024;
constexpr chip::System::Clock::Timeout kBdxTimeout  = chip::System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr chip::System::Clock::Timeout kBdxPollFreq = chip::System::Clock::Milliseconds32(500);

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

OTAProviderExample::OTAProviderExample()
{
    memset(mOTAFilePath, 0, kFilepathBufLen);
    mQueryImageBehavior   = kRespondWithNotAvailable;
    mDelayedActionTimeSec = 0;
    mCandidates.clear();
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

void OTAProviderExample::SetOTACandidates(std::vector<OTAProviderExample::DeviceSoftwareVersionModel> candidates)
{
    mCandidates = std::move(candidates);
}

static bool CompareSoftwareVersions(const OTAProviderExample::DeviceSoftwareVersionModel & a,
                                    const OTAProviderExample::DeviceSoftwareVersionModel & b)
{
    return (a.softwareVersion < b.softwareVersion);
}

bool OTAProviderExample::SelectOTACandidate(const uint16_t requestorVendorID, const uint16_t requestorProductID,
                                            const uint32_t requestorSoftwareVersion,
                                            OTAProviderExample::DeviceSoftwareVersionModel & finalCandidate)
{
    bool candidateFound = false;
    std::sort(mCandidates.begin(), mCandidates.end(), CompareSoftwareVersions);
    for (auto candidate : mCandidates)
    {
        // VendorID and ProductID will be the primary key when querying
        // the DCL servers. If not we can add the vendor/product ID checks here.
        if (candidate.softwareVersionValid && requestorSoftwareVersion < candidate.softwareVersion &&
            requestorSoftwareVersion >= candidate.minApplicableSoftwareVersion &&
            requestorSoftwareVersion <= candidate.maxApplicableSoftwareVersion)
        {
            candidateFound = true;
            finalCandidate = candidate;
        }
    }
    return candidateFound;
}

void OTAProviderExample::SetUserConsentDelegate(chip::ota::UserConsentDelegate * delegate)
{
    VerifyOrReturn(delegate != nullptr, ChipLogError(SoftwareUpdate, "User consent delegate is null"));
    mUserConsentDelegate                      = delegate;
    mUserConsentDelegate->userConsentCallback = OnUserConsent;
}

EmberAfStatus OTAProviderExample::HandleQueryImage(chip::app::CommandHandler * commandObj,
                                                   const chip::app::ConcreteCommandPath & commandPath,
                                                   const QueryImage::DecodableType & commandData)
{
    ChipLogDetail(SoftwareUpdate, "Requestor endpoint:%hu node-id:%llu", commandPath.mEndpointId,
                  emberAfCurrentCommand()->SourceNodeId());

    OTAQueryStatus queryStatus  = OTAQueryStatus::kNotAvailable;
    OTAProviderExample::DeviceSoftwareVersionModel candidate;
    bool otaAvailable                     = false;
    uint32_t newSoftwareVersion           = 0;
    const char * newSoftwareVersionString = nullptr;
    const char * otaFilePath              = nullptr;
    uint8_t updateToken[kUpdateTokenLen]  = { 0 };
    char strBuf[kUpdateTokenStrLen]       = { 0 };
    char uriBuf[kUriMaxLen]               = { 0 };
    bool userConsentNeeded                = (commandData.requestorCanConsent.HasValue() && commandData.requestorCanConsent.Value() == true);
    QueryImageResponse::Type response;

    // This use-case is a subset of the ota-candidates-file option.
    // Can be removed once all other platforms (ESP, etc.)
    // start using the ota-candidates-file method.
    if (strlen(mOTAFilePath)) // If OTA file is directly provided
    {
        otaAvailable       = true;
        newSoftwareVersion = commandData.softwareVersion + 1; // This implementation will always indicate that an update is
                                                              // available (if the user provides a file).
        newSoftwareVersionString = "Example-Image-V0.1";
        otaFilePath              = mOTAFilePath;
        queryStatus              = OTAQueryStatus::kUpdateAvailable;
    }
    else if (!mCandidates.empty()) // If list of OTA candidates is supplied instead
    {
        otaAvailable = SelectOTACandidate(commandData.vendorId, commandData.productId, commandData.softwareVersion, candidate);
        if (otaAvailable)
        {
            newSoftwareVersion       = candidate.softwareVersion;
            newSoftwareVersionString = candidate.softwareVersionString;
            otaFilePath              = candidate.otaURL;
            queryStatus              = OTAQueryStatus::kUpdateAvailable;
        }
    }

    if (queryStatus == OTAQueryStatus::kUpdateAvailable)
    {
        switch (mUserConsentStatus)
        {
        case UserConsentState::kIdle: {
            if (userConsentNeeded == false)
            {
                if (mUserConsentDelegate == nullptr)
                {
                    // Not sure, should we return error or Busy/NotAvailable?
                    ChipLogError(SoftwareUpdate, "User consent delegate is null");
                    return EMBER_ZCL_STATUS_FAILURE;
                }

                queryStatus = OTAQueryStatus::kBusy;
                ChipLogDetail(SoftwareUpdate, "Obtaining user consent...");
                mUserConsentDelegate->ObtainUserConsentAsync(emberAfCurrentCommand()->SourceNodeId(), commandPath.mEndpointId,
                                                             commandData.softwareVersion, newSoftwareVersion);
                chip::DeviceLayer::SystemLayer().StartTimer(kDefaultUserConsentTimeout, OnUserConsentTimeout, nullptr);
            }
        }
        break;

        case UserConsentState::kInProgress: {
            ChipLogDetail(SoftwareUpdate, "User consent is in progress, responding with busy");
            queryStatus = OTAQueryStatus::kBusy;
        }
        break;

        case UserConsentState::kGranted: {
            if (mUserConsentEndpoint == commandPath.mEndpointId && mUserConsentNodeId == emberAfCurrentCommand()->SourceNodeId())
            {
                ChipLogDetail(SoftwareUpdate, "User consent has been granted");
                queryStatus = OTAQueryStatus::kUpdateAvailable;
                ClearUserConsent();
            }
            else
            {
                ChipLogDetail(SoftwareUpdate, "User consent has been granted, but not for this node");
                queryStatus = OTAQueryStatus::kBusy;
            }
        }
        break;
        }
    }

    if (queryStatus == OTAQueryStatus::kUpdateAvailable)
    {
        GenerateUpdateToken(updateToken, kUpdateTokenLen);
        GetUpdateTokenString(ByteSpan(updateToken), strBuf, kUpdateTokenStrLen);
        ChipLogDetail(SoftwareUpdate, "generated updateToken: %s", strBuf);

        // TODO: This uses the current node as the provider to supply the OTA image. This can be configurable such that the provider
        // supplying the response is not the provider supplying the OTA image.
        FabricIndex fabricIndex = commandObj->GetAccessingFabricIndex();
        FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
        NodeId nodeId           = fabricInfo->GetPeerId().GetNodeId();

        // Only doing BDX transport for now
        MutableCharSpan uri(uriBuf, kUriMaxLen);
        chip::bdx::MakeURI(nodeId, CharSpan::fromCharString(mOTAFilePath), uri);
        ChipLogDetail(SoftwareUpdate, "Generated URI: %.*s", static_cast<int>(uri.size()), uri.data());

        // Initialize the transfer session in prepartion for a BDX transfer
        mBdxOtaSender.SetFilepath(mOTAFilePath);
        BitFlags<TransferControlFlags> bdxFlags;
        bdxFlags.Set(TransferControlFlags::kReceiverDrive);
        CHIP_ERROR err = mBdxOtaSender.PrepareForTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kSender,
                                                          bdxFlags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollFreq);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "Failed to initialize BDX transfer session: %s", chip::ErrorStr(err));
            return EMBER_ZCL_STATUS_FAILURE;
        }

        response.imageURI.Emplace(chip::CharSpan(uriBuf, strlen(uriBuf)));
        response.softwareVersion.Emplace(newSoftwareVersion);
        response.softwareVersionString.Emplace(chip::CharSpan(kExampleSoftwareString, strlen(kExampleSoftwareString)));
        response.updateToken.Emplace(chip::ByteSpan(updateToken));
    }

    response.status = queryStatus;
    response.delayedActionTime.Emplace(mDelayedActionTimeSec);
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

    OTAApplyUpdateAction updateAction = OTAApplyUpdateAction::kProceed; // For now, just allow any update request
    char tokenBuf[kUpdateTokenStrLen] = { 0 };

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
