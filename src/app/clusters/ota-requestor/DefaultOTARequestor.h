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
#include <app/server/Server.h>
#include <protocols/bdx/BdxMessages.h>

#include "BDXDownloader.h"
#include "OTARequestorDriver.h"
#include "OTARequestorInterface.h"
#include "OTARequestorStorage.h"

namespace chip {

// This class implements all of the core logic of the OTA Requestor
class DefaultOTARequestor : public OTARequestorInterface, public BDXDownloader::StateDelegate
{
public:
    DefaultOTARequestor() : mOnConnectedCallback(OnConnected, this), mOnConnectionFailureCallback(OnConnectionFailure, this) {}

    //////////// OTARequestorInterface Implementation ///////////////
    void Reset(void) override;

    void HandleAnnounceOTAProvider(
        app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
        const app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType & commandData) override;

    // Application API to send the QueryImage command and start the image update process with the next available Provider
    CHIP_ERROR TriggerImmediateQuery(FabricIndex fabricIndex) override;

    // Internal API meant for use by OTARequestorDriver to send the QueryImage command and start the image update process
    // with the Provider currently set
    void TriggerImmediateQueryInternal() override;

    // Initiate download of the new image
    void DownloadUpdate() override;

    // Set the requestor state to kDelayedOnUserConsent
    void DownloadUpdateDelayedOnUserConsent() override;

    // Initiate the session to send ApplyUpdateRequest command
    void ApplyUpdate() override;

    // Initiate the session to send NotifyUpdateApplied command
    void NotifyUpdateApplied() override;

    // Get the value of the UpdateStateProgress attribute (in percentage) of the OTA Software Update Requestor Cluster on the given
    // endpoint
    CHIP_ERROR GetUpdateStateProgressAttribute(EndpointId endpointId, app::DataModel::Nullable<uint8_t> & progress) override;

    // Get the value of the UpdateState attribute of the OTA Software Update Requestor Cluster on the given endpoint
    CHIP_ERROR GetUpdateStateAttribute(EndpointId endpointId, OTAUpdateStateEnum & state) override;

    // Get the current state of the OTA update
    OTAUpdateStateEnum GetCurrentUpdateState() override { return mCurrentUpdateState; }

    // Get the target version of the OTA update
    uint32_t GetTargetVersion() override { return mTargetVersion; }

    // Application directs the Requestor to cancel image update in progress. All the Requestor state is
    // cleared, UpdateState is reset to Idle
    void CancelImageUpdate() override;

    // Clear all entries with the specified fabric index in the default OTA provider list
    CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex) override;

    void SetCurrentProviderLocation(ProviderLocationType providerLocation) override
    {
        mProviderLocation.SetValue(providerLocation);
    }

    void GetProviderLocation(Optional<ProviderLocationType> & providerLocation) override { providerLocation = mProviderLocation; }

    // Set the metadata value for the provider to be used in the next query and OTA update process
    // NOTE: Does not persist across reboot.
    void SetMetadataForProvider(ByteSpan metadataForProvider) override { mMetadataForProvider.SetValue(metadataForProvider); }

    // Add a default OTA provider to the cached list
    CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType & providerLocation) override;

    // Retrieve an iterator to the cached default OTA provider list
    ProviderLocationList::Iterator GetDefaultOTAProviderListIterator(void) override { return mDefaultOtaProviderList.Begin(); }

    //////////// BDXDownloader::StateDelegate Implementation ///////////////
    void OnDownloadStateChanged(OTADownloader::State state,
                                app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason) override;
    void OnUpdateProgressChanged(app::DataModel::Nullable<uint8_t> percent) override;

    //////////// DefaultOTARequestor public APIs ///////////////

    /**
     * Called to perform some initialization. Note that some states that must be initalized in the CHIP context will be deferred to
     * InitState.
     */
    CHIP_ERROR Init(Server & server, OTARequestorStorage & storage, OTARequestorDriver & driver, BDXDownloader & downloader);

