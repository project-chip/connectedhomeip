/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

constexpr size_t kOtaHeaderMaxSize = 1024;

MTROTAHeader * ParseOTAHeader(const char * otaFilePath)
{
    uint8_t otaFileContent[kOtaHeaderMaxSize];
    chip::ByteSpan buffer(otaFileContent);

    std::ifstream otaFile(otaFilePath, std::ifstream::in);
    if (!otaFile.is_open() || !otaFile.good()) {
        ChipLogError(SoftwareUpdate, "Error opening OTA image file: %s", otaFilePath);
        return nil;
    }

    otaFile.read(reinterpret_cast<char *>(otaFileContent), kOtaHeaderMaxSize);
    if (otaFile.bad()) {
        ChipLogError(SoftwareUpdate, "Error reading OTA image file: %s", otaFilePath);
        return nil;
    }

    return [[MTROTAHeader alloc] initWithData:[NSData dataWithBytes:buffer.data() length:buffer.size()]];
}

// Parses the JSON filepath and extracts DeviceSoftwareVersionModel parameters
static bool ParseJsonFileAndPopulateCandidates(
    const char * filepath, NSMutableArray<DeviceSoftwareVersionModel *> ** _Nonnull candidates)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    std::ifstream ifs;

    builder["collectComments"] = true; // allow C/C++ type comments in JSON file
    ifs.open(filepath);

    if (!ifs.good()) {
        ChipLogError(SoftwareUpdate, "Error opening ifstream with file: \"%s\"", filepath);
        return false;
    }

    if (!parseFromStream(builder, ifs, &root, &errs)) {
        ChipLogError(SoftwareUpdate, "Error parsing JSON from file: \"%s\"", filepath);
        return false;
    }

    const Json::Value devSofVerModValue = root["deviceSoftwareVersionModel"];
    if (!devSofVerModValue || !devSofVerModValue.isArray()) {
        ChipLogError(SoftwareUpdate, "Error: Key deviceSoftwareVersionModel not found or its value is not of type Array");
        return false;
    }

    *candidates = [[NSMutableArray alloc] init];

    bool ret = false;
    for (auto iter : devSofVerModValue) {
        DeviceSoftwareVersionModel * candidate = [[DeviceSoftwareVersionModel alloc] init];

        auto vendorId = [NSNumber numberWithUnsignedInt:iter.get("vendorId", 1).asUInt()];
        auto productId = [NSNumber numberWithUnsignedInt:iter.get("productId", 1).asUInt()];
        auto softwareVersion = [NSNumber numberWithUnsignedLongLong:iter.get("softwareVersion", 10).asUInt64()];
        auto softwareVersionString = [NSString stringWithUTF8String:iter.get("softwareVersionString", "1.0.0").asCString()];
        auto cDVersionNumber = [NSNumber numberWithUnsignedInt:iter.get("cDVersionNumber", 0).asUInt()];
        auto softwareVersionValid = iter.get("softwareVersionValid", true).asBool() ? YES : NO;
        auto minApplicableSoftwareVersion =
            [NSNumber numberWithUnsignedLongLong:iter.get("minApplicableSoftwareVersion", 0).asUInt64()];
        auto maxApplicableSoftwareVersion =
            [NSNumber numberWithUnsignedLongLong:iter.get("maxApplicableSoftwareVersion", 1000).asUInt64()];
        auto otaURL = [NSString stringWithUTF8String:iter.get("otaURL", "https://test.com").asCString()];

        candidate.deviceModelData.vendorID = vendorId;
        candidate.deviceModelData.productID = productId;
        candidate.softwareVersion = softwareVersion;
        candidate.softwareVersionString = softwareVersionString;
        candidate.deviceModelData.cDVersionNumber = cDVersionNumber;
        candidate.deviceModelData.softwareVersionValid = softwareVersionValid;
        candidate.deviceModelData.minApplicableSoftwareVersion = minApplicableSoftwareVersion;
        candidate.deviceModelData.maxApplicableSoftwareVersion = maxApplicableSoftwareVersion;
        candidate.deviceModelData.otaURL = otaURL;
        [*candidates addObject:candidate];
        ret = true;
    }

    return ret;
}

CHIP_ERROR OTASoftwareUpdateSetFilePath::RunCommand()
{
    auto error = SetCandidatesFromFilePath(mOTACandidatesFilePath);
    SetCommandExitStatus(nil);
    return error;
}

