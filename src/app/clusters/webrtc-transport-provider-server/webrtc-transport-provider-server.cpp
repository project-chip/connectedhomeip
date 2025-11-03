/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "webrtc-transport-provider-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <iterator>
#include <memory>

using namespace chip;
using namespace chip::app;
using chip::Protocols::InteractionModel::Status;

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

namespace {

static constexpr uint16_t kMaxSessionId = 65534;

NodeId GetNodeIdFromCtx(const CommandHandler & commandHandler)
{
    auto descriptor = commandHandler.GetSubjectDescriptor();

    if (descriptor.authMode != Access::AuthMode::kCase)
    {
        return kUndefinedNodeId;
    }
    return descriptor.subject;
}

/**
 * @brief Validates spec-level SFrame constraints (data model constraints).
 *
 * Checks that SFrameConfig meets the data model requirements:
 * - CipherSuite >= 1
 * - BaseKey length <= 128 bytes
 * - KID length 2-8 bytes
 *
 * @param[in] sframeConfig The SFrame configuration to validate
 *
 * @return true if all spec constraints are satisfied, false otherwise.
 */
bool SFrameFollowsSpecConstraints(const Clusters::WebRTCTransportProvider::Structs::SFrameStruct::DecodableType & sframeConfig)
{
    // Spec constraint: CipherSuite >= 1
    if (sframeConfig.cipherSuite < 1)
    {
        return false;
    }

    // Spec constraint: BaseKey length <= 128
    if (sframeConfig.baseKey.size() > 128)
    {
        return false;
    }

    // Spec constraint: KID length must be 2-8
    if (sframeConfig.kid.size() < 2 || sframeConfig.kid.size() > 8)
    {
        return false;
    }

    return true;
}

/**
 * @brief Checks if a URL has a turns or stuns scheme.
 *
 * @param[in] url The URL to check
 *
 * @return true if the URL starts with "turns:" or "stuns:", false otherwise.
 */
bool HasTurnsOrStunsScheme(const CharSpan & url)
{
    // Convert CharSpan to string for easier comparison
    std::string urlStr(url.data(), url.size());

    // Check for "turns:" or "stuns:" prefix (case-sensitive per URL spec)
    return (urlStr.size() >= 6 && urlStr.substr(0, 6) == "turns:") || (urlStr.size() >= 6 && urlStr.substr(0, 6) == "stuns:");
}

/**
 * @brief Validates ICEServers list constraints (data model constraints).
 *
 * Checks that ICEServers list meets the data model requirements:
 * - ICEServers list: max 10 entries
 * - URLs list per ICEServerStruct: max 10 items, each URL max 2000 characters
 * - Username: optional, max 508 bytes
 * - Credential: optional, max 512 bytes
 * - CAID: optional, range 0-65534
 *
 * @param[in] iceServers The ICEServers list to validate
 * @param[in] commandName Name of the command (for logging)
 *
 * @return Status::Success if all constraints are satisfied
 *         Status::ConstraintError if a constraint is violated
 *         Status::InvalidCommand if the list structure is invalid
 */
Status ICEServersFollowsSpecConstraints(const DataModel::DecodableList<ICEServerDecodableStruct> & iceServers,
                                        const char * commandName)
{
    size_t iceServerCount = 0;
    auto iter             = iceServers.begin();
    while (iter.Next())
    {
        iceServerCount++;
        if (iceServerCount > 10)
        {
            ChipLogError(Zcl, "%s: ICEServers list exceeds maximum of 10 entries", commandName);
            return Status::ConstraintError;
        }

        const auto & iceServer = iter.GetValue();

        // Validate URLs list: max 10 items, each URL max 2000 characters
        size_t urlCount = 0;
        auto urlIter    = iceServer.URLs.begin();
        while (urlIter.Next())
        {
            urlCount++;
            if (urlCount > 10)
            {
                ChipLogError(Zcl, "%s: ICEServer URLs list exceeds maximum of 10", commandName);
                return Status::ConstraintError;
            }

            // Check URL length: max 2000 characters
            const auto & url = urlIter.GetValue();
            if (url.size() > 2000)
            {
                ChipLogError(Zcl, "%s: ICEServer URL exceeds maximum length of 2000 characters", commandName);
                return Status::ConstraintError;
            }
        }

        // Check URLs list validity
        CHIP_ERROR urlListErr = urlIter.GetStatus();
        if (urlListErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "%s: ICEServer URLs list error: %" CHIP_ERROR_FORMAT, commandName, urlListErr.Format());
            return Status::InvalidCommand;
        }

        // Username field: optional, max 508 bytes
        if (iceServer.username.HasValue() && iceServer.username.Value().size() > 508)
        {
            ChipLogError(Zcl, "%s: ICEServer Username exceeds maximum length of 508", commandName);
            return Status::ConstraintError;
        }

        // Credential field: optional, max 512 bytes
        if (iceServer.credential.HasValue() && iceServer.credential.Value().size() > 512)
        {
            ChipLogError(Zcl, "%s: ICEServer Credential exceeds maximum length of 512", commandName);
            return Status::ConstraintError;
        }

        // CAID field: optional, range 0-65534
        if (iceServer.caid.HasValue() && iceServer.caid.Value() > 65534)
        {
            ChipLogError(Zcl, "%s: ICEServer CAID exceeds maximum value of 65534", commandName);
            return Status::ConstraintError;
        }
    }

