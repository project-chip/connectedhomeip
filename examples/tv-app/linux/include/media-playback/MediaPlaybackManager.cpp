/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MediaPlaybackManager.h"

using namespace std;
using namespace chip::app::Clusters::MediaPlayback;

PlaybackStateEnum MediaPlaybackManager::HandleGetCurrentState()
{
    return PlaybackStateEnum::kPlaying;
}

uint64_t MediaPlaybackManager::HandleGetStartTime()
{
    return 0;
}

uint64_t MediaPlaybackManager::HandleGetDuration()
{
    return 0;
}

Structs::PlaybackPosition::Type MediaPlaybackManager::HandleGetSampledPosition()
{
    Structs::PlaybackPosition::Type sampledPosition;
    sampledPosition.updatedAt = 0;
    sampledPosition.position  = 0;
    return sampledPosition;
}

float MediaPlaybackManager::HandleGetPlaybackSpeed()
{
    return 0;
}

uint64_t MediaPlaybackManager::HandleGetSeekRangeStart()
{
    return 0;
}

uint64_t MediaPlaybackManager::HandleGetSeekRangeEnd()
{
    return 0;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandlePlay()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandlePause()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleStop()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleFastForward()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandlePrevious()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleRewind()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleSkipBackward(const uint64_t & deltaPositionMilliseconds)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleSkipForward(const uint64_t & deltaPositionMilliseconds)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleSeekRequest(const uint64_t & positionMilliseconds)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleNext()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleStartOverRequest()
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    return response;
}
