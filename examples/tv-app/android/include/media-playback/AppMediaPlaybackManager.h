/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include "../../java/ContentAppAttributeDelegate.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/media-playback-server/media-playback-server.h>
#include <cstdint>
#include <jni.h>
#include <vector>

enum MediaPlaybackRequestAttribute : chip::AttributeId
{
    MEDIA_PLAYBACK_ATTRIBUTE_PLAYBACK_STATE         = chip::app::Clusters::MediaPlayback::Attributes::CurrentState::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_START_TIME             = chip::app::Clusters::MediaPlayback::Attributes::StartTime::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_DURATION               = chip::app::Clusters::MediaPlayback::Attributes::Duration::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_SPEED                  = chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_END         = chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_START       = chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_ACTIVE_AUDIO_TRACK     = chip::app::Clusters::MediaPlayback::Attributes::ActiveAudioTrack::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_AVAILABLE_AUDIO_TRACKS = chip::app::Clusters::MediaPlayback::Attributes::AvailableAudioTracks::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_ACTIVE_TEXT_TRACK      = chip::app::Clusters::MediaPlayback::Attributes::ActiveTextTrack::Id,
    MEDIA_PLAYBACK_ATTRIBUTE_AVAILABLE_TEXT_TRACKS  = chip::app::Clusters::MediaPlayback::Attributes::AvailableTextTracks::Id,
};

enum MediaPlaybackRequest : uint8_t
{
    MEDIA_PLAYBACK_REQUEST_PLAY                  = 0,
    MEDIA_PLAYBACK_REQUEST_PAUSE                 = 1,
    MEDIA_PLAYBACK_REQUEST_STOP                  = 2,
    MEDIA_PLAYBACK_REQUEST_START_OVER            = 3,
    MEDIA_PLAYBACK_REQUEST_PREVIOUS              = 4,
    MEDIA_PLAYBACK_REQUEST_NEXT                  = 5,
    MEDIA_PLAYBACK_REQUEST_REWIND                = 6,
    MEDIA_PLAYBACK_REQUEST_FAST_FORWARD          = 7,
    MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD          = 8,
    MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD         = 9,
    MEDIA_PLAYBACK_REQUEST_SEEK                  = 10,
    MEDIA_PLAYBACK_REQUEST_ACTIVATE_AUDIO_TRACK  = 11,
    MEDIA_PLAYBACK_REQUEST_ACTIVATE_TEXT_TRACK   = 12,
    MEDIA_PLAYBACK_REQUEST_DEACTIVATE_TEXT_TRACK = 13,
};

using chip::EndpointId;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using MediaPlaybackDelegate       = chip::app::Clusters::MediaPlayback::Delegate;
using PlaybackResponseType        = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type;
using ContentAppAttributeDelegate = chip::AppPlatform::ContentAppAttributeDelegate;
using TrackType                   = chip::app::Clusters::MediaPlayback::Structs::TrackStruct::Type;

class AppMediaPlaybackManager : public MediaPlaybackDelegate
{
public:
    AppMediaPlaybackManager(ContentAppAttributeDelegate * attributeDelegate);

    chip::app::Clusters::MediaPlayback::PlaybackStateEnum HandleGetCurrentState() override;
    uint64_t HandleGetStartTime() override;
    uint64_t HandleGetDuration() override;
    CHIP_ERROR HandleGetSampledPosition(AttributeValueEncoder & aEncoder) override;
    float HandleGetPlaybackSpeed() override;
    uint64_t HandleGetSeekRangeStart() override;
    uint64_t HandleGetSeekRangeEnd() override;
    CHIP_ERROR HandleGetActiveAudioTrack(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAvailableAudioTracks(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetActiveTextTrack(AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAvailableTextTracks(AttributeValueEncoder & aEncoder) override;

    void HandlePlay(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandlePause(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleStop(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleFastForward(CommandResponseHelper<PlaybackResponseType> & helper,
                           const chip::Optional<bool> & audioAdvanceUnmuted) override;
    void HandlePrevious(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleRewind(CommandResponseHelper<PlaybackResponseType> & helper,
                      const chip::Optional<bool> & audioAdvanceUnmuted) override;
    void HandleSkipBackward(CommandResponseHelper<PlaybackResponseType> & helper,
                            const uint64_t & deltaPositionMilliseconds) override;
    void HandleSkipForward(CommandResponseHelper<PlaybackResponseType> & helper,
                           const uint64_t & deltaPositionMilliseconds) override;
    void HandleSeek(CommandResponseHelper<PlaybackResponseType> & helper, const uint64_t & positionMilliseconds) override;
    void HandleNext(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleStartOver(CommandResponseHelper<PlaybackResponseType> & helper) override;
    bool HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex) override;
    bool HandleActivateTextTrack(const chip::CharSpan & trackId) override;
    bool HandleDeactivateTextTrack() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

    void SetEndpointId(EndpointId epId) { mEndpointId = epId; };

private:
    uint64_t HandleMediaRequestGetAttribute(chip::AttributeId attribute);
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type
    HandleMediaRequest(MediaPlaybackRequest mediaPlaybackRequest, uint64_t deltaPositionMilliseconds);

    EndpointId mEndpointId;

    // TODO: set this based upon meta data from app
    static constexpr uint32_t kEndpointFeatureMap = 3;
    static constexpr uint16_t kClusterRevision    = 2;

    ContentAppAttributeDelegate * mAttributeDelegate;
};