    // Check the validity of the ICEServers list structure.
    CHIP_ERROR listErr = iter.GetStatus();
    if (listErr != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "%s: ICEServers list error: %" CHIP_ERROR_FORMAT, commandName, listErr.Format());
        return Status::InvalidCommand;
    }

    return Status::Success;
}

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

WebRTCTransportProviderServer::WebRTCTransportProviderServer(Delegate & delegate, EndpointId endpointId) :
    AttributeAccessInterface(MakeOptional(endpointId), WebRTCTransportProvider::Id),
    CommandHandlerInterface(MakeOptional(endpointId), WebRTCTransportProvider::Id), mDelegate(delegate)
{}

WebRTCTransportProviderServer::~WebRTCTransportProviderServer()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR WebRTCTransportProviderServer::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR WebRTCTransportProviderServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // The only attribute from the spec is "CurrentSessions" (attribute ID 0x0000),
    // which is a list[WebRTCSessionStruct].
    if (aPath.mClusterId == Id && aPath.mAttributeId == Attributes::CurrentSessions::Id)
    {
        // We encode mCurrentSessions as a list of WebRTCSessionStruct
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
            for (auto & session : mCurrentSessions)
            {
                ReturnErrorOnFailure(encoder.Encode(session));
            }
            return CHIP_NO_ERROR;
        });
    }

    // If not our attribute, let default logic handle
    return CHIP_NO_ERROR;
}

// CommandHandlerInterface
void WebRTCTransportProviderServer::InvokeCommand(HandlerContext & ctx)
{
    ChipLogDetail(Zcl, "WebRTCTransportProvider: InvokeCommand called with CommandId=0x%08" PRIx32, ctx.mRequestPath.mCommandId);

    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::SolicitOffer::Id:
        CommandHandlerInterface::HandleCommand<Commands::SolicitOffer::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleSolicitOffer(subCtx, req); });
        break;

    case Commands::ProvideOffer::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvideOffer::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleProvideOffer(subCtx, req); });
        break;

    case Commands::ProvideAnswer::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvideAnswer::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleProvideAnswer(subCtx, req); });
        break;

    case Commands::ProvideICECandidates::Id:
        CommandHandlerInterface::HandleCommand<Commands::ProvideICECandidates::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleProvideICECandidates(subCtx, req); });
        break;

    case Commands::EndSession::Id:
        CommandHandlerInterface::HandleCommand<Commands::EndSession::DecodableType>(
            ctx, [this](HandlerContext & subCtx, const auto & req) { HandleEndSession(subCtx, req); });
        break;

    default:
        // Mark unrecognized command as UnsupportedCommand
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        break;
    }
}

// Helper functions
WebRTCSessionStruct * WebRTCTransportProviderServer::FindSession(uint16_t sessionId)
{
    for (auto & session : mCurrentSessions)
    {
        if (session.id == sessionId)
        {
            return &session;
        }
    }

    return nullptr;
}

WebRTCTransportProviderServer::UpsertResultEnum WebRTCTransportProviderServer::UpsertSession(const WebRTCSessionStruct & session)
{
    assertChipStackLockedByCurrentThread();

    UpsertResultEnum result;
    auto it = std::find_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                           [id = session.id](const auto & existing) { return existing.id == id; });

    if (it != mCurrentSessions.end())
    {
        *it    = session;
        result = UpsertResultEnum::kUpdated;
    }
    else
    {
        mCurrentSessions.push_back(session);
        result = UpsertResultEnum::kInserted;
    }

    MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportProvider::Id,
                                           WebRTCTransportProvider::Attributes::CurrentSessions::Id);

    return result;
}

