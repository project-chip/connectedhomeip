/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaCommandBase.h"
#include "MediaSubscriptionBase.h"

#include <controller/CHIPCluster.h>
#include <functional>

#include <app-common/zap-generated/cluster-objects.h>

// COMMAND CLASSES
class PlayCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Play::Type,
                                            chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    PlayCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class PauseCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Pause::Type,
                                             chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    PauseCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class StopPlaybackCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Stop::Type,
                                                    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    StopPlaybackCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class NextCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Next::Type,
                                            chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    NextCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class SeekCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Seek::Type,
                                            chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    SeekCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(uint64_t position, std::function<void(CHIP_ERROR)> responseCallback);
};

class SkipForwardCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::SkipForward::Type,
                                                   chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    SkipForwardCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(uint64_t deltaPositionMilliseconds, std::function<void(CHIP_ERROR)> responseCallback);
};

class SkipBackwardCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::SkipBackward::Type,
                                                    chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    SkipBackwardCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(uint64_t deltaPositionMilliseconds, std::function<void(CHIP_ERROR)> responseCallback);
};

class FastForwardCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::FastForward::Type,
                                                   chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    FastForwardCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class RewindCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Rewind::Type,
                                              chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    RewindCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class PreviousCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::Previous::Type,
                                                chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    PreviousCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

class StartOverCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::StartOver::Type,
                                                 chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType>
{
public:
    StartOverCommand() : MediaCommandBase(chip::app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Invoke(std::function<void(CHIP_ERROR)> responseCallback);
};

// SUBSCRIBER CLASSES
class CurrentStateSubscriber : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo>
{
public:
    CurrentStateSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};

class StartTimeSubscriber : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::StartTime::TypeInfo>
{
public:
    StartTimeSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};

class DurationSubscriber : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo>
{
public:
    DurationSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};

class SampledPositionSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo>
{
public:
    SampledPositionSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};

class PlaybackSpeedSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo>
{
public:
    PlaybackSpeedSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};

class SeekRangeEndSubscriber : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo>
{
public:
    SeekRangeEndSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};

class SeekRangeStartSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo>
{
public:
    SeekRangeStartSubscriber() : MediaSubscriptionBase(chip::app::Clusters::MediaPlayback::Id) {}
};
