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

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <clusters/CameraAvStreamManagement/Commands.h>
#include <clusters/CameraAvStreamManagement/Ids.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/Ids.h>
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
constexpr uint32_t kDefaultMaxBitRateBps                = 2000000;
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

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode,
                                                                       AvAnalysisCameraClient::Callback & aCallback)
{
    return StartRequest(PendingRequest::kAllocate, aCameraNode, 0, aCallback);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode,
                                                                         uint16_t aAnalysisStreamId,
                                                                         AvAnalysisCameraClient::Callback & aCallback)
{
    return StartRequest(PendingRequest::kDeallocate, aCameraNode, aAnalysisStreamId, aCallback);
}

void DefaultAvAnalysisCameraClient::Cancel()
{
    VerifyOrReturn(mPendingRequest != PendingRequest::kNone);

    // Deregister from a session establishment still in progress
    mOnConnectedCallback.Cancel();
    mOnConnectionFailureCallback.Cancel();

    // Abort a discovery read or command exchange still in progress; callbacks die with them
    mReadClient.reset();
    mCommandSender.reset();

    // Forget the pending request without delivering a completion
    mPendingRequest    = PendingRequest::kNone;
    mPendingCallback   = nullptr;
    mResponseDelivered = false;
    mProfile           = CameraProfile{};
    mSessionHolder.Release();
    mExchangeMgr = nullptr;
}

CHIP_ERROR DefaultAvAnalysisCameraClient::StartRequest(PendingRequest aRequest, const ScopedNodeId & aCameraNode,
                                                       uint16_t aAnalysisStreamId, AvAnalysisCameraClient::Callback & aCallback)
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

void DefaultAvAnalysisCameraClient::StartProfileDiscovery()
{
    // Baseline from configuration; discovered values override below
    FillProfileFromConfiguration(mProfile);
    mProfile.avsmEndpoint = kInvalidEndpointId;
    mDiscoveryError       = CHIP_NO_ERROR;

    VerifyOrReturn(mSessionHolder && mExchangeMgr != nullptr, OnProfileDiscoveryComplete(CHIP_ERROR_INCORRECT_STATE));

    // Which endpoint on the camera hosts CameraAVStreamManagement? Read every endpoint's Descriptor
    // ServerList (wildcard endpoint) and look for the cluster id.
    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(Descriptor::Id, Descriptor::Attributes::ServerList::Id);

    ReadPrepareParams readParams(mSessionHolder.Get().Value());
    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;

    mReadClient = Platform::MakeUnique<ReadClient>(InteractionModelEngine::GetInstance(), mExchangeMgr, mBufferedReadCallback,
                                                   ReadClient::InteractionType::Read);
    VerifyOrReturn(mReadClient != nullptr, OnProfileDiscoveryComplete(CHIP_ERROR_NO_MEMORY));

    CHIP_ERROR err = mReadClient->SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        mReadClient.reset();
        OnProfileDiscoveryComplete(err);
    }
}

void DefaultAvAnalysisCameraClient::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                    const StatusIB & aStatus)
{
    VerifyOrReturn(aPath.mClusterId == Descriptor::Id && aPath.mAttributeId == Descriptor::Attributes::ServerList::Id);
    VerifyOrReturn(aStatus.IsSuccess() && apData != nullptr);
    // First matching endpoint wins
    VerifyOrReturn(mProfile.avsmEndpoint == kInvalidEndpointId);

    DataModel::DecodableList<ClusterId> serverList;
    VerifyOrReturn(DataModel::Decode(*apData, serverList) == CHIP_NO_ERROR);

    auto iter = serverList.begin();
    while (iter.Next())
    {
        if (iter.GetValue() == CameraAvStreamManagement::Id)
        {
            mProfile.avsmEndpoint = aPath.mEndpointId;
            return;
        }
    }
}

void DefaultAvAnalysisCameraClient::OnError(CHIP_ERROR aError)
{
    mDiscoveryError = aError;
}

void DefaultAvAnalysisCameraClient::OnDone(ReadClient * apReadClient)
{
    mReadClient.reset();

    if (mProfile.avsmEndpoint == kInvalidEndpointId)
    {
        ChipLogProgress(Zcl,
                        "AvAnalysisCameraClient: CameraAVStreamManagement endpoint not discovered (%" CHIP_ERROR_FORMAT
                        "), using configured endpoint %u",
                        mDiscoveryError.Format(), mCameraEndpoint);
        mProfile.avsmEndpoint = mCameraEndpoint;
    }

    OnProfileDiscoveryComplete(CHIP_NO_ERROR);
}

