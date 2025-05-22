/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandSender.h>
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-server.h>
#include <controller/CHIPDeviceController.h>

/**
 * @brief This class handles sending CHIP commands for WebRTCTransportProvider cluster, including
 * sending a ProvideOffer command to a remote camera device.
 *
 * The class acts as a command sender and implements the `chip::app::CommandSender::Callback` interface
 * to handle responses, errors, and completion events for the commands it sends. It relies on a
 * DeviceCommissioner to manage the lower-level device connection logic.
 */
class WebRTCProviderClient : public chip::app::CommandSender::Callback
{
public:
    using ICECandidateStruct = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
    using StreamUsageEnum    = chip::app::Clusters::Globals::StreamUsageEnum;

    /**
     * @brief Construct a new WebRTCProviderClient object. Also initializes callbacks
     * for device connection success/failure events.
     */
    WebRTCProviderClient() :
        mOnConnectedCallback(OnDeviceConnected, this), mOnConnectionFailureCallback(OnDeviceConnectionFailure, this)
    {}

    /**
     * @brief Initializes the WebRTCProviderClient with a ScopedNodeId, an EndpointId, and an optional
     *        pointer to the WebRTCTransportRequestorServer.
     *
     * @param peerId              The PeerId (fabric + nodeId) for the remote device.
     * @param endpointId          The Matter endpoint on the remote device for WebRTCTransportProvider cluster.
     * @param requestorServer     Pointer to a WebRTCTransportRequestorServer instance.
     */
    void Init(const chip::ScopedNodeId & peerId, chip::EndpointId endpointId,
              chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorServer * requestorServer);

    /**
     * @brief Sends a SolicitOffer command to the remote device.
     *
     * This method populates the SolicitOffer command parameters, requests that the Provider initiates
     * a new session with the Offer / Answer flow.
     *
     * @param streamUsage            An enum value describing how the stream is intended to be used.
     * @param originatingEndpointId  The endpoint ID that initiates the offer.
     * @param videoStreamId          Optional Video stream ID if relevant.
     * @param audioStreamId          Optional Audio stream ID if relevant.
     *
     * @return CHIP_NO_ERROR on success, or an appropriate CHIP_ERROR on failure.
     */
    CHIP_ERROR
    SolicitOffer(StreamUsageEnum streamUsage, chip::EndpointId originatingEndpointId,
                 chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamId,
                 chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamId);

    /**
     * @brief Sends a ProvideOffer command to the remote device.
     *
     * This method populates the ProvideOffer command parameters, buffers the SDP locally to ensure
     * the data remains valid for the asynchronous command flow, and then requests a device connection
     * to eventually send the command.
     *
     * @param webRTCSessionId        Nullable ID for the WebRTC session.
     * @param sdp                    The raw SDP (Session Description Protocol) data as a standard string.
     * @param streamUsage            An enum value describing how the stream is intended to be used.
     * @param originatingEndpointId  The endpoint ID that initiates the offer.
     * @param videoStreamId          Optional Video stream ID if relevant.
     * @param audioStreamId          Optional Audio stream ID if relevant.
     *
     * @return CHIP_NO_ERROR on success, or an appropriate CHIP_ERROR on failure.
     */
    CHIP_ERROR
    ProvideOffer(chip::app::DataModel::Nullable<uint16_t> webRTCSessionId, std::string sdp, StreamUsageEnum streamUsage,
                 chip::EndpointId originatingEndpointId, chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamId,
                 chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamId);

    /**
     * @brief Sends a ProvideAnswer command to the remote device.
     *
     * Invoke this after you have received an *Offer* and generated the local SDP answer for the same WebRTC session.
     *
     * @param webRTCSessionId   The unique identifier for the established WebRTC session negotiated in the earlier *Offer*.
     * @param sdp               The raw SDP (Session Description Protocol) data as a standard string.
     *
     * @return CHIP_NO_ERROR on success, or an appropriate CHIP_ERROR on failure.
     */
    CHIP_ERROR ProvideAnswer(uint16_t webRTCSessionId, const std::string & sdp);

