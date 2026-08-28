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
    // The stream id is set to 0: it is assigned by the camera and returned by the Allocate response
    return StartRequest(Request::CommandType::kVideoStreamAllocate, aCameraNode, 0, aCallback);
}

CHIP_ERROR DefaultAvAnalysisCameraClient::RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                                                                         AvAnalysisCameraClient::Callback & aCallback)
{
    return StartRequest(Request::CommandType::kVideoStreamDeallocate, aCameraNode, aVideoStreamId, aCallback);
}

void DefaultAvAnalysisCameraClient::Cancel()
{
    VerifyOrReturn(mRequest.InFlight());

    // Deregister from a session establishment still in progress
    mOnConnectedCallback.Cancel();
    mOnConnectionFailureCallback.Cancel();

    // Abort a discovery read or command exchange still in progress; callbacks die with them
    ResetReadClient();
    mCommandSender.reset();

    // Forget the request without delivering a completion
    mRequest.Reset();
}

CHIP_ERROR DefaultAvAnalysisCameraClient::StartRequest(Request::CommandType aCommandType, const ScopedNodeId & aCameraNode,
                                                       uint16_t aVideoStreamId, AvAnalysisCameraClient::Callback & aCallback)
{
    VerifyOrReturnError(mCASESessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mRequest.InFlight(), CHIP_ERROR_BUSY);

    mRequest.Begin(aCommandType, aVideoStreamId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisCameraClient::StartProfileDiscovery()
{
    VerifyOrReturn(mRequest.HasSession(), OnProfileDiscoveryComplete(CHIP_ERROR_INCORRECT_STATE));

    // Phase 1: which endpoint on the camera hosts CameraAVStreamManagement? Read every endpoint's
    // Descriptor ServerList (wildcard endpoint) and look for the cluster id.
    mRequest.Advance(Request::Phase::kDiscoveringEndpoint);
    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(Descriptor::Id, Descriptor::Attributes::ServerList::Id);

    CHIP_ERROR err = SendDiscoveryRead(readPaths, MATTER_ARRAY_SIZE(readPaths));
    if (err != CHIP_NO_ERROR)
    {
        OnProfileDiscoveryComplete(err);
    }
}

CHIP_ERROR DefaultAvAnalysisCameraClient::SendDiscoveryRead(AttributePathParams * aPaths, size_t aPathCount)
{
    VerifyOrReturnError(mRequest.HasSession(), CHIP_ERROR_INCORRECT_STATE);
    // A ReadClient may only be destroyed from its OnDone, so it must not be replaced while a
    // previous one's callbacks can still fire.
    VerifyOrReturnError(!mReadClient, CHIP_ERROR_INCORRECT_STATE);

    ReadPrepareParams readParams(mRequest.Session().Value());
    readParams.mpAttributePathParamsList    = aPaths;
    readParams.mAttributePathParamsListSize = aPathCount;

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

void DefaultAvAnalysisCameraClient::ResetReadClient()
{
    // The ReadClient holds the buffered callback by reference, so it goes first
    mReadClient.reset();
    mReadCallback.reset();
}

void DefaultAvAnalysisCameraClient::StartCapabilitiesRead()
{
    // Phase 2: read the AVSM capability attributes from the discovered endpoint. All of them are
    // mandatory for a camera that serves video, so an allocation fails if any goes unreported.
    mRequest.Advance(Request::Phase::kDiscoveringCapabilities);
    const EndpointId avsmEndpoint = mRequest.Profile().avsmEndpoint;
    AttributePathParams readPaths[4];
    readPaths[0] = AttributePathParams(avsmEndpoint, CameraAvStreamManagement::Id, Globals::Attributes::FeatureMap::Id);
    readPaths[1] = AttributePathParams(avsmEndpoint, CameraAvStreamManagement::Id, Attributes::VideoSensorParams::Id);
    readPaths[2] = AttributePathParams(avsmEndpoint, CameraAvStreamManagement::Id, Attributes::RateDistortionTradeOffPoints::Id);
    readPaths[3] = AttributePathParams(avsmEndpoint, CameraAvStreamManagement::Id, Attributes::SupportedStreamUsages::Id);

    CHIP_ERROR err = SendDiscoveryRead(readPaths, MATTER_ARRAY_SIZE(readPaths));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: capabilities read not started: %" CHIP_ERROR_FORMAT, err.Format());
        OnProfileDiscoveryComplete(err);
    }
}

void DefaultAvAnalysisCameraClient::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                    const StatusIB & aStatus)
{
    VerifyOrReturn(aStatus.IsSuccess() && apData != nullptr);

    switch (mRequest.GetPhase())
    {
    case Request::Phase::kDiscoveringEndpoint:
        HandleServerListReport(aPath, *apData);
        break;
    case Request::Phase::kDiscoveringCapabilities:
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
    VerifyOrReturn(mRequest.Profile().avsmEndpoint == kInvalidEndpointId);

    DataModel::DecodableList<ClusterId> serverList;
    VerifyOrReturn(DataModel::Decode(aData, serverList) == CHIP_NO_ERROR);

    auto iter = serverList.begin();
    while (iter.Next())
    {
        if (iter.GetValue() == CameraAvStreamManagement::Id)
        {
            mRequest.Profile().avsmEndpoint = aPath.mEndpointId;
            return;
        }
    }
}

void DefaultAvAnalysisCameraClient::HandleCapabilityReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData)
{
    CameraProfile & profile = mRequest.Profile();
    VerifyOrReturn(aPath.mClusterId == CameraAvStreamManagement::Id && aPath.mEndpointId == profile.avsmEndpoint);

    switch (aPath.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id: {
        uint32_t featureMap = 0;
        VerifyOrReturn(DataModel::Decode(aData, featureMap) == CHIP_NO_ERROR);
        profile.hasWatermark = (featureMap & to_underlying(Feature::kWatermark)) != 0;
        profile.hasOSD       = (featureMap & to_underlying(Feature::kOnScreenDisplay)) != 0;
        mRequest.MarkCapabilityReported(Request::Capability::kFeatureMap);
        break;
    }
    case Attributes::VideoSensorParams::Id: {
        Structs::VideoSensorParamsStruct::DecodableType sensorParams;
        VerifyOrReturn(DataModel::Decode(aData, sensorParams) == CHIP_NO_ERROR);
        profile.maxWidth     = sensorParams.sensorWidth;
        profile.maxHeight    = sensorParams.sensorHeight;
        profile.maxFrameRate = sensorParams.maxFPS;
        mRequest.MarkCapabilityReported(Request::Capability::kVideoSensorParams);
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
            profile.minBitRateBps = minBitRate;
            profile.minWidth      = minWidth;
            profile.minHeight     = minHeight;
        }
        mRequest.MarkCapabilityReported(Request::Capability::kRateDistortionTradeOffPoints);
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
        mRequest.SetAnalysisUsageSupported(analysisSupported);
        mRequest.MarkCapabilityReported(Request::Capability::kSupportedStreamUsages);
        break;
    }
    default:
        break;
    }
}

