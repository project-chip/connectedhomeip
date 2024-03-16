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
#include "TvApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <cstdint>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include "MediaPlaybackManager.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::MediaPlayback;
using namespace chip::Uint8;
using chip::CharSpan;

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
    ChipLogProgress(Zcl, "TV Android App: MediaPlayback::PostClusterInit");
    TvAppJNIMgr().PostClusterInit(chip::app::Clusters::MediaPlayback::Id, endpoint);
}

void MediaPlaybackManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "TV Android App: MediaPlayback::SetDefaultDelegate");
    MediaPlaybackManager * mgr = new MediaPlaybackManager();
    mgr->InitializeWithObjects(manager);
    chip::app::Clusters::MediaPlayback::SetDefaultDelegate(static_cast<EndpointId>(endpoint), mgr);
}

PlaybackStateEnum MediaPlaybackManager::HandleGetCurrentState()
{
    uint64_t ret = HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_PLAYBACK_STATE);
    return static_cast<PlaybackStateEnum>(ret);
}

uint64_t MediaPlaybackManager::HandleGetStartTime()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_START_TIME);
}

uint64_t MediaPlaybackManager::HandleGetDuration()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_DURATION);
}

float MediaPlaybackManager::HandleGetPlaybackSpeed()
{
    long ret = HandleMediaRequestGetLongAttribute(MEDIA_PLAYBACK_ATTRIBUTE_SPEED);
    return static_cast<float>(ret);
}

uint64_t MediaPlaybackManager::HandleGetSeekRangeStart()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_START);
}

uint64_t MediaPlaybackManager::HandleGetSeekRangeEnd()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_END);
}

CHIP_ERROR MediaPlaybackManager::HandleGetActiveAudioTrack(AttributeValueEncoder & aEncoder)
{
    return HandleGetActiveTrack(true, aEncoder);
}

CHIP_ERROR MediaPlaybackManager::HandleGetActiveTrack(bool audio, AttributeValueEncoder & aEncoder)
{
    DeviceLayer::StackUnlock unlock;
    Structs::TrackStruct::Type response;
    Structs::TrackAttributesStruct::Type trackAttributes;
    response.trackAttributes = Nullable<Structs::TrackAttributesStruct::Type>(trackAttributes);

    jobject trackObj;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "MediaPlaybackManager::HandleGetActiveAudioTrack");
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetActiveTrackMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();
    trackObj = env->CallObjectMethod(mMediaPlaybackManagerObject.ObjectRef(), mGetActiveTrackMethod, static_cast<jboolean>(audio));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::HandleGetActiveAudioTrack");
        env->ExceptionDescribe();
        env->ExceptionClear();
        goto exit;
    }

    if (trackObj != nullptr)
    {
        jclass trackClass   = env->GetObjectClass(trackObj);
        jfieldID getIdField = env->GetFieldID(trackClass, "id", "Ljava/lang/String;");
        jstring jid         = static_cast<jstring>(env->GetObjectField(trackObj, getIdField));
        JniUtfString id(env, jid);
        if (jid != nullptr)
        {
            response.id = id.charSpan();
        }

        jfieldID getLanguageCodeField = env->GetFieldID(trackClass, "languageCode", "Ljava/lang/String;");
        jstring jlanguagecode         = static_cast<jstring>(env->GetObjectField(trackObj, getLanguageCodeField));
        JniUtfString languageCode(env, jlanguagecode);
        if (jlanguagecode != nullptr)
        {
            trackAttributes.languageCode = languageCode.charSpan();
        }

        jfieldID getDisplayNameField = env->GetFieldID(trackClass, "displayName", "Ljava/lang/String;");
        jstring jdisplayname         = static_cast<jstring>(env->GetObjectField(trackObj, getDisplayNameField));
        JniUtfString displayName(env, jdisplayname);
        if (jdisplayname != nullptr)
        {
            trackAttributes.displayName = Optional<chip::app::DataModel::Nullable<chip::Span<const char>>>(
                Nullable<chip::Span<const char>>(displayName.charSpan()));
        }
    }
    else
    {
        return aEncoder.EncodeNull();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::HandleGetActiveAudioTrack status error: %s", err.AsString());
    }

    return aEncoder.Encode(response);
}