    /**
     * @brief Sends a ProvideICECandidates command to the remote device.
     *
     * This method populates the ProvideICECandidates command parameters, packages the provided ICE
     * candidate strings, and queues them for sending to the target device. This is typically used
     * to inform the remote side about potential network endpoints it can use to establish or
     * enhance a WebRTC session.
     *
     * @param webRTCSessionId   The unique identifier for the WebRTC session to which these
     *                          ICE candidates apply.
     * @param ICECandidates     A list of ICE candidate structs.
     *
     * @return CHIP_NO_ERROR on success, or an appropriate CHIP_ERROR on failure.
     */
    CHIP_ERROR ProvideICECandidates(uint16_t webRTCSessionId, const std::vector<std::string> & iceCandidates);

    /**
     * @brief Notify WebRTCProviderClient that the Offer command has been received.
     *
     * Hanlde the stream requestor with WebRTC session details. It is sent following the receipt of a SolicitOffer
     * command or a re-Offer initiated by the Provider.
     *
     * @param webRTCSessionId   The unique identifier for the WebRTC session.
     */
    void HandleOfferReceived(uint16_t webRTCSessionId);

    /**
     * @brief Notify WebRTCProviderClient that the Answer command has been received.
     *
     * This allows the client to take appropriate transitions upon receiving
     * confirmation that the remote decryptor has been received.
     *
     * @param webRTCSessionId   The unique identifier for the WebRTC session.
     */
    void HandleAnswerReceived(uint16_t webRTCSessionId);

    /////////// CommandSender Callback Interface /////////
    virtual void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                            const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override;

    virtual void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override;

    virtual void OnDone(chip::app::CommandSender * client) override;

private:
    static constexpr uint16_t kMaxDeviceLabelLength = 64;

    enum class CommandType : uint8_t
    {
        kUndefined            = 0,
        kSolicitOffer         = 1,
        kProvideOffer         = 2,
        kProvideAnswer        = 3,
        kProvideICECandidates = 4,
        kEndSession           = 5,
    };

    enum class State : uint8_t
    {
        Idle,             ///< Default state, no communication initiated yet
        Connecting,       ///< Waiting for OnDeviceConnected or OnDeviceConnectionFailure callbacks to be called
        AwaitingResponse, ///< Waiting for command response from camera
        AwaitingOffer,    ///< Waiting for Offer command from camera
        AwaitingAnswer,   ///< Waiting for Answer command from camera
    };

    void MoveToState(const State targetState);
    const char * GetStateStr() const;

    template <class T>
    CHIP_ERROR SendCommand(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle,
                           chip::CommandId commandId, const T & requestData)
    {
        mCommandSender =
            std::make_unique<chip::app::CommandSender>(this, &exchangeMgr,
                                                       /* isTimedRequest = */ false,
                                                       /* suppressResponse   = */ false, sessionHandle->AllowsLargePayload());
        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::app::CommandPathParams commandPath = { mEndpointId, chip::app::Clusters::WebRTCTransportProvider::Id, commandId,
                                                     chip::app::CommandPathFlags::kEndpointIdValid };

        chip::app::CommandSender::AddRequestDataParameters addRequestDataParams(chip::NullOptional);
        ReturnErrorOnFailure(mCommandSender->AddRequestData(commandPath, requestData, addRequestDataParams));

        return mCommandSender->SendCommandRequest(sessionHandle);
    }

    CHIP_ERROR SendCommandForType(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle,
                                  CommandType commandType);

    static void OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                  const chip::SessionHandle & sessionHandle);

    static void OnDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    static void OnSessionEstablishTimeout(chip::System::Layer * systemLayer, void * appState);

    void HandleSolicitOfferResponse(chip::TLV::TLVReader & data);

    void HandleProvideOfferResponse(chip::TLV::TLVReader & data);

    // Private data members
    chip::ScopedNodeId mPeerId;
    chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
    CommandType mCommandType     = CommandType::kUndefined;
    uint16_t mCurrentSessionId   = 0;
    std::unique_ptr<chip::app::CommandSender> mCommandSender;

    State mState = State::Idle;

    chip::app::Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorServer * mRequestorServer = nullptr;

    // Data needed to send the WebRTCTransportProvider commands
    chip::app::Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Type mSolicitOfferData;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Type mProvideOfferData;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideAnswer::Type mProvideAnswerData;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideICECandidates::Type mProvideICECandidatesData;
    StreamUsageEnum mCurrentStreamUsage = StreamUsageEnum::kUnknownEnumValue;

    // We store the SDP here so that mProvideOfferData.sdp points to a stable buffer.
    std::string mSdpString;

    // Store the ICECandidates here to use to send asynchronously.
    std::vector<std::string> mClientICECandidates;
    std::vector<ICECandidateStruct> mICECandidateStructList;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};