void DefaultAvAnalysisCameraClient::OnError(CHIP_ERROR aError)
{
    mRequest.SetDiscoveryError(aError);
}

void DefaultAvAnalysisCameraClient::OnDone(ReadClient * apReadClient)
{
    ResetReadClient();

    switch (mRequest.GetPhase())
    {
    case Request::Phase::kDiscoveringEndpoint:
        CompleteEndpointDiscovery();
        break;
    case Request::Phase::kDiscoveringCapabilities:
        CompleteCapabilityDiscovery();
        break;
    default:
        ChipLogError(Zcl, "AvAnalysisCameraClient: unexpected read completion");
        break;
    }
}

void DefaultAvAnalysisCameraClient::CompleteEndpointDiscovery()
{
    if (mRequest.Profile().avsmEndpoint == kInvalidEndpointId)
    {
        // Without a CameraAVStreamManagement endpoint there is nothing to send commands to
        ChipLogError(Zcl, "AvAnalysisCameraClient: no CameraAVStreamManagement endpoint on the camera (%" CHIP_ERROR_FORMAT ")",
                     mRequest.DiscoveryError().Format());
        FinishRequest(Status::Failure, mRequest.VideoStreamId());
        return;
    }

    // Deallocation only needs the endpoint; the capability attributes feed the allocate request
    if (mRequest.GetCommandType() == Request::CommandType::kVideoStreamDeallocate)
    {
        OnProfileDiscoveryComplete(CHIP_NO_ERROR);
        return;
    }

    StartCapabilitiesRead();
}

