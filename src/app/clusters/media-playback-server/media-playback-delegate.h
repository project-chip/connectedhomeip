/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
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
    virtual PlaybackStateEnum HandleGetCurrentState()                                  = 0;
    virtual uint64_t HandleGetStartTime()                                              = 0;
    virtual uint64_t HandleGetDuration()                                               = 0;
    virtual CHIP_ERROR HandleGetSampledPosition(app::AttributeValueEncoder & aEncoder) = 0;
    virtual float HandleGetPlaybackSpeed()                                             = 0;
    virtual uint64_t HandleGetSeekRangeStart()                                         = 0;
    virtual uint64_t HandleGetSeekRangeEnd()                                           = 0;

    virtual void HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)        = 0;
    virtual void HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)       = 0;
    virtual void HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)        = 0;
    virtual void HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper) = 0;
    virtual void HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)    = 0;
    virtual void HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)      = 0;
    virtual void HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                    const uint64_t & deltaPositionMilliseconds)                      = 0;
    virtual void HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                   const uint64_t & deltaPositionMilliseconds)                       = 0;
    virtual void HandleSeek(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                            const uint64_t & positionMilliseconds)                                   = 0;
    virtual void HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)        = 0;
    virtual void HandleStartOver(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)   = 0;

    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