CHIP_ERROR MediaPlaybackManager::HandleGetAvailableAudioTracks(AttributeValueEncoder & aEncoder)
{
    return HandleGetAvailableTracks(true, aEncoder);
}

CHIP_ERROR MediaPlaybackManager::HandleGetAvailableTracks(bool audio, AttributeValueEncoder & aEncoder)
{
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "MediaPlaybackManager::HandleGetAvailableAudioTracks");
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetAvailableTracksMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    return aEncoder.EncodeList([this, env, audio](const auto & encoder) -> CHIP_ERROR {
        jobjectArray trackList = (jobjectArray) env->CallObjectMethod(mMediaPlaybackManagerObject.ObjectRef(),
                                                                      mGetAvailableTracksMethod, static_cast<jboolean>(audio));
        if (env->ExceptionCheck())
        {
            ChipLogError(Zcl, "Java exception in MediaPlaybackManager::HandleGetAvailableAudioTracks");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return CHIP_ERROR_INCORRECT_STATE;
        }

        jint length = env->GetArrayLength(trackList);

        for (jint i = 0; i < length; i++)
        {
            Structs::TrackStruct::Type track;
            Structs::TrackAttributesStruct::Type trackAttributes;
            track.trackAttributes = Nullable<Structs::TrackAttributesStruct::Type>(trackAttributes);

            jobject trackObject = env->GetObjectArrayElement(trackList, i);
            jclass trackClass   = env->GetObjectClass(trackObject);

            jfieldID getIdField = env->GetFieldID(trackClass, "id", "Ljava/lang/String;");
            jstring jid         = static_cast<jstring>(env->GetObjectField(trackObject, getIdField));
            JniUtfString id(env, jid);
            if (jid != nullptr)
            {
                track.id = id.charSpan();
            }

            jfieldID getLanguageCodeField = env->GetFieldID(trackClass, "languageCode", "Ljava/lang/String;");
            jstring jlanguagecode         = static_cast<jstring>(env->GetObjectField(trackObject, getLanguageCodeField));
            JniUtfString languageCode(env, jlanguagecode);
            if (jlanguagecode != nullptr)
            {
                trackAttributes.languageCode = languageCode.charSpan();
            }

            jfieldID getDisplayNameField = env->GetFieldID(trackClass, "displayName", "Ljava/lang/String;");
            jstring jdisplayname         = static_cast<jstring>(env->GetObjectField(trackObject, getDisplayNameField));
            JniUtfString displayName(env, jdisplayname);
            if (jdisplayname != nullptr)
            {
                trackAttributes.displayName = Optional<chip::app::DataModel::Nullable<chip::Span<const char>>>(
                    Nullable<chip::Span<const char>>(displayName.charSpan()));
            }

            ReturnErrorOnFailure(encoder.Encode(track));
        }
        return CHIP_NO_ERROR;
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::HandleGetAvailableAudioTracks status error: %s", err.AsString());
    }

    return err;
}

CHIP_ERROR MediaPlaybackManager::HandleGetActiveTextTrack(AttributeValueEncoder & aEncoder)
{
    return HandleGetActiveTrack(false, aEncoder);
}

CHIP_ERROR MediaPlaybackManager::HandleGetAvailableTextTracks(AttributeValueEncoder & aEncoder)
{
    return HandleGetAvailableTracks(false, aEncoder);
}

void MediaPlaybackManager::HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PLAY, 0));
}

void MediaPlaybackManager::HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PAUSE, 0));
}

void MediaPlaybackManager::HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_STOP, 0));
}

void MediaPlaybackManager::HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                             const chip::Optional<bool> & audioAdvanceUnmuted)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_FAST_FORWARD, 0));
}

void MediaPlaybackManager::HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PREVIOUS, 0));
}

void MediaPlaybackManager::HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                        const chip::Optional<bool> & audioAdvanceUnmuted)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_REWIND, 0));
}

void MediaPlaybackManager::HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                              const uint64_t & deltaPositionMilliseconds)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD, deltaPositionMilliseconds));
}

void MediaPlaybackManager::HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                             const uint64_t & deltaPositionMilliseconds)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD, deltaPositionMilliseconds));
}

void MediaPlaybackManager::HandleSeek(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                      const uint64_t & positionMilliseconds)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SEEK, positionMilliseconds));
}

