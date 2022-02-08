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
using namespace chip::ota;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

constexpr uint8_t kUpdateTokenLen            = 32;                      // must be between 8 and 32
constexpr uint8_t kUpdateTokenStrLen         = kUpdateTokenLen * 2 + 1; // Hex string needs 2 hex chars for every byte
constexpr size_t kUriMaxLen                  = 256;
constexpr uint32_t kMinimumDelayedActionTime = 120; // Spec mentions delayed action time should be at least 120 seconds

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

UserConsentSubject OTAProviderExample::GetUserConsentSubject(const app::CommandHandler * commandObj,
                                                             const app::ConcreteCommandPath & commandPath,
                                                             const QueryImage::DecodableType & commandData, uint32_t targetVersion)
{
    UserConsentSubject subject;
    subject.fabricIndex             = commandObj->GetSubjectDescriptor().fabricIndex;
    subject.requestorNodeId         = commandObj->GetSubjectDescriptor().subject;
    subject.providerEndpointId      = commandPath.mEndpointId;
    subject.requestorVendorId       = commandData.vendorId;
    subject.requestorProductId      = commandData.productId;
    subject.requestorCurrentVersion = commandData.softwareVersion;
    subject.requestorTargetVersion  = targetVersion;
    if (commandData.metadataForProvider.HasValue())
    {
        subject.metadata = commandData.metadataForProvider.Value();
    }
    return subject;
}

EmberAfStatus OTAProviderExample::HandleQueryImage(chip::app::CommandHandler * commandObj,
                                                   const chip::app::ConcreteCommandPath & commandPath,
                                                   const QueryImage::DecodableType & commandData)
{
    OTAQueryStatus queryStatus = OTAQueryStatus::kNotAvailable;
    OTAProviderExample::DeviceSoftwareVersionModel candidate;
    uint32_t newSoftwareVersion           = 0;
    const char * newSoftwareVersionString = nullptr;
    const char * otaFilePath              = nullptr;
    uint8_t updateToken[kUpdateTokenLen]  = { 0 };
    char strBuf[kUpdateTokenStrLen]       = { 0 };
    char uriBuf[kUriMaxLen]               = { 0 };
    uint32_t delayedActionTimeSec         = mDelayedActionTimeSec;
    bool requestorCanConsent              = commandData.requestorCanConsent.ValueOr(false);
    QueryImageResponse::Type response;

    switch (mQueryImageBehavior)
    {
    case kRespondWithUnknown:
        // This use-case is a subset of the ota-candidates-file option.
        // Can be removed once all other platforms start using the ota-candidates-file method.
        if (strlen(mOTAFilePath) > 0) // If OTA file is directly provided
        {
            // TODO: Following details shall be read from the OTA file

            // If software version is provided using command line then use it.
            // Otherwise, bump the software version received in QueryImage by 1.
            newSoftwareVersion = commandData.softwareVersion + 1;
            if (mSoftwareVersion.HasValue())
            {
                newSoftwareVersion = mSoftwareVersion.Value();
            }

            // If software version string is provided using command line then use it.
            // Otherwise, use default string.
            newSoftwareVersionString = "Example-Image-V0.1";
            if (mSoftwareVersionString)
            {
                newSoftwareVersionString = mSoftwareVersionString;
            }

            otaFilePath = mOTAFilePath;
            queryStatus = OTAQueryStatus::kUpdateAvailable;
        }
        else if (!mCandidates.empty()) // If list of OTA candidates is supplied instead
        {
            if (SelectOTACandidate(commandData.vendorId, commandData.productId, commandData.softwareVersion, candidate))
            {
                newSoftwareVersion       = candidate.softwareVersion;
                newSoftwareVersionString = candidate.softwareVersionString;
                otaFilePath              = candidate.otaURL;
                queryStatus              = OTAQueryStatus::kUpdateAvailable;
            }
        }

        if (queryStatus == OTAQueryStatus::kUpdateAvailable && mUserConsentDelegate != nullptr)
        {
            UserConsentState state = mUserConsentDelegate->GetUserConsentState(
                GetUserConsentSubject(commandObj, commandPath, commandData, newSoftwareVersion));
            ChipLogProgress(SoftwareUpdate, "User Consent state: %s", mUserConsentDelegate->UserConsentStateToString(state));
            switch (state)
            {
            case UserConsentState::kGranted:
                queryStatus = OTAQueryStatus::kUpdateAvailable;
                break;

            case UserConsentState::kObtaining:
                queryStatus          = OTAQueryStatus::kBusy;
                delayedActionTimeSec = std::max(kMinimumDelayedActionTime, delayedActionTimeSec);
                break;

            case UserConsentState::kDenied:
            case UserConsentState::kUnknown:
                queryStatus          = OTAQueryStatus::kNotAvailable;
                delayedActionTimeSec = std::max(kMinimumDelayedActionTime, delayedActionTimeSec);
                break;
            }
        }
        break;

    case kRespondWithUpdateAvailable:
        queryStatus = OTAQueryStatus::kUpdateAvailable;
        break;

    case kRespondWithBusy:
        queryStatus          = OTAQueryStatus::kBusy;
        delayedActionTimeSec = std::max(kMinimumDelayedActionTime, delayedActionTimeSec);
        break;

    case kRespondWithNotAvailable:
        queryStatus          = OTAQueryStatus::kNotAvailable;
        delayedActionTimeSec = std::max(kMinimumDelayedActionTime, delayedActionTimeSec);
        break;

    default:
        queryStatus          = OTAQueryStatus::kNotAvailable;
        delayedActionTimeSec = std::max(kMinimumDelayedActionTime, delayedActionTimeSec);
        break;
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
        chip::bdx::MakeURI(nodeId, CharSpan::fromCharString(otaFilePath), uri);
        ChipLogDetail(SoftwareUpdate, "Generated URI: %.*s", static_cast<int>(uri.size()), uri.data());

        // Initialize the transfer session in prepartion for a BDX transfer
        mBdxOtaSender.SetFilepath(otaFilePath);
        BitFlags<TransferControlFlags> bdxFlags;
        bdxFlags.Set(TransferControlFlags::kReceiverDrive);
        CHIP_ERROR err = mBdxOtaSender.PrepareForTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kSender,
                                                          bdxFlags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollFreq);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "Failed to initialize BDX transfer session: %s", chip::ErrorStr(err));
            return EMBER_ZCL_STATUS_FAILURE;
        }

        response.imageURI.Emplace(chip::CharSpan::fromCharString(uriBuf));
        response.softwareVersion.Emplace(newSoftwareVersion);
        response.softwareVersionString.Emplace(chip::CharSpan::fromCharString(newSoftwareVersionString));
        response.updateToken.Emplace(chip::ByteSpan(updateToken));
    }

    response.status = queryStatus;
    response.delayedActionTime.Emplace(delayedActionTimeSec);
    if (mUserConsentNeeded)
    {
        response.userConsentNeeded.Emplace(mUserConsentNeeded);
    }
    else
    {
        response.userConsentNeeded.Emplace(requestorCanConsent);
    }
    // For test coverage, sending empty metadata when (requestorNodeId % 2) == 0 and not sending otherwise.
    if (commandObj->GetSubjectDescriptor().subject % 2 == 0)
    {
        response.metadataForRequestor.Emplace(chip::ByteSpan());
    }

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