void WebRTCTransportProviderServer::RemoveSession(uint16_t sessionId)
{
    assertChipStackLockedByCurrentThread();

    size_t originalSize = mCurrentSessions.size();

    // Erase-Remove idiom
    mCurrentSessions.erase(std::remove_if(mCurrentSessions.begin(), mCurrentSessions.end(),
                                          [sessionId](const WebRTCSessionStruct & s) { return s.id == sessionId; }),
                           mCurrentSessions.end());

    // If a session was removed, the size will be smaller.
    if (mCurrentSessions.size() < originalSize)
    {
        // Notify the stack that the CurrentSessions attribute has changed.
        MatterReportingAttributeChangeCallback(AttributeAccessInterface::GetEndpointId().Value(), WebRTCTransportProvider::Id,
                                               WebRTCTransportProvider::Attributes::CurrentSessions::Id);
    }
}

WebRTCSessionStruct * WebRTCTransportProviderServer::CheckForMatchingSession(HandlerContext & ctx, uint16_t sessionId)
{
    WebRTCSessionStruct * session = FindSession(sessionId);
    if (session == nullptr)
    {
        return nullptr;
    }

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // Ensure the session’s peer matches the current command invoker
    if (peerNodeId != session->peerNodeID || peerFabricIndex != session->GetFabricIndex())
    {
        return nullptr;
    }

    return session;
}

CHIP_ERROR WebRTCTransportProviderServer::GenerateSessionId(uint16_t & outSessionId)
{
    static uint16_t nextSessionId = 0;
    uint16_t candidateId          = 0;

    // Try at most kMaxSessionId+1 attempts to find a free ID
    // This ensures we never loop infinitely even if all IDs are somehow in use
    for (uint16_t attempts = 0; attempts <= kMaxSessionId; attempts++)
    {
        candidateId = nextSessionId++;

        // Handle wrap-around per spec
        if (nextSessionId > kMaxSessionId)
        {
            nextSessionId = 0;
        }

        if (FindSession(candidateId) == nullptr)
        {
            outSessionId = candidateId;
            return CHIP_NO_ERROR;
        }
    }

    // All session IDs are in use
    ChipLogError(Zcl, "All session IDs are in use! Cannot generate new session ID.");
    return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
}

Status WebRTCTransportProviderServer::CheckPrivacyModes(const char * commandName, StreamUsageEnum streamUsage)
{
    bool hardPrivacyModeActive = false;
    CHIP_ERROR err             = mDelegate.IsHardPrivacyModeActive(hardPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "%s: Failed to check Hard Privacy mode: %" CHIP_ERROR_FORMAT, commandName, err.Format());
        return Status::Failure;
    }

    if (hardPrivacyModeActive)
    {
        ChipLogError(Zcl, "%s: Hard Privacy mode is enabled", commandName);
        return Status::InvalidInState;
    }

    bool softLivestreamPrivacyModeActive = false;
    err                                  = mDelegate.IsSoftLivestreamPrivacyModeActive(softLivestreamPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "%s: Failed to check Soft LivestreamPrivacy mode: %" CHIP_ERROR_FORMAT, commandName, err.Format());
        return Status::Failure;
    }

    if (softLivestreamPrivacyModeActive && streamUsage == StreamUsageEnum::kLiveView)
    {
        ChipLogError(Zcl, "%s: Soft LivestreamPrivacy mode is enabled and StreamUsage is LiveView", commandName);
        return Status::InvalidInState;
    }

    bool softRecordingPrivacyModeActive = false;
    err                                 = mDelegate.IsSoftRecordingPrivacyModeActive(softRecordingPrivacyModeActive);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "%s: Failed to check SoftRecordingPrivacyModeActive: %" CHIP_ERROR_FORMAT, commandName, err.Format());
        return Status::Failure;
    }

    if (softRecordingPrivacyModeActive && (streamUsage == StreamUsageEnum::kRecording || streamUsage == StreamUsageEnum::kAnalysis))
    {
        ChipLogError(Zcl, "%s: Soft RecordingPrivacy mode is enabled and StreamUsage is Recording or Analysis", commandName);
        return Status::InvalidInState;
    }

    return Status::Success;
}

