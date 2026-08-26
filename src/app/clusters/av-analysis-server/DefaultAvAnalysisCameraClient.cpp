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
#include <clusters/CameraAvStreamManagement/Structs.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/shared/GlobalIds.h>
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

CHIP_ERROR DefaultAvAnalysisCameraClient::Init(CASESessionManager * aCASESessionManager)
{
    VerifyOrReturnError(aCASESessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mCASESessionManager = aCASESessionManager;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode,
                                                                       AvAnalysisCameraClient::Callback & aCallback)
{
    return StartRequest(PendingRequest::kAllocate, aCameraNode, 0, aCallback);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                                                                         AvAnalysisCameraClient::Callback & aCallback)
{
    return StartRequest(PendingRequest::kDeallocate, aCameraNode, aVideoStreamId, aCallback);
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
    mDiscoveryPhase    = DiscoveryPhase::kIdle;
    mSessionHolder.Release();
    mExchangeMgr = nullptr;
}

CHIP_ERROR DefaultAvAnalysisCameraClient::StartRequest(PendingRequest aRequest, const ScopedNodeId & aCameraNode,
                                                       uint16_t aVideoStreamId, AvAnalysisCameraClient::Callback & aCallback)
{
    VerifyOrReturnError(mCASESessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingRequest == PendingRequest::kNone, CHIP_ERROR_BUSY);

    mPendingRequest       = aRequest;
    mPendingVideoStreamId = aVideoStreamId;
    mPendingCallback      = &aCallback;

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisCameraClient::StartProfileDiscovery()
{
    // Built-in defaults; discovered values override as the two discovery reads complete
    FillProfileDefaults(mProfile);
    mDiscoveryError         = CHIP_NO_ERROR;
    mAnalysisUsageSupported = true;

    VerifyOrReturn(mSessionHolder && mExchangeMgr != nullptr, OnProfileDiscoveryComplete(CHIP_ERROR_INCORRECT_STATE));

    // Phase 1: which endpoint on the camera hosts CameraAVStreamManagement? Read every endpoint's
    // Descriptor ServerList (wildcard endpoint) and look for the cluster id.
    mDiscoveryPhase = DiscoveryPhase::kFindEndpoint;
    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(Descriptor::Id, Descriptor::Attributes::ServerList::Id);

    CHIP_ERROR err = SendDiscoveryRead(readPaths, MATTER_ARRAY_SIZE(readPaths));
    if (err != CHIP_NO_ERROR)
    {
        mDiscoveryPhase = DiscoveryPhase::kIdle;
        OnProfileDiscoveryComplete(err);
    }
}

CHIP_ERROR DefaultAvAnalysisCameraClient::SendDiscoveryRead(AttributePathParams * aPaths, size_t aPathCount)
{
    VerifyOrReturnError(mSessionHolder && mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReadPrepareParams readParams(mSessionHolder.Get().Value());
    readParams.mpAttributePathParamsList    = aPaths;
    readParams.mAttributePathParamsListSize = aPathCount;

    mReadClient = Platform::MakeUnique<ReadClient>(InteractionModelEngine::GetInstance(), mExchangeMgr, mBufferedReadCallback,
                                                   ReadClient::InteractionType::Read);
    VerifyOrReturnError(mReadClient != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = mReadClient->SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        mReadClient.reset();
    }
    return err;
}

void DefaultAvAnalysisCameraClient::StartCapabilitiesRead()
{
    // Phase 2: read the AVSM capability attributes from the discovered endpoint. Each field falls
    // back to its configured default when the camera does not report it.
    mDiscoveryPhase = DiscoveryPhase::kReadCapabilities;
    AttributePathParams readPaths[4];
    readPaths[0] = AttributePathParams(mProfile.avsmEndpoint, CameraAvStreamManagement::Id, Globals::Attributes::FeatureMap::Id);
    readPaths[1] = AttributePathParams(mProfile.avsmEndpoint, CameraAvStreamManagement::Id, Attributes::VideoSensorParams::Id);
    readPaths[2] =
        AttributePathParams(mProfile.avsmEndpoint, CameraAvStreamManagement::Id, Attributes::RateDistortionTradeOffPoints::Id);
    readPaths[3] = AttributePathParams(mProfile.avsmEndpoint, CameraAvStreamManagement::Id, Attributes::SupportedStreamUsages::Id);

    CHIP_ERROR err = SendDiscoveryRead(readPaths, MATTER_ARRAY_SIZE(readPaths));
    if (err != CHIP_NO_ERROR)
    {
        // Proceed on the configured baseline rather than failing the request
        ChipLogProgress(Zcl, "AvAnalysisCameraClient: capabilities read not started: %" CHIP_ERROR_FORMAT, err.Format());
        mDiscoveryPhase = DiscoveryPhase::kIdle;
        OnProfileDiscoveryComplete(CHIP_NO_ERROR);
    }
}

void DefaultAvAnalysisCameraClient::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                    const StatusIB & aStatus)
{
    VerifyOrReturn(aStatus.IsSuccess() && apData != nullptr);

    switch (mDiscoveryPhase)
    {
    case DiscoveryPhase::kFindEndpoint:
        HandleServerListReport(aPath, *apData);
        break;
    case DiscoveryPhase::kReadCapabilities:
        HandleCapabilityReport(aPath, *apData);
        break;
    default:
        break;
    }
}

void DefaultAvAnalysisCameraClient::HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData)
{
    VerifyOrReturn(aPath.mClusterId == Descriptor::Id && aPath.mAttributeId == Descriptor::Attributes::ServerList::Id);
    // First matching endpoint wins
    VerifyOrReturn(mProfile.avsmEndpoint == kInvalidEndpointId);

    DataModel::DecodableList<ClusterId> serverList;
    VerifyOrReturn(DataModel::Decode(aData, serverList) == CHIP_NO_ERROR);

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

void DefaultAvAnalysisCameraClient::HandleCapabilityReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData)
{
    VerifyOrReturn(aPath.mClusterId == CameraAvStreamManagement::Id && aPath.mEndpointId == mProfile.avsmEndpoint);

    switch (aPath.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id: {
        uint32_t featureMap = 0;
        VerifyOrReturn(DataModel::Decode(aData, featureMap) == CHIP_NO_ERROR);
        mProfile.hasWatermark = (featureMap & to_underlying(Feature::kWatermark)) != 0;
        mProfile.hasOSD       = (featureMap & to_underlying(Feature::kOnScreenDisplay)) != 0;
        break;
    }
    case Attributes::VideoSensorParams::Id: {
        Structs::VideoSensorParamsStruct::DecodableType sensorParams;
        VerifyOrReturn(DataModel::Decode(aData, sensorParams) == CHIP_NO_ERROR);
        mProfile.maxWidth     = sensorParams.sensorWidth;
        mProfile.maxHeight    = sensorParams.sensorHeight;
        mProfile.maxFrameRate = sensorParams.maxFPS;
        mProfile.minWidth     = std::min(mProfile.minWidth, mProfile.maxWidth);
        mProfile.minHeight    = std::min(mProfile.minHeight, mProfile.maxHeight);
        mProfile.minFrameRate = std::min(mProfile.minFrameRate, mProfile.maxFrameRate);
        break;
    }
    case Attributes::RateDistortionTradeOffPoints::Id: {
        DataModel::DecodableList<Structs::RateDistortionTradeOffPointsStruct::DecodableType> points;
        VerifyOrReturn(DataModel::Decode(aData, points) == CHIP_NO_ERROR);

        // Lowest H.264 trade-off point bounds the smallest stream the camera encodes
        bool found          = false;
        uint32_t minBitRate = 0;
        uint16_t minWidth   = 0;
        uint16_t minHeight  = 0;
        auto iter           = points.begin();
        while (iter.Next())
        {
            const auto & point = iter.GetValue();
            if (point.codec != VideoCodecEnum::kH264)
            {
                continue;
            }
            if (!found || point.minBitRate < minBitRate)
            {
                minBitRate = point.minBitRate;
            }
            if (!found || point.resolution.width < minWidth)
            {
                minWidth  = point.resolution.width;
                minHeight = point.resolution.height;
            }
            found = true;
        }
        if (found)
        {
            mProfile.minBitRateBps = minBitRate;
            mProfile.minWidth      = minWidth;
            mProfile.minHeight     = minHeight;
            mProfile.maxBitRateBps = std::max(mProfile.maxBitRateBps, mProfile.minBitRateBps);
        }
        break;
    }
    case Attributes::SupportedStreamUsages::Id: {
        DataModel::DecodableList<Globals::StreamUsageEnum> usages;
        VerifyOrReturn(DataModel::Decode(aData, usages) == CHIP_NO_ERROR);

        bool analysisSupported = false;
        auto iter              = usages.begin();
        while (iter.Next())
        {
            if (iter.GetValue() == Globals::StreamUsageEnum::kAnalysis)
            {
                analysisSupported = true;
                break;
            }
        }
        mAnalysisUsageSupported = analysisSupported;
        break;
    }
    default:
        break;
    }
}

void DefaultAvAnalysisCameraClient::OnError(CHIP_ERROR aError)
{
    mDiscoveryError = aError;
}

void DefaultAvAnalysisCameraClient::OnDone(ReadClient * apReadClient)
{
    mReadClient.reset();

    if (mDiscoveryPhase == DiscoveryPhase::kFindEndpoint)
    {
        if (mProfile.avsmEndpoint == kInvalidEndpointId)
        {
            // Without a CameraAVStreamManagement endpoint there is nothing to send commands to
            ChipLogError(Zcl, "AvAnalysisCameraClient: no CameraAVStreamManagement endpoint on the camera (%" CHIP_ERROR_FORMAT ")",
                         mDiscoveryError.Format());
            mDiscoveryPhase = DiscoveryPhase::kIdle;
            FinishRequest(Status::Failure, mPendingVideoStreamId);
            return;
        }
        // Deallocation only needs the endpoint; the capability attributes feed the allocate request
        if (mPendingRequest == PendingRequest::kDeallocate)
        {
            mDiscoveryPhase = DiscoveryPhase::kIdle;
            OnProfileDiscoveryComplete(CHIP_NO_ERROR);
            return;
        }
        StartCapabilitiesRead();
        return;
    }

    mDiscoveryPhase = DiscoveryPhase::kIdle;

    // A camera without Analysis stream usage cannot serve this stream; fail before allocating
    // anything. Deallocation must still work regardless of the advertised usages.
    if (!mAnalysisUsageSupported && mPendingRequest == PendingRequest::kAllocate)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: camera does not support the Analysis stream usage");
        FinishRequest(Status::InvalidInState, mPendingVideoStreamId);
        return;
    }

    OnProfileDiscoveryComplete(CHIP_NO_ERROR);
}

