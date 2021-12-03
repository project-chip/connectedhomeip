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

#include "BDXDownloader.h"
#include "OTARequestorDriver.h"
#include "OTARequestorInterface.h"
#include <app/CASESessionManager.h>
#include <protocols/bdx/BdxMessages.h>

// This class implements all of the core logic of the OTA Requestor
class OTARequestor : public OTARequestorInterface
{
public:
    // Application interface declarations -- start

    // Application directs the Requestor to start the Image Query process
    // and download the new image if available
    void TriggerImmediateQuery();

    // A setter for the delegate class pointer
    void SetOtaRequestorDriver(OTARequestorDriver * driver) { mOtaRequestorDriver = driver; }

    // TODO: this should really be OTADownloader, but right now OTARequestor has information that we need to initialize a
    // BDXDownloader specifically.
    // The BDXDownloader instance should already have the ImageProcessingDelegate set.
    void SetBDXDownloader(chip::BDXDownloader * downloader) { mBdxDownloader = downloader; }

    // Application directs the Requestor to abort any processing related to
    // the image update
    void AbortImageUpdate();

    // Application interface declarations -- end

    // Virtual functions from OTARequestorInterface start
    // Handler for the AnnounceOTAProvider command
    EmberAfStatus HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData);

    // Virtual functions from OTARequestorInterface -- end
    void ConnectToProvider();

    void mOnConnected(void * context, chip::DeviceProxy * deviceProxy);
    void mOnQueryImageResponse(
        void * context,
        const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType & response);

    // When the Requestor is used as a test tool (Tesm Mode) the Provider parameters may be supplied explicitly
    void TestModeSetProviderParameters(chip::NodeId NodeId, chip::FabricIndex FabIndex)
    {
        mProviderNodeId      = NodeId;
        mProviderFabricIndex = FabIndex;
    }

    // Temporary until IP address resolution is implemented in the Exchange Layer
    void SetIpAddress(chip::Inet::IPAddress IpAddress) { mIpAddress = IpAddress; }

private:
    // Enums
    // Various cases for when OnConnected callback could be called
    enum OnConnectedState
    {
        kQueryImage = 0,
        kStartBDX,
    };

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

    // Variables
    // TODO: align on variable naming standard
    OTARequestorDriver * mOtaRequestorDriver;
    chip::NodeId mProviderNodeId;
    chip::FabricIndex mProviderFabricIndex;
    uint32_t mOtaStartDelayMs                      = 0;
    chip::CASESessionManager * mCASESessionManager = nullptr;
    OnConnectedState onConnectedState              = kQueryImage;
    chip::Messaging::ExchangeContext * exchangeCtx = nullptr;
    chip::BDXDownloader * mBdxDownloader; // TODO: this should be OTADownloader
    BDXMessenger mBdxMessenger;           // TODO: ideally this is held by the application

    // TODO: Temporary until IP address resolution is implemented in the Exchange layer
    chip::Inet::IPAddress mIpAddress;

    // Functions
    CHIP_ERROR SetupCASESessionManager(chip::FabricIndex fabricIndex);
};
