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

#include <fstream>
#include <json/json.h>

// TODO: Objective-C Matter.framework needs to expose this.
#include <lib/core/OTAImageHeader.h>

constexpr size_t kOtaHeaderMaxSize = 1024;

bool ParseOTAHeader(chip::OTAImageHeaderParser & parser, const char * otaFilePath, chip::OTAImageHeader & header)
{
    uint8_t otaFileContent[kOtaHeaderMaxSize];
    chip::ByteSpan buffer(otaFileContent);

    std::ifstream otaFile(otaFilePath, std::ifstream::in);
    if (!otaFile.is_open() || !otaFile.good()) {
        ChipLogError(SoftwareUpdate, "Error opening OTA image file: %s", otaFilePath);
        return false;
    }

    otaFile.read(reinterpret_cast<char *>(otaFileContent), kOtaHeaderMaxSize);
    if (otaFile.bad()) {
        ChipLogError(SoftwareUpdate, "Error reading OTA image file: %s", otaFilePath);
        return false;
    }

    parser.Init();
    if (!parser.IsInitialized()) {
        return false;
    }

    CHIP_ERROR error = parser.AccumulateAndDecode(buffer, header);
    if (error != CHIP_NO_ERROR) {
        ChipLogError(SoftwareUpdate, "Error parsing OTA image header: %" CHIP_ERROR_FORMAT, error.Format());
        return false;
    }

    return true;
}

// Parses the JSON filepath and extracts DeviceSoftwareVersionModel parameters
static bool ParseJsonFileAndPopulateCandidates(
    const char * filepath, NSMutableArray<DeviceSoftwareVersionModel *> ** _Nonnull candidates)
{
    bool ret = false;
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::ifstream ifs;

    builder["collectComments"] = true; // allow C/C++ type comments in JSON file
    ifs.open(filepath);

    if (!ifs.good()) {
        ChipLogError(SoftwareUpdate, "Error opening ifstream with file: \"%s\"", filepath);
        return ret;
    }

    if (!parseFromStream(builder, ifs, &root, &errs)) {
        ChipLogError(SoftwareUpdate, "Error parsing JSON from file: \"%s\"", filepath);
        return ret;
    }

    const Json::Value devSofVerModValue = root["deviceSoftwareVersionModel"];
    if (!devSofVerModValue || !devSofVerModValue.isArray()) {
        ChipLogError(SoftwareUpdate, "Error: Key deviceSoftwareVersionModel not found or its value is not of type Array");
    } else {
        *candidates = [[NSMutableArray alloc] init];
        for (auto iter : devSofVerModValue) {
            DeviceSoftwareVersionModel * candidate = [[DeviceSoftwareVersionModel alloc] init];
            candidate.deviceModelData.vendorId = [NSNumber numberWithUnsignedInt:iter.get("vendorId", 1).asUInt()];
            candidate.deviceModelData.productId = [NSNumber numberWithUnsignedInt:iter.get("productId", 1).asUInt()];
            candidate.softwareVersion = [NSNumber numberWithUnsignedLong:iter.get("softwareVersion", 10).asUInt64()];
            candidate.softwareVersionString =
                [NSString stringWithUTF8String:iter.get("softwareVersionString", "1.0.0").asCString()];
            candidate.deviceModelData.cDVersionNumber = [NSNumber numberWithUnsignedInt:iter.get("cDVersionNumber", 0).asUInt()];
            candidate.deviceModelData.softwareVersionValid = iter.get("softwareVersionValid", true).asBool() ? YES : NO;
            candidate.deviceModelData.minApplicableSoftwareVersion =
                [NSNumber numberWithUnsignedLong:iter.get("minApplicableSoftwareVersion", 0).asUInt64()];
            candidate.deviceModelData.maxApplicableSoftwareVersion =
                [NSNumber numberWithUnsignedLong:iter.get("maxApplicableSoftwareVersion", 1000).asUInt64()];
            candidate.deviceModelData.otaURL = [NSString stringWithUTF8String:iter.get("otaURL", "https://test.com").asCString()];
            [*candidates addObject:candidate];
            ret = true;
        }
    }
    return ret;
}

CHIP_ERROR OTASoftwareUpdateSetFilePath::RunCommand()
{
    SetCandidatesFromFilePath(mOTACandidatesFilePath);
    SetCommandExitStatus(nil);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTASoftwareUpdateSetStatus::RunCommand()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    error = SetUserConsentStatus(mUserConsentStatus);
    SetCommandExitStatus(nil);

    return error;
}

void OTASoftwareUpdateBase::SetCandidatesFromFilePath(char * _Nonnull filePath)
{
    NSMutableArray<DeviceSoftwareVersionModel *> * candidates;
    ChipLogDetail(chipTool, "Setting candidates from file path: %s", filePath);
    ParseJsonFileAndPopulateCandidates(filePath, &candidates);
    for (DeviceSoftwareVersionModel * candidate : candidates) {
        chip::OTAImageHeaderParser parser;
        chip::OTAImageHeader header;
        ParseOTAHeader(parser, [candidate.deviceModelData.otaURL UTF8String], header);
        ChipLogDetail(chipTool, "Validating image list candidate %s: ", [candidate.deviceModelData.otaURL UTF8String]);
        VerifyOrDie([candidate.deviceModelData.vendorId unsignedIntValue] == header.mVendorId);
        VerifyOrDie([candidate.deviceModelData.productId unsignedIntValue] == header.mProductId);
        VerifyOrDie([candidate.softwareVersion unsignedLongValue] == header.mSoftwareVersion);
        VerifyOrDie([candidate.softwareVersionString length] == header.mSoftwareVersionString.size());
        VerifyOrDie(memcmp([candidate.softwareVersionString UTF8String], header.mSoftwareVersionString.data(),
                        header.mSoftwareVersionString.size())
            == 0);
        if (header.mMinApplicableVersion.HasValue()) {
            VerifyOrDie(
                [candidate.deviceModelData.minApplicableSoftwareVersion unsignedLongValue] == header.mMinApplicableVersion.Value());
        }
        if (header.mMaxApplicableVersion.HasValue()) {
            VerifyOrDie(
                [candidate.deviceModelData.maxApplicableSoftwareVersion unsignedLongValue] == header.mMaxApplicableVersion.Value());
        }
        parser.Clear();
    }
    mOTADelegate.candidates = candidates;
}

CHIP_ERROR OTASoftwareUpdateBase::SetUserConsentStatus(char * _Nonnull otaSTatus)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (strcmp(otaSTatus, "granted") == 0) {
        mOTADelegate.userConsentState = OTAProviderUserGranted;
    } else if (strcmp(otaSTatus, "obtaining") == 0) {
        mOTADelegate.userConsentState = OTAProviderUserObtaining;
    } else if (strcmp(otaSTatus, "denied") == 0) {
        mOTADelegate.userConsentState = OTAProviderUserDenied;
    } else {
        ChipLogError(chipTool, "Only accepts the following: granted, obtaining, and denied.");
        error = CHIP_ERROR_INTERNAL;
    }
    return error;
}

CHIP_ERROR OTASoftwareUpdateBase::Run()
{
    if (!IsInteractive()) {
        ChipLogError(chipTool, "OTA software update commands can only be ran in interactive mode.");
        return CHIP_ERROR_INTERNAL;
    }
    return CHIPCommandBridge::Run();
}
