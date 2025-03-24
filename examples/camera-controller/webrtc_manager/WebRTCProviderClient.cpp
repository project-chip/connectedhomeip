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

#include "WebRTCProviderClient.h"
#include "WebRTCManager.h"

using namespace ::chip;
using namespace ::chip::app;

void WebRTCProviderClient::Init(Controller::DeviceCommissioner & commissioner, NodeId nodeId, EndpointId endpointId)
{
    // Ensure that mCommissioner is not already initialized
    VerifyOrDie(mCommissioner == nullptr);

    ChipLogProgress(NotSpecified, "Initilize WebRTCProviderClient");
    mCommissioner  = &commissioner;
    mDestinationId = nodeId;
    mEndpointId    = endpointId;
}

CHIP_ERROR WebRTCProviderClient::ProvideOffer(
    DataModel::Nullable<uint16_t> webRTCSessionID, std::string sdp, Clusters::WebRTCTransportProvider::StreamUsageEnum streamUsage,
    EndpointId originatingEndpointID, Optional<DataModel::Nullable<uint16_t>> videoStreamID,
    Optional<DataModel::Nullable<uint16_t>> audioStreamID,
    Optional<DataModel::List<const Clusters::WebRTCTransportProvider::Structs::ICEServerStruct::Type>> ICEServers,
    Optional<chip::CharSpan> ICETransportPolicy)
{
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(NotSpecified, "Sending ProvideOffer to node " ChipLogFormatX64, ChipLogValueX64(mDestinationId));

    mProvideOffer.webRTCSessionID       = webRTCSessionID;
    mProvideOffer.streamUsage           = streamUsage;
    mProvideOffer.originatingEndpointID = originatingEndpointID;

    mProvideOffer.videoStreamID      = videoStreamID;
    mProvideOffer.audioStreamID      = audioStreamID;
    mProvideOffer.ICEServers         = ICEServers;
    mProvideOffer.ICETransportPolicy = ICETransportPolicy;

    mSdp              = sdp;
    mProvideOffer.sdp = CharSpan(mSdp.data(), mSdp.size());

    //
    // Kick off the request
    //
    mCommandType = CommandType::kProvideOffer;
    return mCommissioner->GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void WebRTCProviderClient::OnResponse(CommandSender * client, const ConcreteCommandPath & path, const StatusIB & status,
                                      TLV::TLVReader * data)
{
    ChipLogProgress(NotSpecified, "WebRTCProviderClient: OnResponse.");

    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(NotSpecified, "Response Failure: %s", ErrorStr(error));
        return;
    }

    if (data != nullptr)
    {
        WebRTCManager::Instance().HandleCommandResponse(path, *data);
    }
}

void WebRTCProviderClient::OnError(const CommandSender * client, CHIP_ERROR error)
{
    // Handle the error, then reset mCommandSender
    ChipLogProgress(NotSpecified, "WebRTCProviderClient: OnError: Error: %s", ErrorStr(error));
}

void WebRTCProviderClient::OnDone(CommandSender * client)
{
    ChipLogProgress(NotSpecified, "WebRTCProviderClient: OnDone.");

    switch (mCommandType)
    {
    case CommandType::kSolicitOffer:
        ChipLogProgress(NotSpecified, "WebRTCProviderClient: Command SolicitOffer has been successfully processed.");
        break;

    case CommandType::kProvideOffer:
        ChipLogProgress(NotSpecified, "WebRTCProviderClient: Command ProvideOffer has been successfully processed.");
        break;

    case CommandType::kProvideAnswer:
        ChipLogProgress(NotSpecified, "WebRTCProviderClient: Command ProvideAnswer has been successfully processed.");
        break;

    case CommandType::kProvideICECandidates:
        ChipLogProgress(NotSpecified, "WebRTCProviderClient: Command ProvideICECandidates has been successfully processed.");
        break;

    case CommandType::kEndSession:
        ChipLogProgress(NotSpecified, "WebRTCProviderClient: Command EndSession has been successfully processed.");
        break;

    default:
        // We shouldn't reach here unless new commands were introduced but not handled
        ChipLogError(NotSpecified, "WebRTCProviderClient: Unknown or unhandled command type in OnDone.");
        break;
    }

    mCommandType = CommandType::kUndefined;
    mCommandSender.reset();
}

CHIP_ERROR WebRTCProviderClient::SendCommandForType(CommandType commandType, DeviceProxy * device)
{
    ChipLogProgress(NotSpecified, "Sending command with Endpoint ID: %d, Command Type: %d", mEndpointId,
                    static_cast<int>(commandType));

    switch (commandType)
    {
    case CommandType::kProvideOffer:
        return SendCommand(device, mEndpointId, Clusters::WebRTCTransportProvider::Id,
                           Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id, mProvideOffer);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void WebRTCProviderClient::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                               const SessionHandle & sessionHandle)
{
    WebRTCProviderClient * self = reinterpret_cast<WebRTCProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));

    OperationalDeviceProxy device(&exchangeMgr, sessionHandle);

    CHIP_ERROR err = self->SendCommandForType(self->mCommandType, &device);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to send WebRTCProviderClient command.");
        self->OnDone(nullptr);
    }
}

void WebRTCProviderClient::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    WebRTCProviderClient * self = reinterpret_cast<WebRTCProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    self->OnDone(nullptr);
}