CHIP_ERROR OTASoftwareUpdateSetParams::RunCommand()
{
    auto error = SetParams(mAction, mStatus, mUserConsentStatus, mUserConsentNeeded, mDelayedActionTime, mTimedInvokeTimeoutMs);
    SetCommandExitStatus(nil);
    return error;
}

CHIP_ERROR OTASoftwareUpdateSetParams::SetParams(chip::Optional<uint16_t> action, chip::Optional<uint16_t> status,
    chip::Optional<uint16_t> consent, chip::Optional<uint16_t> userConsentNeeded, chip::Optional<uint64_t> delayedActionTime,
    chip::Optional<uint64_t> timedInvokeTimeoutMs)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (action.HasValue()) {
        error = SetActionReplyStatus(action.Value());
        if (error != CHIP_NO_ERROR) {
            return error;
        }
    }
    if (delayedActionTime.HasValue()) {
        mOTADelegate.delayedActionTime = @(delayedActionTime.Value());
    }
    if (timedInvokeTimeoutMs.HasValue()) {
        mOTADelegate.timedInvokeTimeoutMs = @(timedInvokeTimeoutMs.Value());
    }
    if (status.HasValue()) {
        error = SetReplyStatus(status.Value());
        if (error != CHIP_NO_ERROR) {
            return error;
        }
    }
    if (consent.HasValue()) {
        error = SetUserConsentStatus(consent.Value());
        if (error != CHIP_NO_ERROR) {
            return error;
        }
    }
    if (userConsentNeeded.HasValue()) {
        error = SetUserConsentNeeded(userConsentNeeded.Value());
        if (error != CHIP_NO_ERROR) {
            return error;
        }
    }

    return error;
}

CHIP_ERROR OTASoftwareUpdateBase::SetActionReplyStatus(uint16_t action)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (action == 0) {
        mOTADelegate.action = MTROTASoftwareUpdateProviderOTAApplyUpdateActionProceed;
        ChipLogDetail(chipTool, "Successfully set action to: MTROTASoftwareUpdateProviderOTAApplyUpdateActionProceed");
    } else if (action == 1) {
        mOTADelegate.action = MTROTASoftwareUpdateProviderOTAApplyUpdateActionAwaitNextAction;
        ChipLogDetail(chipTool, "Successfully set action to: MTROTASoftwareUpdateProviderOTAApplyUpdateActionAwaitNextAction");
    } else if (action == 2) {
        mOTADelegate.action = MTROTASoftwareUpdateProviderOTAApplyUpdateActionDiscontinue;
        ChipLogDetail(chipTool, "Successfully set action to: MTROTASoftwareUpdateProviderOTAApplyUpdateActionDiscontinue");
    } else {
        ChipLogError(chipTool, "Only accepts the following: 0 (Proceed), 1 (Await Next Action), 2 (Discontinue)");
        error = CHIP_ERROR_INTERNAL;
    }
    return error;
}
CHIP_ERROR OTASoftwareUpdateBase::SetReplyStatus(uint16_t status)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (status == 0) {
        mOTADelegate.queryImageStatus = MTROTASoftwareUpdateProviderOTAQueryStatusUpdateAvailable;
        ChipLogDetail(chipTool, "Successfully set status to: MTROTASoftwareUpdateProviderOTAQueryStatusUpdateAvailable");
    } else if (status == 1) {
        mOTADelegate.queryImageStatus = MTROTASoftwareUpdateProviderOTAQueryStatusBusy;
        ChipLogDetail(chipTool, "Successfully set status to: MTROTASoftwareUpdateProviderOTAQueryStatusBusy");
    } else if (status == 2) {
        mOTADelegate.queryImageStatus = MTROTASoftwareUpdateProviderOTAQueryStatusNotAvailable;
        ChipLogDetail(chipTool, "Successfully set status to: MTROTASoftwareUpdateProviderOTAQueryStatusNotAvailable");
    } else if (status == 4) {
        mOTADelegate.queryImageStatus = MTROTASoftwareUpdateProviderOTAQueryStatusDownloadProtocolNotSupported;
        ChipLogDetail(
            chipTool, "Successfully set status to: MTROTASoftwareUpdateProviderOTAQueryStatusDownloadProtocolNotSupported");
    } else {
        ChipLogError(chipTool, "Only accepts the following: 0 (Available), 1 (Busy), 2 (Not Available), 3 (Not Supported)");
        error = CHIP_ERROR_INTERNAL;
    }
    return error;
}