Status WebRTCTransportProviderServer::CheckTurnsOrStunsRequiresUTCTime(
    const char * commandName, const Optional<DataModel::DecodableList<ICEServerDecodableStruct>> & iceServers)
{
    if (!iceServers.HasValue())
    {
        return Status::Success;
    }

    // Check if any URL uses turns or stuns scheme
    bool hasTurnsOrStuns = false;
    auto iter            = iceServers.Value().begin();
    while (iter.Next())
    {
        const auto & iceServer = iter.GetValue();
        auto urlIter           = iceServer.URLs.begin();
        while (urlIter.Next())
        {
            if (HasTurnsOrStunsScheme(urlIter.GetValue()))
            {
                hasTurnsOrStuns = true;
                break;
            }
        }
        if (hasTurnsOrStuns)
        {
            break;
        }
    }

    // If turns/stuns URLs are present, verify UTCTime is not null
    if (hasTurnsOrStuns)
    {
        bool isUTCTimeNull = false;
        CHIP_ERROR err     = mDelegate.IsUTCTimeNull(isUTCTimeNull);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "%s: Failed to check UTCTime: %" CHIP_ERROR_FORMAT, commandName, err.Format());
            return Status::Failure;
        }

        if (isUTCTimeNull)
        {
            ChipLogError(Zcl, "%s: turns/stuns URLs require non-null UTCTime", commandName);
            return Status::InvalidInState;
        }
    }

    return Status::Success;
}

