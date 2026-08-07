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

#include <app/clusters/av-analysis-server/DefaultAvAnalysisCameraClient.h>

#include <clusters/CameraAvStreamManagement/Commands.h>
#include <clusters/CameraAvStreamManagement/Ids.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::CameraAvStreamManagement;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace {

// VideoStreamAllocate fields other than StreamUsage are implementation defined (spec 11.9.8). These
// defaults request a modest H.264 stream suitable for analysis; they are constraints, the camera picks
// the actual stream parameters within them.
constexpr uint16_t kDefaultMinFrameRate                 = 15;
constexpr uint16_t kDefaultMaxFrameRate                 = 30;
constexpr uint16_t kDefaultMinResolutionWidth           = 640;
constexpr uint16_t kDefaultMinResolutionHeight          = 480;
constexpr uint16_t kDefaultMaxResolutionWidth           = 1920;
constexpr uint16_t kDefaultMaxResolutionHeight          = 1080;
constexpr uint32_t kDefaultMinBitRateBps                = 500000;
constexpr uint32_t kDefaultMaxBitRateBps                = 4000000;
constexpr uint16_t kDefaultKeyFrameIntervalMilliseconds = 4000;

} // namespace

CHIP_ERROR DefaultAvAnalysisCameraClient::Init(CASESessionManager * aCASESessionManager, EndpointId aCameraEndpoint)
{
    VerifyOrReturnError(aCASESessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aCameraEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    mCASESessionManager = aCASESessionManager;
    mCameraEndpoint     = aCameraEndpoint;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode, Callback & aCallback)
{
    return StartRequest(PendingRequest::kAllocate, aCameraNode, 0, aCallback);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode,
                                                                         uint16_t aAnalysisStreamId, Callback & aCallback)
{
    return StartRequest(PendingRequest::kDeallocate, aCameraNode, aAnalysisStreamId, aCallback);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::StartRequest(PendingRequest aRequest, const ScopedNodeId & aCameraNode,
                                                       uint16_t aAnalysisStreamId, Callback & aCallback)
{
    VerifyOrReturnError(mCASESessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingRequest == PendingRequest::kNone, CHIP_ERROR_BUSY);

    mPendingRequest    = aRequest;
    mPendingStreamId   = aAnalysisStreamId;
    mPendingCallback   = &aCallback;
    mResponseDelivered = false;

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisCameraClient::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                      const SessionHandle & sessionHandle)
{
    auto * self    = static_cast<DefaultAvAnalysisCameraClient *>(context);
    CHIP_ERROR err = self->SendPendingCommand(exchangeMgr, sessionHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: failed to send command: %" CHIP_ERROR_FORMAT, err.Format());
        self->FinishRequest(Status::Failure, self->mPendingStreamId);
    }
}

void DefaultAvAnalysisCameraClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    auto * self = static_cast<DefaultAvAnalysisCameraClient *>(context);
    ChipLogError(Zcl, "AvAnalysisCameraClient: could not reach camera node: %" CHIP_ERROR_FORMAT, error.Format());
    self->FinishRequest(Status::Failure, self->mPendingStreamId);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::SendPendingCommand(Messaging::ExchangeManager & aExchangeMgr,
                                                             const SessionHandle & aSessionHandle)
{
    mCommandSender = std::make_unique<CommandSender>(this, &aExchangeMgr);
    VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mPendingRequest == PendingRequest::kAllocate)
    {
        Commands::VideoStreamAllocate::Type request;
        request.streamUsage          = Globals::StreamUsageEnum::kAnalysis;
        request.videoCodec           = VideoCodecEnum::kH264;
        request.minFrameRate         = kDefaultMinFrameRate;
        request.maxFrameRate         = kDefaultMaxFrameRate;
        request.minResolution.width  = kDefaultMinResolutionWidth;
        request.minResolution.height = kDefaultMinResolutionHeight;
        request.maxResolution.width  = kDefaultMaxResolutionWidth;
        request.maxResolution.height = kDefaultMaxResolutionHeight;
        request.minBitRate           = kDefaultMinBitRateBps;
        request.maxBitRate           = kDefaultMaxBitRateBps;
        request.keyFrameInterval     = kDefaultKeyFrameIntervalMilliseconds;

        CommandPathParams commandPath = { mCameraEndpoint, CameraAvStreamManagement::Id, Commands::VideoStreamAllocate::Id,
                                          CommandPathFlags::kEndpointIdValid };
        ReturnErrorOnFailure(mCommandSender->AddRequestData(commandPath, request));
    }
    else
    {
        Commands::VideoStreamDeallocate::Type request;
        request.videoStreamID = mPendingStreamId;

        CommandPathParams commandPath = { mCameraEndpoint, CameraAvStreamManagement::Id, Commands::VideoStreamDeallocate::Id,
                                          CommandPathFlags::kEndpointIdValid };
        ReturnErrorOnFailure(mCommandSender->AddRequestData(commandPath, request));
    }

    return mCommandSender->SendCommandRequest(aSessionHandle);
}

void DefaultAvAnalysisCameraClient::OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                               const StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    if (mPendingRequest == PendingRequest::kAllocate)
    {
        uint16_t videoStreamId = 0;
        Status status          = aStatusIB.mStatus;

        if (status == Status::Success)
        {
            if (apData != nullptr && aPath.mCommandId == Commands::VideoStreamAllocateResponse::Id)
            {
                Commands::VideoStreamAllocateResponse::DecodableType response;
                if (DataModel::Decode(*apData, response) == CHIP_NO_ERROR)
                {
                    videoStreamId = response.videoStreamID;
                }
                else
                {
                    status = Status::Failure;
                }
            }
            else
            {
                // Success without the VideoStreamAllocateResponse payload is a protocol violation.
                status = Status::Failure;
            }
        }
        FinishRequest(status, videoStreamId);
    }
    else
    {
        FinishRequest(aStatusIB.mStatus, mPendingStreamId);
    }
}

void DefaultAvAnalysisCameraClient::OnError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    // A camera status response surfaces here as an IM status error; propagate it per spec 11.9.8.
    Status status = Status::Failure;
    if (aError.IsIMStatus())
    {
        status = StatusIB(aError).mStatus;
    }
    FinishRequest(status, mPendingStreamId);
}

void DefaultAvAnalysisCameraClient::OnDone(CommandSender * apCommandSender)
{
    // The interaction is complete; if no response/error was seen, report failure (exactly-once contract).
    if (!mResponseDelivered)
    {
        FinishRequest(Status::Failure, mPendingStreamId);
    }
    mCommandSender.reset();
}

void DefaultAvAnalysisCameraClient::FinishRequest(Status aStatus, uint16_t aStreamId)
{
    VerifyOrReturn(!mResponseDelivered && mPendingCallback != nullptr);
    mResponseDelivered = true;

    PendingRequest completed = mPendingRequest;
    Callback * callback      = mPendingCallback;
    mPendingRequest          = PendingRequest::kNone;
    mPendingCallback         = nullptr;

    if (completed == PendingRequest::kAllocate)
    {
        callback->OnVideoStreamAllocated(aStatus, aStreamId);
    }
    else
    {
        callback->OnVideoStreamDeallocated(aStatus, aStreamId);
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
