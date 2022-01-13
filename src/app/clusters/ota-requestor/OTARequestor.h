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

/* This file contains the declarations for the Matter OTA Requestor implementation and API.
 * Applications implementing the OTA Requestor functionality must include this file.
 */

#pragma once

#include <app/CASESessionManager.h>
#include <app/server/Server.h>
#include <platform/OTARequestorDriver.h>
#include <platform/OTARequestorInterface.h>
#include <protocols/bdx/BdxMessages.h>

#include "BDXDownloader.h"

namespace chip {

// This class implements all of the core logic of the OTA Requestor
class OTARequestor : public OTARequestorInterface, public BDXDownloader::StateDelegate
{
public:
    // Various actions to take when OnConnected callback is called
    enum OnConnectedAction
    {
        kQueryImage = 0,
        kStartBDX,
        kApplyUpdate,
    };

    OTARequestor() : mOnConnectedCallback(OnConnected, this), mOnConnectionFailureCallback(OnConnectionFailure, this) {}

    // Application interface declarations -- start

    // Application directs the Requestor to start the Image Query process
    // and download the new image if available
    OTATriggerResult TriggerImmediateQuery() override;

    // Initiate download of the new image
    void DownloadUpdate() override;

    // Send ApplyImage
    void ApplyUpdate() override;

    // Handle download state change
    void OnDownloadStateChanged(OTADownloader::State state) override;

    // A setter for the delegate class pointer
    void SetOtaRequestorDriver(OTARequestorDriver * driver) { mOtaRequestorDriver = driver; }

    // TODO: this should really be OTADownloader, but right now OTARequestor has information that we need to initialize a
    // BDXDownloader specifically.
    // The BDXDownloader instance should already have the ImageProcessingDelegate set.
    void SetBDXDownloader(chip::BDXDownloader * downloader) { mBdxDownloader = downloader; }

    // Application directs the Requestor to abort the download in progress. All the Requestor state (such
    // as the QueryImageResponse content) is preserved
    void AbortImageUpdate();

    // Application directs the Requestor to abort the download in progress. All the Requestor state is
    // cleared, UploadState is reset to Idle
    void AbortAndResetState();

    // Application notifies the Requestor on the user consent action, TRUE if consent is given,
    // FALSE otherwise
    void OnUserConsent(bool result);

    /* Commented out until the API is supported
    // Application directs the Requestor to download the image using the suppiled parameter and without
    // issuing QueryImage
    OTATriggerResult ResumeImageDownload(const BdxDownloadParameters & bdxParameters){ return kTriggerSuccessful;}
    */

    // Application interface declarations -- end

    // Virtual functions from OTARequestorInterface -- start

    // Handler for the AnnounceOTAProvider command
    EmberAfStatus HandleAnnounceOTAProvider(
        app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
        const app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData) override;

    // Virtual functions from OTARequestorInterface -- end
    /**
     * Called to set the server instance which used to get access to the system resources necessary to open CASE sessions and drive
     * BDX transfers
     */
    void SetServerInstance(Server * server)
    {
        mServer             = server;
        mCASESessionManager = server->GetCASESessionManager();
    }

    /**
     * Called to establish a session to mProviderNodeId on mProviderFabricIndex. This must be called from the same externally
     * synchronized context as any other Matter stack method.
     *
     * @param onConnectedAction  The action to take once session to provider has been established
     */
    void ConnectToProvider(OnConnectedAction onConnectedAction);

    /**
     * Called to indicate test mode. This is when the Requestor is used as a test tool and the the provider parameters are supplied
     * explicitly.
     */
    void TestModeSetProviderParameters(NodeId nodeId, FabricIndex fabIndex, EndpointId endpointId) override
    {
        mProviderNodeId      = nodeId;
        mProviderFabricIndex = fabIndex;
        mProviderEndpointId  = endpointId;
    }

private:
    using QueryImageResponseDecodableType  = app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType;
    using ApplyUpdateResponseDecodableType = app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType;

    static constexpr size_t kMaxUpdateTokenLen = 32;

    // TODO: the application should define this, along with initializing the BDXDownloader

