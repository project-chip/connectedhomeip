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
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include "MediaPlaybackManager.h"

using namespace chip;
using namespace chip::app::Clusters::MediaPlayback;

MediaPlaybackManager MediaPlaybackManager::sInstance;

namespace {
static MediaPlaybackManager mediaPlaybackManager;
} // namespace

/** @brief Media PlayBack Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfMediaPlaybackClusterInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Android App: MediaPlayback::SetDefaultDelegate");
    chip::app::Clusters::MediaPlayback::SetDefaultDelegate(endpoint, &mediaPlaybackManager);
}

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
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PLAY, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandlePause()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PAUSE, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleStop()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_STOP, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleFastForward()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_FAST_FORWARD, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandlePrevious()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PREVIOUS, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleRewind()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_REWIND, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleSkipBackward(const uint64_t & deltaPositionMilliseconds)
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD, deltaPositionMilliseconds);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleSkipForward(const uint64_t & deltaPositionMilliseconds)
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD, deltaPositionMilliseconds);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleSeekRequest(const uint64_t & positionMilliseconds)
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SEEK, positionMilliseconds);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleNext()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_NEXT, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleStartOverRequest()
{
    return MediaPlaybackMgr().HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_START_OVER, 0);
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleMediaRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                          uint64_t deltaPositionMilliseconds)
{
    Commands::PlaybackResponse::Type response;

    jint ret       = -1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "MediaPlaybackManager::Request %d-%ld", mediaPlaybackRequest,
                    static_cast<long>(deltaPositionMilliseconds));
    VerifyOrExit(mMediaPlaybackManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRequestMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    env->ExceptionClear();
    ret = env->CallIntMethod(mMediaPlaybackManagerObject, mRequestMethod, static_cast<jlong>(mediaPlaybackRequest),
                             static_cast<jlong>(deltaPositionMilliseconds));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::Request %d", mediaPlaybackRequest);
        env->ExceptionDescribe();
        env->ExceptionClear();
        response.status = StatusEnum::kInvalidStateForCommand;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        response.status = StatusEnum::kInvalidStateForCommand;
    }

    return response;
}

void MediaPlaybackManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for MediaPlaybackManager"));

    mMediaPlaybackManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mMediaPlaybackManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef MediaPlaybackManager"));

    jclass mMediaPlaybackManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(mMediaPlaybackManagerClass != nullptr, ChipLogError(Zcl, "Failed to get MediaPlaybackManager Java class"));

    mGetAttributeMethod = env->GetMethodID(mMediaPlaybackManagerClass, "getAttributes", "(I)J");
    if (mGetAttributeMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'getMediaPlaybackAttribute' method");
        env->ExceptionClear();
    }

    mRequestMethod = env->GetMethodID(mMediaPlaybackManagerClass, "request", "(IJ)I");
    if (mRequestMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'proxyMediaPlaybackRequest' method");
        env->ExceptionClear();
    }
}
