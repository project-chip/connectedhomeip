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

/**
 * @file casting-CHIPEventTLVValueDecoder.cpp
 *
 * Slim event TLV decoder for the TV Casting App.
 * Contains DecodeEventValue() cases for ONLY the casting clusters.
 *
 * === Included Clusters (18) ===
 *   AccountLogin, ApplicationBasic, ApplicationLauncher, AudioOutput,
 *   Binding, Channel, ContentAppObserver, ContentControl,
 *   ContentLauncher, Descriptor, KeypadInput, LevelControl,
 *   LowPower, MediaInput, MediaPlayback, OnOff,
 *   TargetNavigator, WakeOnLAN
 *
 * To add a cluster:
 *   1. Copy its case block from zap-generated/CHIPEventTLVValueDecoder.cpp
 *   2. Add it to the switch statement below (keep alphabetical order)
 *   3. Update this cluster list
 *   4. Ensure the cluster's .ipp files are included in casting-cluster-objects.cpp
 */

#include <controller/java/CHIPAttributeTLVValueDecoder.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/TypeTraits.h>

namespace chip {

jobject DecodeEventValue(const app::ConcreteEventPath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aPath.mClusterId)
    {
    // --- AccountLogin (0x050E) ---
    case app::Clusters::AccountLogin::Id: {
        using namespace app::Clusters::AccountLogin;
        switch (aPath.mEventId)
        {
        case Events::LoggedOut::Id: {
            Events::LoggedOut::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_node;
            if (!cppValue.node.HasValue())
            {
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateOptional(nullptr, value_node);
            }
            else
            {
                jobject value_nodeInsideOptional;
                std::string value_nodeInsideOptionalClassName     = "java/lang/Long";
                std::string value_nodeInsideOptionalCtorSignature = "(J)V";
                jlong jnivalue_nodeInsideOptional                 = static_cast<jlong>(cppValue.node.Value());
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_nodeInsideOptionalClassName.c_str(), value_nodeInsideOptionalCtorSignature.c_str(),
                    jnivalue_nodeInsideOptional, value_nodeInsideOptional);
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateOptional(value_nodeInsideOptional, value_node);
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_fabricIndexClassName.c_str(), value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                value_fabricIndex);

            jclass loggedOutStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccountLoginClusterLoggedOutEvent", loggedOutStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccountLoginClusterLoggedOutEvent");
                return nullptr;
            }

            jmethodID loggedOutStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, loggedOutStructClass, "<init>",
                                                                "(Ljava/util/Optional;Ljava/lang/Integer;)V", &loggedOutStructCtor);
            if (err != CHIP_NO_ERROR || loggedOutStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$AccountLoginClusterLoggedOutEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(loggedOutStructClass, loggedOutStructCtor, value_node, value_fabricIndex);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- ApplicationBasic (0x050D) ---
    case app::Clusters::ApplicationBasic::Id: {
        using namespace app::Clusters::ApplicationBasic;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- ApplicationLauncher (0x050C) ---
    case app::Clusters::ApplicationLauncher::Id: {
        using namespace app::Clusters::ApplicationLauncher;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- AudioOutput (0x050B) ---
    case app::Clusters::AudioOutput::Id: {
        using namespace app::Clusters::AudioOutput;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- Binding (0x001E) ---
    case app::Clusters::Binding::Id: {
        using namespace app::Clusters::Binding;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- Channel (0x0504) ---
    case app::Clusters::Channel::Id: {
        using namespace app::Clusters::Channel;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- ContentAppObserver (0x0510) ---
    case app::Clusters::ContentAppObserver::Id: {
        using namespace app::Clusters::ContentAppObserver;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- ContentControl (0x050F) ---
    case app::Clusters::ContentControl::Id: {
        using namespace app::Clusters::ContentControl;
        switch (aPath.mEventId)
        {
        case Events::RemainingScreenTimeExpired::Id: {
            Events::RemainingScreenTimeExpired::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass remainingScreenTimeExpiredStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ContentControlClusterRemainingScreenTimeExpiredEvent",
                remainingScreenTimeExpiredStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ContentControlClusterRemainingScreenTimeExpiredEvent");
                return nullptr;
            }

            jmethodID remainingScreenTimeExpiredStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, remainingScreenTimeExpiredStructClass, "<init>", "()V",
                                                                &remainingScreenTimeExpiredStructCtor);
            if (err != CHIP_NO_ERROR || remainingScreenTimeExpiredStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$ContentControlClusterRemainingScreenTimeExpiredEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(remainingScreenTimeExpiredStructClass, remainingScreenTimeExpiredStructCtor);

            return value;
        }
        case Events::EnteringBlockContentTimeWindow::Id: {
            Events::EnteringBlockContentTimeWindow::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass enteringBlockContentTimeWindowStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ContentControlClusterEnteringBlockContentTimeWindowEvent",
                enteringBlockContentTimeWindowStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ContentControlClusterEnteringBlockContentTimeWindowEvent");
                return nullptr;
            }

            jmethodID enteringBlockContentTimeWindowStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, enteringBlockContentTimeWindowStructClass, "<init>", "()V",
                                                                &enteringBlockContentTimeWindowStructCtor);
            if (err != CHIP_NO_ERROR || enteringBlockContentTimeWindowStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$ContentControlClusterEnteringBlockContentTimeWindowEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(enteringBlockContentTimeWindowStructClass, enteringBlockContentTimeWindowStructCtor);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- ContentLauncher (0x050A) ---
    case app::Clusters::ContentLauncher::Id: {
        using namespace app::Clusters::ContentLauncher;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- Descriptor (0x001D) ---
    case app::Clusters::Descriptor::Id: {
        using namespace app::Clusters::Descriptor;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- KeypadInput (0x0509) ---
    case app::Clusters::KeypadInput::Id: {
        using namespace app::Clusters::KeypadInput;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- LevelControl (0x0008) ---
    case app::Clusters::LevelControl::Id: {
        using namespace app::Clusters::LevelControl;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- LowPower (0x0508) ---
    case app::Clusters::LowPower::Id: {
        using namespace app::Clusters::LowPower;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- MediaInput (0x0507) ---
    case app::Clusters::MediaInput::Id: {
        using namespace app::Clusters::MediaInput;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- MediaPlayback (0x0506) ---
    case app::Clusters::MediaPlayback::Id: {
        using namespace app::Clusters::MediaPlayback;
        switch (aPath.mEventId)
        {
        case Events::StateChanged::Id: {
            Events::StateChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_currentState;
            std::string value_currentStateClassName     = "java/lang/Integer";
            std::string value_currentStateCtorSignature = "(I)V";
            jint jnivalue_currentState                  = static_cast<jint>(cppValue.currentState);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_currentStateClassName.c_str(), value_currentStateCtorSignature.c_str(), jnivalue_currentState,
                value_currentState);

            jobject value_startTime;
            std::string value_startTimeClassName     = "java/lang/Long";
            std::string value_startTimeCtorSignature = "(J)V";
            jlong jnivalue_startTime                 = static_cast<jlong>(cppValue.startTime);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_startTimeClassName.c_str(), value_startTimeCtorSignature.c_str(), jnivalue_startTime, value_startTime);

            jobject value_duration;
            std::string value_durationClassName     = "java/lang/Long";
            std::string value_durationCtorSignature = "(J)V";
            jlong jnivalue_duration                 = static_cast<jlong>(cppValue.duration);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_durationClassName.c_str(), value_durationCtorSignature.c_str(), jnivalue_duration, value_duration);

            jobject value_sampledPosition;
            jobject value_sampledPosition_updatedAt;
            std::string value_sampledPosition_updatedAtClassName     = "java/lang/Long";
            std::string value_sampledPosition_updatedAtCtorSignature = "(J)V";
            jlong jnivalue_sampledPosition_updatedAt                 = static_cast<jlong>(cppValue.sampledPosition.updatedAt);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_sampledPosition_updatedAtClassName.c_str(), value_sampledPosition_updatedAtCtorSignature.c_str(),
                jnivalue_sampledPosition_updatedAt, value_sampledPosition_updatedAt);
            jobject value_sampledPosition_position;
            if (cppValue.sampledPosition.position.IsNull())
            {
                value_sampledPosition_position = nullptr;
            }
            else
            {
                std::string value_sampledPosition_positionClassName     = "java/lang/Long";
                std::string value_sampledPosition_positionCtorSignature = "(J)V";
                jlong jnivalue_sampledPosition_position = static_cast<jlong>(cppValue.sampledPosition.position.Value());
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_sampledPosition_positionClassName.c_str(), value_sampledPosition_positionCtorSignature.c_str(),
                    jnivalue_sampledPosition_position, value_sampledPosition_position);
            }

            {
                jclass playbackPositionStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterPlaybackPositionStruct",
                    playbackPositionStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterPlaybackPositionStruct");
                    return nullptr;
                }

                jmethodID playbackPositionStructStructCtor_0;
                err = chip::JniReferences::GetInstance().FindMethod(env, playbackPositionStructStructClass_0, "<init>",
                                                                    "(Ljava/lang/Long;Ljava/lang/Long;)V",
                                                                    &playbackPositionStructStructCtor_0);
                if (err != CHIP_NO_ERROR || playbackPositionStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterPlaybackPositionStruct constructor");
                    return nullptr;
                }

                value_sampledPosition = env->NewObject(playbackPositionStructStructClass_0, playbackPositionStructStructCtor_0,
                                                       value_sampledPosition_updatedAt, value_sampledPosition_position);
            }

            jobject value_playbackSpeed;
            std::string value_playbackSpeedClassName     = "java/lang/Float";
            std::string value_playbackSpeedCtorSignature = "(F)V";
            jfloat jnivalue_playbackSpeed                = static_cast<jfloat>(cppValue.playbackSpeed);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jfloat>(
                value_playbackSpeedClassName.c_str(), value_playbackSpeedCtorSignature.c_str(), jnivalue_playbackSpeed,
                value_playbackSpeed);

            jobject value_seekRangeEnd;
            std::string value_seekRangeEndClassName     = "java/lang/Long";
            std::string value_seekRangeEndCtorSignature = "(J)V";
            jlong jnivalue_seekRangeEnd                 = static_cast<jlong>(cppValue.seekRangeEnd);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_seekRangeEndClassName.c_str(), value_seekRangeEndCtorSignature.c_str(), jnivalue_seekRangeEnd,
                value_seekRangeEnd);

            jobject value_seekRangeStart;
            std::string value_seekRangeStartClassName     = "java/lang/Long";
            std::string value_seekRangeStartCtorSignature = "(J)V";
            jlong jnivalue_seekRangeStart                 = static_cast<jlong>(cppValue.seekRangeStart);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_seekRangeStartClassName.c_str(), value_seekRangeStartCtorSignature.c_str(), jnivalue_seekRangeStart,
                value_seekRangeStart);

            jobject value_data;
            if (!cppValue.data.HasValue())
            {
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateOptional(nullptr, value_data);
            }
            else
            {
                jobject value_dataInsideOptional;
                jbyteArray value_dataInsideOptionalByteArray = env->NewByteArray(static_cast<jsize>(cppValue.data.Value().size()));
                env->SetByteArrayRegion(value_dataInsideOptionalByteArray, 0, static_cast<jsize>(cppValue.data.Value().size()),
                                        reinterpret_cast<const jbyte *>(cppValue.data.Value().data()));
                value_dataInsideOptional = value_dataInsideOptionalByteArray;
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateOptional(value_dataInsideOptional, value_data);
            }

            jobject value_audioAdvanceUnmuted;
            std::string value_audioAdvanceUnmutedClassName     = "java/lang/Boolean";
            std::string value_audioAdvanceUnmutedCtorSignature = "(Z)V";
            jboolean jnivalue_audioAdvanceUnmuted              = static_cast<jboolean>(cppValue.audioAdvanceUnmuted);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                value_audioAdvanceUnmutedClassName.c_str(), value_audioAdvanceUnmutedCtorSignature.c_str(),
                jnivalue_audioAdvanceUnmuted, value_audioAdvanceUnmuted);

            jclass stateChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$MediaPlaybackClusterStateChangedEvent", stateChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$MediaPlaybackClusterStateChangedEvent");
                return nullptr;
            }

            jmethodID stateChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, stateChangedStructClass, "<init>",
                "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;Lchip/devicecontroller/"
                "ChipStructs$MediaPlaybackClusterPlaybackPositionStruct;Ljava/lang/Float;Ljava/lang/Long;Ljava/lang/Long;Ljava/"
                "util/Optional;Ljava/lang/Boolean;)V",
                &stateChangedStructCtor);
            if (err != CHIP_NO_ERROR || stateChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$MediaPlaybackClusterStateChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(stateChangedStructClass, stateChangedStructCtor, value_currentState, value_startTime,
                                           value_duration, value_sampledPosition, value_playbackSpeed, value_seekRangeEnd,
                                           value_seekRangeStart, value_data, value_audioAdvanceUnmuted);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- OnOff (0x0006) ---
    case app::Clusters::OnOff::Id: {
        using namespace app::Clusters::OnOff;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- TargetNavigator (0x0505) ---
    case app::Clusters::TargetNavigator::Id: {
        using namespace app::Clusters::TargetNavigator;
        switch (aPath.mEventId)
        {
        case Events::TargetUpdated::Id: {
            Events::TargetUpdated::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_targetList;
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateArrayList(value_targetList);

            auto iter_value_targetList_0 = cppValue.targetList.begin();
            while (iter_value_targetList_0.Next())
            {
                auto & entry_0 = iter_value_targetList_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_identifier;
                std::string newElement_0_identifierClassName     = "java/lang/Integer";
                std::string newElement_0_identifierCtorSignature = "(I)V";
                jint jninewElement_0_identifier                  = static_cast<jint>(entry_0.identifier);
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0_identifierClassName.c_str(), newElement_0_identifierCtorSignature.c_str(),
                    jninewElement_0_identifier, newElement_0_identifier);
                jobject newElement_0_name;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.name, newElement_0_name));

                {
                    jclass targetInfoStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$TargetNavigatorClusterTargetInfoStruct",
                        targetInfoStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$TargetNavigatorClusterTargetInfoStruct");
                        return nullptr;
                    }

                    jmethodID targetInfoStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(env, targetInfoStructStructClass_1, "<init>",
                                                                        "(Ljava/lang/Integer;Ljava/lang/String;)V",
                                                                        &targetInfoStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || targetInfoStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$TargetNavigatorClusterTargetInfoStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(targetInfoStructStructClass_1, targetInfoStructStructCtor_1,
                                                  newElement_0_identifier, newElement_0_name);
                }
                TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().AddToList(value_targetList, newElement_0);
            }

            jobject value_currentTarget;
            std::string value_currentTargetClassName     = "java/lang/Integer";
            std::string value_currentTargetCtorSignature = "(I)V";
            jint jnivalue_currentTarget                  = static_cast<jint>(cppValue.currentTarget);
            TEMPORARY_RETURN_IGNORED chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_currentTargetClassName.c_str(), value_currentTargetCtorSignature.c_str(), jnivalue_currentTarget,
                value_currentTarget);

            jobject value_data;
            jbyteArray value_dataByteArray = env->NewByteArray(static_cast<jsize>(cppValue.data.size()));
            env->SetByteArrayRegion(value_dataByteArray, 0, static_cast<jsize>(cppValue.data.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.data.data()));
            value_data = value_dataByteArray;

            jclass targetUpdatedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TargetNavigatorClusterTargetUpdatedEvent", targetUpdatedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TargetNavigatorClusterTargetUpdatedEvent");
                return nullptr;
            }

            jmethodID targetUpdatedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, targetUpdatedStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/lang/Integer;[B)V", &targetUpdatedStructCtor);
            if (err != CHIP_NO_ERROR || targetUpdatedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TargetNavigatorClusterTargetUpdatedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(targetUpdatedStructClass, targetUpdatedStructCtor, value_targetList, value_currentTarget,
                                           value_data);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    // --- WakeOnLAN (0x0503) ---
    case app::Clusters::WakeOnLan::Id: {
        using namespace app::Clusters::WakeOnLan;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    default:
        *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
        break;
    }
    return nullptr;
}

} // namespace chip