void DefaultAvAnalysisCameraClient::OnProfileDiscoveryComplete(CHIP_ERROR aError)
{
    if (aError != CHIP_NO_ERROR || !mSessionHolder || mExchangeMgr == nullptr)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: profile discovery failed: %" CHIP_ERROR_FORMAT, aError.Format());
        FinishRequest(Status::Failure, mPendingVideoStreamId);
        return;
    }

    CHIP_ERROR err = SendPendingCommand(*mExchangeMgr, mSessionHolder.Get().Value());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: failed to send command: %" CHIP_ERROR_FORMAT, err.Format());
        FinishRequest(Status::Failure, mPendingVideoStreamId);
    }
}

void DefaultAvAnalysisCameraClient::FillProfileDefaults(CameraProfile & outProfile)
{
    outProfile.avsmEndpoint  = kInvalidEndpointId;
    outProfile.hasWatermark  = false;
    outProfile.hasOSD        = false;
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
    self->FinishRequest(Status::Failure, self->mPendingVideoStreamId);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::SendPendingCommand(Messaging::ExchangeManager & aExchangeMgr,
                                                             const SessionHandle & aSessionHandle)
{
    mCommandSender = std::make_unique<CommandSender>(this, &aExchangeMgr);
    VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    mResponseDelivered = false;

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
        request.videoStreamID = mPendingVideoStreamId;

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
        FinishRequest(aStatusIB.mStatus, mPendingVideoStreamId);
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
    FinishRequest(status, mPendingVideoStreamId);
}

void DefaultAvAnalysisCameraClient::OnDone(CommandSender * apCommandSender)
{
    // The interaction is complete; if no response/error was seen, report failure (exactly-once contract).
    if (!mResponseDelivered)
    {
        FinishRequest(Status::Failure, mPendingVideoStreamId);
    }
    mCommandSender.reset();
}

void DefaultAvAnalysisCameraClient::FinishRequest(Status aStatus, uint16_t aStreamId)
{
    VerifyOrReturn(mPendingCallback != nullptr);
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
