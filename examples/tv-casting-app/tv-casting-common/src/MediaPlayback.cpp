/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
