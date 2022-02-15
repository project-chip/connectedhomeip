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
#include <app/clusters/ota-provider/ota-provider-delegate.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/UserConsentDelegate.h>
#include <vector>

/**
 * A reference implementation for an OTA Provider. Includes a method for providing a path to a local OTA file to serve.
 */
class OTAProviderExample : public chip::app::Clusters::OTAProviderDelegate
{
public:
    OTAProviderExample();

    void SetOTAFilePath(const char * path);
    BdxOtaSender * GetBdxOtaSender() { return &mBdxOtaSender; }

    // Inherited from OTAProviderDelegate
    EmberAfStatus HandleQueryImage(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override;
    EmberAfStatus HandleApplyUpdateRequest(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override;
    EmberAfStatus HandleNotifyUpdateApplied(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override;

    enum QueryImageBehaviorType
    {
        kRespondWithUnknown,
        kRespondWithUpdateAvailable,
        kRespondWithBusy,
        kRespondWithNotAvailable
    };
    static constexpr uint16_t SW_VER_STR_MAX_LEN = 32;
    static constexpr uint16_t OTA_URL_MAX_LEN    = 512;
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
    void SetOTACandidates(std::vector<OTAProviderExample::DeviceSoftwareVersionModel> candidates);
    void SetQueryImageBehavior(QueryImageBehaviorType behavior) { mQueryImageBehavior = behavior; }
    void SetApplyUpdateAction(chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction action)
    {
        mUpdateAction = action;
    }
    void SetDelayedActionTimeSec(uint32_t time) { mDelayedActionTimeSec = time; }
    void SetUserConsentDelegate(chip::ota::UserConsentDelegate * delegate) { mUserConsentDelegate = delegate; }
    void SetSoftwareVersion(uint32_t softwareVersion) { mSoftwareVersion.SetValue(softwareVersion); }
    void SetSoftwareVersionString(const char * versionString) { mSoftwareVersionString = versionString; }
    void SetUserConsentNeeded(bool needed) { mUserConsentNeeded = needed; }

private:
    BdxOtaSender mBdxOtaSender;
    std::vector<DeviceSoftwareVersionModel> mCandidates;
    static constexpr size_t kFilepathBufLen = 256;
    char mOTAFilePath[kFilepathBufLen]; // null-terminated
    QueryImageBehaviorType mQueryImageBehavior;
    chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction mUpdateAction;
    uint32_t mDelayedActionTimeSec;
    bool SelectOTACandidate(const uint16_t requestorVendorID, const uint16_t requestorProductID,
                            const uint32_t requestorSoftwareVersion,
                            OTAProviderExample::DeviceSoftwareVersionModel & finalCandidate);
    chip::ota::UserConsentDelegate * mUserConsentDelegate = nullptr;

    chip::ota::UserConsentSubject
    GetUserConsentSubject(const chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
                          uint32_t targetVersion);
    chip::Optional<uint32_t> mSoftwareVersion;
    const char * mSoftwareVersionString = nullptr;
    bool mUserConsentNeeded             = false;
};
