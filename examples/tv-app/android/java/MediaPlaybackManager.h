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

enum MediaPlaybackRequestAttribute : uint8_t
{
    MEDIA_PLAYBACK_ATTRIBUTE_PLAYBACK_STATE   = 0,
    MEDIA_PLAYBACK_ATTRIBUTE_START_TIME       = 1,
    MEDIA_PLAYBACK_ATTRIBUTE_DURATION         = 2,
    MEDIA_PLAYBACK_ATTRIBUTE_SPEED            = 3,
    MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_END   = 4,
    MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_START = 5,
    MEDIA_PLAYBACK_ATTRIBUTE_COUNT,
};

enum MediaPlaybackRequest : uint8_t
{
    MEDIA_PLAYBACK_REQUEST_PLAY          = 0,
    MEDIA_PLAYBACK_REQUEST_PAUSE         = 1,
    MEDIA_PLAYBACK_REQUEST_STOP          = 2,
    MEDIA_PLAYBACK_REQUEST_START_OVER    = 3,
    MEDIA_PLAYBACK_REQUEST_PREVIOUS      = 4,
    MEDIA_PLAYBACK_REQUEST_NEXT          = 5,
    MEDIA_PLAYBACK_REQUEST_REWIND        = 6,
    MEDIA_PLAYBACK_REQUEST_FAST_FORWARD  = 7,
    MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD  = 8,
    MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD = 9,
    MEDIA_PLAYBACK_REQUEST_SEEK          = 10,
};

class MediaPlaybackManager : public chip::app::Clusters::MediaPlayback::Delegate
{
public:
    static void NewManager(jint endpoint, jobject manager);
    void InitializeWithObjects(jobject managerObject);

    chip::app::Clusters::MediaPlayback::PlaybackStateEnum HandleGetCurrentState() override;
    uint64_t HandleGetStartTime() override;
    uint64_t HandleGetDuration() override;
    chip::app::Clusters::MediaPlayback::Structs::PlaybackPosition::Type HandleGetSampledPosition() override;
    float HandleGetPlaybackSpeed() override;
    uint64_t HandleGetSeekRangeStart() override;
    uint64_t HandleGetSeekRangeEnd() override;

    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandlePlay() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandlePause() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandleStop() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandleFastForward() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandlePrevious() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandleRewind() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type
    HandleSkipBackward(const uint64_t & deltaPositionMilliseconds) override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type
    HandleSkipForward(const uint64_t & deltaPositionMilliseconds) override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type
    HandleSeekRequest(const uint64_t & positionMilliseconds) override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandleNext() override;
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type HandleStartOverRequest() override;

private:
    jobject mMediaPlaybackManagerObject = nullptr;
    jmethodID mRequestMethod            = nullptr;
    jmethodID mGetAttributeMethod       = nullptr;
    jmethodID mGetPositionMethod        = nullptr;

    uint64_t HandleMediaRequestGetAttribute(MediaPlaybackRequestAttribute attribute);
    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type
    HandleMediaRequest(MediaPlaybackRequest mediaPlaybackRequest, uint64_t deltaPositionMilliseconds);
};
