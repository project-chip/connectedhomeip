/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MatterCallbackHandler-JNI.h"

using namespace chip;

CHIP_ERROR CallbackBaseJNI::SetUp(JNIEnv * env, jobject inHandler)
{
    ChipLogProgress(AppServer, "CallbackBaseJNI::SetUp called");
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mObject.Init(inHandler) == CHIP_NO_ERROR, ChipLogError(AppServer, "Failed to Init mObject"));

    mClazz = env->GetObjectClass(mObject.ObjectRef());
    VerifyOrExit(mClazz != nullptr, ChipLogError(AppServer, "Failed to get handler Java class"));

    mSuperClazz = env->GetSuperclass(mClazz);
    VerifyOrExit(mSuperClazz != nullptr, ChipLogError(AppServer, "Failed to get handler's parent's Java class"));

    mMethod = env->GetMethodID(mSuperClazz, "handleInternal", mMethodSignature);
    if (mMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access 'handleInternal' method with signature %s", mMethodSignature);
        env->ExceptionClear();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CallbackBaseJNI::SetUp error: %s", err.AsString());
        return err;
    }

    return err;
}

void FailureHandlerJNI::Handle(CHIP_ERROR callbackErr)
{
    ChipLogProgress(AppServer, "Handle(CHIP_ERROR) called");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    UtfString jniCallbackErrString(env, callbackErr.AsString());

    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    env->CallVoidMethod(mObject.ObjectRef(), mMethod, static_cast<jint>(callbackErr.AsInteger()), jniCallbackErrString.jniValue());
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Handle(CHIP_ERROR) status error: %s", err.AsString());
    }
}

void SubscriptionEstablishedHandlerJNI::Handle()
{
    ChipLogProgress(AppServer, "SubscriptionEstablishedHandlerJNI::Handle called");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    env->CallVoidMethod(mObject.ObjectRef(), mMethod);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SubscriptionEstablishedHandlerJNI::Handle status error: %s", err.AsString());
    }
}

jobject ConvertToLongJObject(uint64_t responseData)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass responseTypeClass = env->FindClass("java/lang/Long");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(J)V");
    return env->NewObject(responseTypeClass, constructor, responseData);
}

jobject ConvertToFloatJObject(float responseData)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass responseTypeClass = env->FindClass("java/lang/Float");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(F)V");
    return env->NewObject(responseTypeClass, constructor, responseData);
}

jobject ConvertToShortJObject(uint16_t responseData)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass responseTypeClass = env->FindClass("java/lang/Short");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(S)V");
    return env->NewObject(responseTypeClass, constructor, responseData);
}

jobject ConvertToByteJObject(uint8_t responseData)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass responseTypeClass = env->FindClass("java/lang/Byte");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(B)V");
    return env->NewObject(responseTypeClass, constructor, responseData);
}

jstring ConvertToJString(chip::CharSpan responseData)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    return env->NewStringUTF(std::string(responseData.data(), responseData.size()).c_str());
}

jobject ConvertToIntegerJObject(uint32_t responseData)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jclass responseTypeClass = env->FindClass("java/lang/Integer");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(I)V");
    return env->NewObject(responseTypeClass, constructor, responseData);
}

// COMMISSIONING AND CONNECTION
jobject OnConnectionSuccessHandlerJNI::ConvertToJObject(TargetVideoPlayerInfo * targetVideoPlayerInfo)
{
    ChipLogProgress(AppServer, "OnConnectionSuccessHandlerJNI::ConvertToJObject called");
    jobject videoPlayer = nullptr;
    CHIP_ERROR err      = convertTargetVideoPlayerInfoToJVideoPlayer(targetVideoPlayerInfo, videoPlayer);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OnConnectionSuccessHandlerJNI::ConvertToJObject failed with %" CHIP_ERROR_FORMAT, err.Format());
    }
    return videoPlayer;
}

jobject OnNewOrUpdatedEndpointHandlerJNI::ConvertToJObject(TargetEndpointInfo * targetEndpointInfo)
{
    ChipLogProgress(AppServer, "OnNewOrUpdatedEndpointHandlerJNI::ConvertToJObject called");
    jobject contentApp = nullptr;
    CHIP_ERROR err     = convertTargetEndpointInfoToJContentApp(targetEndpointInfo, contentApp);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "OnNewOrUpdatedEndpointHandlerJNI::ConvertToJObject failed with %" CHIP_ERROR_FORMAT, err.Format());
    }
    return contentApp;
}