private:
    using QueryImageResponseDecodableType  = app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType;
    using ApplyUpdateResponseDecodableType = app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType;

    using OTAChangeReasonEnum = app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;

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
                !event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
            {
                sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
            }
            CHIP_ERROR err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                       event.MsgData.Retain(), sendFlags);
            if (err != CHIP_NO_ERROR)
            {
                Reset();
            }

            return err;
        }

        CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                     chip::System::PacketBufferHandle && payload) override
        {
            if (mDownloader == nullptr)
            {
                ChipLogError(BDX, "BDXDownloader instance is null, can't pass message");
                return CHIP_NO_ERROR;
            }

            mDownloader->OnMessageReceived(payloadHeader, std::move(payload));

            // For a receiver using BDX Protocol, all received messages will require a response except for a StatusReport
            if (!payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
            {
                ec->WillSendMessage();
            }

            return CHIP_NO_ERROR;
        }

        void OnResponseTimeout(chip::Messaging::ExchangeContext * ec) override
        {
            ChipLogError(BDX, "exchange timed out");
            // Null out mExchangeCtx before calling OnDownloadTimeout, in case
            // the downloader decides to call Reset() on us.  If we don't, we
            // will end up closing the exchange from Reset and then the caller
            // will close it _again_ (see API documentation for
            // OnResponseTimeout), which will lead to refcount underflow.
            mExchangeCtx = nullptr;
            if (mDownloader != nullptr)
            {
                mDownloader->OnDownloadTimeout();
            }
        }

        void OnExchangeClosing(Messaging::ExchangeContext * ec) override { mExchangeCtx = nullptr; }

        void Init(chip::BDXDownloader * downloader, chip::Messaging::ExchangeContext * ec)
        {
            mExchangeCtx = ec;
            mDownloader  = downloader;
        }

        void Reset()
        {
            VerifyOrReturn(mExchangeCtx != nullptr);
            mExchangeCtx->Close();
            mExchangeCtx = nullptr;
        }

    private:
        chip::Messaging::ExchangeContext * mExchangeCtx;
        chip::BDXDownloader * mDownloader;
    };

    /**
     * Callback to initialize states and server attributes in the CHIP context
     */
    static void InitState(intptr_t context);

    /**
     * Map a CHIP_ERROR to an IdleStateReason enum type
     */
    IdleStateReason MapErrorToIdleStateReason(CHIP_ERROR error);

    ScopedNodeId GetProviderScopedId() const
    {
        return ScopedNodeId(mProviderLocation.Value().providerNodeID, mProviderLocation.Value().fabricIndex);
    }

    /**
     * Record the new update state by updating the corresponding server attribute and logging a StateTransition event
     */
    void RecordNewUpdateState(OTAUpdateStateEnum newState, OTAChangeReasonEnum reason, CHIP_ERROR error = CHIP_NO_ERROR);

    /**
     * Record the error update state and transition to the idle state
     */
    void RecordErrorUpdateState(CHIP_ERROR error, OTAChangeReasonEnum reason = OTAChangeReasonEnum::kFailure);

    /**
     * Generate an update token using the operational node ID in case of token lost, received in QueryImageResponse
     */
    CHIP_ERROR GenerateUpdateToken();

    /**
     * Send QueryImage request using values matching Basic cluster
     */
    CHIP_ERROR SendQueryImageRequest(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    /**
     * Validate and extract mandatory information from QueryImageResponse
     */
    CHIP_ERROR ExtractUpdateDescription(const QueryImageResponseDecodableType & response, UpdateDescription & update) const;

    // Various actions to take when OnConnected callback is called
    enum OnConnectedAction
    {
        kQueryImage = 0,
        kDownload,
        kApplyUpdate,
        kNotifyUpdateApplied,
    };

    /**
     * Called to establish a session to provider indicated by mProviderLocation
     *
     * @param onConnectedAction  The action to take once session to provider has been established
     */
    void ConnectToProvider(OnConnectedAction onConnectedAction);

    /**
     * Called to tear down a session to provider indicated by mProviderLocation
     */
    void DisconnectFromProvider();

    /**
     * Start download of the software image returned in QueryImageResponse
     */
    CHIP_ERROR StartDownload(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    /**
     * Send ApplyUpdate request using values obtained from QueryImageResponse
     */
    CHIP_ERROR SendApplyUpdateRequest(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    /**
     * Send NotifyUpdateApplied request
     */
    CHIP_ERROR SendNotifyUpdateAppliedRequest(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);

    /**
     * Store current update information to KVS
     */
    void StoreCurrentUpdateInfo();

    /**
     * Load current update information to KVS
     */
    void LoadCurrentUpdateInfo();

    /**
     * Session connection callbacks
     */
    static void OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);
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

    /**
     * Commissioning callback
     */
    static void OnCommissioningCompleteRequestor(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    OTARequestorStorage * mStorage           = nullptr;
    OTARequestorDriver * mOtaRequestorDriver = nullptr;
    CASESessionManager * mCASESessionManager = nullptr;
    OnConnectedAction mOnConnectedAction     = kQueryImage;
    BDXDownloader * mBdxDownloader           = nullptr; // TODO: this should be OTADownloader
    BDXMessenger mBdxMessenger;                         // TODO: ideally this is held by the application
    uint8_t mUpdateTokenBuffer[kMaxUpdateTokenLen];
    Optional<ByteSpan> mMetadataForProvider;
    ByteSpan mUpdateToken;
    uint32_t mCurrentVersion = 0;
    uint32_t mTargetVersion  = 0;
    char mFileDesignatorBuffer[bdx::kMaxFileDesignatorLen];
    CharSpan mFileDesignator;
    OTAUpdateStateEnum mCurrentUpdateState = OTAUpdateStateEnum::kUnknown;
    Server * mServer                       = nullptr;
    ProviderLocationList mDefaultOtaProviderList;
    // Provider location used for the current/last update in progress. Note that on reboot, this value will be read from the
    // persistent storage (if available), used for sending the NotifyApplied message, and then cleared. This will ensure determinism
    // in the OTARequestorDriver on reboot.
    Optional<ProviderLocationType> mProviderLocation;
    SessionHolder mSessionHolder;
};

} // namespace chip