// Command Handlers
void WebRTCTransportProviderServer::HandleSolicitOffer(HandlerContext & ctx, const Commands::SolicitOffer::DecodableType & req)
{
    auto videoStreamID = req.videoStreamID;
    auto audioStreamID = req.audioStreamID;

    // ===== Validate all conformance and constraint checks (data model validation) =====

    // Validate the streamUsage field against the allowed enum values.
    if (req.streamUsage == StreamUsageEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Invalid streamUsage value %u.", to_underlying(req.streamUsage));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // At least one of Video Stream ID and Audio Stream ID has to be present
    if (!req.videoStreamID.HasValue() && !req.audioStreamID.HasValue())
    {
        ChipLogError(Zcl, "HandleSolicitOffer: one of VideoStreamID or AudioStreamID must be present");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    if (req.SFrameConfig.HasValue())
    {
        if (!SFrameFollowsSpecConstraints(req.SFrameConfig.Value()))
        {
            ChipLogError(Zcl, "HandleSolicitOffer: SFrame spec constraint validation failed");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    // ICEServers field SHALL be a list of ICEServerStruct containing ICE servers and credentials.
    // Validate ICEServers list constraints (max 10 entries, each with max 10 URLs max 2000 chars, Username max 508, Credential max
    // 512, CAID 0-65534)
    if (req.ICEServers.HasValue())
    {
        Status validationStatus = ICEServersFollowsSpecConstraints(req.ICEServers.Value(), "HandleSolicitOffer");
        if (validationStatus != Status::Success)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, validationStatus);
            return;
        }
    }

    // ICETransportPolicy: constraint is max 16 characters
    if (req.ICETransportPolicy.HasValue())
    {
        if (req.ICETransportPolicy.Value().size() > 16)
        {
            ChipLogError(Zcl, "HandleSolicitOffer: ICETransportPolicy exceeds maximum length of 16");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    // ===== Cluster logic starts here =====

    Status status = CheckPrivacyModes("HandleSolicitOffer", req.streamUsage);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // TODO:#41458 If the passed in StreamUsage is not found in the StreamUsagePriorities, return DYNAMIC_CONSTRAINT_ERROR

    // Validate VideoStreamID against AllocatedVideoStreams.
    // If present and null then a stream has to have been allocated.
    // If present and not null, then the stream ID has to exist
    if (req.videoStreamID.HasValue())
    {
        if (req.videoStreamID.Value().IsNull())
        {
            // Is there an allocated stream, delegate handles matching against an allocated stream in the HandleSolicitOffer method
            if (!mDelegate.HasAllocatedVideoStreams())
            {
                ChipLogError(Zcl, "HandleSolicitOffer: video requested when there are no AllocatedVideoStreams");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
                return;
            }
        }
        else
        {
            // Delegate should validate against AllocatedVideoStreams
            if (mDelegate.ValidateVideoStreamID(req.videoStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleSolicitOffer: VideoStreamID %u does not match AllocatedVideoStreams",
                             req.videoStreamID.Value().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }
    }

    // Validate AudioStreamID against AllocatedAudioStreams if present
    // If present and null then a stream has to have been allocated.
    // If present and not null, then the stream ID has to exist
    if (req.audioStreamID.HasValue())
    {
        if (req.audioStreamID.Value().IsNull())
        {
            // Is there an allocated stream, delegate handles matching against an allocated stream in the HandleSolicitOffer method
            if (!mDelegate.HasAllocatedAudioStreams())
            {
                ChipLogError(Zcl, "HandleSolicitOffer: audio requested when there are no AllocatedAudioStreams");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
                return;
            }
        }
        else
        {
            // Delegate should validate against AllocatedVideoStreams
            if (mDelegate.ValidateAudioStreamID(req.audioStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleSolicitOffer: AudioStreamID %u does not match AllocatedAudioStreams",
                             req.audioStreamID.Value().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }
    }

    if (req.SFrameConfig.HasValue())
    {
        const auto & sframeConfig = req.SFrameConfig.Value();
        CHIP_ERROR err            = mDelegate.ValidateSFrameConfig(sframeConfig.cipherSuite, sframeConfig.baseKey.size());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleSolicitOffer: SFrame configuration validation failed: %" CHIP_ERROR_FORMAT, err.Format());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
            return;
        }
    }

    // For each URL in the URLs field of each ICEServerStruct, if the URL scheme is
    // 'turns' or 'stuns', verify that Time Synchronization cluster's UTCTime attribute is not null.
    status = CheckTurnsOrStunsRequiresUTCTime("HandleSolicitOffer", req.ICEServers);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Check resource management and stream priorities. If the IDs are null the delegate will populate with
    // a stream that matches the stream usage
    CHIP_ERROR err = mDelegate.ValidateStreamUsage(req.streamUsage, videoStreamID, audioStreamID);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Cannot provide the stream usage requested");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
        return;
    }

    // Prepare the arguments for the delegate.
    Delegate::OfferRequestArgs args;
    uint16_t sessionId;
    err = GenerateSessionId(sessionId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleSolicitOffer: Cannot generate session ID: %" CHIP_ERROR_FORMAT, err.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(err));
        return;
    }
    args.sessionId             = sessionId;
    args.streamUsage           = req.streamUsage;
    args.videoStreamId         = videoStreamID;
    args.audioStreamId         = audioStreamID;
    args.peerNodeId            = GetNodeIdFromCtx(ctx.mCommandHandler);
    args.fabricIndex           = ctx.mCommandHandler.GetAccessingFabricIndex();
    args.originatingEndpointId = req.originatingEndpointID;

    if (req.SFrameConfig.HasValue())
    {
        args.sFrameConfig.SetValue(req.SFrameConfig.Value());
    }

    // ICEServers: copy the validated list
    if (req.ICEServers.HasValue())
    {
        std::vector<ICEServerDecodableStruct> localIceServers;

        auto iter = req.ICEServers.Value().begin();
        while (iter.Next())
        {
            // Just move the decodable struct as-is, only valid during this method call
            localIceServers.push_back(std::move(iter.GetValue()));
        }

        args.iceServers.SetValue(std::move(localIceServers));
    }

    // ICETransportPolicy: copy the validated policy
    if (req.ICETransportPolicy.HasValue())
    {
        std::string policy(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size());
        args.iceTransportPolicy.SetValue(policy);
    }

    // Delegate processing:
    // The delegate implementation SHALL:
    // - Populate a new WebRTCSessionStruct with the requested values.
    // - If in standby mode, set deferredOffer to true (and perform steps to exit standby within 30 seconds).
    // - If not in standby mode, ensure that VideoStreamID and AudioStreamID are valid (or allocate/select new streams if null).
    // - Internally increment the ReferenceCount on any used video/audio streams.
    // - If resources cannot be allocated, invoke End with Reason OutOfResources.
    WebRTCSessionStruct outSession;
    bool deferredOffer = false;

    auto delegateStatus =
        Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleSolicitOffer(args, outSession, deferredOffer));
    if (!delegateStatus.IsSuccess())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, delegateStatus);
        return;
    }

    // Store or update the session.
    if (UpsertSession(outSession) == UpsertResultEnum::kInserted)
    {
        ChipLogProgress(Zcl, "WebRTCTransportProvider: Inserted a new session with ID=%u.", outSession.id);
    }
    else
    {
        ChipLogProgress(Zcl, "WebRTCTransportProvider: Updated existing session with ID=%u.", outSession.id);
    }

    // Prepare the SolicitOfferResponse command.
    Commands::SolicitOfferResponse::Type resp;
    resp.webRTCSessionID = outSession.id;
    resp.deferredOffer   = deferredOffer;

    // If VideoStreamID was in the request, it should be in the response
    if (req.videoStreamID.HasValue())
    {
        resp.videoStreamID.SetValue(outSession.videoStreamID);
    }

    // If AudioStreamID was in the request, it should be in the response
    if (req.audioStreamID.HasValue())
    {
        resp.audioStreamID.SetValue(outSession.audioStreamID);
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
}

void WebRTCTransportProviderServer::HandleProvideOffer(HandlerContext & ctx, const Commands::ProvideOffer::DecodableType & req)
{
    auto webRTCSessionID = req.webRTCSessionID;
    auto videoStreamID   = req.videoStreamID;
    auto audioStreamID   = req.audioStreamID;

    NodeId peerNodeId           = GetNodeIdFromCtx(ctx.mCommandHandler);
    FabricIndex peerFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    WebRTCSessionStruct outSession;

    // Prepare delegate arguments for the session
    Delegate::ProvideOfferRequestArgs args;

    // ===== Validate all conformance and constraint checks (data model validation) =====

    // Validate the streamUsage field against the allowed enum values.
    if (req.streamUsage == StreamUsageEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleProvideOffer: Invalid streamUsage value %u.", to_underlying(req.streamUsage));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // At least one of Video Stream ID and Audio Stream ID must be present
    if (!req.videoStreamID.HasValue() && !req.audioStreamID.HasValue())
    {
        ChipLogError(Zcl, "HandleProvideOffer: one of VideoStreamID or AudioStreamID must be present");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    if (req.SFrameConfig.HasValue())
    {
        if (!SFrameFollowsSpecConstraints(req.SFrameConfig.Value()))
        {
            ChipLogError(Zcl, "HandleProvideOffer: SFrame spec constraint validation failed");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    // ICEServers field SHALL be a list of ICEServerStruct containing ICE servers and credentials.
    // Validate ICEServers list constraints (max 10 entries, each with max 10 URLs max 2000 chars, Username max 508, Credential max
    // 512, CAID 0-65534)
    if (req.ICEServers.HasValue())
    {
        Status validationStatus = ICEServersFollowsSpecConstraints(req.ICEServers.Value(), "HandleProvideOffer");
        if (validationStatus != Status::Success)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, validationStatus);
            return;
        }
    }

    // ICETransportPolicy: constraint is max 16 characters (data model constraint)
    if (req.ICETransportPolicy.HasValue())
    {
        if (req.ICETransportPolicy.Value().size() > 16)
        {
            ChipLogError(Zcl, "HandleProvideOffer: ICETransportPolicy exceeds maximum length of 16");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }

    // ===== Cluster logic starts here =====

    // If WebRTCSessionID is not null:
    // - If it does not match a value in CurrentSessions: Respond with NOT_FOUND
    // - If the accessing Peer Node ID and fabric do not match the PeerNodeID and associated fabric
    //   for the WebRTCSessionID entry in CurrentSessions: Respond with NOT_FOUND
    // Both checks are performed by CheckForMatchingSession()
    if (!webRTCSessionID.IsNull())
    {
        uint16_t sessionId                    = webRTCSessionID.Value();
        WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
        if (existingSession == nullptr)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
            return;
        }

        // Use the existing session for further processing (re-offer case).
        outSession = *existingSession;

        // re-use the same session id for offer processing in delegate
        args.sessionId = sessionId;
    }
    else
    {
        // WebRTCSessionID is null - new session request

        // Check privacy modes (per spec: only for new sessions)
        Status status = CheckPrivacyModes("HandleProvideOffer", req.streamUsage);
        if (status != Status::Success)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
            return;
        }

        // TODO:#41458 If the passed in StreamUsage is not found in the StreamUsagePriorities, return DYNAMIC_CONSTRAINT_ERROR

        // If VideoStreamID is present and is not null and does not match a value in AllocatedVideoStreams:
        // Fail the command with the status code DYNAMIC_CONSTRAINT_ERROR
        if (videoStreamID.HasValue() && !videoStreamID.Value().IsNull())
        {
            if (mDelegate.ValidateVideoStreamID(videoStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleProvideOffer: VideoStreamID %u does not match AllocatedVideoStreams",
                             videoStreamID.Value().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }

        // If VideoStreamID is present and is null:
        // If AllocatedVideoStreams is empty: Fail the command with the status code INVALID_IN_STATE
        // Automatically select an existing video stream per the Resource Management and Stream Priorities.
        if (videoStreamID.HasValue() && videoStreamID.Value().IsNull())
        {
            if (!mDelegate.HasAllocatedVideoStreams())
            {
                ChipLogError(Zcl, "HandleProvideOffer: AllocatedVideoStreams is empty");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
                return;
            }
            // Automatic selection will be handled by the delegate in HandleProvideOffer.
        }

        // If AudioStreamID is present and is not null and does not match a value in AllocatedAudioStreams:
        // Fail the command with the status code DYNAMIC_CONSTRAINT_ERROR
        if (audioStreamID.HasValue() && !audioStreamID.Value().IsNull())
        {
            if (mDelegate.ValidateAudioStreamID(audioStreamID.Value().Value()) != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleProvideOffer: AudioStreamID %u does not match AllocatedAudioStreams",
                             audioStreamID.Value().Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }

        // If AudioStreamID is present and is null:
        // If AllocatedAudioStreams is empty: Fail the command with the status code INVALID_IN_STATE
        // Automatically select an existing audio stream per the Resource Management and Stream Priorities.
        if (audioStreamID.HasValue() && audioStreamID.Value().IsNull())
        {
            if (!mDelegate.HasAllocatedAudioStreams())
            {
                ChipLogError(Zcl, "HandleProvideOffer: AllocatedAudioStreams is empty");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
                return;
            }
            // Automatic selection will be handled by the delegate in HandleProvideOffer.
        }

        // If not able to meet the Resource Management and Stream Priorities conditions or unable to provide another WebRTC session:
        // Respond with a response status of RESOURCE_EXHAUSTED
        CHIP_ERROR err = mDelegate.ValidateStreamUsage(req.streamUsage, videoStreamID, audioStreamID);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Cannot provide stream usage requested");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
            return;
        }

        // If SFrameConfig is present:
        // If the CipherSuite field of the passed in SFrameConfig is not a supported value:
        // Fail the command with the status code DYNAMIC_CONSTRAINT_ERROR
        // If the length of the BaseKey field of the passed in SFrameConfig does not match the expected length for a key using the
        // CipherSuite: Fail the command with the status code DYNAMIC_CONSTRAINT_ERROR
        if (req.SFrameConfig.HasValue())
        {
            const auto & sframeConfig = req.SFrameConfig.Value();
            err                       = mDelegate.ValidateSFrameConfig(sframeConfig.cipherSuite, sframeConfig.baseKey.size());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "HandleProvideOffer: SFrame configuration validation failed: %" CHIP_ERROR_FORMAT, err.Format());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
                return;
            }
        }

        // For each URL in the URLs field of each ICEServerStruct in the passed in ICEServers:
        // If the URL scheme is turns or stuns, and the UTCTime attribute of the Time Synchronization cluster is null:
        // Fail the command with the status code INVALID_IN_STATE
        status = CheckTurnsOrStunsRequiresUTCTime("HandleProvideOffer", req.ICEServers);
        if (status != Status::Success)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
            return;
        }

        // Generate new session id
        uint16_t sessionId;
        err = GenerateSessionId(sessionId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "HandleProvideOffer: Cannot generate session ID: %" CHIP_ERROR_FORMAT, err.Format());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(err));
            return;
        }
        args.sessionId = sessionId;
    }

    args.streamUsage           = req.streamUsage;
    args.videoStreamId         = videoStreamID;
    args.audioStreamId         = audioStreamID;
    args.peerNodeId            = peerNodeId;
    args.fabricIndex           = peerFabricIndex;
    args.sdp                   = std::string(req.sdp.data(), req.sdp.size());
    args.originatingEndpointId = req.originatingEndpointID;

    if (req.SFrameConfig.HasValue())
    {
        args.sFrameConfig.SetValue(req.SFrameConfig.Value());
    }

    // ICEServers: copy the validated list
    if (req.ICEServers.HasValue())
    {
        std::vector<ICEServerDecodableStruct> localIceServers;

        auto iter = req.ICEServers.Value().begin();
        while (iter.Next())
        {
            localIceServers.push_back(std::move(iter.GetValue()));
        }

        args.iceServers.SetValue(std::move(localIceServers));
    }

    // ICETransportPolicy: copy the validated policy
    if (req.ICETransportPolicy.HasValue())
    {
        std::string policy(req.ICETransportPolicy.Value().data(), req.ICETransportPolicy.Value().size());
        args.iceTransportPolicy.SetValue(policy);
    }

    // Delegate processing: process the SDP offer, create session, increment reference counts.
    auto delegateStatus = Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideOffer(args, outSession));
    if (!delegateStatus.IsSuccess())
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, delegateStatus);
        return;
    }

    // Update/Insert the WebRTCSessionStruct in CurrentSessions.
    UpsertSession(outSession);

    // Build and send the ProvideOfferResponse.
    Commands::ProvideOfferResponse::Type resp;
    resp.webRTCSessionID = outSession.id;

    // Set VideoStreamID only if present in the original request.
    if (req.videoStreamID.HasValue())
    {
        resp.videoStreamID.SetValue(outSession.videoStreamID);
    }

    // Set AudioStreamID only if present in the original request.
    if (req.audioStreamID.HasValue())
    {
        resp.audioStreamID.SetValue(outSession.audioStreamID);
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, resp);
}

