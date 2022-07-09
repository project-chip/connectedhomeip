/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 #include "OTASoftwareUpdateInteractive.h"

 #include <json/json.h>
 #include <fstream>

// TODO: Objective-C Matter.framework needs to expose this.
 #include <lib/core/OTAImageHeader.h>

// constexpr uint8_t kUpdateTokenLen    = 32;                      // must be between 8 and 32
// constexpr uint8_t kUpdateTokenStrLen = kUpdateTokenLen * 2 + 1; // Hex string needs 2 hex chars for every byte
constexpr size_t kOtaHeaderMaxSize   = 1024;

static bool CompareSoftwareVersions(const DeviceSoftwareVersionModel & a,
                                    const DeviceSoftwareVersionModel & b)
{
    return (a.softwareVersion < b.softwareVersion);
}

 bool ParseOTAHeader(chip::OTAImageHeaderParser & parser, const char * otaFilePath, chip::OTAImageHeader & header)
{
    uint8_t otaFileContent[kOtaHeaderMaxSize];
    chip::ByteSpan buffer(otaFileContent);

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

 // Parses the JSON filepath and extracts DeviceSoftwareVersionModel parameters
static bool ParseJsonFileAndPopulateCandidates(const char * filepath,
                                               std::vector<DeviceSoftwareVersionModel> & candidates)
{
    bool ret = false;
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::ifstream ifs;

    builder["collectComments"] = true; // allow C/C++ type comments in JSON file
    ifs.open(filepath);

    if (!ifs.good())
    {
        ChipLogError(SoftwareUpdate, "Error opening ifstream with file: \"%s\"", filepath);
        return ret;
    }

    if (!parseFromStream(builder, ifs, &root, &errs))
    {
        ChipLogError(SoftwareUpdate, "Error parsing JSON from file: \"%s\"", filepath);
        return ret;
    }

    const Json::Value devSofVerModValue = root["deviceSoftwareVersionModel"];
    if (!devSofVerModValue || !devSofVerModValue.isArray())
    {
        ChipLogError(SoftwareUpdate, "Error: Key deviceSoftwareVersionModel not found or its value is not of type Array");
    }
    else
    {
        for (auto iter : devSofVerModValue)
        {
            DeviceSoftwareVersionModel candidate;
            candidate.vendorId        = static_cast<chip::VendorId>(iter.get("vendorId", 1).asUInt());
            candidate.productId       = static_cast<uint16_t>(iter.get("productId", 1).asUInt());
            candidate.softwareVersion = static_cast<uint32_t>(iter.get("softwareVersion", 10).asUInt64());
            strncpy(candidate.softwareVersionString, 
            iter.get("softwareVersionString", "1.0.0").asCString(),
            SW_VER_STR_MAX_LEN);
            candidate.cDVersionNumber              = static_cast<uint16_t>(iter.get("cDVersionNumber", 0).asUInt());
            candidate.softwareVersionValid         = iter.get("softwareVersionValid", true).asBool() ? true : false;
            candidate.minApplicableSoftwareVersion = static_cast<uint32_t>(iter.get("minApplicableSoftwareVersion", 0).asUInt64());
            candidate.maxApplicableSoftwareVersion =
                static_cast<uint32_t>(iter.get("maxApplicableSoftwareVersion", 1000).asUInt64());
            strncpy(candidate.otaURL, iter.get("otaURL", "https://test.com").asCString(), OTA_URL_MAX_LEN);
            candidates.push_back(candidate);
            ret = true;
        }
    }
    return ret;
}

CHIP_ERROR OTASoftwareUpdateSetFilePath::RunCommand()
{
    if (!IsInteractive()){
        ChipLogError(chipTool, "OTA Software Can only be ran in Interactive mode.");
        return CHIP_ERROR_INTERNAL;
    }
    
    SetCandidatesFromFilePath(mOTACandidatesFilePath);
    SetCommandExitStatus(nil);

    return CHIP_NO_ERROR;
}

void OTASoftwareUpdateBase::SetCandidatesFromFilePath(char * _Nonnull filePath)
{
    std::vector<DeviceSoftwareVersionModel> candidates;
    ChipLogDetail(chipTool, "Setting candidates from file path: %s", filePath);
    ParseJsonFileAndPopulateCandidates(filePath, candidates);
    mCandidates = std::move(candidates);
    for (auto candidate : mCandidates)
    {
        chip::OTAImageHeaderParser parser;
        chip::OTAImageHeader header;
        ParseOTAHeader(parser, candidate.otaURL, header);
        ChipLogDetail(chipTool, "Validating image list candidate %s: ", candidate.otaURL);
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
    // if (mQueryImageStatus == OTAQueryStatus::kUpdateAvailable)
    // {
    //     GenerateUpdateToken(updateToken, kUpdateTokenLen);
    //     GetUpdateTokenString(ByteSpan(updateToken), strBuf, kUpdateTokenStrLen);
    //     ChipLogDetail(SoftwareUpdate, "Generated updateToken: %s", strBuf);

    //     // TODO: This uses the current node as the provider to supply the OTA image. This can be configurable such that the
    //     // provider supplying the response is not the provider supplying the OTA image.
    //     FabricIndex fabricIndex       = commandObj->GetAccessingFabricIndex();
    //     const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    //     NodeId nodeId                 = fabricInfo->GetPeerId().GetNodeId();

    //     // Generate the ImageURI if one is not already preset
    //     if (strlen(mImageUri) == 0)
    //     {
    //         // Only supporting BDX protocol for now
    //         MutableCharSpan uri(mImageUri);
    //         CHIP_ERROR error = chip::bdx::MakeURI(nodeId, CharSpan::fromCharString(mOTACandidatesFilePath), uri);
    //         if (error != CHIP_NO_ERROR)
    //         {
    //             ChipLogError(SoftwareUpdate, "Cannot generate URI");
    //             memset(mImageUri, 0, sizeof(mImageUri));
    //         }
    //         else
    //         {
    //             ChipLogDetail(SoftwareUpdate, "Generated URI: %s", mImageUri);
    //         }
    //     }

    //     // Initialize the transfer session in prepartion for a BDX transfer
    //     BitFlags<TransferControlFlags> bdxFlags;
    //     bdxFlags.Set(TransferControlFlags::kReceiverDrive);
    //     if (mBdxOtaSender.InitializeTransfer(commandObj->GetSubjectDescriptor().fabricIndex,
    //                                          commandObj->GetSubjectDescriptor().subject) == CHIP_NO_ERROR)
    //     {
    //         CHIP_ERROR error =
    //             mBdxOtaSender.PrepareForTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kSender, bdxFlags,
    //                                              kMaxBdxBlockSize, kBdxTimeout, chip::System::Clock::Milliseconds32(mPollInterval));
    //         if (error != CHIP_NO_ERROR)
    //         {
    //             ChipLogError(SoftwareUpdate, "Cannot prepare for transfer: %" CHIP_ERROR_FORMAT, error.Format());
    //             commandObj->AddStatus(commandPath, Status::Failure);
    //             return;
    //         }

    //         response.imageURI.Emplace(chip::CharSpan::fromCharString(mImageUri));
    //         response.softwareVersion.Emplace(mSoftwareVersion);
    //         response.softwareVersionString.Emplace(chip::CharSpan::fromCharString(mSoftwareVersionString));
    //         response.updateToken.Emplace(chip::ByteSpan(updateToken));
    //     }
    //     else
    //     {
    //         // Another BDX transfer in progress
    //         mQueryImageStatus = OTAQueryStatus::kBusy;
    //     }
    // }

    // // Delay action time is only applicable when the provider is busy
    // if (mQueryImageStatus == OTAQueryStatus::kBusy)
    // {
    //     response.delayedActionTime.Emplace(mDelayedQueryActionTimeSec);
    // }

    // // Set remaining fields common to all status types
    // response.status = mQueryImageStatus;
    // if (mUserConsentNeeded && requestorCanConsent)
    // {
    //     response.userConsentNeeded.Emplace(true);
    // }
    // else
    // {
    //     response.userConsentNeeded.Emplace(false);
    // }
    // // For test coverage, sending empty metadata when (requestorNodeId % 2) == 0 and not sending otherwise.
    // if (commandObj->GetSubjectDescriptor().subject % 2 == 0)
    // {
    //     response.metadataForRequestor.Emplace(chip::ByteSpan());
    // }

    // // Either sends the response or an error status
    // commandObj->AddResponse(commandPath, response);

}

bool OTASoftwareUpdateBase::SelectOTACandidate(const uint16_t requestorVendorID, const uint16_t requestorProductID,
                                            const uint32_t requestorSoftwareVersion,
                                            DeviceSoftwareVersionModel & finalCandidate)
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