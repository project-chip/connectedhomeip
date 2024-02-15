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

#pragma once

#include "ConversionUtils.h"
#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

#include <controller/CHIPCluster.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <app-common/zap-generated/cluster-objects.h>

class CallbackBaseJNI
{
public:
    CallbackBaseJNI(const char * methodSignature) { mMethodSignature = methodSignature; }
    CHIP_ERROR SetUp(JNIEnv * env, jobject inHandler);

protected:
    chip::JniGlobalReference mObject;
    jclass mClazz                 = nullptr;
    jclass mSuperClazz            = nullptr;
    jmethodID mMethod             = nullptr;
    const char * mMethodSignature = nullptr;
};

class FailureHandlerJNI : public CallbackBaseJNI
{
public:
    FailureHandlerJNI() : CallbackBaseJNI("(ILjava/lang/String;)V") {}
    void Handle(CHIP_ERROR err);
};

class MatterCallbackHandlerJNI : public FailureHandlerJNI
{
};

class SubscriptionEstablishedHandlerJNI : public CallbackBaseJNI
{
public:
    SubscriptionEstablishedHandlerJNI() : CallbackBaseJNI("()V") {}
    void Handle();
};

// helper functions for conversions
jobject ConvertToLongJObject(uint64_t responseData);
jobject ConvertToFloatJObject(float responseData);
jobject ConvertToShortJObject(uint8_t responseData);
jobject ConvertToByteJObject(uint8_t responseData);
jobject ConvertToIntegerJObject(uint32_t responseData);
jstring ConvertToJString(chip::CharSpan responseData);

template <typename T>
class SuccessHandlerJNI : public CallbackBaseJNI
{
public:
    SuccessHandlerJNI(const char * methodSignature) : CallbackBaseJNI(methodSignature) {}

    virtual ~SuccessHandlerJNI() = 0;

    virtual jobject ConvertToJObject(T responseData) = 0;

    void Handle(T responseData)
    {
        ChipLogProgress(AppServer, "SuccessHandlerJNI::Handle called");

        JNIEnv * env          = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
        jobject jResponseData = ConvertToJObject(responseData);

        chip::DeviceLayer::StackUnlock unlock;
        CHIP_ERROR err = CHIP_NO_ERROR;
        VerifyOrExit(mObject.HasValidObjectRef(), err = CHIP_ERROR_INCORRECT_STATE);
        VerifyOrExit(mMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        env->CallVoidMethod(mObject.ObjectRef(), mMethod, jResponseData);
    exit:
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SuccessHandlerJNI::Handle status error: %s", err.AsString());
        }
    }
};

template <typename T>
SuccessHandlerJNI<T>::~SuccessHandlerJNI(){};

// COMMISSIONING AND CONNECTION
class SessionEstablishmentStartedHandlerJNI : public SuccessHandlerJNI<void *>
{
public:
    SessionEstablishmentStartedHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject ConvertToJObject(void * responseData)
    {
        // return nullptr because the Java callback extends SuccessCallback<Void> and its handle() expects a Void param.
        // It expects a Void becauase no value is passed as part of this callback.
        return nullptr;
    }
};

class SessionEstablishedHandlerJNI : public SuccessHandlerJNI<void *>
{
public:
    SessionEstablishedHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject ConvertToJObject(void * responseData)
    {
        // return nullptr because the Java callback extends SuccessCallback<Void> and its handle() expects a Void param.
        // It expects a Void becauase no value is passed as part of this callback.
        return nullptr;
    }
};

class OnConnectionSuccessHandlerJNI : public SuccessHandlerJNI<TargetVideoPlayerInfo *>
{
public:
    OnConnectionSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject ConvertToJObject(TargetVideoPlayerInfo * responseData);
};

class OnNewOrUpdatedEndpointHandlerJNI : public SuccessHandlerJNI<TargetEndpointInfo *>
{
public:
    OnNewOrUpdatedEndpointHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject ConvertToJObject(TargetEndpointInfo * responseData);
};

// MEDIA PLAYBACK
class CurrentStateSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType>
{
public:
    CurrentStateSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType responseData);
};

class DurationSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType>
{
public:
    DurationSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Long;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::MediaPlayback::Attributes::Duration::TypeInfo::DecodableArgType responseData);
};

class SampledPositionSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType>
{
public:
    SampledPositionSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject
    ConvertToJObject(chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::TypeInfo::DecodableArgType responseData);
};

class PlaybackSpeedSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType>
{
public:
    PlaybackSpeedSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Float;)V") {}
    jobject
    ConvertToJObject(chip::app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::TypeInfo::DecodableArgType responseData);
};

class SeekRangeEndSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType>
{
public:
    SeekRangeEndSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Long;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::TypeInfo::DecodableArgType responseData);
};

class SeekRangeStartSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo::DecodableArgType>
{
public:
    SeekRangeStartSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Long;)V") {}
    jobject
    ConvertToJObject(chip::app::Clusters::MediaPlayback::Attributes::SeekRangeStart::TypeInfo::DecodableArgType responseData);
};

// TARGET NAVIGATOR
class CurrentTargetSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType>
{
public:
    CurrentTargetSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Byte;)V") {}
    jobject
    ConvertToJObject(chip::app::Clusters::TargetNavigator::Attributes::CurrentTarget::TypeInfo::DecodableArgType responseData);
};

class TargetListSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType>
{
public:
    TargetListSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Object;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::TargetNavigator::Attributes::TargetList::TypeInfo::DecodableArgType responseData);
};

// LEVEL CONTROL
class CurrentLevelSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType>
{
public:
    CurrentLevelSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Byte;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo::DecodableArgType responseData);
};

class MinLevelSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType>
{
public:
    MinLevelSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Byte;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::LevelControl::Attributes::MinLevel::TypeInfo::DecodableArgType responseData);
};

class MaxLevelSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType>
{
public:
    MaxLevelSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Byte;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::LevelControl::Attributes::MaxLevel::TypeInfo::DecodableArgType responseData);
};

// CONTENT LAUNCHER
class SupportedStreamingProtocolsSuccessHandlerJNI
    : public SuccessHandlerJNI<
          chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType>
{
public:
    SupportedStreamingProtocolsSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Integer;)V") {}
    jobject ConvertToJObject(
        chip::app::Clusters::ContentLauncher::Attributes::SupportedStreamingProtocols::TypeInfo::DecodableArgType responseData);
};

// APPLICATION BASIC
class VendorNameSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType>
{
public:
    VendorNameSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/String;)V") {}
    jobject
    ConvertToJObject(chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo::DecodableArgType responseData);
};

class VendorIDSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>
{
public:
    VendorIDSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Integer;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType responseData);
};

class ApplicationNameSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType>
{
public:
    ApplicationNameSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/String;)V") {}
    jobject
    ConvertToJObject(chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo::DecodableArgType responseData);
};

class ProductIDSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType>
{
public:
    ProductIDSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/Integer;)V") {}
    jobject ConvertToJObject(chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo::DecodableArgType responseData);
};

class ApplicationVersionSuccessHandlerJNI
    : public SuccessHandlerJNI<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType>
{
public:
    ApplicationVersionSuccessHandlerJNI() : SuccessHandlerJNI("(Ljava/lang/String;)V") {}
    jobject ConvertToJObject(
        chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo::DecodableArgType responseData);
};
