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

EmberAfMediaPlaybackStatus MediaPlaybackManager::proxyMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                           uint64_t deltaPositionMilliseconds)
{
    switch (mediaPlaybackRequest)
    {
    case MEDIA_PLAYBACK_REQUEST_PLAY:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_PAUSE:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_STOP:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_START_OVER:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_PREVIOUS:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_NEXT:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_REWIND:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_FAST_FORWARD:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD:
    // TODO: Insert code here
    case MEDIA_PLAYBACK_REQUEST_SEEK:
        return EMBER_ZCL_MEDIA_PLAYBACK_STATUS_SUCCESS;
        break;
    default: {
        return EMBER_ZCL_MEDIA_PLAYBACK_STATUS_SUCCESS;
    }
    }
}

EmberAfMediaPlaybackStatus mediaPlaybackClusterSendMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                        uint64_t deltaPositionMilliseconds)
{
    return MediaPlaybackManager().proxyMediaPlaybackRequest(mediaPlaybackRequest, deltaPositionMilliseconds);
}
