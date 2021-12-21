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
#include <app-common/app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>

using namespace chip;

MediaPlaybackManager MediaPlaybackManager::sInstance;

class MediaPlayBackAttrAccess : public app::AttributeAccessInterface
{
public:
    MediaPlayBackAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), app::Clusters::MediaPlayback::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        int attrId = -1;

        switch (aPath.mAttributeId)
        {
        case app::Clusters::MediaPlayback::Attributes::PlaybackState::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_STATE_ATTRIBUTE_ID;
            break;
        }
        case app::Clusters::MediaPlayback::Attributes::StartTime::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_START_TIME_ATTRIBUTE_ID;
            break;
        }
        case app::Clusters::MediaPlayback::Attributes::Duration::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_DURATION_ATTRIBUTE_ID;
            break;
        }
        case app::Clusters::MediaPlayback::Attributes::Position::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_PLAYBACK_POSITION_ATTRIBUTE_ID;
            break;
        }
        case app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_PLAYBACK_SPEED_ATTRIBUTE_ID;
            break;
        }
        case app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_PLAYBACK_SEEK_RANGE_END_ATTRIBUTE_ID;
            break;
        }
        case app::Clusters::MediaPlayback::Attributes::SeekRangeStart::Id: {
            attrId = ZCL_MEDIA_PLAYBACK_PLAYBACK_SEEK_RANGE_START_ATTRIBUTE_ID;
            break;
        }
        }

        if (attrId >= 0)
        {
            return MediaPlaybackMgr().GetAttribute(aEncoder, attrId);
        }

        return CHIP_NO_ERROR;
    }
};

MediaPlayBackAttrAccess gMediaPlayBackAttrAccess;

/** @brief Media PlayBack Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfMediaPlaybackClusterInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gMediaPlayBackAttrAccess);
        attrAccessRegistered = true;
    }
}

chip::app::Clusters::MediaPlayback::StatusEnum
mediaPlaybackClusterSendMediaPlaybackRequest(MediaPlaybackRequest mediaPlaybackRequest, uint64_t deltaPositionMilliseconds)
{
    return MediaPlaybackMgr().Request(mediaPlaybackRequest, deltaPositionMilliseconds);
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

CHIP_ERROR MediaPlaybackManager::GetAttribute(chip::app::AttributeValueEncoder & aEncoder, int attributeId)
{
    jlong jAttributeValue = -1;
    CHIP_ERROR err        = CHIP_NO_ERROR;
    JNIEnv * env          = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received MediaPlaybackManager::GetAttribute:%d", attributeId);
    VerifyOrExit(mMediaPlaybackManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetAttributeMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    jAttributeValue = env->CallLongMethod(mMediaPlaybackManagerObject, mGetAttributeMethod, static_cast<jint>(attributeId));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::GetAttribute");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (jAttributeValue >= 0)
    {
        switch (attributeId)
        {
        case ZCL_MEDIA_PLAYBACK_PLAYBACK_SPEED_ATTRIBUTE_ID: {
            // TODO: Convert to single once it is supported
            // float speed = static_cast<float>(jAttributeValue) / 10000.0f;
            err = aEncoder.Encode(static_cast<uint64_t>(jAttributeValue));
            break;
        }

        default: {
            err = aEncoder.Encode(static_cast<uint64_t>(jAttributeValue));
        }
        }
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::GetAttribute status error: %s", err.AsString());
    }

    return err;
}

chip::app::Clusters::MediaPlayback::StatusEnum MediaPlaybackManager::Request(MediaPlaybackRequest mediaPlaybackRequest,
                                                                             uint64_t deltaPositionMilliseconds)
{
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
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::GetAttribute");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return chip::app::Clusters::MediaPlayback::StatusEnum::kInvalidStateForCommand;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        return chip::app::Clusters::MediaPlayback::StatusEnum::kInvalidStateForCommand;
    }

    return static_cast<chip::app::Clusters::MediaPlayback::StatusEnum>(ret);
}
