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

#include "ota-requestor.h"

#include <app/util/util.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

// Global instance of the OTARequestorInterface.
OTARequestorInterface *globalOTARequestorInstance = nullptr;

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

void SetRequestorInstance(OTARequestorInterface *instance)
{
    globalOTARequestorInstance = instance;
}

OTARequestorInterface *GetRequestorInstance()
{
    return globalOTARequestorInstance;
}


void StartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    static_cast<OTARequestor *>(appState)->ConnectToProvider();
}

EmberAfStatus OTARequestor::HandleAnnounceOTAProvider(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    auto & providerLocation   = commandData.providerLocation;
    auto & announcementReason = commandData.announcementReason;

    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
    }

    mProviderNodeId      = providerLocation;
    mProviderFabricIndex = commandObj->GetExchangeContext()->GetSessionHandle().GetFabricIndex();

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");
    ChipLogDetail(SoftwareUpdate, "  FabricIndex: %" PRIu8, mProviderFabricIndex);
    ChipLogDetail(SoftwareUpdate, "  ProviderNodeID: 0x" ChipLogFormatX64, ChipLogValueX64(mProviderNodeId));
    ChipLogDetail(SoftwareUpdate, "  VendorID: 0x%" PRIx16, commandData.vendorId);
    ChipLogDetail(SoftwareUpdate, "  AnnouncementReason: %" PRIu8, announcementReason);
    if (commandData.metadataForNode.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  MetadataForNode: %zu", commandData.metadataForNode.Value().size());
    }

    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelayMs.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t msToStart = 0;
    switch (announcementReason)
    {
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_SIMPLE_ANNOUNCEMENT):
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_UPDATE_AVAILABLE):
        msToStart = mOtaStartDelayMs;
        break;
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_URGENT_UPDATE_AVAILABLE):
        msToStart = kImmediateStartDelayMs;
        break;
    default:
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %" PRIu8, static_cast<uint8_t>(announcementReason));
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
    }

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(msToStart), StartDelayTimerHandler, this);

    return EMBER_ZCL_STATUS_SUCCESS;
}


CHIP_ERROR OTARequestor::SetupCASESessionManager(chip::FabricIndex fabricIndex)
{
    // A previous CASE session had been established
    if (mCASESessionManager != nullptr)
    {
        if (mCASESessionManager->GetFabricInfo()->GetFabricIndex() != fabricIndex)
        {
            // CSM is per fabric so if fabric index does not match the previous session, CSM needs to be set up again
            chip::Platform::Delete(mCASESessionManager);
            mCASESessionManager = nullptr;
        }
        else
        {
            // Fabric index matches so use previous instance
            return CHIP_NO_ERROR;
        }
    }

    // CSM has not been setup so create a new instance of it
    if (mCASESessionManager == nullptr)
    {
        Server * server         = &(Server::GetInstance());
        FabricInfo * fabricInfo = server->GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabricInfo == nullptr)
        {
            ChipLogError(SoftwareUpdate, "Did not find fabric for index %d", fabricIndex);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::DeviceProxyInitParams initParams = {
            .sessionManager = &(server->GetSecureSessionManager()),
            .exchangeMgr    = &(server->GetExchangeManager()),
            .idAllocator    = &(server->GetSessionIDAllocator()),
            .fabricInfo     = fabricInfo,
            // TODO: Determine where this should be instantiated
            .imDelegate = chip::Platform::New<chip::Controller::DeviceControllerInteractionModelDelegate>(),
        };

        CASESessionManagerConfig sessionManagerConfig = {
            .sessionInitParams = initParams,
            .dnsCache          = nullptr,
        };

        mCASESessionManager = chip::Platform::New<CASESessionManager>(sessionManagerConfig);
    }

    if (mCASESessionManager == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed in creating an instance of CASESessionManager");
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}


// Converted from SendQueryImageCommand(). Uses code from https://github.com/project-chip/connectedhomeip/pull/12013
void OTARequestor::ConnectToProvider()
{
    chip::NodeId peerNodeId = mProviderNodeId;
    chip::FabricIndex peerFabricIndex = mProviderFabricIndex;
    chip::Server * server             = &(chip::Server::GetInstance());

    CHIP_ERROR err = SetupCASESessionManager(peerFabricIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot send QueryImage command without CASESessionManager: %" CHIP_ERROR_FORMAT,
                     err.Format());
    }

    onConnectedState = kQueryImage;
    err = mCASESessionManager->FindOrEstablishSession(peerNodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot establish connection to peer device: %" CHIP_ERROR_FORMAT, err.Format());
    }


    // Old ConnectToProvider(). Delete

    // if (mConnectToProviderCallback != nullptr)
    // {
    //     ChipLogProgress(SoftwareUpdate, "Attempting to connect to 0x" ChipLogFormatX64 " on FabricIndex 0x%" PRIu8,
    //                     ChipLogValueX64(mProviderNodeId), mProviderFabricIndex);

    //     mConnectToProviderCallback(mProviderNodeId, mProviderFabricIndex);
    // }
    // else
    // {
    //     ChipLogError(SoftwareUpdate, "ConnectToProviderCallback is not set");
    // }
}


