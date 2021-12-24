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

/****************************************************************************
 * @file
 * @brief Routines for the Media Playback plugin, the
 *server implementation of the Media Playback cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/media-playback-server/media-playback-server.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaPlayback;

StatusEnum mediaPlaybackClusterSendMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                        uint64_t deltaPositionMilliseconds);

static void writePlaybackState(EndpointId endpoint, MediaPlayback::PlaybackStateEnum playbackState)
{
    EmberAfStatus status = Attributes::PlaybackState::Set(endpoint, playbackState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

static PlaybackStateEnum readPlaybackStatus(EndpointId endpoint)
{
    chip::app::Clusters::MediaPlayback::PlaybackStateEnum playbackState;
    EmberAfStatus status = Attributes::PlaybackState::Get(endpoint, &playbackState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to read media playback attribute.");
    }

    return playbackState;
}

void storeNewPlaybackState(EndpointId endpoint, MediaPlayback::PlaybackStateEnum newPlaybackState)
{
    MediaPlayback::PlaybackStateEnum mediaPlaybackClusterPlaybackState = readPlaybackStatus(endpoint);

    if (mediaPlaybackClusterPlaybackState == newPlaybackState)
    {
        return;
    }
    else
    {
        writePlaybackState(endpoint, newPlaybackState);
    }
}

static void sendResponse(app::CommandHandler * command, const char * responseName, CommandId commandId,
                         StatusEnum mediaPlaybackStatus)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), MediaPlayback::Id, commandId };
    TLV::TLVWriter * writer       = nullptr;

    VerifyOrExit(command != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = command->PrepareCommand(path));
    VerifyOrExit((writer = command->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), mediaPlaybackStatus));
    SuccessOrExit(err = command->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send %s. Error:%s", responseName, ErrorStr(err));
    }
}

bool emberAfMediaPlaybackClusterPlayRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::PlayRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PLAY, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), MediaPlayback::PlaybackStateEnum::kPlaying);
    sendResponse(command, "MediaPlayResponse", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterPauseRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::PauseRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PAUSE, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), MediaPlayback::PlaybackStateEnum::kPaused);
    sendResponse(command, "MediaPauseResponse", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterStopRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_STOP, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), MediaPlayback::PlaybackStateEnum::kNotPlaying);
    sendResponse(command, "MediaStopResponse", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterFastForwardRequestCallback(app::CommandHandler * command,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::FastForwardRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_FAST_FORWARD, 0);
    sendResponse(command, "MediaFastForward", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterPreviousRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                        const Commands::PreviousRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PREVIOUS, 0);
    sendResponse(command, "MediaPrevious", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterRewindRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                      const Commands::RewindRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_REWIND, 0);
    sendResponse(command, "MediaRewind", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterSkipBackwardRequestCallback(app::CommandHandler * command,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::SkipBackwardRequest::DecodableType & commandData)
{
    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;

    StatusEnum status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD, deltaPositionMilliseconds);
    sendResponse(command, "MediaSkipBackward", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterSkipForwardRequestCallback(app::CommandHandler * command,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::SkipForwardRequest::DecodableType & commandData)
{
    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;

    StatusEnum status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD, deltaPositionMilliseconds);
    sendResponse(command, "MediaSkipForward", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterSeekRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::SeekRequest::DecodableType & commandData)
{
    auto & positionMilliseconds = commandData.position;

    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SEEK, positionMilliseconds);
    sendResponse(command, "MediaSeek", Commands::PlaybackResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterNextRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::NextRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_NEXT, 0);
    sendResponse(command, "MediaNext", Commands::PlaybackResponse::Id, status);
    return true;
}
bool emberAfMediaPlaybackClusterStartOverRequestCallback(app::CommandHandler * command,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::StartOverRequest::DecodableType & commandData)
{
    StatusEnum status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_START_OVER, 0);
    sendResponse(command, "MediaStartOver", Commands::PlaybackResponse::Id, status);
    return true;
}

void MatterMediaPlaybackPluginServerInitCallback() {}