void DefaultAvAnalysisCameraClient::OnProfileDiscoveryComplete(CHIP_ERROR aError)
{
    if (aError != CHIP_NO_ERROR || !mSessionHolder || mExchangeMgr == nullptr)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: profile discovery failed: %" CHIP_ERROR_FORMAT, aError.Format());
        FinishRequest(Status::Failure, mPendingStreamId);
        return;
    }

    CHIP_ERROR err = SendPendingCommand(*mExchangeMgr, mSessionHolder.Get().Value());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: failed to send command: %" CHIP_ERROR_FORMAT, err.Format());
        FinishRequest(Status::Failure, mPendingStreamId);
    }
}

void DefaultAvAnalysisCameraClient::FillProfileFromConfiguration(CameraProfile & outProfile) const
{
    outProfile.avsmEndpoint  = mCameraEndpoint;
    outProfile.hasWatermark  = mCameraHasWatermark;
    outProfile.hasOSD        = mCameraHasOSD;
    outProfile.minFrameRate  = kDefaultMinFrameRate;
    outProfile.maxFrameRate  = kDefaultMaxFrameRate;
    outProfile.minWidth      = kDefaultMinResolutionWidth;
    outProfile.minHeight     = kDefaultMinResolutionHeight;
    outProfile.maxWidth      = kDefaultMaxResolutionWidth;
    outProfile.maxHeight     = kDefaultMaxResolutionHeight;
    outProfile.minBitRateBps = kDefaultMinBitRateBps;
    outProfile.maxBitRateBps = kDefaultMaxBitRateBps;
}

Commands::VideoStreamAllocate::Type DefaultAvAnalysisCameraClient::BuildAllocateRequest(const CameraProfile & aProfile)
{
    Commands::VideoStreamAllocate::Type request;
    request.streamUsage          = Globals::StreamUsageEnum::kAnalysis;
    request.videoCodec           = VideoCodecEnum::kH264;
    request.minFrameRate         = aProfile.minFrameRate;
    request.maxFrameRate         = aProfile.maxFrameRate;
    request.minResolution.width  = aProfile.minWidth;
    request.minResolution.height = aProfile.minHeight;
    request.maxResolution.width  = aProfile.maxWidth;
    request.maxResolution.height = aProfile.maxHeight;
    request.minBitRate           = aProfile.minBitRateBps;
    request.maxBitRate           = aProfile.maxBitRateBps;
    request.keyFrameInterval     = kDefaultKeyFrameIntervalMilliseconds;

    // Feature-conditional fields: present exactly when the camera supports the feature.
    if (aProfile.hasWatermark)
    {
        request.watermarkEnabled = MakeOptional(false);
    }
    if (aProfile.hasOSD)
    {
        request.OSDEnabled = MakeOptional(false);
    }

    return request;
}

void DefaultAvAnalysisCameraClient::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                      const SessionHandle & sessionHandle)
{
    auto * self        = static_cast<DefaultAvAnalysisCameraClient *>(context);
    self->mExchangeMgr = &exchangeMgr;
    self->mSessionHolder.Grab(sessionHandle);
    self->StartProfileDiscovery();
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
        Commands::VideoStreamAllocate::Type request = BuildAllocateRequest(mProfile);

        CommandPathParams commandPath = { mProfile.avsmEndpoint, CameraAvStreamManagement::Id, Commands::VideoStreamAllocate::Id,
                                          CommandPathFlags::kEndpointIdValid };
        ReturnErrorOnFailure(mCommandSender->AddRequestData(commandPath, request));
    }
    else
    {
        Commands::VideoStreamDeallocate::Type request;
        request.videoStreamID = mPendingStreamId;

        CommandPathParams commandPath = { mProfile.avsmEndpoint, CameraAvStreamManagement::Id, Commands::VideoStreamDeallocate::Id,
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

    PendingRequest completed                    = mPendingRequest;
    AvAnalysisCameraClient::Callback * callback = mPendingCallback;
    mPendingRequest                             = PendingRequest::kNone;
    mPendingCallback                            = nullptr;
    mProfile                                    = CameraProfile{};
    mSessionHolder.Release();
    mExchangeMgr = nullptr;

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
