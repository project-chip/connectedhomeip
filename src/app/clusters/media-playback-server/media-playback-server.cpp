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

#include <app/Command.h>
#include <app/clusters/media-playback-server/media-playback-server.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>

EmberAfMediaPlaybackStatus mediaPlaybackClusterSendMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                        uint64_t deltaPositionMilliseconds);

static void writePlaybackState(chip::EndpointId endpoint, uint8_t playbackState)
{
    EmberAfStatus status = emberAfWriteServerAttribute(
        endpoint, ZCL_MEDIA_PLAYBACK_CLUSTER_ID, ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID, &playbackState, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

static uint8_t readPlaybackStatus(chip::EndpointId endpoint)
{
    uint8_t playbackState;
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_MEDIA_PLAYBACK_CLUSTER_ID,
                                                      ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID, &playbackState, sizeof(uint8_t));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to read media playback attribute.");
    }

    return playbackState;
}

void storeNewPlaybackState(chip::EndpointId endpoint, uint8_t newPlaybackState)
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

static void sendResponse(chip::app::Command * command, const char * responseName, chip::CommandId commandId,
                         EmberAfMediaPlaybackStatus mediaPlaybackStatus)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    chip::app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_MEDIA_PLAYBACK_CLUSTER_ID, commandId,
                                               (chip::app::CommandPathFlags::kEndpointIdValid) };
    chip::TLV::TLVWriter * writer          = nullptr;

    VerifyOrExit(command != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = command->PrepareCommand(cmdParams));
    VerifyOrExit((writer = command->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(err = writer->Put(chip::TLV::ContextTag(0), mediaPlaybackStatus));
    SuccessOrExit(err = command->FinishCommand());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to send %s. Error:%s", responseName, chip::ErrorStr(err));
    }
}

bool emberAfMediaPlaybackClusterMediaPlayCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PLAY, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_PLAYING);
    sendResponse(command, "MediaPlayResponse", ZCL_MEDIA_PLAY_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaPauseCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PAUSE, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_PAUSED);
    sendResponse(command, "MediaPauseResponse", ZCL_MEDIA_PAUSE_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaStopCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_STOP, 0);
    storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_NOT_PLAYING);
    sendResponse(command, "MediaStopResponse", ZCL_MEDIA_STOP_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaFastForwardCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_FAST_FORWARD, 0);
    sendResponse(command, "MediaFastForward", ZCL_MEDIA_FAST_FORWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaPreviousCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_PREVIOUS, 0);
    sendResponse(command, "MediaPrevious", ZCL_MEDIA_PREVIOUS_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaRewindCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_REWIND, 0);
    sendResponse(command, "MediaRewind", ZCL_MEDIA_REWIND_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipBackwardCallback(chip::app::Command * command, uint64_t deltaPositionMilliseconds)
{
    EmberAfMediaPlaybackStatus status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD, deltaPositionMilliseconds);
    sendResponse(command, "MediaSkipBackward", ZCL_MEDIA_SKIP_BACKWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipForwardCallback(chip::app::Command * command, uint64_t deltaPositionMilliseconds)
{
    EmberAfMediaPlaybackStatus status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD, deltaPositionMilliseconds);
    sendResponse(command, "MediaSkipForward", ZCL_MEDIA_SKIP_FORWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSeekCallback(chip::app::Command * command, uint64_t positionMilliseconds)
{
    EmberAfMediaPlaybackStatus status =
        mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_SEEK, positionMilliseconds);
    sendResponse(command, "MediaSeek", ZCL_MEDIA_SEEK_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaNextCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_NEXT, 0);
    sendResponse(command, "MediaNext", ZCL_MEDIA_NEXT_RESPONSE_COMMAND_ID, status);
    return true;
}
bool emberAfMediaPlaybackClusterMediaStartOverCallback(chip::app::Command * command)
{
    EmberAfMediaPlaybackStatus status = mediaPlaybackClusterSendMediaPlaybackRequest(MEDIA_PLAYBACK_REQUEST_START_OVER, 0);
    sendResponse(command, "MediaStartOver", ZCL_MEDIA_START_OVER_RESPONSE_COMMAND_ID, status);
    return true;
}