void MediaPlaybackManager::HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_NEXT, 0));
}

void MediaPlaybackManager::HandleStartOver(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_START_OVER, 0));
}

bool MediaPlaybackManager::HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex)
{
    return HandleActivateTrack(true, trackId);
}

bool MediaPlaybackManager::HandleActivateTrack(bool audio, const chip::CharSpan & trackId)
{
    DeviceLayer::StackUnlock unlock;
    std::string id(trackId.data(), trackId.size());

    jint ret       = -1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "MediaPlaybackManager::HandleActivateAudioTrack");
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mActivateTrackMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    {
        UtfString jniid(env, id.c_str());
        ret = env->CallIntMethod(mMediaPlaybackManagerObject.ObjectRef(), mActivateTrackMethod, static_cast<jboolean>(audio),
                                 jniid.jniValue());
        if (env->ExceptionCheck())
        {
            ChipLogError(AppServer, "Java exception in MediaPlaybackManager::HandleActivateTrack %s", id.c_str());
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
exit:
    return static_cast<int>(ret) == 0;
}

bool MediaPlaybackManager::HandleActivateTextTrack(const chip::CharSpan & trackId)
{
    return HandleActivateTrack(false, trackId);
}

bool MediaPlaybackManager::HandleDeactivateTextTrack()
{
    DeviceLayer::StackUnlock unlock;
    jint ret       = -1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "MediaPlaybackManager::HandleDeactivateTextTrack");
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mDeactivateTextTrackMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();
    ret = env->CallIntMethod(mMediaPlaybackManagerObject.ObjectRef(), mDeactivateTextTrackMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::HandleDeactivateTextTrack");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
exit:
    return static_cast<int>(ret) == 0;
}

void MediaPlaybackManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for MediaPlaybackManager"));

    VerifyOrReturn(mMediaPlaybackManagerObject.Init(managerObject) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mMediaPlaybackManagerObject"));

    jclass mMediaPlaybackManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(mMediaPlaybackManagerClass != nullptr, ChipLogError(Zcl, "Failed to get MediaPlaybackManager Java class"));

    mGetAttributeMethod = env->GetMethodID(mMediaPlaybackManagerClass, "getAttributes", "(I)J");
    if (mGetAttributeMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'getAttributes' method");
        env->ExceptionClear();
    }

    mRequestMethod = env->GetMethodID(mMediaPlaybackManagerClass, "request", "(IJ)I");
    if (mRequestMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'request' method");
        env->ExceptionClear();
    }

    mGetPositionMethod =
        env->GetMethodID(mMediaPlaybackManagerClass, "getPosition", "()Lcom/matter/tv/server/tvapp/MediaPlaybackPosition;");
    if (mGetPositionMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'getPosition' method");
        env->ExceptionClear();
    }

    mGetAvailableTracksMethod =
        env->GetMethodID(mMediaPlaybackManagerClass, "getAvailableTracks", "(Z)[Lcom/matter/tv/server/tvapp/MediaTrack;"); // TODO
    if (mGetAvailableTracksMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'getAvailableTracks' method");
        env->ExceptionClear();
    }

    mActivateTrackMethod = env->GetMethodID(mMediaPlaybackManagerClass, "activateTrack", "(ZLjava/lang/String;)I");
    if (mActivateTrackMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'activateTrack' method");
        env->ExceptionClear();
    }

    mDeactivateTextTrackMethod = env->GetMethodID(mMediaPlaybackManagerClass, "deactivateTextTrack", "()I");
    if (mDeactivateTextTrackMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'deactivateTextTrack' method");
        env->ExceptionClear();
    }

    mGetActiveTrackMethod =
        env->GetMethodID(mMediaPlaybackManagerClass, "getActiveTrack", "(Z)Lcom/matter/tv/server/tvapp/MediaTrack;");
    if (mGetActiveTrackMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access MediaPlaybackManager 'getActiveTrack' method");
        env->ExceptionClear();
    }
}

uint64_t MediaPlaybackManager::HandleMediaRequestGetAttribute(MediaPlaybackRequestAttribute attribute)
{
    DeviceLayer::StackUnlock unlock;
    uint64_t ret          = std::numeric_limits<uint64_t>::max();
    jlong jAttributeValue = -1;
    CHIP_ERROR err        = CHIP_NO_ERROR;
    JNIEnv * env          = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, ret, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaPlaybackManager::HandleMediaRequestGetAttribute:%d", attribute);
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetAttributeMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    jAttributeValue =
        env->CallLongMethod(mMediaPlaybackManagerObject.ObjectRef(), mGetAttributeMethod, static_cast<jint>(attribute));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::GetAttribute");
        env->ExceptionDescribe();
        env->ExceptionClear();
        goto exit;
    }

    if (jAttributeValue >= 0)
    {
        ret = static_cast<uint64_t>(jAttributeValue);
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

    return ret;
}

long MediaPlaybackManager::HandleMediaRequestGetLongAttribute(MediaPlaybackRequestAttribute attribute)
{
    DeviceLayer::StackUnlock unlock;
    long ret              = 0;
    jlong jAttributeValue = -1;
    CHIP_ERROR err        = CHIP_NO_ERROR;
    JNIEnv * env          = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, false, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "Received MediaPlaybackManager::HandleMediaRequestGetLongAttribute:%d", attribute);
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetAttributeMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();

    jAttributeValue =
        env->CallLongMethod(mMediaPlaybackManagerObject.ObjectRef(), mGetAttributeMethod, static_cast<jint>(attribute));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::GetAttribute");
        env->ExceptionDescribe();
        env->ExceptionClear();
        goto exit;
    }

    ret = static_cast<long>(jAttributeValue);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::GetAttribute status error: %s", err.AsString());
    }

    return ret;
}