void WebRTCTransportProviderServer::HandleProvideAnswer(HandlerContext & ctx, const Commands::ProvideAnswer::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto sdpSpan       = req.sdp;

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
    if (existingSession == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    std::string sdpAnswer(sdpSpan.data(), sdpSpan.size());
    ctx.mCommandHandler.AddStatus(
        ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideAnswer(sessionId, sdpAnswer)));
}

void WebRTCTransportProviderServer::HandleProvideICECandidates(HandlerContext & ctx,
                                                               const Commands::ProvideICECandidates::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;

    std::vector<ICECandidateStruct> candidates;
    auto iter = req.ICECandidates.begin();
    while (iter.Next())
    {
        // Get current candidate.
        const ICECandidateStruct & candidate = iter.GetValue();

        // Validate SDPMid constraint: if present, must have min length 1
        if (!candidate.SDPMid.IsNull() && candidate.SDPMid.Value().empty())
        {
            ChipLogError(Zcl, "HandleProvideICECandidates: SDPMid must have minimum length of 1 when present");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }

        candidates.push_back(candidate);
    }

    // Check the validity of the list.
    CHIP_ERROR listErr = iter.GetStatus();
    if (listErr != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleProvideICECandidates: ICECandidates list error: %" CHIP_ERROR_FORMAT, listErr.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
    if (existingSession == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Delegate the handling of ICE candidates.
    ctx.mCommandHandler.AddStatus(
        ctx.mRequestPath,
        Protocols::InteractionModel::ClusterStatusCode(mDelegate.HandleProvideICECandidates(sessionId, candidates)));
}

void WebRTCTransportProviderServer::HandleEndSession(HandlerContext & ctx, const Commands::EndSession::DecodableType & req)
{
    // Extract command fields from the request.
    uint16_t sessionId = req.webRTCSessionID;
    auto reason        = req.reason;

    // Validate the reason field against the allowed enum values.
    if (reason == WebRTCEndReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "HandleEndSession: Invalid reason value %u.", to_underlying(reason));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    WebRTCSessionStruct * existingSession = CheckForMatchingSession(ctx, sessionId);
    if (existingSession == nullptr)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Delegate handles decrementing reference counts on video/audio streams if applicable.
    CHIP_ERROR err = mDelegate.HandleEndSession(sessionId, reason, existingSession->videoStreamID, existingSession->audioStreamID);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::ClusterStatusCode(err));

    // Remove the session entry from CurrentSessions.
    RemoveSession(sessionId);
}

} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterWebRTCTransportProviderPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing WebRTC Transport Provider cluster.");
}

void MatterWebRTCTransportProviderPluginServerShutdownCallback()
{
    ChipLogProgress(Zcl, "Shutdown WebRTC Transport Provider cluster.");
}
