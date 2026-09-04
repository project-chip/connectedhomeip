/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/av-analysis-server/DefaultAvAnalysisWebRTCClient.h>

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/WebRTCTransportProvider/Ids.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR DefaultAvAnalysisWebRTCClient::Init(CASESessionManager * aCASESessionManager,
                                               AvAnalysisWebRTCPeerDelegate * aPeerDelegate,
                                               WebRTCTransportRequestor::WebRTCTransportRequestorCluster * aRequestorCluster,
                                               uint8_t aMaxSessions)
{
    VerifyOrReturnError(aCASESessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aPeerDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aRequestorCluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aMaxSessions > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mSessions, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mSessions.Calloc(aMaxSessions), CHIP_ERROR_NO_MEMORY);

    mCASESessionManager = aCASESessionManager;
    mPeerDelegate       = aPeerDelegate;
    mRequestorCluster   = aRequestorCluster;
    mMaxSessions        = aMaxSessions;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::RequestSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                                                         uint16_t aVideoStreamId, AvAnalysisWebRTCClient::Callback & aCallback)
{
    ReturnErrorOnFailure(CanStartRequest());

    mRequest.BeginProvideOffer(aWebRTCEndpoint, aVideoStreamId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::EndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                                                     uint16_t aWebRTCSessionId, AvAnalysisWebRTCClient::Callback & aCallback)
{
    // Ending a session this client tracks
    VerifyOrReturnError(FindTrackedSession(aWebRTCSessionId) != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CanStartRequest());

    mRequest.BeginEndSession(aWebRTCEndpoint, aWebRTCSessionId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisWebRTCClient::Cancel()
{
    VerifyOrReturn(mRequest.InFlight());

    // Deregister from a session establishment still in progress
    mOnConnectedCallback.Cancel();
    mOnConnectionFailureCallback.Cancel();

    // Abort a provider check or command exchange still in progress; callbacks die with them
    ResetReadClient();
    mCommandSender.reset();

    // Forget the request without delivering a completion
    mRequest.Reset();
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::CanStartRequest() const
{
    VerifyOrReturnError(mCASESessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mRequest.InFlight(), CHIP_ERROR_BUSY);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisWebRTCClient::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                      const SessionHandle & sessionHandle)
{
    auto * self = static_cast<DefaultAvAnalysisWebRTCClient *>(context);
    VerifyOrReturn(self->mRequest.InPhase(Request::Phase::kConnecting),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected session establishment"));

    self->mRequest.HoldSession(sessionHandle, exchangeMgr);

    if (self->mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        // TODO: send EndSession{UserHangup} on the held session
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: EndSession sending is not implemented yet");
        self->FinishRequest(Status::Failure, self->mRequest.WebRTCSessionId());
        return;
    }

    // the provided endpoint must host WebRTCTransportProvider before a session may be initiated toward it
    self->mRequest.Advance(Request::Phase::kCheckingProvider);
    CHIP_ERROR err = self->SendProviderCheckRead();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: provider check not started: %" CHIP_ERROR_FORMAT, err.Format());
        self->FinishRequest(Status::Failure, 0);
    }
}

void DefaultAvAnalysisWebRTCClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    auto * self = static_cast<DefaultAvAnalysisWebRTCClient *>(context);
    VerifyOrReturn(self->mRequest.InPhase(Request::Phase::kConnecting));

    ChipLogError(Zcl, "AvAnalysisWebRTCClient: could not reach camera node: %" CHIP_ERROR_FORMAT, error.Format());
    self->FinishRequest(Status::Failure, self->mRequest.WebRTCSessionId());
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendProviderCheckRead()
{
    VerifyOrReturnError(mRequest.HasSession(), CHIP_ERROR_INCORRECT_STATE);
    // A ReadClient may only be destroyed from its OnDone
    VerifyOrReturnError(!mReadClient, CHIP_ERROR_INCORRECT_STATE);

    AttributePathParams readPath(mRequest.WebRTCEndpoint(), Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    ReadPrepareParams readParams(mRequest.Session().Value());
    readParams.mpAttributePathParamsList    = &readPath;
    readParams.mAttributePathParamsListSize = 1;

    mReadCallback = Platform::MakeUnique<BufferedReadCallback>(*this);
    VerifyOrReturnError(mReadCallback != nullptr, CHIP_ERROR_NO_MEMORY);

    mReadClient = Platform::MakeUnique<ReadClient>(InteractionModelEngine::GetInstance(), &mRequest.ExchangeManager(),
                                                   *mReadCallback, ReadClient::InteractionType::Read);
    if (mReadClient == nullptr)
    {
        mReadCallback.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = mReadClient->SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        ResetReadClient();
    }
    return err;
}

void DefaultAvAnalysisWebRTCClient::ResetReadClient()
{
    // The ReadClient holds the buffered callback by reference, so it goes first
    mReadClient.reset();
    mReadCallback.reset();
}

void DefaultAvAnalysisWebRTCClient::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                    const StatusIB & aStatus)
{
    VerifyOrReturn(aStatus.IsSuccess() && apData != nullptr);
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCheckingProvider));

    HandleServerListReport(aPath, *apData);
}

void DefaultAvAnalysisWebRTCClient::HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData)
{
    VerifyOrReturn(aPath.mClusterId == Descriptor::Id && aPath.mAttributeId == Descriptor::Attributes::ServerList::Id &&
                   aPath.mEndpointId == mRequest.WebRTCEndpoint());

    DataModel::DecodableList<ClusterId> serverList;
    VerifyOrReturn(DataModel::Decode(aData, serverList) == CHIP_NO_ERROR);

    auto iter = serverList.begin();
    while (iter.Next())
    {
        if (iter.GetValue() == WebRTCTransportProvider::Id)
        {
            mRequest.SetProviderFound(true);
            return;
        }
    }
}

void DefaultAvAnalysisWebRTCClient::OnError(CHIP_ERROR aError)
{
    ChipLogError(Zcl, "AvAnalysisWebRTCClient: provider check error: %" CHIP_ERROR_FORMAT, aError.Format());
}

void DefaultAvAnalysisWebRTCClient::OnDone(ReadClient * apReadClient)
{
    ResetReadClient();

    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCheckingProvider),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected read completion"));

    OnProviderCheckComplete();
}

