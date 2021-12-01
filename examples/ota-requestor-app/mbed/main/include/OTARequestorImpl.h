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
#include <OTARequestorDriverImpl.h>

class OTARequestorImpl
{
    typedef void (*ConnectToProviderCallback)(chip::NodeId, chip::FabricIndex, chip::Optional<chip::ByteSpan>);
    typedef void (*DownloadUpdateCallback)(void);

public:
    struct OTAUpdateDetails
    {
        uint8_t updateToken[32];
        uint8_t updateTokenLen;
        uint32_t updateVersion;
    };

    static OTARequestorImpl & GetInstance() { return sInstance; }

    void Init(ConnectToProviderCallback connectProviderCallback = nullptr, DownloadUpdateCallback downloadUpdateCallback = nullptr)
    {
        mConnectProviderCallback = connectProviderCallback;
        mDownloadUpdateCallback  = downloadUpdateCallback;
    }

    // Handler for the AnnounceOTAProvider command
    bool HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData);

    // Handler for the QueryImageResponse command
    bool HandleQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, chip::CharSpan imageURI,
                                  uint32_t softwareVersion, chip::CharSpan softwareVersionString, chip::ByteSpan updateToken,
                                  bool userConsentNeeded, chip::ByteSpan metadataForRequestor);

    // Handler for the ApplyUpdateResponse command
    bool HandleApplyUpdateResponse(
        chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
    {
        return false;
    }

    // Application directs the Requestor to start the Image Query process
    // and download the new image if available
    void TriggerImmediateQuery() {}

    // Application directs the Requestor to abort any processing related to
    // the image update
    void AbortImageUpdate() {}

    // A setter for the delegate class pointer
    void SetOtaRequestorDriver(OTARequestorDriverImpl * driver);

    void ConnectProvider(chip::NodeId peerNodeId, chip::FabricIndex peerFabricIndex, const char * ipAddress);

private:
    OTARequestorImpl();

    static OTARequestorImpl sInstance;

    chip::NodeId mProviderNodeId;
    chip::FabricIndex mProviderFabricIndex;
    EmberAfOTAAnnouncementReason mAnnouncementReason;
    chip::Optional<chip::ByteSpan> mProviderIpAddress;

    OTAUpdateDetails mUpdateDetails;

    ConnectToProviderCallback mConnectProviderCallback;
    DownloadUpdateCallback mDownloadUpdateCallback;

    OTARequestorDriverImpl * mOtaRequestorDriver;
};

inline void OTARequestorImpl::SetOtaRequestorDriver(OTARequestorDriverImpl * driver)
{
    mOtaRequestorDriver = driver;
}