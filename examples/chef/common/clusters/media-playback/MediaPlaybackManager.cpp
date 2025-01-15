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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <map>
#include <string>
#ifdef MATTER_DM_PLUGIN_MEDIA_PLAYBACK_SERVER
#include "MediaPlaybackManager.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::MediaPlayback;

using chip::CharSpan;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using chip::Protocols::InteractionModel::Status;

PlaybackStateEnum MediaPlaybackManager::HandleGetCurrentState()
{
    PlaybackStateEnum currentState = PlaybackStateEnum::kPlaying;

    Status status = Attributes::CurrentState::Get(mEndpoint, &currentState);
    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to get CurrentStage attribute, err:0x%x", to_underlying(status));
    }
    return currentState;
}

uint64_t MediaPlaybackManager::HandleGetStartTime()
{
    return mStartTime;
}

uint64_t MediaPlaybackManager::HandleGetDuration()
{
    return mDuration;
}

CHIP_ERROR MediaPlaybackManager::HandleGetSampledPosition(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mPlaybackPosition);
}

float MediaPlaybackManager::HandleGetPlaybackSpeed()
{
    float playbackSpeed = 1.0;

    Status status = Attributes::PlaybackSpeed::Get(mEndpoint, &playbackSpeed);
    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to get PlaybackSpeed attribute, err:0x%x", to_underlying(status));
    }
    return playbackSpeed;
}

uint64_t MediaPlaybackManager::HandleGetSeekRangeStart()
{
    return mStartTime;
}

uint64_t MediaPlaybackManager::HandleGetSeekRangeEnd()
{
    return mDuration;
}

CHIP_ERROR MediaPlaybackManager::HandleGetActiveAudioTrack(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mActiveAudioTrack);
}

