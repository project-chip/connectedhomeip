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
#include <credentials/FabricTable.h>
#include <crypto/RandUtils.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMemString.h>
#include <protocols/bdx/BdxUri.h>

#include <fstream>
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
using chip::Protocols::InteractionModel::Status;
using namespace chip;
using namespace chip::ota;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

constexpr uint8_t kUpdateTokenLen    = 32;                      // must be between 8 and 32
constexpr uint8_t kUpdateTokenStrLen = kUpdateTokenLen * 2 + 1; // Hex string needs 2 hex chars for every byte
constexpr size_t kOtaHeaderMaxSize   = 1024;

// Arbitrary BDX Transfer Params
constexpr uint32_t kMaxBdxBlockSize                = 1024;
constexpr chip::System::Clock::Timeout kBdxTimeout = chip::System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr uint32_t kBdxServerPollIntervalMillis    = 50;                                     // poll every 50ms by default

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
    memset(mOTAFilePath, 0, sizeof(mOTAFilePath));
    memset(mImageUri, 0, sizeof(mImageUri));
    mIgnoreQueryImageCount     = 0;
    mIgnoreApplyUpdateCount    = 0;
    mQueryImageStatus          = OTAQueryStatus::kNotAvailable;
    mUpdateAction              = OTAApplyUpdateAction::kDiscontinue;
    mDelayedQueryActionTimeSec = 0;
    mDelayedApplyActionTimeSec = 0;
    mUserConsentDelegate       = nullptr;
    mUserConsentNeeded         = false;
    mPollInterval              = kBdxServerPollIntervalMillis;
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
        memset(mOTAFilePath, 0, sizeof(mOTAFilePath));
    }
}

void OTAProviderExample::SetImageUri(const char * imageUri)
{
    if (imageUri != nullptr)
    {
        chip::Platform::CopyString(mImageUri, imageUri);
    }
    else
    {
        memset(mImageUri, 0, sizeof(mImageUri));
    }
}

void OTAProviderExample::SetOTACandidates(std::vector<OTAProviderExample::DeviceSoftwareVersionModel> candidates)
{
    mCandidates = std::move(candidates);

    // Validate that each candidate matches the info in the image header
    for (auto candidate : mCandidates)
    {
        OTAImageHeaderParser parser;
        OTAImageHeader header;
        ParseOTAHeader(parser, candidate.otaURL, header);

        ChipLogDetail(SoftwareUpdate, "Validating image list candidate %s: ", candidate.otaURL);
        VerifyOrDie(candidate.vendorId == header.mVendorId);
        VerifyOrDie(candidate.productId == header.mProductId);
        VerifyOrDie(candidate.softwareVersion == header.mSoftwareVersion);
        VerifyOrDie(strlen(candidate.softwareVersionString) == header.mSoftwareVersionString.size());
        VerifyOrDie(memcmp(candidate.softwareVersionString, header.mSoftwareVersionString.data(),
                           header.mSoftwareVersionString.size()) == 0);
        if (header.mMinApplicableVersion.HasValue())
        {
            VerifyOrDie(candidate.minApplicableSoftwareVersion == header.mMinApplicableVersion.Value());
        }
        if (header.mMaxApplicableVersion.HasValue())
        {
            VerifyOrDie(candidate.maxApplicableSoftwareVersion == header.mMaxApplicableVersion.Value());
        }
        parser.Clear();
    }
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
    subject.requestorVendorId       = commandData.vendorID;
    subject.requestorProductId      = commandData.productID;
    subject.requestorCurrentVersion = commandData.softwareVersion;
    subject.requestorTargetVersion  = targetVersion;
    if (commandData.metadataForProvider.HasValue())
    {
        subject.metadata = commandData.metadataForProvider.Value();
    }
    return subject;
}

bool OTAProviderExample::ParseOTAHeader(OTAImageHeaderParser & parser, const char * otaFilePath, OTAImageHeader & header)
{
    uint8_t otaFileContent[kOtaHeaderMaxSize];
    ByteSpan buffer(otaFileContent);

    std::ifstream otaFile(otaFilePath, std::ifstream::in);
    if (!otaFile.is_open() || !otaFile.good())
    {
        ChipLogError(SoftwareUpdate, "Error opening OTA image file: %s", otaFilePath);
        return false;
    }

    otaFile.read(reinterpret_cast<char *>(otaFileContent), kOtaHeaderMaxSize);
    if (otaFile.bad())
    {
        ChipLogError(SoftwareUpdate, "Error reading OTA image file: %s", otaFilePath);
        return false;
    }

    parser.Init();
    if (!parser.IsInitialized())
    {
        return false;
    }

    CHIP_ERROR error = parser.AccumulateAndDecode(buffer, header);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Error parsing OTA image header: %" CHIP_ERROR_FORMAT, error.Format());
        return false;
    }

    return true;
}

