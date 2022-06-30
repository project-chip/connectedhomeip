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

using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using MediaPlaybackDelegate = chip::app::Clusters::MediaPlayback::Delegate;
using PlaybackResponseType  = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type;
using PlaybackPositionType  = chip::app::Clusters::MediaPlayback::Structs::PlaybackPosition::Type;

class MediaPlaybackManager : public MediaPlaybackDelegate
{
public:
    chip::app::Clusters::MediaPlayback::PlaybackStateEnum HandleGetCurrentState() override;
    uint64_t HandleGetStartTime() override;
    uint64_t HandleGetDuration() override;
    CHIP_ERROR HandleGetSampledPosition(AttributeValueEncoder & aEncoder) override;
    float HandleGetPlaybackSpeed() override;
    uint64_t HandleGetSeekRangeStart() override;
    uint64_t HandleGetSeekRangeEnd() override;

    void HandlePlay(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandlePause(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleStop(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleFastForward(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandlePrevious(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleRewind(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleSkipBackward(CommandResponseHelper<PlaybackResponseType> & helper,
                            const uint64_t & deltaPositionMilliseconds) override;
    void HandleSkipForward(CommandResponseHelper<PlaybackResponseType> & helper,
                           const uint64_t & deltaPositionMilliseconds) override;
    void HandleSeek(CommandResponseHelper<PlaybackResponseType> & helper, const uint64_t & positionMilliseconds) override;
    void HandleNext(CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleStartOver(CommandResponseHelper<PlaybackResponseType> & helper) override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

protected:
    // NOTE: it does not make sense to have default state of playing with a speed of 0, but
    // the CI test cases expect these values, and need to be fixed.
    chip::app::Clusters::MediaPlayback::PlaybackStateEnum mCurrentState =
        chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPlaying;
    PlaybackPositionType mPlaybackPosition = { 0, chip::app::DataModel::Nullable<uint64_t>(0) };
    float mPlaybackSpeed                   = 0;
    uint64_t mStartTime                    = 0;
    // Magic number for testing.
    uint64_t mDuration = 80000;

    static const int kPlaybackMaxForwardSpeed = 10;
    static const int kPlaybackMaxRewindSpeed  = -10;

private:
    // TODO: set this based upon meta data from app
    uint32_t mDynamicEndpointFeatureMap = 3;
};
