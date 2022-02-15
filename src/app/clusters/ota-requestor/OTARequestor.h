/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <app/clusters/ota-requestor/ota-requestor-server.h>
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
        kNotifyUpdateApplied,
    };

    OTARequestor() : mOnConnectedCallback(OnConnected, this), mOnConnectionFailureCallback(OnConnectionFailure, this) {}

    //////////// OTARequestorInterface Implementation ///////////////
    EmberAfStatus HandleAnnounceOTAProvider(
        app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
        const app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData) override;

    // Application directs the Requestor to start the Image Query process
    // and download the new image if available
    OTATriggerResult TriggerImmediateQuery() override;

    // Initiate download of the new image
    void DownloadUpdate() override;

    // Send ApplyImage
    void ApplyUpdate() override;

    // Send NotifyUpdateApplied, update Basic cluster SoftwareVersion attribute, log the VersionApplied event
    void NotifyUpdateApplied(uint32_t version) override;

    // Get image update progress in percents unit
    CHIP_ERROR GetUpdateProgress(EndpointId endpointId, app::DataModel::Nullable<uint8_t> & progress) override;

    // Get requestor state
    CHIP_ERROR GetState(EndpointId endpointId, app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum & state) override;

    // Application directs the Requestor to cancel image update in progress. All the Requestor state is
    // cleared, UpdateState is reset to Idle
    void CancelImageUpdate() override;

    // Retrieve the default OTA provider list as an encoded list
    CHIP_ERROR GetDefaultOtaProviderList(app::AttributeValueEncoder & encoder) override;

    // Clear all entries with the specified fabric index in the default OTA provider list
    CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex) override;

    // Add a default OTA provider to the cached list
    CHIP_ERROR AddDefaultOtaProvider(
        app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type const & providerLocation) override;

    //////////// BDXDownloader::StateDelegate Implementation ///////////////
    void OnDownloadStateChanged(OTADownloader::State state,
                                app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason) override;
    void OnUpdateProgressChanged(app::DataModel::Nullable<uint8_t> percent) override;

    /**
     * Called to perform some initialization including:
     *   - Set server instance used to get access to the system resources necessary to open CASE sessions and drive
     *     BDX transfers
     *   - Set the OTA requestor driver instance used to communicate download progress and errors
     *   - Set the BDX downloader instance used for initiating BDX downloads
     */
    void Init(Server * server, OTARequestorDriver * driver, BDXDownloader * downloader)
    {
        mServer             = server;
        mCASESessionManager = server->GetCASESessionManager();
        mOtaRequestorDriver = driver;
        mBdxDownloader      = downloader;

        uint32_t version;
        ReturnOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(version));
        mCurrentVersion = version;

        OtaRequestorServerSetUpdateState(mCurrentUpdateState);
        app::DataModel::Nullable<uint8_t> percent;
        percent.SetNull();
        OtaRequestorServerSetUpdateStateProgress(percent);
    }

    /**
     * Called to establish a session to mProviderLocation. This must be called from the same externally
     * synchronized context as any other Matter stack method.
     *
     * @param onConnectedAction  The action to take once session to provider has been established
     */
    void ConnectToProvider(OnConnectedAction onConnectedAction);

    /**
     * Called to set optional requestorCanConsent value provided by Requestor.
     */
    void SetRequestorCanConsent(bool requestorCanConsent) { mRequestorCanConsent.SetValue(requestorCanConsent); }

    // Application notifies the Requestor on the user consent action, TRUE if consent is given,
    // FALSE otherwise
    void OnUserConsent(bool result);

private:
    using QueryImageResponseDecodableType  = app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType;
    using ApplyUpdateResponseDecodableType = app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType;
    using ProviderLocationType             = app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type;
    using OTAUpdateStateEnum               = app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum;
    using OTAChangeReasonEnum              = app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;

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
     * Update the cached default OTA provider location (with a value from the default list) to use for the next periodic query
     */
    void UpdateDefaultProviderLocation(void);

    /**
     * Record the new update state by updating the corresponding server attribute and logging a StateTransition event
     */
    void RecordNewUpdateState(OTAUpdateStateEnum newState, OTAChangeReasonEnum reason);

    /**
     * Record the error update state by informing the driver of the error and calling `RecordNewUpdateState`
     */
    void RecordErrorUpdateState(UpdateFailureState failureState, CHIP_ERROR error,
                                OTAChangeReasonEnum reason = OTAChangeReasonEnum::kFailure);

    /**
     * Generate an update token using the operational node ID in case of token lost, received in QueryImageResponse
     */
    CHIP_ERROR GenerateUpdateToken();

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
     * Send NotifyUpdateApplied request
     */
    CHIP_ERROR SendNotifyUpdateAppliedRequest(OperationalDeviceProxy & deviceProxy);

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
    static void OnQueryImageFailure(void * context, CHIP_ERROR error);

    /**
     * ApplyUpdate callbacks
     */
    static void OnApplyUpdateResponse(void * context, const ApplyUpdateResponseDecodableType & response);
    static void OnApplyUpdateFailure(void * context, CHIP_ERROR error);

    /**
     * NotifyUpdateApplied callbacks
     */
    static void OnNotifyUpdateAppliedResponse(void * context, const app::DataModel::NullObjectType & response);
    static void OnNotifyUpdateAppliedFailure(void * context, CHIP_ERROR error);

    OTARequestorDriver * mOtaRequestorDriver  = nullptr;
    uint32_t mOtaStartDelayMs                 = 0;
    CASESessionManager * mCASESessionManager  = nullptr;
    OnConnectedAction mOnConnectedAction      = kQueryImage;
    Messaging::ExchangeContext * mExchangeCtx = nullptr;
    BDXDownloader * mBdxDownloader            = nullptr; // TODO: this should be OTADownloader
    BDXMessenger mBdxMessenger;                          // TODO: ideally this is held by the application
    uint8_t mUpdateTokenBuffer[kMaxUpdateTokenLen];
    ByteSpan mUpdateToken;
    uint32_t mCurrentVersion               = 0;
    uint32_t mTargetVersion                = 0;
    OTAUpdateStateEnum mCurrentUpdateState = OTAUpdateStateEnum::kIdle;
    Server * mServer                       = nullptr;
    chip::Optional<bool> mRequestorCanConsent;
    ObjectPool<ProviderLocationType, CHIP_CONFIG_MAX_FABRICS> mDefaultOtaProviderList;
    Optional<ProviderLocationType> mProviderLocation; // Provider location used for the current update in progress
};

} // namespace chip
