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

#include "WebRTCTransportProviderClient.h"
#include "WebRTCTransportRequestorManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CASESessionManager.h>
#include <app/InteractionModelEngine.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using WebRTCSessionStruct                              = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
static constexpr ClusterStatus kUndefinedClusterStatus = 0xFF;

void WebRTCTransportProviderClient::Init(uint32_t nodeId, uint8_t fabricIndex, uint16_t endpoint)
{
    mPeerId     = ScopedNodeId(nodeId, fabricIndex);
    mEndpointId = static_cast<EndpointId>(endpoint);
}

void WebRTCTransportProviderClient::InitCallbacks(OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                                  OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                                  OnCommandSenderDoneCallback onCommandSenderDoneCallback)
{
    gOnCommandSenderResponseCallback = onCommandSenderResponseCallback;
    gOnCommandSenderErrorCallback    = onCommandSenderErrorCallback;
    gOnCommandSenderDoneCallback     = onCommandSenderDoneCallback;
}

PyChipError WebRTCTransportProviderClient::SendCommand(void * appContext, uint16_t endpointId, uint32_t clusterId,
                                                       uint32_t commandId, const uint8_t * payload, size_t length)
{
    CHIP_ERROR error     = CHIP_NO_ERROR;
    ClusterId aClusterID = static_cast<ClusterId>(clusterId);
    VerifyOrReturnValue(aClusterID == Clusters::WebRTCTransportProvider::Id, ToPyChipError(CHIP_ERROR_INTERNAL),
                        ChipLogError(Camera, "Unsupported cluster ID: 0x%" PRIx32, aClusterID));
    mAppContext          = appContext; // update closure to invoke response handling
    CommandId aCommandID = static_cast<CommandId>(commandId);

    switch (aCommandID)
    {
    case Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Id:
        error = SolicitOffer(payload, length);
        break;
    case Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id:
        error = ProvideOffer(payload, length);
        break;
    default:
        ChipLogError(Camera, "Unexpected command ID: 0x%" PRIx32, aCommandID);
        error = CHIP_ERROR_INTERNAL;
        break;
    }

    return ToPyChipError(error);
}

void WebRTCTransportProviderClient::OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                                               const chip::app::StatusIB & status, chip::TLV::TLVReader * data)
{
    ChipLogProgress(Camera, "WebRTCTransportProviderClient: OnResponse received for cluster: 0x%" PRIx32 " command: 0x%" PRIx32,
                    path.mClusterId, path.mCommandId);

    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Camera, "Response Failure: %s", ErrorStr(error));
        this->OnError(client, error);
        return;
    }

    VerifyOrReturn(path.mClusterId == Clusters::WebRTCTransportProvider::Id,
                   ChipLogError(Camera, "Unexpected cluster ID: 0x%" PRIx32, path.mClusterId));

    if (data == nullptr)
    {
        ChipLogError(Camera, "Response failure: data pointer is null");
        this->OnError(client, CHIP_ERROR_INTERNAL);
        return;
    }

    // Handle different command responses
    switch (path.mCommandId)
    {
    case Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::Id:
        ChipLogDetail(Camera, "Processing SolicitOfferResponse");
        HandleSolicitOfferResponse(*data);
        break;

    case Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::Id:
        ChipLogDetail(Camera, "Processing ProvideOfferResponse");
        HandleProvideOfferResponse(*data);
        break;

    default:
        ChipLogDetail(Camera, "Unexpected command ID: 0x%" PRIx32, path.mCommandId);
        this->OnError(client, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    // Call python closure response callback
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t buffer[CHIP_SYSTEM_CONFIG_MAX_LARGE_BUFFER_SIZE_BYTES];
    uint32_t size = 0;
    if (data != nullptr)
    {
        // Python need to read from full TLV data the TLVReader may contain some unclean states.
        TLV::TLVWriter writer;
        writer.Init(buffer);
        err = writer.CopyContainer(TLV::AnonymousTag(), *data);
        if (err != CHIP_NO_ERROR)
        {
            this->OnError(client, err);
            return;
        }
        size = writer.GetLengthWritten();
    }
    if (gOnCommandSenderResponseCallback != nullptr && mAppContext != nullptr)
    {
        gOnCommandSenderResponseCallback(
            mAppContext, path.mEndpointId, path.mClusterId, path.mCommandId, 0, to_underlying(status.mStatus),
            status.mClusterStatus.has_value() ? *status.mClusterStatus : kUndefinedClusterStatus, buffer, size);
    }
}

void WebRTCTransportProviderClient::OnError(const chip::app::CommandSender * client, CHIP_ERROR error)
{
    ChipLogError(Camera, "WebRTCTransportProviderClient: OnError for command %u: %" CHIP_ERROR_FORMAT,
                 static_cast<unsigned>(mCommandType), error.Format());
    StatusIB status(error);
    if (gOnCommandSenderErrorCallback != nullptr && mAppContext != nullptr)
    {
        gOnCommandSenderErrorCallback(mAppContext, to_underlying(status.mStatus),
                                      status.mClusterStatus.value_or(kUndefinedClusterStatus),
                                      // If we have an actual IM status, pass 0
                                      // for the error code, because otherwise
                                      // the callee will think we have a stack
                                      // exception.
                                      error.IsIMStatus() ? ToPyChipError(CHIP_NO_ERROR) : ToPyChipError(error));
    }
}

void WebRTCTransportProviderClient::OnDone(chip::app::CommandSender * client)
{
    MoveToState(State::Idle);
    ChipLogProgress(Camera, "WebRTCTransportProviderClient: OnDone for command %u.", static_cast<unsigned>(mCommandType));
    if (gOnCommandSenderDoneCallback != nullptr && mAppContext != nullptr)
    {
        gOnCommandSenderDoneCallback(mAppContext);
    }
    // Reset python closure
    mAppContext = nullptr;
    // Reset command type, free up the CommandSender
    mCommandType = CommandType::kUndefined;
    mCommandSender.reset();
}

void WebRTCTransportProviderClient::OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                                      const chip::SessionHandle & sessionHandle)
{
    WebRTCTransportProviderClient * self = reinterpret_cast<WebRTCTransportProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnected: context is null"));

    ChipLogProgress(Camera, "CASE session established, sending WebRTCTransportProvider command...");
    self->SendCommandForType(exchangeMgr, sessionHandle, self->mCommandType);
}

