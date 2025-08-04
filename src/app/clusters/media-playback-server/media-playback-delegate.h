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

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeValueEncoder.h>
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

using TrackStruct = chip::app::Clusters::MediaPlayback::Structs::TrackStruct::Type;

/** @brief
 *    Defines methods for implementing application-specific logic for the Media Playback Cluster.
 */
class Delegate
{
public:
    virtual PlaybackStateEnum HandleGetCurrentState()                                       = 0;
    virtual uint64_t HandleGetStartTime()                                                   = 0;
    virtual uint64_t HandleGetDuration()                                                    = 0;
    virtual CHIP_ERROR HandleGetSampledPosition(app::AttributeValueEncoder & aEncoder)      = 0;
    virtual float HandleGetPlaybackSpeed()                                                  = 0;
    virtual uint64_t HandleGetSeekRangeStart()                                              = 0;
    virtual uint64_t HandleGetSeekRangeEnd()                                                = 0;
    virtual CHIP_ERROR HandleGetActiveAudioTrack(app::AttributeValueEncoder & aEncoder)     = 0;
    virtual CHIP_ERROR HandleGetAvailableAudioTracks(app::AttributeValueEncoder & aEncoder) = 0;
    virtual CHIP_ERROR HandleGetActiveTextTrack(app::AttributeValueEncoder & aEncoder)      = 0;
    virtual CHIP_ERROR HandleGetAvailableTextTracks(app::AttributeValueEncoder & aEncoder)  = 0;

    virtual void HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)               = 0;
    virtual void HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)              = 0;
    virtual void HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)               = 0;
    virtual void HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                   const chip::Optional<bool> & audioAdvanceUnmuted)                        = 0;
    virtual void HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)           = 0;
    virtual void HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                              const chip::Optional<bool> & audioAdvanceUnmuted)                             = 0;
    virtual void HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                    const uint64_t & deltaPositionMilliseconds)                             = 0;
    virtual void HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                   const uint64_t & deltaPositionMilliseconds)                              = 0;
    virtual void HandleSeek(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                            const uint64_t & positionMilliseconds)                                          = 0;
    virtual void HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)               = 0;
    virtual void HandleStartOver(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)          = 0;
    virtual bool HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex) = 0;
    virtual bool HandleActivateTextTrack(const chip::CharSpan & trackId)                                    = 0;
    virtual bool HandleDeactivateTextTrack()                                                                = 0;

    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint)      = 0;
    virtual uint16_t GetClusterRevision(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
