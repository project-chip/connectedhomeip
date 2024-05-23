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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/clusters/ota-provider/OTAProviderUserConsentDelegate.h>
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <lib/core/OTAImageHeader.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <vector>

/**
 * A reference implementation for an OTA Provider. Includes a method for providing a path to a local OTA file to serve.
 */
class OTAProviderExample : public chip::app::Clusters::OTAProviderDelegate
{
public:
    OTAProviderExample();

    using OTAQueryStatus       = chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus;
    using OTAApplyUpdateAction = chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction;

    static constexpr uint16_t SW_VER_STR_MAX_LEN = 64;
    static constexpr uint16_t OTA_URL_MAX_LEN    = 512;
    static constexpr size_t kFilepathBufLen      = 256;
    static constexpr size_t kUriMaxLen           = 256;

    typedef struct DeviceSoftwareVersionModel
    {
        chip::VendorId vendorId;
        uint16_t productId;
        uint32_t softwareVersion;
        char softwareVersionString[SW_VER_STR_MAX_LEN];
        uint16_t cDVersionNumber;
        bool softwareVersionValid;
        uint32_t minApplicableSoftwareVersion;
        uint32_t maxApplicableSoftwareVersion;
        char otaURL[OTA_URL_MAX_LEN];
    } DeviceSoftwareVersionModel;

    //////////// OTAProviderDelegate Implementation ///////////////
    void HandleQueryImage(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override;
    void HandleApplyUpdateRequest(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override;
    void HandleNotifyUpdateApplied(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override;

    //////////// OTAProviderExample public APIs ///////////////
    void SetOTAFilePath(const char * path);
    void SetImageUri(const char * imageUri);
    BdxOtaSender * GetBdxOtaSender() { return &mBdxOtaSender; }

    void SetOTACandidates(std::vector<OTAProviderExample::DeviceSoftwareVersionModel> candidates);
    void SetIgnoreQueryImageCount(uint32_t count) { mIgnoreQueryImageCount = count; }
    void SetIgnoreApplyUpdateCount(uint32_t count) { mIgnoreApplyUpdateCount = count; }
    void SetQueryImageStatus(OTAQueryStatus status) { mQueryImageStatus = status; }
    void SetApplyUpdateAction(chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction action)
    {
        mUpdateAction = action;
    }
    void SetDelayedQueryActionTimeSec(uint32_t time) { mDelayedQueryActionTimeSec = time; }
    void SetDelayedApplyActionTimeSec(uint32_t time) { mDelayedApplyActionTimeSec = time; }
    void SetUserConsentDelegate(chip::ota::OTAProviderUserConsentDelegate * delegate) { mUserConsentDelegate = delegate; }
    void SetUserConsentNeeded(bool needed) { mUserConsentNeeded = needed; }
    void SetPollInterval(uint32_t interval)
    {
        if (interval != 0)
            mPollInterval = interval;
    }

private:
    bool SelectOTACandidate(const uint16_t requestorVendorID, const uint16_t requestorProductID,
                            const uint32_t requestorSoftwareVersion,
                            OTAProviderExample::DeviceSoftwareVersionModel & finalCandidate);

    chip::ota::UserConsentSubject
    GetUserConsentSubject(const chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
                          uint32_t targetVersion);

    bool ParseOTAHeader(chip::OTAImageHeaderParser & parser, const char * otaFilePath, chip::OTAImageHeader & header);

    /**
     * Called to send the response for a QueryImage command. If an error is encountered, an error status will be sent.
     */
    void
    SendQueryImageResponse(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                           const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData);

    BdxOtaSender mBdxOtaSender;
    std::vector<DeviceSoftwareVersionModel> mCandidates;
    char mOTAFilePath[kFilepathBufLen]; // null-terminated
    char mImageUri[kUriMaxLen];
    OTAQueryStatus mQueryImageStatus;
    OTAApplyUpdateAction mUpdateAction;
    uint32_t mIgnoreQueryImageCount;
    uint32_t mIgnoreApplyUpdateCount;
    uint32_t mDelayedQueryActionTimeSec;
    uint32_t mDelayedApplyActionTimeSec;
    chip::ota::OTAProviderUserConsentDelegate * mUserConsentDelegate;
    bool mUserConsentNeeded;
    uint32_t mSoftwareVersion;
    char mSoftwareVersionString[SW_VER_STR_MAX_LEN];
    uint32_t mPollInterval;
};