void WebRTCTransportProviderClient::OnDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error)
{
    LogErrorOnFailure(error);
    WebRTCTransportProviderClient * self = reinterpret_cast<WebRTCTransportProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectionFailure: context is null"));
    self->OnError(nullptr, error);
}

CHIP_ERROR WebRTCTransportProviderClient::ProvideOffer(const uint8_t * payload, size_t length)
{
    ChipLogProgress(Camera, "Sending ProvideOffer to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));
    TLV::TLVReader data;
    data.Init(payload, length);
    data.Next();

    Clusters::WebRTCTransportProvider::Commands::ProvideOffer::DecodableType value;
    CHIP_ERROR error = value.Decode(data, mPeerId.GetFabricIndex());
    VerifyOrReturnError(error == CHIP_NO_ERROR, error,
                        ChipLogError(Camera, "Failed to decode command payload value. Error: %" CHIP_ERROR_FORMAT, error.Format()));

    if (mState != State::Idle)
    {
        ChipLogError(Camera, "Operation NOT POSSIBLE: another sync is in progress");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Store the command type
    mCommandType = CommandType::kProvideOffer;

    // Stash data in class members so the CommandSender can safely reference them async
    mProvideOfferData.webRTCSessionID       = value.webRTCSessionID;
    mProvideOfferData.sdp                   = value.sdp;
    mProvideOfferData.streamUsage           = value.streamUsage;
    mProvideOfferData.originatingEndpointID = value.originatingEndpointID;
    mProvideOfferData.videoStreamID         = value.videoStreamID;
    mProvideOfferData.audioStreamID         = value.audioStreamID;

    // ICE info are sent during the ICE candidate exchange phase of this flow.
    mProvideOfferData.ICEServers         = NullOptional;
    mProvideOfferData.ICETransportPolicy = NullOptional;

    // Store the streamUsage from the original command so we can build the WebRTCSessionStruct when the response arrives.
    mCurrentStreamUsage = value.streamUsage;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Connecting);

    // WebRTC ProvideOffer requires a large payload session establishment
    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                           TransportPayloadCapability::kLargePayload);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCTransportProviderClient::SolicitOffer(const uint8_t * payload, size_t length)
{
    ChipLogProgress(Camera, "Sending SolicitOffer to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    if (mState != State::Idle)
    {
        ChipLogError(Camera, "Operation NOT POSSIBLE: another sync is in progress");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    TLV::TLVReader data;
    data.Init(payload, length);
    data.Next();

    Clusters::WebRTCTransportProvider::Commands::SolicitOffer::DecodableType value;
    CHIP_ERROR error = value.Decode(data, mPeerId.GetFabricIndex());
    VerifyOrReturnError(error == CHIP_NO_ERROR, error,
                        ChipLogError(Camera, "Failed to decode command payload value. Error: %" CHIP_ERROR_FORMAT, error.Format()));

    // Store the command type
    mCommandType = CommandType::kSolicitOffer;
    // Stash data in class members so the CommandSender can safely reference them async
    mSolicitOfferData.streamUsage           = value.streamUsage; // streamUsage;
    mSolicitOfferData.originatingEndpointID = value.originatingEndpointID;
    mSolicitOfferData.videoStreamID         = value.videoStreamID;
    mSolicitOfferData.audioStreamID         = value.audioStreamID;

    // ICE info are sent during the ICE candidate exchange phase of this flow.
    mSolicitOfferData.ICEServers         = NullOptional;
    mSolicitOfferData.ICETransportPolicy = NullOptional;

    // Store the streamUsage from the original command so we can build the WebRTCSessionStruct when the response arrives.
    mCurrentStreamUsage = value.streamUsage;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Connecting);

    // WebRTC ProvideOffer requires a large payload session establishment
    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                           TransportPayloadCapability::kLargePayload);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCTransportProviderClient::SendCommandForType(chip::Messaging::ExchangeManager & exchangeMgr,
                                                             const chip::SessionHandle & sessionHandle, CommandType commandType)
{
    ChipLogProgress(Camera, "Sending command with Endpoint ID: %d, Command Type: %d", mEndpointId, static_cast<int>(commandType));

    switch (commandType)
    {
    case CommandType::kProvideOffer:
        return SendCommand(exchangeMgr, sessionHandle, Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id,
                           mProvideOfferData);
    case CommandType::kSolicitOffer:
        return SendCommand(exchangeMgr, sessionHandle, Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Id,
                           mSolicitOfferData);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void WebRTCTransportProviderClient::MoveToState(const State targetState)
{
    mState = targetState;
}

void WebRTCTransportProviderClient::HandleProvideOfferResponse(TLV::TLVReader data)
{
    ChipLogProgress(Camera, "WebRTCTransportProviderClient::HandleProvideOfferResponse.");

    Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    VerifyOrReturn(error == CHIP_NO_ERROR,
                   ChipLogError(Camera, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format()));

    // Create a new session record and populate fields from the decoded command response and current secure session info
    WebRTCSessionStruct session;
    session.id             = value.webRTCSessionID;
    session.peerNodeID     = mPeerId.GetNodeId();
    session.fabricIndex    = mPeerId.GetFabricIndex();
    session.peerEndpointID = mEndpointId;
    session.streamUsage    = mCurrentStreamUsage;

    // Populate optional fields for video/audio stream IDs if present; set them to Null otherwise
    session.videoStreamID = value.videoStreamID.HasValue() ? value.videoStreamID.Value() : DataModel::MakeNullable<uint16_t>();
    session.audioStreamID = value.audioStreamID.HasValue() ? value.audioStreamID.Value() : DataModel::MakeNullable<uint16_t>();

    WebRTCTransportRequestorManager::Instance().UpsertSession(session);
}

void WebRTCTransportProviderClient::HandleSolicitOfferResponse(TLV::TLVReader data)
{
    ChipLogProgress(Camera, "WebRTCTransportProviderClient::HandleSolicitOfferResponse.");

    Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    VerifyOrReturn(error == CHIP_NO_ERROR,
                   ChipLogError(Camera, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format()));

    // Create a new session record and populate fields from the decoded command response and current secure session info
    WebRTCSessionStruct session;
    session.id             = value.webRTCSessionID;
    session.peerNodeID     = mPeerId.GetNodeId();
    session.fabricIndex    = mPeerId.GetFabricIndex();
    session.peerEndpointID = mEndpointId;
    session.streamUsage    = mCurrentStreamUsage;

    // Populate optional fields for video/audio stream IDs if present; set them to Null otherwise
    session.videoStreamID = value.videoStreamID.HasValue() ? value.videoStreamID.Value() : chip::app::DataModel::NullNullable;
    session.audioStreamID = value.audioStreamID.HasValue() ? value.audioStreamID.Value() : chip::app::DataModel::NullNullable;

    // If DeferredOffer == FALSE these fields MUST be valid
    if (!value.deferredOffer)
    {
        if (session.videoStreamID.IsNull() && session.audioStreamID.IsNull())
        {
            ChipLogError(Camera, "Provider reported DeferredOffer=FALSE but did not supply valid Video and Audio stream IDs");
            return;
        }
    }

    // Insert or update the Requestor cluster's CurrentSessions.
    WebRTCTransportRequestorManager::Instance().UpsertSession(session);
}