Commands::PlaybackResponse::Type MediaPlaybackManager::HandleMediaRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                          uint64_t deltaPositionMilliseconds)

{
    DeviceLayer::StackUnlock unlock;
    Commands::PlaybackResponse::Type response;

    jint ret       = -1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, response, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "MediaPlaybackManager::Request %d-%ld", mediaPlaybackRequest,
                    static_cast<long>(deltaPositionMilliseconds));
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRequestMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();
    ret = env->CallIntMethod(mMediaPlaybackManagerObject.ObjectRef(), mRequestMethod, static_cast<jint>(mediaPlaybackRequest),
                             static_cast<jlong>(deltaPositionMilliseconds));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::Request %d", mediaPlaybackRequest);
        env->ExceptionDescribe();
        env->ExceptionClear();
        response.status = StatusEnum::kInvalidStateForCommand;
    }
    response.status = static_cast<StatusEnum>(ret);

exit:
    if (err != CHIP_NO_ERROR)
    {
        response.status = StatusEnum::kInvalidStateForCommand;
        ChipLogError(Zcl, "MediaPlaybackManager::HandleMediaRequest status error: %s", err.AsString());
    }

    return response;
}

CHIP_ERROR MediaPlaybackManager::HandleGetSampledPosition(AttributeValueEncoder & aEncoder)
{
    DeviceLayer::StackUnlock unlock;
    Structs::PlaybackPositionStruct::Type response;
    response.updatedAt = 0;
    response.position  = Nullable<uint64_t>(0);

    jobject positionObj;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT, ChipLogError(Zcl, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    ChipLogProgress(Zcl, "MediaPlaybackManager::HandleGetSampledPosition");
    VerifyOrExit(mMediaPlaybackManagerObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mGetPositionMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->ExceptionClear();
    positionObj = env->CallObjectMethod(mMediaPlaybackManagerObject.ObjectRef(), mGetPositionMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in MediaPlaybackManager::HandleGetSampledPosition");
        env->ExceptionDescribe();
        env->ExceptionClear();
        goto exit;
    }

    {
        jclass inputClass    = env->GetObjectClass(positionObj);
        jfieldID positionId  = env->GetFieldID(inputClass, "position", "J");
        jfieldID updatedAtId = env->GetFieldID(inputClass, "updatedAt", "J");
        response.position    = Nullable<uint64_t>(static_cast<uint64_t>(env->GetLongField(positionObj, positionId)));
        response.updatedAt   = static_cast<uint64_t>(env->GetLongField(positionObj, updatedAtId));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::GetAttribute status error: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return aEncoder.Encode(response);
}

uint32_t MediaPlaybackManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

uint16_t MediaPlaybackManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "MediaPlaybackManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
