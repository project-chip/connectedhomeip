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
using namespace chip::app::DataModel;
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

CHIP_ERROR MediaPlaybackManager::HandleGetSampledPosition(AttributeValueEncoder & aEncoder)
{
    Structs::PlaybackPosition::Type sampledPosition;
    sampledPosition.updatedAt = 0;
    sampledPosition.position  = Nullable<uint64_t>(0);

    return aEncoder.Encode(sampledPosition);
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

void MediaPlaybackManager::HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                              const uint64_t & deltaPositionMilliseconds)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                             const uint64_t & deltaPositionMilliseconds)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleSeekRequest(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                             const uint64_t & positionMilliseconds)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

void MediaPlaybackManager::HandleStartOverRequest(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    // TODO: Insert code here
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}