void OTAProviderExample::SendQueryImageResponse(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const QueryImage::DecodableType & commandData)
{
    VerifyOrReturn(commandObj != nullptr, ChipLogError(SoftwareUpdate, "Invalid commandObj, cannot send QueryImageResponse"));

    QueryImageResponse::Type response;
    bool requestorCanConsent             = commandData.requestorCanConsent.ValueOr(false);
    uint8_t updateToken[kUpdateTokenLen] = { 0 };
    char strBuf[kUpdateTokenStrLen]      = { 0 };

    // Set fields specific for an available status response
    if (mQueryImageStatus == OTAQueryStatus::kUpdateAvailable)
    {
        GenerateUpdateToken(updateToken, kUpdateTokenLen);
        GetUpdateTokenString(ByteSpan(updateToken), strBuf, kUpdateTokenStrLen);
        ChipLogDetail(SoftwareUpdate, "Generated updateToken: %s", strBuf);

        // TODO: This uses the current node as the provider to supply the OTA image. This can be configurable such that the
        // provider supplying the response is not the provider supplying the OTA image.
        FabricIndex fabricIndex       = commandObj->GetAccessingFabricIndex();
        const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
        NodeId nodeId                 = fabricInfo->GetPeerId().GetNodeId();

        // Generate the ImageURI if one is not already preset
        if (strlen(mImageUri) == 0)
        {
            // Only supporting BDX protocol for now
            MutableCharSpan uri(mImageUri);
            CHIP_ERROR error = chip::bdx::MakeURI(nodeId, CharSpan::fromCharString(mOTAFilePath), uri);
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Cannot generate URI");
                memset(mImageUri, 0, sizeof(mImageUri));
            }
            else
            {
                ChipLogDetail(SoftwareUpdate, "Generated URI: %s", mImageUri);
            }
        }

        // Initialize the transfer session in prepartion for a BDX transfer
        BitFlags<TransferControlFlags> bdxFlags;
        bdxFlags.Set(TransferControlFlags::kReceiverDrive);
        if (mBdxOtaSender.InitializeTransfer(commandObj->GetSubjectDescriptor().fabricIndex,
                                             commandObj->GetSubjectDescriptor().subject) == CHIP_NO_ERROR)
        {
            CHIP_ERROR error =
                mBdxOtaSender.PrepareForTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kSender, bdxFlags,
                                                 kMaxBdxBlockSize, kBdxTimeout, chip::System::Clock::Milliseconds32(mPollInterval));
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Cannot prepare for transfer: %" CHIP_ERROR_FORMAT, error.Format());
                commandObj->AddStatus(commandPath, Status::Failure);
                return;
            }

            response.imageURI.Emplace(chip::CharSpan::fromCharString(mImageUri));
            response.softwareVersion.Emplace(mSoftwareVersion);
            response.softwareVersionString.Emplace(chip::CharSpan::fromCharString(mSoftwareVersionString));
            response.updateToken.Emplace(chip::ByteSpan(updateToken));
        }
        else
        {
            // Another BDX transfer in progress
            mQueryImageStatus = OTAQueryStatus::kBusy;
        }
    }

    // Delay action time is only applicable when the provider is busy
    if (mQueryImageStatus == OTAQueryStatus::kBusy)
    {
        response.delayedActionTime.Emplace(mDelayedQueryActionTimeSec);
    }

    // Set remaining fields common to all status types
    response.status = mQueryImageStatus;
    if (mUserConsentNeeded && requestorCanConsent)
    {
        response.userConsentNeeded.Emplace(true);
    }
    else
    {
        response.userConsentNeeded.Emplace(false);
    }
    // For test coverage, sending empty metadata when (requestorNodeId % 2) == 0 and not sending otherwise.
    if (commandObj->GetSubjectDescriptor().subject % 2 == 0)
    {
        response.metadataForRequestor.Emplace(chip::ByteSpan());
    }

    // Either sends the response or an error status
    commandObj->AddResponse(commandPath, response);
}