    // This class is purely for delivering messages and sending outgoing messages to/from the BDXDownloader.
    class BDXMessenger : public chip::BDXDownloader::MessagingDelegate, public chip::Messaging::ExchangeDelegate
    {
    public:
        CHIP_ERROR SendMessage(const chip::bdx::TransferSession::OutputEvent & event) override
        {
            ChipLogDetail(SoftwareUpdate, "BDX::SendMessage");
            VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

            chip::Messaging::SendFlags sendFlags;
            if (event.msgTypeData.HasMessageType(chip::bdx::MessageType::ReceiveInit))
            {
                sendFlags.Set(chip::Messaging::SendMessageFlags::kFromInitiator);
            }
            if (!event.msgTypeData.HasMessageType(chip::bdx::MessageType::BlockAckEOF) &&
                !event.msgTypeData.HasMessageType(chip::Protocols::SecureChannel::MsgType::StatusReport))
            {
                sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
            }
            ReturnErrorOnFailure(mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                           event.MsgData.Retain(), sendFlags));
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                     chip::System::PacketBufferHandle && payload) override
        {
            if (mDownloader == nullptr)
            {
                ChipLogError(BDX, "BDXDownloader instance is null, can't pass message");
                return CHIP_NO_ERROR;
            }
            else
            {
                mDownloader->OnMessageReceived(payloadHeader, payload.Retain());
            }

            // For a receiver using BDX Protocol, all received messages will require a response except for a StatusReport
            if (!payloadHeader.HasMessageType(chip::Protocols::SecureChannel::MsgType::StatusReport))
            {
                ec->WillSendMessage();
            }
            return CHIP_NO_ERROR;
        }

        void OnResponseTimeout(chip::Messaging::ExchangeContext * ec) override
        {
            ChipLogError(BDX, "exchange timed out");
            if (mDownloader != nullptr)
            {
                mDownloader->OnDownloadTimeout();
            }
        }

        void Init(chip::BDXDownloader * downloader, chip::Messaging::ExchangeContext * ec)
        {
            mExchangeCtx = ec;
            mDownloader  = downloader;
        }

    private:
        chip::Messaging::ExchangeContext * mExchangeCtx;
        chip::BDXDownloader * mDownloader;
    };

    /**
     * Send QueryImage request using values matching Basic cluster
     */
    CHIP_ERROR SendQueryImageRequest(OperationalDeviceProxy & deviceProxy);

    /**
     * Validate and extract mandatory information from QueryImageResponse
     */
    CHIP_ERROR ExtractUpdateDescription(const QueryImageResponseDecodableType & response, UpdateDescription & update) const;

    /**
     * Start download of the software image returned in QueryImageResponse
     */
    CHIP_ERROR StartDownload(OperationalDeviceProxy & deviceProxy);

    /**
     * Send ApplyUpdate request using values obtained from QueryImageResponse
     */
    CHIP_ERROR SendApplyUpdateRequest(OperationalDeviceProxy & deviceProxy);

    /**
     * Session connection callbacks
     */
    static void OnConnected(void * context, OperationalDeviceProxy * deviceProxy);
    static void OnConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error);
    Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    /**
     * QueryImage callbacks
     */
    static void OnQueryImageResponse(void * context, const QueryImageResponseDecodableType & response);
    static void OnQueryImageFailure(void * context, EmberAfStatus status);

    /**
     * ApplyUpdate callbacks
     */
    static void OnApplyUpdateResponse(void * context, const ApplyUpdateResponseDecodableType & response);
    static void OnApplyUpdateFailure(void * context, EmberAfStatus);

    OTARequestorDriver * mOtaRequestorDriver  = nullptr;
    NodeId mProviderNodeId                    = kUndefinedNodeId;
    FabricIndex mProviderFabricIndex          = kUndefinedFabricIndex;
    EndpointId mProviderEndpointId            = kRootEndpointId;
    uint32_t mOtaStartDelayMs                 = 0;
    CASESessionManager * mCASESessionManager  = nullptr;
    OnConnectedAction mOnConnectedAction      = kQueryImage;
    Messaging::ExchangeContext * mExchangeCtx = nullptr;
    BDXDownloader * mBdxDownloader            = nullptr; // TODO: this should be OTADownloader
    BDXMessenger mBdxMessenger;                          // TODO: ideally this is held by the application
    uint8_t mUpdateTokenBuffer[kMaxUpdateTokenLen];
    ByteSpan mUpdateToken;
    uint32_t mUpdateVersion = 0;
    Server * mServer        = nullptr;
};

} // namespace chip
