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

#include <stdint.h>

enum MediaPlaybackRequest : uint8_t
{
    MEDIA_PLAYBACK_REQUEST_PLAY          = 0,
    MEDIA_PLAYBACK_REQUEST_PAUSE         = 1,
    MEDIA_PLAYBACK_REQUEST_STOP          = 2,
    MEDIA_PLAYBACK_REQUEST_START_OVER    = 3,
    MEDIA_PLAYBACK_REQUEST_PREVIOUS      = 4,
    MEDIA_PLAYBACK_REQUEST_NEXT          = 5,
    MEDIA_PLAYBACK_REQUEST_REWIND        = 6,
    MEDIA_PLAYBACK_REQUEST_FAST_FORWARD  = 7,
    MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD  = 8,
    MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD = 9,
    MEDIA_PLAYBACK_REQUEST_SEEK          = 10,
};
