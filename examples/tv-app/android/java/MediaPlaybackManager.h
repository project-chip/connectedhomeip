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
#include <app/util/attribute-storage.h>
#include <jni.h>
#include <lib/core/CHIPError.h>

class MediaPlaybackManager
{
public:
    void InitializeWithObjects(jobject managerObject);
    CHIP_ERROR GetAttribute(chip::app::AttributeValueEncoder & aEncoder, int attributeId);
    EmberAfMediaPlaybackStatus Request(MediaPlaybackRequest mediaPlaybackRequest, uint64_t deltaPositionMilliseconds);

private:
    friend MediaPlaybackManager & MediaPlaybackMgr();

    static MediaPlaybackManager sInstance;
    jobject mMediaPlaybackManagerObject = nullptr;
    jmethodID mRequestMethod            = nullptr;
    jmethodID mGetAttributeMethod       = nullptr;
};

inline MediaPlaybackManager & MediaPlaybackMgr()
{
    return MediaPlaybackManager::sInstance;
}