#if 0 // LISS for now !!!!!!!!!!!!!!!!!!!!!!!


ExampleOTARequestor ExampleOTARequestor::sInstance;

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

// OTA Software Update Requestor Cluster AnnounceOtaProvider Command callback (from client)
bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    EmberAfStatus status = ExampleOTARequestor::GetInstance().HandleAnnounceOTAProvider(commandObj, commandPath, commandData);
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

ExampleOTARequestor::ExampleOTARequestor()
{
    mOtaStartDelayMs     = 0;
    mProviderNodeId      = chip::kUndefinedNodeId;
    mProviderFabricIndex = chip::kUndefinedFabricIndex;
}

void ExampleOTARequestor::Init(uint32_t startDelayMs)
{
    mOtaStartDelayMs = startDelayMs;
}

void ExampleOTARequestor::ConnectToProvider()
{

    if (mConnectToProviderCallback != nullptr)
    {
        ChipLogProgress(SoftwareUpdate, "Attempting to connect to 0x" ChipLogFormatX64 " on FabricIndex 0x%" PRIu8,
                        ChipLogValueX64(mProviderNodeId), mProviderFabricIndex);

        mConnectToProviderCallback(mProviderNodeId, mProviderFabricIndex);
    }
    else
    {
        ChipLogError(SoftwareUpdate, "ConnectToProviderCallback is not set");
    }
}

EmberAfStatus ExampleOTARequestor::HandleAnnounceOTAProvider(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    auto & providerLocation   = commandData.providerLocation;
    auto & announcementReason = commandData.announcementReason;

    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
    }

    mProviderNodeId      = providerLocation;
    mProviderFabricIndex = commandObj->GetExchangeContext()->GetSessionHandle().GetFabricIndex();

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");
    ChipLogDetail(SoftwareUpdate, "  FabricIndex: %" PRIu8, mProviderFabricIndex);
    ChipLogDetail(SoftwareUpdate, "  ProviderNodeID: 0x" ChipLogFormatX64, ChipLogValueX64(mProviderNodeId));
    ChipLogDetail(SoftwareUpdate, "  VendorID: 0x%" PRIx16, commandData.vendorId);
    ChipLogDetail(SoftwareUpdate, "  AnnouncementReason: %" PRIu8, announcementReason);
    if (commandData.metadataForNode.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  MetadataForNode: %zu", commandData.metadataForNode.Value().size());
    }

    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelayMs.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t msToStart = 0;
    switch (announcementReason)
    {
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_SIMPLE_ANNOUNCEMENT):
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_UPDATE_AVAILABLE):
        msToStart = mOtaStartDelayMs;
        break;
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_URGENT_UPDATE_AVAILABLE):
        msToStart = kImmediateStartDelayMs;
        break;
    default:
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %" PRIu8, static_cast<uint8_t>(announcementReason));
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
    }

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(msToStart), StartDelayTimerHandler, this);

    return EMBER_ZCL_STATUS_SUCCESS;
}

void ExampleOTARequestor::StartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    static_cast<ExampleOTARequestor *>(appState)->ConnectToProvider();
}

#endif // LISS
