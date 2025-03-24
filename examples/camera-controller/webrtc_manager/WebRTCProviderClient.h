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
    /**
     * @brief Construct a new WebRTCProviderClient object. Also initializes callbacks
     * for device connection success/failure events.
     */
    WebRTCProviderClient() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    /**
     * @brief Initializes the WebRTCProviderClient with a DeviceCommissioner, NodeId, and EndpointId.
     *
     * @param commissioner A reference to the DeviceCommissioner managing device connections.
     * @param nodeId The Node ID of the remote device (or Matter bridge).
     * @param endpointId The endpoint on which to send commands (if not the root endpoint).
     */
    void Init(chip::Controller::DeviceCommissioner & commissioner, chip::NodeId nodeId, chip::EndpointId endpointId);

    /**
     * @brief Sends a ProvideOffer command to the remote device (WebRTCTransportProvider cluster).
     *
     * This method populates the ProvideOffer command parameters, buffers the SDP locally to ensure
     * the data remains valid for the asynchronous command flow, and then requests a device connection
     * to eventually send the command.
     *
     * @param webRTCSessionID        Nullable ID for the WebRTC session.
     * @param sdp                    The raw SDP (Session Description Protocol) data as a standard string.
     * @param streamUsage            An enum value describing how the stream is intended to be used.
     * @param originatingEndpointID  The endpoint ID that initiates the offer.
     * @param videoStreamID          Optional Video stream ID if relevant.
     * @param audioStreamID          Optional Audio stream ID if relevant.
     * @param ICEServers             Optional list of ICE server structures, if using custom STUN/TURN servers.
     * @param ICETransportPolicy     Optional policy for ICE transport (e.g., 'all', 'relay', etc.).
     *
     * @return CHIP_NO_ERROR on success, or an appropriate CHIP_ERROR on failure.
     */
    CHIP_ERROR
    ProvideOffer(chip::app::DataModel::Nullable<uint16_t> webRTCSessionID, std::string sdp,
                 chip::app::Clusters::WebRTCTransportProvider::StreamUsageEnum streamUsage, chip::EndpointId originatingEndpointID,
                 chip::Optional<chip::app::DataModel::Nullable<uint16_t>> videoStreamID,
                 chip::Optional<chip::app::DataModel::Nullable<uint16_t>> audioStreamID,
                 chip::Optional<
                     chip::app::DataModel::List<const chip::app::Clusters::WebRTCTransportProvider::Structs::ICEServerStruct::Type>>
                     ICEServers,
                 chip::Optional<chip::CharSpan> ICETransportPolicy);

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

    template <class T>
    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId, const T & value)
    {
        chip::app::CommandPathParams commandPath = { endpointId, clusterId, commandId,
                                                     (chip::app::CommandPathFlags::kEndpointIdValid) };
        mCommandSender = std::make_unique<chip::app::CommandSender>(this, device->GetExchangeManager(), false, false,
                                                                    device->GetSecureSession().Value()->AllowsLargePayload());

        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::app::CommandSender::AddRequestDataParameters addRequestDataParams(chip::NullOptional);
        ReturnErrorOnFailure(mCommandSender->AddRequestData(commandPath, value, addRequestDataParams));
        ReturnErrorOnFailure(mCommandSender->SendCommandRequest(device->GetSecureSession().Value()));

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendCommandForType(CommandType commandType, chip::DeviceProxy * device);

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    // Private data members
    chip::Controller::DeviceCommissioner * mCommissioner = nullptr;
    std::unique_ptr<chip::app::CommandSender> mCommandSender;
    chip::NodeId mDestinationId  = chip::kUndefinedNodeId;
    chip::EndpointId mEndpointId = chip::kRootEndpointId;
    CommandType mCommandType     = CommandType::kUndefined;

    // string buffer for storing the SDP so that ProvideOffer::sdp can safely reference it.
    std::string mSdp;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Type mProvideOffer;
};
