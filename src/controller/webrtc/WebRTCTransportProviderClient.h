/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/CommandSender.h>
#include <app/ConcreteCommandPath.h>
#include <app/MessageDef/StatusIB.h>
#include <app/OperationalSessionSetup.h>
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-server.h>
#include <controller/python/chip/native/PyChipError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/TLV.h>

using PyObject                        = void *;
using OnCommandSenderResponseCallback = void (*)(PyObject appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                 chip::CommandId commandId, size_t index,
                                                 std::underlying_type_t<chip::Protocols::InteractionModel::Status> status,
                                                 chip::ClusterStatus clusterStatus, const uint8_t * payload, uint32_t length);
using OnCommandSenderErrorCallback    = void (*)(PyObject appContext,
                                              std::underlying_type_t<chip::Protocols::InteractionModel::Status> status,
                                              chip::ClusterStatus clusterStatus, PyChipError chiperror);
using OnCommandSenderDoneCallback     = void (*)(PyObject appContext);

class WebRTCTransportProviderClient : public chip::app::CommandSender::Callback
{
public:
    using StreamUsageEnum = chip::app::Clusters::Globals::StreamUsageEnum;

    WebRTCTransportProviderClient() :
        mOnConnectedCallback(OnDeviceConnected, this), mOnConnectionFailureCallback(OnDeviceConnectionFailure, this){};
    ~WebRTCTransportProviderClient() = default;

    // methods to be called from python
    void Init(uint32_t nodeId, uint8_t fabricIndex, uint16_t endpoint);

    PyChipError SendCommand(void * appContext, uint16_t endpointId, uint32_t clusterId, uint32_t commandId, const uint8_t * payload,
                            size_t length);

    void InitCallbacks(OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                       OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                       OnCommandSenderDoneCallback onCommandSenderDoneCallback);

    /////////// CommandSender Callback Interface /////////
    void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                    const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override;

    void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override;

    void OnDone(chip::app::CommandSender * client) override;

private:
    enum class CommandType : uint8_t
    {
        kUndefined    = 0,
        kSolicitOffer = 1,
        kProvideOffer = 2,
    };

    enum class State : uint8_t
    {
        Idle,       ///< Default state, no communication initiated yet
        Connecting, ///< Waiting for OnDeviceConnected or OnDeviceConnectionFailure callbacks to be called
    };

    chip::ScopedNodeId mPeerId;
    chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
    CommandType mCommandType     = CommandType::kUndefined;
    std::unique_ptr<chip::app::CommandSender> mCommandSender;

    // Data needed to send the WebRTCTransportProvider commands
    chip::app::Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Type mSolicitOfferData;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Type mProvideOfferData;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideAnswer::Type mProvideAnswerData;
    chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideICECandidates::Type mProvideICECandidatesData;
    StreamUsageEnum mCurrentStreamUsage = StreamUsageEnum::kUnknownEnumValue;
    PyObject mAppContext                = nullptr;

    // We store the SDP here so that mProvideOfferData.sdp points to a stable buffer.
    std::string mSdpString;
    State mState = State::Idle;

    // Store the ICECandidates here to use to send asynchronously.
    std::vector<std::string> mClientICECandidates;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    OnCommandSenderResponseCallback gOnCommandSenderResponseCallback = nullptr;
    OnCommandSenderErrorCallback gOnCommandSenderErrorCallback       = nullptr;
    OnCommandSenderDoneCallback gOnCommandSenderDoneCallback         = nullptr;

    static void OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                  const chip::SessionHandle & sessionHandle);

    static void OnDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    CHIP_ERROR ProvideOffer(const uint8_t * payload, size_t length);
    CHIP_ERROR SolicitOffer(const uint8_t * payload, size_t length);

    // Command Sender Callback methods
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

    void MoveToState(const State targetState);

    void HandleProvideOfferResponse(chip::TLV::TLVReader data);
    void HandleSolicitOfferResponse(chip::TLV::TLVReader data);
};