// MEDIA PLAYBACK
jobject CurrentStateSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "CurrentStateSuccessHandlerJNI::ConvertToJObject called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr);
    JniLocalReferenceScope scope(env);

    jclass enumClass = nullptr;
    CHIP_ERROR err =
        JniReferences::GetInstance().GetLocalClassRef(env, "com/chip/casting/MediaPlaybackTypes$PlaybackStateEnum", enumClass);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        return nullptr;
    }

    jfieldID enumType = nullptr;
    switch (responseData)
    {
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPlaying:
        enumType = env->GetStaticFieldID(enumClass, "Playing", "Lcom/chip/casting/MediaPlaybackTypes$PlaybackStateEnum;");
        break;
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPaused:
        enumType = env->GetStaticFieldID(enumClass, "Paused", "Lcom/chip/casting/MediaPlaybackTypes$PlaybackStateEnum;");
        break;
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kNotPlaying:
        enumType = env->GetStaticFieldID(enumClass, "NotPlaying", "Lcom/chip/casting/MediaPlaybackTypes$PlaybackStateEnum;");
        break;
    case chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kBuffering:
        enumType = env->GetStaticFieldID(enumClass, "Buffering", "Lcom/chip/casting/MediaPlaybackTypes$PlaybackStateEnum;");
        break;
    default:
        enumType = env->GetStaticFieldID(enumClass, "Unknown", "Lcom/chip/casting/MediaPlaybackTypes$PlaybackStateEnum;");
        break;
    }

    if (enumType != nullptr)
    {
        return env->GetStaticObjectField(enumClass, enumType);
    }
    return nullptr;
}

jobject DurationSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "DurationSuccessHandlerJNI::ConvertToJObject called");
    return responseData.IsNull() ? nullptr : ConvertToLongJObject(responseData.Value());
}

jobject SampledPositionSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "SampledPositionSuccessHandlerJNI::ConvertToJObject called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr);
    JniLocalReferenceScope scope(env);

    jobject jSampledPosition = nullptr;
    if (!responseData.IsNull())
    {
        const chip::app::Clusters::MediaPlayback::Structs::PlaybackPositionStruct::DecodableType & playbackPosition =
            responseData.Value();

        jclass responseTypeClass = nullptr;
        CHIP_ERROR err           = JniReferences::GetInstance().GetLocalClassRef(
            env, "com/chip/casting/MediaPlaybackTypes$PlaybackPositionStruct", responseTypeClass);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
            return nullptr;
        }

        if (playbackPosition.position.IsNull())
        {
            jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(Ljava/lang/Long;)V");
            jSampledPosition      = env->NewObject(responseTypeClass, constructor, playbackPosition.updatedAt);
        }
        else
        {
            jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(Ljava/lang/Long;java/lang/Long;)V");
            jSampledPosition =
                env->NewObject(responseTypeClass, constructor, playbackPosition.updatedAt, playbackPosition.position.Value());
        }
    }

    return jSampledPosition;
}

jobject PlaybackSpeedSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "PlaybackSpeedSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToFloatJObject(responseData);
}

jobject SeekRangeEndSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "SeekRangeEndSuccessHandlerJNI::ConvertToJObject called");
    return responseData.IsNull() ? nullptr : ConvertToLongJObject(responseData.Value());
}

jobject SeekRangeStartSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "SeekRangeStartSuccessHandlerJNI::ConvertToJObject called");
    return responseData.IsNull() ? nullptr : ConvertToLongJObject(responseData.Value());
}

// TARGET NAVIGATOR
jobject CurrentTargetSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "CurrentTargetSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToByteJObject(responseData);
}

jobject TargetListSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "TargetListSuccessHandlerJNI::ConvertToJObject called");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr);
    JniLocalReferenceScope scope(env);

    jobject jArrayList;
    chip::JniReferences::GetInstance().CreateArrayList(jArrayList);
    auto iter = responseData.begin();
    while (iter.Next())
    {
        const chip::app::Clusters::TargetNavigator::Structs::TargetInfoStruct::DecodableType & targetInfo = iter.GetValue();

        jclass responseTypeClass = nullptr;
        CHIP_ERROR err = JniReferences::GetInstance().GetLocalClassRef(env, "com/chip/casting/TargetNavigatorTypes$TargetInfo",
                                                                       responseTypeClass);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
            return nullptr;
        }

        jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/String;)V");
        chip::UtfString targetInfoName(env, targetInfo.name);
        jobject jTargetInfo = env->NewObject(responseTypeClass, constructor, ConvertToIntegerJObject(targetInfo.identifier),
                                             targetInfoName.jniValue());

        chip::JniReferences::GetInstance().AddToList(jArrayList, jTargetInfo);
    }
    return jArrayList;
}

// LEVEL CONTROL
jobject CurrentLevelSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "CurrentLevelSuccessHandlerJNI::ConvertToJObject called");
    return responseData.IsNull() ? nullptr : ConvertToByteJObject(responseData.Value());
}

jobject MinLevelSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "MinLevelSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToByteJObject(responseData);
}

jobject MaxLevelSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "MaxLevelSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToByteJObject(responseData);
}

// CONTENT LAUNCHER
jobject SupportedStreamingProtocolsSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "SupportedStreamingProtocolsSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToIntegerJObject(responseData.Raw());
}

// APPLICATION BASIC
jobject VendorNameSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "VendorNameSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToJString(responseData);
}

jobject VendorIDSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "VendorIDSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToIntegerJObject(responseData);
}

jobject ApplicationNameSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "ApplicationNameSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToJString(responseData);
}

jobject ProductIDSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "ProductIDSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToIntegerJObject(responseData);
}

jobject ApplicationVersionSuccessHandlerJNI::ConvertToJObject(
    chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType responseData)
{
    ChipLogProgress(AppServer, "ApplicationVersionSuccessHandlerJNI::ConvertToJObject called");
    return ConvertToJString(responseData);
}
