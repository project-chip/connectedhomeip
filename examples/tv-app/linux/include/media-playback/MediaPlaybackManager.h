/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app-common/zap-generated/enums.h>
#include <app/clusters/media-playback-server/media-playback-server.h>
#include <app/util/af-types.h>

#include <lib/core/CHIPError.h>

class MediaPlaybackManager
{
public:
    CHIP_ERROR Init();
    void storeNewPlaybackState(chip::EndpointId endpoint, uint8_t newPlaybackState);
    EmberAfMediaPlaybackStatus proxyMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                         uint64_t deltaPositionMilliseconds);

private:
    uint8_t oldPlaybackState;
};
