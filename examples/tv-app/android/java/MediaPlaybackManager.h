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

#include <app/clusters/media-playback-server/media-playback-server.h>
#include <cstdint>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <vector>

enum MediaPlaybackRequestAttribute : uint8_t
{
    MEDIA_PLAYBACK_ATTRIBUTE_PLAYBACK_STATE         = 0,
    MEDIA_PLAYBACK_ATTRIBUTE_START_TIME             = 1,
    MEDIA_PLAYBACK_ATTRIBUTE_DURATION               = 2,
    MEDIA_PLAYBACK_ATTRIBUTE_SPEED                  = 3,
    MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_END         = 4,
    MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_START       = 5,
    MEDIA_PLAYBACK_ATTRIBUTE_ACTIVE_AUDIO_TRACK     = 6,
    MEDIA_PLAYBACK_ATTRIBUTE_AVAILABLE_AUDIO_TRACKS = 7,
    MEDIA_PLAYBACK_ATTRIBUTE_ACTIVE_TEXT_TRACK      = 8,
    MEDIA_PLAYBACK_ATTRIBUTE_AVAILABLE_TEXT_TRACKS  = 9,
    MEDIA_PLAYBACK_ATTRIBUTE_COUNT,
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

using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using MediaPlaybackDelegate = chip::app::Clusters::MediaPlayback::Delegate;
using PlaybackResponseType  = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type;
using TrackType             = chip::app::Clusters::MediaPlayback::Structs::TrackStruct::Type;

class MediaPlaybackManager : public MediaPlaybackDelegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    chip::app::Clusters::MediaPlayback::PlaybackStateEnum HandleGetCurrentState() override;
    uint64_t HandleGetStartTime() override;
    uint64_t HandleGetDuration() override;
    CHIP_ERROR HandleGetSampledPosition(AttributeValueEncoder & aEncoder) override;
    float HandleGetPlaybackSpeed() override;
    uint64_t HandleGetSeekRangeStart() override;
    uint64_t HandleGetSeekRangeEnd() override;
    CHIP_ERROR HandleGetActiveTrack(bool audio, AttributeValueEncoder & aEncoder);
    CHIP_ERROR HandleGetAvailableTracks(bool audio, AttributeValueEncoder & aEncoder);
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
    bool HandleActivateTrack(bool audio, const chip::CharSpan & trackId);
    bool HandleDeactivateTextTrack() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

private:
    chip::JniGlobalReference mMediaPlaybackManagerObject;
    jmethodID mRequestMethod             = nullptr;
    jmethodID mGetAttributeMethod        = nullptr;
    jmethodID mGetPositionMethod         = nullptr;
    jmethodID mGetAvailableTracksMethod  = nullptr;
    jmethodID mActivateTrackMethod       = nullptr;
    jmethodID mDeactivateTextTrackMethod = nullptr;
    jmethodID mGetActiveTrackMethod      = nullptr;

    uint64_t HandleMediaRequestGetAttribute(MediaPlaybackRequestAttribute attribute);
    long HandleMediaRequestGetLongAttribute(MediaPlaybackRequestAttribute attribute);
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type
    HandleMediaRequest(MediaPlaybackRequest mediaPlaybackRequest, uint64_t deltaPositionMilliseconds);

    // TODO: set this based upon meta data from app
    static constexpr uint32_t kEndpointFeatureMap = 3;
    static constexpr uint16_t kClusterRevision    = 2;
};