CHIP_ERROR MediaPlaybackManager::HandleGetAvailableAudioTracks(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto const & audioTrack : mAvailableAudioTracks)
        {
            ReturnErrorOnFailure(encoder.Encode(audioTrack));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MediaPlaybackManager::HandleGetActiveTextTrack(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(mActiveTextTrack);
}

CHIP_ERROR MediaPlaybackManager::HandleGetAvailableTextTracks(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto const & textTrack : mAvailableTextTracks)
        {
            ReturnErrorOnFailure(encoder.Encode(textTrack));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MediaPlaybackManager::HandleSetCurrentState(PlaybackStateEnum currentState)
{
    Status status = Attributes::CurrentState::Set(mEndpoint, currentState);

    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to set CurrentState attribute, 0x%x", to_underlying(status));
    }

    return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
}

CHIP_ERROR MediaPlaybackManager::HandleSetPlaybackSpeed(float playbackSpeed)
{
    Status status = Attributes::PlaybackSpeed::Set(mEndpoint, playbackSpeed);

    if (Status::Success != status)
    {
        ChipLogError(Zcl, "Unable to set PlaybackSpeed attribute, 0x%x", to_underlying(status));
    }

    return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
}

void MediaPlaybackManager::HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    HandleSetCurrentState(PlaybackStateEnum::kPlaying);
    HandleSetPlaybackSpeed(1);

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    HandleSetCurrentState(PlaybackStateEnum::kPaused);
    HandleSetPlaybackSpeed(0);

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    HandleSetCurrentState(PlaybackStateEnum::kNotPlaying);
    HandleSetPlaybackSpeed(0);
    mPlaybackPosition = { 0, chip::app::DataModel::Nullable<uint64_t>(0) };

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                             const chip::Optional<bool> & audioAdvanceUnmuted)
{
    float playbackSpeed = HandleGetPlaybackSpeed();

    if (playbackSpeed == kPlaybackMaxForwardSpeed)
    {
        // if already at max speed, return error
        Commands::PlaybackResponse::Type response;
        response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
        response.status = StatusEnum::kSpeedOutOfRange;
        helper.Success(response);
        return;
    }

    HandleSetCurrentState(PlaybackStateEnum::kPlaying);
    // Normalize to correct range
    playbackSpeed = (playbackSpeed <= 0 ? 1 : playbackSpeed * 2);
    if (playbackSpeed > kPlaybackMaxForwardSpeed)
    {
        playbackSpeed = kPlaybackMaxForwardSpeed;
    }
    HandleSetPlaybackSpeed(playbackSpeed);

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    HandleSetCurrentState(PlaybackStateEnum::kPlaying);
    HandleSetPlaybackSpeed(1);
    mPlaybackPosition = { 0, chip::app::DataModel::Nullable<uint64_t>(0) };

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                        const chip::Optional<bool> & audioAdvanceUnmuted)
{
    float playbackSpeed = HandleGetPlaybackSpeed();

    if (playbackSpeed == kPlaybackMaxRewindSpeed)
    {
        // if already at max speed in reverse, return error
        Commands::PlaybackResponse::Type response;
        response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
        response.status = StatusEnum::kSpeedOutOfRange;
        helper.Success(response);
        return;
    }

    HandleSetCurrentState(PlaybackStateEnum::kPlaying);
    // Normalize to correct range
    playbackSpeed = (playbackSpeed >= 0 ? -1 : playbackSpeed * 2);
    if (playbackSpeed < kPlaybackMaxRewindSpeed)
    {
        playbackSpeed = kPlaybackMaxRewindSpeed;
    }
    HandleSetPlaybackSpeed(playbackSpeed);

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                              const uint64_t & deltaPositionMilliseconds)
{
    uint64_t newPosition = (mPlaybackPosition.position.Value() > deltaPositionMilliseconds
                                ? mPlaybackPosition.position.Value() - deltaPositionMilliseconds
                                : 0);
    mPlaybackPosition    = { 0, chip::app::DataModel::Nullable<uint64_t>(newPosition) };

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                             const uint64_t & deltaPositionMilliseconds)
{
    uint64_t newPosition = mPlaybackPosition.position.Value() + deltaPositionMilliseconds;
    newPosition          = newPosition > mDuration ? mDuration : newPosition;
    mPlaybackPosition    = { 0, chip::app::DataModel::Nullable<uint64_t>(newPosition) };

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleSeek(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                      const uint64_t & positionMilliseconds)
{
    if (positionMilliseconds > mDuration)
    {
        Commands::PlaybackResponse::Type response;
        response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
        response.status = StatusEnum::kSeekOutOfRange;
        helper.Success(response);
    }
    else
    {
        mPlaybackPosition = { 0, chip::app::DataModel::Nullable<uint64_t>(positionMilliseconds) };

        Commands::PlaybackResponse::Type response;
        response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
        response.status = StatusEnum::kSuccess;
        helper.Success(response);
    }
}

void MediaPlaybackManager::HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    HandleSetCurrentState(PlaybackStateEnum::kPlaying);
    HandleSetPlaybackSpeed(1);
    mPlaybackPosition = { 0, chip::app::DataModel::Nullable<uint64_t>(0) };

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleStartOver(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    mPlaybackPosition = { 0, chip::app::DataModel::Nullable<uint64_t>(0) };

    Commands::PlaybackResponse::Type response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

bool MediaPlaybackManager::HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex)
{
    std::string idString(trackId.data(), trackId.size());
    for (auto const & availableAudioTrack : mAvailableAudioTracks)
    {
        std::string nextIdString(availableAudioTrack.id.data(), availableAudioTrack.id.size());
        if (nextIdString == idString)
        {
            mActiveAudioTrack = availableAudioTrack;
            return true;
        }
    }
    return false;
}

bool MediaPlaybackManager::HandleActivateTextTrack(const chip::CharSpan & trackId)
{
    std::string idString(trackId.data(), trackId.size());
    for (auto const & availableTextTrack : mAvailableTextTracks)
    {
        std::string nextIdString(availableTextTrack.id.data(), availableTextTrack.id.size());
        if (nextIdString == idString)
        {
            mActiveTextTrack = availableTextTrack;
            return true;
        }
    }
    return false;
}

bool MediaPlaybackManager::HandleDeactivateTextTrack()
{
    // Handle Deactivate Text Track
    if (mActiveTextTrack.id.data() != nullptr)
    {
        mActiveTextTrack = {};
    }
    return true;
}

uint32_t MediaPlaybackManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return mDynamicEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

uint16_t MediaPlaybackManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}

static std::map<chip::EndpointId, std::unique_ptr<MediaPlaybackManager>> gMediaPlaybackManagerInstance{};

void emberAfMediaPlaybackClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaPlayback::SetDefaultDelegate, endpoint=%x", endpoint);

    gMediaPlaybackManagerInstance[endpoint] = std::make_unique<MediaPlaybackManager>(endpoint);

    SetDefaultDelegate(endpoint, gMediaPlaybackManagerInstance[endpoint].get());
}

#endif /// MATTER_DM_PLUGIN_MEDIA_PLAYBACK_SERVER
