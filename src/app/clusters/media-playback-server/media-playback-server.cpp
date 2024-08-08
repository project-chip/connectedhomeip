/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Routines for the Media Playback plugin, the
 *server implementation of the Media Playback cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/media-playback-server/media-playback-delegate.h>
#include <app/clusters/media-playback-server/media-playback-server.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaPlayback;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::Protocols::InteractionModel::Status;
using StateChangedEvent = chip::app::Clusters::MediaPlayback::Events::StateChanged::Type;
using chip::app::LogEvent;

static constexpr size_t kMediaPlaybackDelegateTableSize =
    MATTER_DM_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kMediaPlaybackDelegateTableSize <= kEmberInvalidEndpointIndex, "kMediaPlayback Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::MediaPlayback::Delegate;

namespace {

Delegate * gDelegateTable[kMediaPlaybackDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogError(Zcl, "MediaPlayback returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetMediaPlaybackDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogError(Zcl, "MediaPlayback NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, MediaPlayback::Id, MATTER_DM_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMediaPlaybackDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Media Playback has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, MediaPlayback::Id, MATTER_DM_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kMediaPlaybackDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class MediaPlaybackAttrAccess : public app::AttributeAccessInterface
{
public:
    MediaPlaybackAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), MediaPlayback::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadCurrentStateAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadStartTimeAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadDurationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSampledPositionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadPlaybackSpeedAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSeekRangeStartAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSeekRangeEndAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadActiveAudioTrackAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadAvailableAudioTracksAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadActiveTextTrackAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadAvailableTextTracksAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

MediaPlaybackAttrAccess gMediaPlaybackAttrAccess;

CHIP_ERROR MediaPlaybackAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    // TODO: Add hasFeature condition

    if (isDelegateNull(delegate, endpoint))
    {
        switch (aPath.mAttributeId)
        {
        case app::Clusters::MediaPlayback::Attributes::AvailableAudioTracks::Id: {
            return aEncoder.EncodeEmptyList();
        }
        case app::Clusters::MediaPlayback::Attributes::AvailableTextTracks::Id: {
            return aEncoder.EncodeEmptyList();
        }
        default: {
            return CHIP_NO_ERROR;
            break;
        }
        }
    }

    switch (aPath.mAttributeId)
    {
    case app::Clusters::MediaPlayback::Attributes::CurrentState::Id:
        return ReadCurrentStateAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::StartTime::Id:
        return ReadStartTimeAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::Duration::Id:
        return ReadDurationAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::SampledPosition::Id:
        return ReadSampledPositionAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::Id:
        return ReadPlaybackSpeedAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::SeekRangeStart::Id:
        return ReadSeekRangeStartAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::Id:
        return ReadSeekRangeEndAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::ActiveAudioTrack::Id:
        return ReadActiveAudioTrackAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::AvailableAudioTracks::Id:
        return ReadAvailableAudioTracksAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::ActiveTextTrack::Id:
        return ReadActiveTextTrackAttribute(aEncoder, delegate);
    case app::Clusters::MediaPlayback::Attributes::AvailableTextTracks::Id:
        return ReadAvailableTextTracksAttribute(aEncoder, delegate);
    case app::Clusters::ContentLauncher::Attributes::FeatureMap::Id:
        return ReadFeatureFlagAttribute(endpoint, aEncoder, delegate);
    case app::Clusters::AccountLogin::Attributes::ClusterRevision::Id:
        return ReadRevisionAttribute(endpoint, aEncoder, delegate);
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                             Delegate * delegate)
{
    uint32_t featureFlag = delegate->GetFeatureMap(endpoint);
    return aEncoder.Encode(featureFlag);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                          Delegate * delegate)
{
    uint16_t clusterRevision = delegate->GetClusterRevision(endpoint);
    return aEncoder.Encode(clusterRevision);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadCurrentStateAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    MediaPlayback::PlaybackStateEnum currentState = delegate->HandleGetCurrentState();
    return aEncoder.Encode(currentState);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadStartTimeAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t startTime = delegate->HandleGetStartTime();
    return aEncoder.Encode(startTime);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadDurationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t duration = delegate->HandleGetDuration();
    return aEncoder.Encode(duration);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadSampledPositionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetSampledPosition(aEncoder);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadPlaybackSpeedAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    float playbackSpeed = delegate->HandleGetPlaybackSpeed();
    return aEncoder.Encode(playbackSpeed);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadSeekRangeStartAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t seekRangeStart = delegate->HandleGetSeekRangeStart();
    return aEncoder.Encode(seekRangeStart);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadSeekRangeEndAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t seekRangeEnd = delegate->HandleGetSeekRangeEnd();
    return aEncoder.Encode(seekRangeEnd);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadActiveAudioTrackAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetActiveAudioTrack(aEncoder);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadAvailableAudioTracksAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetAvailableAudioTracks(aEncoder);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadActiveTextTrackAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetActiveTextTrack(aEncoder);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadAvailableTextTracksAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetAvailableTextTracks(aEncoder);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfMediaPlaybackClusterPlayCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Play::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandlePlay(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterPlayCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterPauseCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                              const Commands::Pause::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandlePause(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterPauseCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterStopCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Stop::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleStop(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterStopCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterFastForwardCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::FastForward::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    auto & audioAdvanceUnmuted = commandData.audioAdvanceUnmuted;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleFastForward(responder, audioAdvanceUnmuted);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterFastForwardCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterPreviousCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::Previous::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandlePrevious(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterPreviousCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterRewindCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                               const Commands::Rewind::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    auto & audioAdvanceUnmuted = commandData.audioAdvanceUnmuted;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleRewind(responder, audioAdvanceUnmuted);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterRewindCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterSkipBackwardCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::SkipBackward::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleSkipBackward(responder, deltaPositionMilliseconds);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterSkipBackwardCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterSkipForwardCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::SkipForward::DecodableType & commandData)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    EndpointId endpoint              = commandPath.mEndpointId;
    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleSkipForward(responder, deltaPositionMilliseconds);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterSkipForwardCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterSeekCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Seek::DecodableType & commandData)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    EndpointId endpoint         = commandPath.mEndpointId;
    auto & positionMilliseconds = commandData.position;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleSeek(responder, positionMilliseconds);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterSeekCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterNextCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                             const Commands::Next::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleNext(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterNextCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfMediaPlaybackClusterStartOverCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::StartOver::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleStartOver(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterStartOverCallback error: %s", err.AsString());
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

/**
 * @brief Media Playback Cluster ActivateAudioTrack Command callback (from client)
 */
bool emberAfMediaPlaybackClusterActivateAudioTrackCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::MediaPlayback::Commands::ActivateAudioTrack::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & trackId          = commandData.trackID;
    auto & audioOutputIndex = commandData.audioOutputIndex;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        if (!delegate->HandleActivateAudioTrack(trackId, audioOutputIndex))
        {
            status = Status::InvalidInState;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterActivateAudioTrackCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Media Playback Cluster ActivateTextTrack Command callback (from client)
 */
bool emberAfMediaPlaybackClusterActivateTextTrackCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::MediaPlayback::Commands::ActivateTextTrack::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    auto & trackId = commandData.trackID;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        if (!delegate->HandleActivateTextTrack(trackId))
        {
            status = Status::InvalidInState;
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterActivateTextTrackCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Media Playback Cluster DeactivateTextTrack Command callback (from client)
 */
bool emberAfMediaPlaybackClusterDeactivateTextTrackCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::MediaPlayback::Commands::DeactivateTextTrack::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleDeactivateTextTrack();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterDeactivateTextTrackCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/** @brief Media Playback Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param attributePath Concrete attribute path that changed
 */
void MatterMediaPlaybackClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    ChipLogProgress(Zcl, "Media Playback Server Cluster Attribute changed [EP:%d, ID:0x%x]", attributePath.mEndpointId,
                    (unsigned int) attributePath.mAttributeId);

    // TODO: Check if event feature is supported and only then continue
    switch (attributePath.mAttributeId)
    {
    case app::Clusters::MediaPlayback::Attributes::CurrentState::Id:
    case app::Clusters::MediaPlayback::Attributes::StartTime::Id:
    case app::Clusters::MediaPlayback::Attributes::Duration::Id:
    case app::Clusters::MediaPlayback::Attributes::SampledPosition::Id:
    case app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::Id:
    case app::Clusters::MediaPlayback::Attributes::SeekRangeStart::Id:
    case app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::Id: {
        EventNumber eventNumber;

        // TODO: Update values
        PlaybackStateEnum currentState = static_cast<PlaybackStateEnum>(0);
        uint64_t startTime             = static_cast<uint64_t>(0);
        uint64_t duration              = static_cast<uint64_t>(0);
        Structs::PlaybackPositionStruct::Type sampledPosition;
        float playbackSpeed      = static_cast<float>(0);
        uint64_t seekRangeEnd    = static_cast<uint64_t>(0);
        uint64_t seekRangeStart  = static_cast<uint64_t>(0);
        chip::ByteSpan data      = ByteSpan();
        bool audioAdvanceUnmuted = false;

        StateChangedEvent event{ currentState, startTime,      duration,           sampledPosition,    playbackSpeed,
                                 seekRangeEnd, seekRangeStart, MakeOptional(data), audioAdvanceUnmuted };

        // TODO: Add endpoint variable instead of 0
        CHIP_ERROR logEventError = LogEvent(event, 0, eventNumber);

        if (CHIP_NO_ERROR != logEventError)
        {
            // TODO: Add endpoint variable instead of 0
            ChipLogError(Zcl, "[Notify] Unable to send notify event: %s [endpointId=%d]", logEventError.AsString(), 0);
        }
        break;
    }

    default: {
        ChipLogProgress(Zcl, "Media Playback Server: unhandled attribute ID");
        break;
    }
    }
}

void MatterMediaPlaybackPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gMediaPlaybackAttrAccess);
}