void DefaultAvAnalysisWebRTCClient::OnProviderCheckComplete()
{
    if (!mRequest.ProviderFound())
    {
        // no WebRTCTransportProvider on the endpoint the command named
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: no WebRTCTransportProvider on endpoint %u", mRequest.WebRTCEndpoint());
        FinishRequest(Status::NotFound, 0);
        return;
    }

    // TODO: request the SDP offer from the peer delegate and send ProvideOffer
    ChipLogError(Zcl, "AvAnalysisWebRTCClient: offer sending is not implemented yet");
    FinishRequest(Status::Failure, 0);
}

// TODO: buffer the SDP and send ProvideOffer
void DefaultAvAnalysisWebRTCClient::OnOfferReady(CHIP_ERROR aError, CharSpan aSdp)
{
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCreatingOffer),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected offer delivery"));
}

// TODO: decode ProvideOfferResponse / propagate the EndSession status
void DefaultAvAnalysisWebRTCClient::OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                               const StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: response for an interaction already finished with"));
}

void DefaultAvAnalysisWebRTCClient::OnError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: error for an interaction already finished with"));

    // A camera status response
    Status status = Status::Failure;
    if (aError.IsIMStatus())
    {
        status = StatusIB(aError).mStatus;
    }
    FinishRequest(status, mRequest.WebRTCSessionId());
}

void DefaultAvAnalysisWebRTCClient::OnDone(CommandSender * apCommandSender)
{
    const bool isOurs = mRequest.WasInvokedBy(apCommandSender);

    // OnDone needs to destroy the sender, and doing it before delivering any
    // completion leaves the client free to accept a request started from that completion.
    if (mCommandSender.get() == apCommandSender)
    {
        mCommandSender.reset();
    }

    // The interaction is over: with no response or error seen
    if (isOurs)
    {
        FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
    }
}

// TODO: route the signal to the tracked session's callback
void DefaultAvAnalysisWebRTCClient::NotifyAnswered(uint16_t aWebRTCSessionId) {}
void DefaultAvAnalysisWebRTCClient::NotifyFailed(uint16_t aWebRTCSessionId) {}
void DefaultAvAnalysisWebRTCClient::NotifyEnded(uint16_t aWebRTCSessionId) {}

void DefaultAvAnalysisWebRTCClient::FinishRequest(Status aStatus, uint16_t aWebRTCSessionId)
{
    // TakeCallback() nulls the callback, so a second completion for the same request is a no-op
    AvAnalysisWebRTCClient::Callback * callback = mRequest.TakeCallback();
    VerifyOrReturn(callback != nullptr);

    const Request::CommandType completed = mRequest.GetCommandType();
    mRequest.Reset();

    if (completed == Request::CommandType::kProvideOffer)
    {
        callback->OnSessionInitiated(aStatus, aWebRTCSessionId);
    }
    else
    {
        callback->OnSessionEnded(aStatus, aWebRTCSessionId);
    }
}

DefaultAvAnalysisWebRTCClient::TrackedSession * DefaultAvAnalysisWebRTCClient::FindTrackedSession(uint16_t aWebRTCSessionId)
{
    for (uint8_t i = 0; i < mMaxSessions; i++)
    {
        if (mSessions[i].inUse && mSessions[i].webRTCSessionId == aWebRTCSessionId)
        {
            return &mSessions[i];
        }
    }
    return nullptr;
}

} // namespace Clusters
} // namespace app
} // namespace chip
