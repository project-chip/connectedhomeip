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

#include <functional>
#include <zap-generated/CHIPClusters.h>

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

class StopPlaybackCommand : public MediaCommandBase<chip::app::Clusters::MediaPlayback::Commands::StopPlayback::Type,
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