void OTAProviderExample::HandleQueryImage(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const QueryImage::DecodableType & commandData)
{
    bool requestorCanConsent = commandData.requestorCanConsent.ValueOr(false);

    if (mIgnoreQueryImageCount > 0)
    {
        ChipLogDetail(SoftwareUpdate, "Skip sending QueryImageResponse, ignore count: %" PRIu32, mIgnoreQueryImageCount);
        mIgnoreQueryImageCount--;
        return;
    }

    if (mQueryImageStatus == OTAQueryStatus::kUpdateAvailable)
    {
        memset(mSoftwareVersionString, 0, sizeof(mSoftwareVersionString));

        if (!mCandidates.empty()) // If list of OTA candidates is supplied
        {
            OTAProviderExample::DeviceSoftwareVersionModel candidate;
            if (SelectOTACandidate(commandData.vendorID, commandData.productID, commandData.softwareVersion, candidate))
            {
                VerifyOrDie(sizeof(mSoftwareVersionString) > strlen(candidate.softwareVersionString));

                // This assumes all candidates have passed verification so the values are safe to use
                mSoftwareVersion = candidate.softwareVersion;
                memcpy(mSoftwareVersionString, candidate.softwareVersionString, strlen(candidate.softwareVersionString));
                SetOTAFilePath(candidate.otaURL);
            }
        }
        else if (strlen(mOTAFilePath) > 0) // If OTA file is directly provided
        {
            // Parse the header and set version info based on the header
            OTAImageHeaderParser parser;
            OTAImageHeader header;
            VerifyOrDie(ParseOTAHeader(parser, mOTAFilePath, header) == true);
            VerifyOrDie(sizeof(mSoftwareVersionString) > header.mSoftwareVersionString.size());
            mSoftwareVersion = header.mSoftwareVersion;
            memcpy(mSoftwareVersionString, header.mSoftwareVersionString.data(), header.mSoftwareVersionString.size());
            parser.Clear();
        }

        // If mUserConsentNeeded (set by the CLI) is true and requestor is capable of taking user consent
        // then delegate obtaining user consent to the requestor
        if (mUserConsentDelegate && (requestorCanConsent && mUserConsentNeeded) == false)
        {
            UserConsentState state = mUserConsentDelegate->GetUserConsentState(
                GetUserConsentSubject(commandObj, commandPath, commandData, mSoftwareVersion));
            ChipLogProgress(SoftwareUpdate, "User Consent state: %s", mUserConsentDelegate->UserConsentStateToString(state));
            switch (state)
            {
            case UserConsentState::kGranted:
                mQueryImageStatus = OTAQueryStatus::kUpdateAvailable;
                break;

            case UserConsentState::kObtaining:
                mQueryImageStatus = OTAQueryStatus::kBusy;
                break;

            case UserConsentState::kDenied:
            case UserConsentState::kUnknown:
                mQueryImageStatus = OTAQueryStatus::kNotAvailable;
                break;
            }
        }
    }

    // Guarantees that either a response or an error status is sent
    SendQueryImageResponse(commandObj, commandPath, commandData);

    // After the first response is sent, default to these values for subsequent queries
    mQueryImageStatus          = OTAQueryStatus::kUpdateAvailable;
    mDelayedQueryActionTimeSec = 0;
}

void OTAProviderExample::HandleApplyUpdateRequest(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                  const ApplyUpdateRequest::DecodableType & commandData)
{
    VerifyOrReturn(commandObj != nullptr, ChipLogError(SoftwareUpdate, "Invalid commandObj, cannot handle ApplyUpdateRequest"));

    if (mIgnoreApplyUpdateCount > 0)
    {
        ChipLogDetail(SoftwareUpdate, "Skip sending ApplyUpdateResponse, ignore count %" PRIu32, mIgnoreApplyUpdateCount);
        mIgnoreApplyUpdateCount--;
        return;
    }

    // TODO: handle multiple transfers by tracking updateTokens
    char tokenBuf[kUpdateTokenStrLen] = { 0 };

    GetUpdateTokenString(commandData.updateToken, tokenBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "%s: token: %s, version: %" PRIu32, __FUNCTION__, tokenBuf, commandData.newVersion);

    ApplyUpdateResponse::Type response;
    response.action            = mUpdateAction;
    response.delayedActionTime = mDelayedApplyActionTimeSec;

    // Reset delay back to 0 for subsequent uses
    mDelayedApplyActionTimeSec = 0;
    // Reset back to success case for subsequent uses
    mUpdateAction = OTAApplyUpdateAction::kProceed;

    // Either sends the response or an error status
    commandObj->AddResponse(commandPath, response);
}

void OTAProviderExample::HandleNotifyUpdateApplied(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                   const NotifyUpdateApplied::DecodableType & commandData)
{
    VerifyOrReturn(commandObj != nullptr, ChipLogError(SoftwareUpdate, "Invalid commandObj, cannot handle NotifyUpdateApplied"));

    char tokenBuf[kUpdateTokenStrLen] = { 0 };

    GetUpdateTokenString(commandData.updateToken, tokenBuf, kUpdateTokenStrLen);
    ChipLogDetail(SoftwareUpdate, "%s: token: %s, version: %" PRIu32, __FUNCTION__, tokenBuf, commandData.softwareVersion);

    commandObj->AddStatus(commandPath, Status::Success);
}