void DefaultAvAnalysisCameraClient::CompleteCapabilityDiscovery()
{
    // Only an allocation reaches this phase: CompleteEndpointDiscovery sends a deallocation straight
    // on, since the capabilities exist to build the allocate request and nothing else.
    if (!mRequest.AnalysisUsageSupported())
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: camera does not support the Analysis stream usage");
        FinishRequest(Status::InvalidInState, mRequest.VideoStreamId());
        return;
    }

    // A missing attribute would leave the request built on invented constraints, not the camera's
    if (!mRequest.AllCapabilitiesReported())
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: camera did not report capabilities 0x%02x", mRequest.MissingCapabilities());
        FinishRequest(Status::Failure, mRequest.VideoStreamId());
        return;
    }

    OnProfileDiscoveryComplete(CHIP_NO_ERROR);
}

void DefaultAvAnalysisCameraClient::OnProfileDiscoveryComplete(CHIP_ERROR aError)
{
    VerifyOrReturn(mRequest.InFlight());

    if (aError != CHIP_NO_ERROR || !mRequest.HasSession())
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: profile discovery failed: %" CHIP_ERROR_FORMAT, aError.Format());
        FinishRequest(Status::Failure, mRequest.VideoStreamId());
        return;
    }

    CHIP_ERROR err = SendPendingCommand();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisCameraClient: failed to send command: %" CHIP_ERROR_FORMAT, err.Format());
        FinishRequest(Status::Failure, mRequest.VideoStreamId());
    }
}

void DefaultAvAnalysisCameraClient::NormalizeProfile(CameraProfile & aProfile)
{
    aProfile.minFrameRate = std::min(aProfile.minFrameRate, aProfile.maxFrameRate);
    aProfile.minWidth     = std::min(aProfile.minWidth, aProfile.maxWidth);
    aProfile.minHeight    = std::min(aProfile.minHeight, aProfile.maxHeight);

    aProfile.maxBitRateBps = std::max(aProfile.maxBitRateBps, aProfile.minBitRateBps);
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
    auto * self = static_cast<DefaultAvAnalysisCameraClient *>(context);
    VerifyOrReturn(self->mRequest.InPhase(Request::Phase::kConnecting),
                   ChipLogError(Zcl, "AvAnalysisCameraClient: unexpected session establishment"));

    self->mRequest.HoldSession(sessionHandle, exchangeMgr);
    self->StartProfileDiscovery();
}

void DefaultAvAnalysisCameraClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    auto * self = static_cast<DefaultAvAnalysisCameraClient *>(context);
    VerifyOrReturn(self->mRequest.InPhase(Request::Phase::kConnecting));

    ChipLogError(Zcl, "AvAnalysisCameraClient: could not reach camera node: %" CHIP_ERROR_FORMAT, error.Format());
    self->FinishRequest(Status::Failure, self->mRequest.VideoStreamId());
}

