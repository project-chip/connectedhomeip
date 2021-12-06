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
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/OperationalDeviceProxy.h>
#include <app/util/af-enums.h>
#include <credentials/FabricTable.h>
#include <lib/support/Span.h>

#include "events/EventQueue.h"
#include <app/clusters/ota-requestor/OTARequestor.h>

class MbedOTARequestor : public OTARequestorInterface
{
    static constexpr uint8_t kVersionBufLen = 32;
    char mUpdateVersion[kVersionBufLen];

    static constexpr size_t kFileNameBufLen = 256;
    char mUpdateFileName[kFileNameBufLen]; // null-terminated

    static constexpr uint8_t kTokenBufLen = 32;
    uint8_t mUpdateToken[kTokenBufLen];

public:
    struct OTAUpdateDetails
    {
        uint32_t updateVersion;
        chip::MutableCharSpan updateVersionString;
        chip::MutableCharSpan updateFileName;
        chip::MutableByteSpan updateToken;
    };

private:
    typedef void (*AnnounceProviderCallback)();
    typedef void (*ProviderResponseCallback)(OTAUpdateDetails * updateDetails);

public:
    MbedOTARequestor(AnnounceProviderCallback announceProviderCallback, ProviderResponseCallback providerResponseCallback);

    // Handler for the AnnounceOTAProvider command
    EmberAfStatus HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData);

    // Handler for the QueryImageResponse command
    bool HandleQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, chip::CharSpan imageURI,
                                  uint32_t softwareVersion, chip::CharSpan softwareVersionString, chip::ByteSpan updateToken,
                                  bool userConsentNeeded, chip::ByteSpan metadataForRequestor);
    // Application directs the Requestor to start the Image Query process
    // and download the new image if available
    void TriggerImmediateQuery() {}

    // A setter for the delegate class pointer
    void SetOtaRequestorDriver(OTARequestorDriver * driver);

    // Application directs the Requestor to abort any processing related to
    // the image update
    void AbortImageUpdate() {}

    void ConnectProvider();

    // Temporary until IP address resolution is implemented in the Exchange Layer
    void SetIpAddress(chip::Inet::IPAddress IpAddress) { mProviderIpAddress = IpAddress; }

private:
    OTAUpdateDetails mUpdateDetails;

    chip::NodeId mProviderNodeId;
    chip::FabricIndex mProviderFabricIndex;
    EmberAfOTAAnnouncementReason mAnnouncementReason;
    chip::Inet::IPAddress mProviderIpAddress;

    AnnounceProviderCallback mAnnounceProviderCallback;
    ProviderResponseCallback mProviderResponseCallback;

    OTARequestorDriver * mOtaRequestorDriver;

    chip::CharSpan GetFileNameFromURI(chip::CharSpan imageURI);
};

inline void MbedOTARequestor::SetOtaRequestorDriver(OTARequestorDriver * driver)
{
    mOtaRequestorDriver = driver;
}
