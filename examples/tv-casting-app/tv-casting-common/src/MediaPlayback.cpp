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

#include "MediaPlayback.h"

using namespace chip;
using namespace chip::app::Clusters;

CHIP_ERROR PlayCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::Play::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR PauseCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::Pause::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR StopPlaybackCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::StopPlayback::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR NextCommand::Invoke(std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::Next::Type request;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR SeekCommand::Invoke(uint64_t position, std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::Seek::Type request;
    request.position = position;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR SkipForwardCommand::Invoke(uint64_t deltaPositionMilliseconds, std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::SkipForward::Type request;
    request.deltaPositionMilliseconds = deltaPositionMilliseconds;
    return MediaCommandBase::Invoke(request, responseCallback);
}

CHIP_ERROR SkipBackwardCommand::Invoke(uint64_t deltaPositionMilliseconds, std::function<void(CHIP_ERROR)> responseCallback)
{
    MediaPlayback::Commands::SkipBackward::Type request;
    request.deltaPositionMilliseconds = deltaPositionMilliseconds;
    return MediaCommandBase::Invoke(request, responseCallback);
}
