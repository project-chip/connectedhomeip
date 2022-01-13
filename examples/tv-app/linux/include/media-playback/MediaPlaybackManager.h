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

class MediaPlaybackManager : public chip::app::Clusters::MediaPlayback::Delegate
{
public:
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
};
