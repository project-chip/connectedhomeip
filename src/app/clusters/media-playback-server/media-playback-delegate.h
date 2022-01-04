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

#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

/** @brief
 *    Defines methods for implementing application-specific logic for the Media Playback Cluster.
 */
class Delegate
{
public:
    virtual PlaybackStateEnum HandleGetCurrentState()                  = 0;
    virtual uint64_t HandleGetStartTime()                              = 0;
    virtual uint64_t HandleGetDuration()                               = 0;
    virtual Structs::PlaybackPosition::Type HandleGetSampledPosition() = 0;
    virtual float HandleGetPlaybackSpeed()                             = 0;
    virtual uint64_t HandleGetSeekRangeStart()                         = 0;
    virtual uint64_t HandleGetSeekRangeEnd()                           = 0;

    virtual Commands::PlaybackResponse::Type HandlePlay()                                                   = 0;
    virtual Commands::PlaybackResponse::Type HandlePause()                                                  = 0;
    virtual Commands::PlaybackResponse::Type HandleStop()                                                   = 0;
    virtual Commands::PlaybackResponse::Type HandleFastForward()                                            = 0;
    virtual Commands::PlaybackResponse::Type HandlePrevious()                                               = 0;
    virtual Commands::PlaybackResponse::Type HandleRewind()                                                 = 0;
    virtual Commands::PlaybackResponse::Type HandleSkipBackward(const uint64_t & deltaPositionMilliseconds) = 0;
    virtual Commands::PlaybackResponse::Type HandleSkipForward(const uint64_t & deltaPositionMilliseconds)  = 0;
    virtual Commands::PlaybackResponse::Type HandleSeekRequest(const uint64_t & positionMilliseconds)       = 0;
    virtual Commands::PlaybackResponse::Type HandleNext()                                                   = 0;
    virtual Commands::PlaybackResponse::Type HandleStartOverRequest()                                       = 0;

    virtual ~Delegate() = default;
};

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