CHIP_ERROR OTASoftwareUpdateBase::SetUserConsentStatus(uint16_t consent)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (consent == 0) {
        mOTADelegate.userConsentState = OTAProviderUserGranted;
        ChipLogDetail(chipTool, "Successfully set User Consent to: OTAProviderUserGranted");
    } else if (consent == 1) {
        mOTADelegate.userConsentState = OTAProviderUserObtaining;
        ChipLogDetail(chipTool, "Successfully set User Consent to: OTAProviderUserObtaining");
    } else if (consent == 2) {
        mOTADelegate.userConsentState = OTAProviderUserDenied;
        ChipLogDetail(chipTool, "Successfully set User Consent to: OTAProviderUserDenied");
    } else {
        ChipLogError(chipTool, "Only accepts the following: 0 (granted), 1 (obtaining), and 2 (denied).");
        error = CHIP_ERROR_INTERNAL;
    }
    return error;
}

CHIP_ERROR OTASoftwareUpdateBase::SetUserConsentNeeded(uint16_t userConsentNeeded)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (userConsentNeeded == 0) {
        mOTADelegate.userConsentNeeded = @(0);
        ChipLogDetail(chipTool, "Successfully set User Consent to: OTAProviderUserGranted");
    } else if (userConsentNeeded == 1) {
        mOTADelegate.userConsentNeeded = @(1);
        ChipLogDetail(chipTool, "Successfully set User Consent to: OTAProviderUserObtaining");
    } else {
        ChipLogError(chipTool, "Only accepts the following: 0 (Not Needed), and 1 (Needed).");
        error = CHIP_ERROR_INTERNAL;
    }
    return error;
}

CHIP_ERROR OTASoftwareUpdateBase::SetCandidatesFromFilePath(char * _Nonnull filePath)
{
    NSMutableArray<DeviceSoftwareVersionModel *> * candidates;
    ChipLogDetail(chipTool, "Setting candidates from file path: %s", filePath);
    VerifyOrReturnError(ParseJsonFileAndPopulateCandidates(filePath, &candidates), CHIP_ERROR_INTERNAL);

    for (DeviceSoftwareVersionModel * candidate : candidates) {
        auto otaURL = [candidate.deviceModelData.otaURL UTF8String];
        auto header = ParseOTAHeader(otaURL);
        VerifyOrReturnError(header != nil, CHIP_ERROR_INVALID_ARGUMENT);

        ChipLogDetail(chipTool, "Validating image list candidate %s: ", [candidate.deviceModelData.otaURL UTF8String]);

        auto vendorId = [candidate.deviceModelData.vendorID unsignedIntValue];
        auto productId = [candidate.deviceModelData.productID unsignedIntValue];
        auto softwareVersion = [candidate.softwareVersion unsignedLongValue];
        auto softwareVersionString = [candidate.softwareVersionString UTF8String];
        auto softwareVersionStringLength = [candidate.softwareVersionString length];
        auto minApplicableSoftwareVersion = [candidate.deviceModelData.minApplicableSoftwareVersion unsignedLongValue];
        auto maxApplicableSoftwareVersion = [candidate.deviceModelData.maxApplicableSoftwareVersion unsignedLongValue];

        auto headerVendorId = [header.vendorID unsignedIntValue];
        auto headerProductId = [header.productID unsignedIntValue];
        auto headerSoftwareVersion = [header.softwareVersion unsignedLongValue];
        auto headerSoftwareVersionString = [header.softwareVersionString UTF8String];
        auto headerSoftwareVersionStringLength = [header.softwareVersionString length];

        VerifyOrReturnError(vendorId == headerVendorId, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(productId == headerProductId, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(softwareVersion == headerSoftwareVersion, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(softwareVersionStringLength == headerSoftwareVersionStringLength, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(memcmp(softwareVersionString, headerSoftwareVersionString, headerSoftwareVersionStringLength) == 0,
            CHIP_ERROR_INVALID_ARGUMENT);

        if (header.minApplicableVersion) {
            auto version = [header.minApplicableVersion unsignedLongValue];
            VerifyOrReturnError(minApplicableSoftwareVersion == version, CHIP_ERROR_INVALID_ARGUMENT);
        }

        if (header.maxApplicableVersion) {
            auto version = [header.maxApplicableVersion unsignedLongValue];
            VerifyOrReturnError(maxApplicableSoftwareVersion == version, CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    mOTADelegate.candidates = candidates;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTASoftwareUpdateBase::Run()
{
    if (!IsInteractive()) {
        ChipLogError(chipTool, "OTA software update commands can only be ran in interactive mode.");
        return CHIP_ERROR_INTERNAL;
    }
    return CHIPCommandBridge::Run();
}
