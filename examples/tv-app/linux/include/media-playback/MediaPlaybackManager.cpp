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
#include <app/Command.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <iostream>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR MediaPlaybackManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: Store feature map once it is supported
    map<string, bool> featureMap;
    featureMap["AS"] = true;

    SuccessOrExit(err);
exit:
    return err;
}

static void writePlaybackState(chip::EndpointId endpoint, uint8_t playbackState)
{
    EmberAfStatus status =
        emberAfWriteServerAttribute(endpoint, ZCL_MEDIA_PLAYBACK_CLUSTER_ID, ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID,
                                    (uint8_t *) &playbackState, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store media playback attribute.");
    }
}

static uint8_t readPlaybackStatus(chip::EndpointId endpoint)
{
    uint8_t playbackState;
    EmberAfStatus status =
        emberAfReadServerAttribute(endpoint, ZCL_MEDIA_PLAYBACK_CLUSTER_ID, ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID,
                                   (uint8_t *) &playbackState, sizeof(uint8_t));
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to read media playback attribute.");
    }

    return playbackState;
}

void MediaPlaybackManager::storeNewPlaybackState(chip::EndpointId endpoint, uint8_t newPlaybackState)
{
    oldPlaybackState = readPlaybackStatus(endpoint);

    if (oldPlaybackState == newPlaybackState)
    {
        return;
    }
    else
    {
        writePlaybackState(endpoint, newPlaybackState);
    }
}

static void sendResponse(const char * responseName, chip::CommandId commandId, EmberAfMediaPlaybackStatus mediaPlaybackStatus)
{
    static_assert(std::is_same<std::underlying_type_t<EmberAfMediaPlaybackStatus>, uint8_t>::value, "Wrong enum size");
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_MEDIA_PLAYBACK_CLUSTER_ID,
                              commandId, "u", mediaPlaybackStatus);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to send %s. Error:%s", responseName, chip::ErrorStr(status));
    }
}

EmberAfMediaPlaybackStatus MediaPlaybackManager::proxyMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest)
{
    switch (mediaPlaybackRequest)
    {
    case Play:
    // TODO: Insert code here
    case Pause:
    // TODO: Insert code here
    case Stop:
    // TODO: Insert code here
    case StartOver:
    // TODO: Insert code here
    case Previous:
    // TODO: Insert code here
    case Next:
    // TODO: Insert code here
    case Rewind:
    // TODO: Insert code here
    case FastForward:
    // TODO: Insert code here
    case SkipForward:
    // TODO: Insert code here
    case Seek:
        return EMBER_ZCL_MEDIA_PLAYBACK_STATUS_SUCCESS;
        break;
    default: {
        return EMBER_ZCL_MEDIA_PLAYBACK_STATUS_SUCCESS;
    }
    }
}

bool emberAfMediaPlaybackClusterMediaPlayCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Play);
    MediaPlaybackManager().storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_PLAYING);
    sendResponse("MediaPlayResponse", ZCL_MEDIA_PLAY_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaPauseCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Pause);
    MediaPlaybackManager().storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_PAUSED);
    sendResponse("MediaPauseResponse", ZCL_MEDIA_PAUSE_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaStopCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Stop);
    MediaPlaybackManager().storeNewPlaybackState(emberAfCurrentEndpoint(), EMBER_ZCL_MEDIA_PLAYBACK_STATE_NOT_PLAYING);
    sendResponse("MediaStopResponse", ZCL_MEDIA_STOP_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaFastForwardCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::FastForward);
    sendResponse("MediaFastForward", ZCL_MEDIA_FAST_FORWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaPreviousCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Previous);
    sendResponse("MediaPrevious", ZCL_MEDIA_PREVIOUS_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaRewindCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Rewind);
    sendResponse("MediaRewind", ZCL_MEDIA_REWIND_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipBackwardCallback(chip::app::Command *, uint64_t deltaPositionMilliseconds)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::SkipBackward);
    sendResponse("MediaSkipBackward", ZCL_MEDIA_SKIP_BACKWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipForwardCallback(chip::app::Command *, uint64_t deltaPositionMilliseconds)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::SkipForward);
    sendResponse("MediaSkipForward", ZCL_MEDIA_SKIP_FORWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaSkipSeekCallback(chip::app::Command *, uint64_t positionMilliseconds)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Seek);
    sendResponse("MediaSeek", ZCL_MEDIA_SKIP_FORWARD_RESPONSE_COMMAND_ID, status);
    return true;
}

bool emberAfMediaPlaybackClusterMediaNextCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::Next);
    sendResponse("MediaNext", ZCL_MEDIA_NEXT_RESPONSE_COMMAND_ID, status);
    return true;
}
bool emberAfMediaPlaybackClusterMediaStartOverCallback(chip::app::Command *)
{
    EmberAfMediaPlaybackStatus status = MediaPlaybackManager().proxyMediaPlaybackRequest(MediaPlaybackRequest::StartOver);
    sendResponse("MediaStartOver", ZCL_MEDIA_START_OVER_RESPONSE_COMMAND_ID, status);
    return true;
}
