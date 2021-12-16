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

EmberAfMediaPlaybackStatus mediaPlaybackClusterSendMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                        uint64_t deltaPositionMilliseconds);

static void writePlaybackState(EndpointId endpoint, uint8_t playbackState)
{
    EmberAfStatus status = Attributes::PlaybackState::Set(endpoint, playbackState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

static uint8_t readPlaybackStatus(EndpointId endpoint)
{
    uint8_t playbackState;
    EmberAfStatus status = Attributes::PlaybackState::Get(endpoint, &playbackState);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to read media playback attribute.");
    }

    return playbackState;
}

void storeNewPlaybackState(EndpointId endpoint, uint8_t newPlaybackState)
{
    uint8_t mediaPlaybackClusterPlaybackState = readPlaybackStatus(endpoint);

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
                         EmberAfMediaPlaybackStatus mediaPlaybackStatus)
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

bool emberAfMediaPlaybackClusterMediaPlayCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::MediaPlay::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PLAY, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_PLAYING);
    sendResponse(command, "MediaPlayResponse", Commands::MediaPlayResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaPauseCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                   const Commands::MediaPause::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PAUSE, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_PAUSED);
    sendResponse(command, "MediaPauseResponse", Commands::MediaPauseResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaStopCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::MediaStop::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_STOP, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_NOT_PLAYING);
    sendResponse(command, "MediaStopResponse", Commands::MediaStopResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaFastForwardCallback(app::CommandHandler * command,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::MediaFastForward::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_FAST_FORWARD, 0);
    sendResponse(command, "MediaFastForward", Commands::MediaFastForwardResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaPreviousCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                      const Commands::MediaPrevious::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PREVIOUS, 0);
    sendResponse(command, "MediaPrevious", Commands::MediaPreviousResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaRewindCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::MediaRewind::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_REWIND, 0);
    sendResponse(command, "MediaRewind", Commands::MediaRewindResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipBackwardCallback(app::CommandHandler * command,
                                                          const app::ConcreteCommandPath & commandPath,
                                                          const Commands::MediaSkipBackward::DecodableType & commandData)
{
    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;

    EmberAfMediaPlaybackStatus status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD, deltaPositionMilliseconds);
    sendResponse(command, "MediaSkipBackward", Commands::MediaSkipBackwardResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipForwardCallback(app::CommandHandler * command,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::MediaSkipForward::DecodableType & commandData)
{
    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;

    EmberAfMediaPlaybackStatus status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD, deltaPositionMilliseconds);
    sendResponse(command, "MediaSkipForward", Commands::MediaSkipForwardResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSeekCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::MediaSeek::DecodableType & commandData)
{
    auto & positionMilliseconds = commandData.position;

    EmberAfMediaPlaybackStatus status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SEEK, positionMilliseconds);
    sendResponse(command, "MediaSeek", Commands::MediaSeekResponse::Id, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaNextCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::MediaNext::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_NEXT, 0);
    sendResponse(command, "MediaNext", Commands::MediaNextResponse::Id, status);
    return true;
}
bool emberAfMediaPlaybackClusterMediaStartOverCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                       const Commands::MediaStartOver::DecodableType & commandData)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_START_OVER, 0);
    sendResponse(command, "MediaStartOver", Commands::MediaStartOverResponse::Id, status);
    return true;
}

void MatterMediaPlaybackPluginServerInitCallback() {}