CHIP_ERROR DefaultAvAnalysisCameraClient::SendPendingCommand()
{
    VerifyOrReturnError(mRequest.HasSession(), CHIP_ERROR_INCORRECT_STATE);

    // A CommandSender may only be destroyed from its own OnDone, so it must not be replaced while a
    // previous one's callbacks can still fire.
    VerifyOrReturnError(!mCommandSender, CHIP_ERROR_INCORRECT_STATE);

    mCommandSender = Platform::MakeUnique<CommandSender>(this, &mRequest.ExchangeManager());
    VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    // Recorded before sending: a send can dispatch the interaction's completion synchronously, and
    // that callback must already recognise this sender as ours.
    mRequest.SetInvokedSender(mCommandSender.get());
    mRequest.Advance(Request::Phase::kInvoking);

    CHIP_ERROR err = AddPendingCommandData();
    if (err == CHIP_NO_ERROR)
    {
        err = mCommandSender->SendCommandRequest(mRequest.Session().Value());
    }
    if (err != CHIP_NO_ERROR)
    {
        // A send that never left means OnDone will never arrive, so the sender is ours to destroy
        mRequest.SetInvokedSender(nullptr);
        mCommandSender.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisCameraClient::AddPendingCommandData()
{
    const auto commandPath = [endpoint = mRequest.Profile().avsmEndpoint](CommandId aCommandId) {
        return CommandPathParams{ endpoint, CameraAvStreamManagement::Id, aCommandId, CommandPathFlags::kEndpointIdValid };
    };

    if (mRequest.GetCommandType() == Request::CommandType::kVideoStreamAllocate)
    {
        NormalizeProfile(mRequest.Profile());
        return mCommandSender->AddRequestData(commandPath(Commands::VideoStreamAllocate::Id),
                                              BuildAllocateRequest(mRequest.Profile()));
    }

    if (mRequest.GetCommandType() == Request::CommandType::kVideoStreamDeallocate)
    {
        Commands::VideoStreamDeallocate::Type request;
        request.videoStreamID = mRequest.VideoStreamId();
        return mCommandSender->AddRequestData(commandPath(Commands::VideoStreamDeallocate::Id), request);
    }

    return CHIP_ERROR_INCORRECT_STATE;
}

void DefaultAvAnalysisCameraClient::OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                               const StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisCameraClient: response for an interaction already finished with"));

    if (mRequest.GetCommandType() == Request::CommandType::kVideoStreamAllocate)
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
                status = Status::Failure;
            }
        }
        FinishRequest(status, videoStreamId);
    }
    else if (mRequest.GetCommandType() == Request::CommandType::kVideoStreamDeallocate)
    {
        VerifyOrReturn(aPath.mCommandId == Commands::VideoStreamDeallocate::Id,
                       ChipLogError(Zcl, "AvAnalysisCameraClient: response on an unexpected path"));
        FinishRequest(aStatusIB.mStatus, mRequest.VideoStreamId());
    }
}

void DefaultAvAnalysisCameraClient::OnError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisCameraClient: error for an interaction already finished with"));

    Status status = Status::Failure;
    if (aError.IsIMStatus())
    {
        status = StatusIB(aError).mStatus;
    }
    FinishRequest(status, mRequest.VideoStreamId());
}

void DefaultAvAnalysisCameraClient::OnDone(CommandSender * apCommandSender)
{
    const bool isOurs = mRequest.WasInvokedBy(apCommandSender);

    if (mCommandSender.get() == apCommandSender)
    {
        mCommandSender.reset();
    }

    if (isOurs)
    {
        FinishRequest(Status::Failure, mRequest.VideoStreamId());
    }
}

void DefaultAvAnalysisCameraClient::FinishRequest(Status aStatus, uint16_t aStreamId)
{
    // TakeCallback() nulls the callback, so a second completion for the same request is a no-op
    AvAnalysisCameraClient::Callback * callback = mRequest.TakeCallback();
    VerifyOrReturn(callback != nullptr);

    const Request::CommandType completed = mRequest.GetCommandType();
    mRequest.Reset();

    if (completed == Request::CommandType::kVideoStreamAllocate)
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
