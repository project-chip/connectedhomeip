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
    case app::Clusters::Identify::Id: {
        using namespace app::Clusters::Identify;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Groups::Id: {
        using namespace app::Clusters::Groups;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Scenes::Id: {
        using namespace app::Clusters::Scenes;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
    case app::Clusters::OnOffSwitchConfiguration::Id: {
        using namespace app::Clusters::OnOffSwitchConfiguration;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
    case app::Clusters::BinaryInputBasic::Id: {
        using namespace app::Clusters::BinaryInputBasic;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PulseWidthModulation::Id: {
        using namespace app::Clusters::PulseWidthModulation;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
    case app::Clusters::AccessControl::Id: {
        using namespace app::Clusters::AccessControl;
        switch (aPath.mEventId)
        {
        case Events::AccessControlEntryChanged::Id: {
            Events::AccessControlEntryChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_adminNodeID;
            if (cppValue.adminNodeID.IsNull())
            {
                value_adminNodeID = nullptr;
            }
            else
            {
                std::string value_adminNodeIDClassName     = "java/lang/Long";
                std::string value_adminNodeIDCtorSignature = "(J)V";
                jlong jnivalue_adminNodeID                 = static_cast<jlong>(cppValue.adminNodeID.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_adminNodeIDClassName.c_str(),
                                                                            value_adminNodeIDCtorSignature.c_str(),
                                                                            jnivalue_adminNodeID, value_adminNodeID);
            }

            jobject value_adminPasscodeID;
            if (cppValue.adminPasscodeID.IsNull())
            {
                value_adminPasscodeID = nullptr;
            }
            else
            {
                std::string value_adminPasscodeIDClassName     = "java/lang/Integer";
                std::string value_adminPasscodeIDCtorSignature = "(I)V";
                jint jnivalue_adminPasscodeID                  = static_cast<jint>(cppValue.adminPasscodeID.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_adminPasscodeIDClassName.c_str(),
                                                                           value_adminPasscodeIDCtorSignature.c_str(),
                                                                           jnivalue_adminPasscodeID, value_adminPasscodeID);
            }

            jobject value_changeType;
            std::string value_changeTypeClassName     = "java/lang/Integer";
            std::string value_changeTypeCtorSignature = "(I)V";
            jint jnivalue_changeType                  = static_cast<jint>(cppValue.changeType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_changeTypeClassName.c_str(), value_changeTypeCtorSignature.c_str(), jnivalue_changeType, value_changeType);

            jobject value_latestValue;
            if (cppValue.latestValue.IsNull())
            {
                value_latestValue = nullptr;
            }
            else
            {
                jobject value_latestValue_privilege;
                std::string value_latestValue_privilegeClassName     = "java/lang/Integer";
                std::string value_latestValue_privilegeCtorSignature = "(I)V";
                jint jnivalue_latestValue_privilege                  = static_cast<jint>(cppValue.latestValue.Value().privilege);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_latestValue_privilegeClassName.c_str(), value_latestValue_privilegeCtorSignature.c_str(),
                    jnivalue_latestValue_privilege, value_latestValue_privilege);
                jobject value_latestValue_authMode;
                std::string value_latestValue_authModeClassName     = "java/lang/Integer";
                std::string value_latestValue_authModeCtorSignature = "(I)V";
                jint jnivalue_latestValue_authMode                  = static_cast<jint>(cppValue.latestValue.Value().authMode);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_latestValue_authModeClassName.c_str(), value_latestValue_authModeCtorSignature.c_str(),
                    jnivalue_latestValue_authMode, value_latestValue_authMode);
                jobject value_latestValue_subjects;
                if (cppValue.latestValue.Value().subjects.IsNull())
                {
                    value_latestValue_subjects = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(value_latestValue_subjects);

                    auto iter_value_latestValue_subjects_3 = cppValue.latestValue.Value().subjects.Value().begin();
                    while (iter_value_latestValue_subjects_3.Next())
                    {
                        auto & entry_3 = iter_value_latestValue_subjects_3.GetValue();
                        jobject newElement_3;
                        std::string newElement_3ClassName     = "java/lang/Long";
                        std::string newElement_3CtorSignature = "(J)V";
                        jlong jninewElement_3                 = static_cast<jlong>(entry_3);
                        chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                            newElement_3ClassName.c_str(), newElement_3CtorSignature.c_str(), jninewElement_3, newElement_3);
                        chip::JniReferences::GetInstance().AddToList(value_latestValue_subjects, newElement_3);
                    }
                }
                jobject value_latestValue_targets;
                if (cppValue.latestValue.Value().targets.IsNull())
                {
                    value_latestValue_targets = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(value_latestValue_targets);

                    auto iter_value_latestValue_targets_3 = cppValue.latestValue.Value().targets.Value().begin();
                    while (iter_value_latestValue_targets_3.Next())
                    {
                        auto & entry_3 = iter_value_latestValue_targets_3.GetValue();
                        jobject newElement_3;
                        jobject newElement_3_cluster;
                        if (entry_3.cluster.IsNull())
                        {
                            newElement_3_cluster = nullptr;
                        }
                        else
                        {
                            std::string newElement_3_clusterClassName     = "java/lang/Long";
                            std::string newElement_3_clusterCtorSignature = "(J)V";
                            jlong jninewElement_3_cluster                 = static_cast<jlong>(entry_3.cluster.Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                                newElement_3_clusterClassName.c_str(), newElement_3_clusterCtorSignature.c_str(),
                                jninewElement_3_cluster, newElement_3_cluster);
                        }
                        jobject newElement_3_endpoint;
                        if (entry_3.endpoint.IsNull())
                        {
                            newElement_3_endpoint = nullptr;
                        }
                        else
                        {
                            std::string newElement_3_endpointClassName     = "java/lang/Integer";
                            std::string newElement_3_endpointCtorSignature = "(I)V";
                            jint jninewElement_3_endpoint                  = static_cast<jint>(entry_3.endpoint.Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                                newElement_3_endpointClassName.c_str(), newElement_3_endpointCtorSignature.c_str(),
                                jninewElement_3_endpoint, newElement_3_endpoint);
                        }
                        jobject newElement_3_deviceType;
                        if (entry_3.deviceType.IsNull())
                        {
                            newElement_3_deviceType = nullptr;
                        }
                        else
                        {
                            std::string newElement_3_deviceTypeClassName     = "java/lang/Long";
                            std::string newElement_3_deviceTypeCtorSignature = "(J)V";
                            jlong jninewElement_3_deviceType                 = static_cast<jlong>(entry_3.deviceType.Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                                newElement_3_deviceTypeClassName.c_str(), newElement_3_deviceTypeCtorSignature.c_str(),
                                jninewElement_3_deviceType, newElement_3_deviceType);
                        }

                        jclass accessControlTargetStructStructClass_4;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlTargetStruct",
                            accessControlTargetStructStructClass_4);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlTargetStruct");
                            return nullptr;
                        }
                        jmethodID accessControlTargetStructStructCtor_4 =
                            env->GetMethodID(accessControlTargetStructStructClass_4, "<init>",
                                             "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;)V");
                        if (accessControlTargetStructStructCtor_4 == nullptr)
                        {
                            ChipLogError(Zcl,
                                         "Could not find ChipStructs$AccessControlClusterAccessControlTargetStruct constructor");
                            return nullptr;
                        }

                        newElement_3 = env->NewObject(accessControlTargetStructStructClass_4, accessControlTargetStructStructCtor_4,
                                                      newElement_3_cluster, newElement_3_endpoint, newElement_3_deviceType);
                        chip::JniReferences::GetInstance().AddToList(value_latestValue_targets, newElement_3);
                    }
                }
                jobject value_latestValue_fabricIndex;
                std::string value_latestValue_fabricIndexClassName     = "java/lang/Integer";
                std::string value_latestValue_fabricIndexCtorSignature = "(I)V";
                jint jnivalue_latestValue_fabricIndex = static_cast<jint>(cppValue.latestValue.Value().fabricIndex);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_latestValue_fabricIndexClassName.c_str(), value_latestValue_fabricIndexCtorSignature.c_str(),
                    jnivalue_latestValue_fabricIndex, value_latestValue_fabricIndex);

                jclass accessControlEntryStructStructClass_1;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlEntryStruct",
                    accessControlEntryStructStructClass_1);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlEntryStruct");
                    return nullptr;
                }
                jmethodID accessControlEntryStructStructCtor_1 = env->GetMethodID(
                    accessControlEntryStructStructClass_1, "<init>",
                    "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/ArrayList;Ljava/util/ArrayList;Ljava/lang/Integer;)V");
                if (accessControlEntryStructStructCtor_1 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterAccessControlEntryStruct constructor");
                    return nullptr;
                }

                value_latestValue =
                    env->NewObject(accessControlEntryStructStructClass_1, accessControlEntryStructStructCtor_1,
                                   value_latestValue_privilege, value_latestValue_authMode, value_latestValue_subjects,
                                   value_latestValue_targets, value_latestValue_fabricIndex);
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass accessControlEntryChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccessControlClusterAccessControlEntryChangedEvent",
                accessControlEntryChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccessControlClusterAccessControlEntryChangedEvent");
                return nullptr;
            }
            jmethodID accessControlEntryChangedStructCtor =
                env->GetMethodID(accessControlEntryChangedStructClass, "<init>",
                                 "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Lchip/devicecontroller/"
                                 "ChipStructs$AccessControlClusterAccessControlEntryStruct;Ljava/lang/Integer;)V");
            if (accessControlEntryChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$AccessControlClusterAccessControlEntryChangedEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(accessControlEntryChangedStructClass, accessControlEntryChangedStructCtor, value_adminNodeID,
                               value_adminPasscodeID, value_changeType, value_latestValue, value_fabricIndex);

            return value;
        }
        case Events::AccessControlExtensionChanged::Id: {
            Events::AccessControlExtensionChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_adminNodeID;
            if (cppValue.adminNodeID.IsNull())
            {
                value_adminNodeID = nullptr;
            }
            else
            {
                std::string value_adminNodeIDClassName     = "java/lang/Long";
                std::string value_adminNodeIDCtorSignature = "(J)V";
                jlong jnivalue_adminNodeID                 = static_cast<jlong>(cppValue.adminNodeID.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_adminNodeIDClassName.c_str(),
                                                                            value_adminNodeIDCtorSignature.c_str(),
                                                                            jnivalue_adminNodeID, value_adminNodeID);
            }

            jobject value_adminPasscodeID;
            if (cppValue.adminPasscodeID.IsNull())
            {
                value_adminPasscodeID = nullptr;
            }
            else
            {
                std::string value_adminPasscodeIDClassName     = "java/lang/Integer";
                std::string value_adminPasscodeIDCtorSignature = "(I)V";
                jint jnivalue_adminPasscodeID                  = static_cast<jint>(cppValue.adminPasscodeID.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_adminPasscodeIDClassName.c_str(),
                                                                           value_adminPasscodeIDCtorSignature.c_str(),
                                                                           jnivalue_adminPasscodeID, value_adminPasscodeID);
            }

            jobject value_changeType;
            std::string value_changeTypeClassName     = "java/lang/Integer";
            std::string value_changeTypeCtorSignature = "(I)V";
            jint jnivalue_changeType                  = static_cast<jint>(cppValue.changeType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_changeTypeClassName.c_str(), value_changeTypeCtorSignature.c_str(), jnivalue_changeType, value_changeType);

            jobject value_latestValue;
            if (cppValue.latestValue.IsNull())
            {
                value_latestValue = nullptr;
            }
            else
            {
                jobject value_latestValue_data;
                jbyteArray value_latestValue_dataByteArray =
                    env->NewByteArray(static_cast<jsize>(cppValue.latestValue.Value().data.size()));
                env->SetByteArrayRegion(value_latestValue_dataByteArray, 0,
                                        static_cast<jsize>(cppValue.latestValue.Value().data.size()),
                                        reinterpret_cast<const jbyte *>(cppValue.latestValue.Value().data.data()));
                value_latestValue_data = value_latestValue_dataByteArray;
                jobject value_latestValue_fabricIndex;
                std::string value_latestValue_fabricIndexClassName     = "java/lang/Integer";
                std::string value_latestValue_fabricIndexCtorSignature = "(I)V";
                jint jnivalue_latestValue_fabricIndex = static_cast<jint>(cppValue.latestValue.Value().fabricIndex);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_latestValue_fabricIndexClassName.c_str(), value_latestValue_fabricIndexCtorSignature.c_str(),
                    jnivalue_latestValue_fabricIndex, value_latestValue_fabricIndex);

                jclass accessControlExtensionStructStructClass_1;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlExtensionStruct",
                    accessControlExtensionStructStructClass_1);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlExtensionStruct");
                    return nullptr;
                }
                jmethodID accessControlExtensionStructStructCtor_1 =
                    env->GetMethodID(accessControlExtensionStructStructClass_1, "<init>", "([BLjava/lang/Integer;)V");
                if (accessControlExtensionStructStructCtor_1 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterAccessControlExtensionStruct constructor");
                    return nullptr;
                }

                value_latestValue =
                    env->NewObject(accessControlExtensionStructStructClass_1, accessControlExtensionStructStructCtor_1,
                                   value_latestValue_data, value_latestValue_fabricIndex);
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass accessControlExtensionChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccessControlClusterAccessControlExtensionChangedEvent",
                accessControlExtensionChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccessControlClusterAccessControlExtensionChangedEvent");
                return nullptr;
            }
            jmethodID accessControlExtensionChangedStructCtor =
                env->GetMethodID(accessControlExtensionChangedStructClass, "<init>",
                                 "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Lchip/devicecontroller/"
                                 "ChipStructs$AccessControlClusterAccessControlExtensionStruct;Ljava/lang/Integer;)V");
            if (accessControlExtensionChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$AccessControlClusterAccessControlExtensionChangedEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(accessControlExtensionChangedStructClass, accessControlExtensionChangedStructCtor, value_adminNodeID,
                               value_adminPasscodeID, value_changeType, value_latestValue, value_fabricIndex);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Actions::Id: {
        using namespace app::Clusters::Actions;
        switch (aPath.mEventId)
        {
        case Events::StateChanged::Id: {
            Events::StateChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_actionID;
            std::string value_actionIDClassName     = "java/lang/Integer";
            std::string value_actionIDCtorSignature = "(I)V";
            jint jnivalue_actionID                  = static_cast<jint>(cppValue.actionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_actionIDClassName.c_str(), value_actionIDCtorSignature.c_str(), jnivalue_actionID, value_actionID);

            jobject value_invokeID;
            std::string value_invokeIDClassName     = "java/lang/Long";
            std::string value_invokeIDCtorSignature = "(J)V";
            jlong jnivalue_invokeID                 = static_cast<jlong>(cppValue.invokeID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_invokeIDClassName.c_str(), value_invokeIDCtorSignature.c_str(), jnivalue_invokeID, value_invokeID);

            jobject value_newState;
            std::string value_newStateClassName     = "java/lang/Integer";
            std::string value_newStateCtorSignature = "(I)V";
            jint jnivalue_newState                  = static_cast<jint>(cppValue.newState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_newStateClassName.c_str(), value_newStateCtorSignature.c_str(), jnivalue_newState, value_newState);

            jclass stateChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ActionsClusterStateChangedEvent", stateChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ActionsClusterStateChangedEvent");
                return nullptr;
            }
            jmethodID stateChangedStructCtor =
                env->GetMethodID(stateChangedStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Integer;)V");
            if (stateChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$ActionsClusterStateChangedEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(stateChangedStructClass, stateChangedStructCtor, value_actionID, value_invokeID, value_newState);

            return value;
        }
        case Events::ActionFailed::Id: {
            Events::ActionFailed::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_actionID;
            std::string value_actionIDClassName     = "java/lang/Integer";
            std::string value_actionIDCtorSignature = "(I)V";
            jint jnivalue_actionID                  = static_cast<jint>(cppValue.actionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_actionIDClassName.c_str(), value_actionIDCtorSignature.c_str(), jnivalue_actionID, value_actionID);

            jobject value_invokeID;
            std::string value_invokeIDClassName     = "java/lang/Long";
            std::string value_invokeIDCtorSignature = "(J)V";
            jlong jnivalue_invokeID                 = static_cast<jlong>(cppValue.invokeID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_invokeIDClassName.c_str(), value_invokeIDCtorSignature.c_str(), jnivalue_invokeID, value_invokeID);

            jobject value_newState;
            std::string value_newStateClassName     = "java/lang/Integer";
            std::string value_newStateCtorSignature = "(I)V";
            jint jnivalue_newState                  = static_cast<jint>(cppValue.newState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_newStateClassName.c_str(), value_newStateCtorSignature.c_str(), jnivalue_newState, value_newState);

            jobject value_error;
            std::string value_errorClassName     = "java/lang/Integer";
            std::string value_errorCtorSignature = "(I)V";
            jint jnivalue_error                  = static_cast<jint>(cppValue.error);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_errorClassName.c_str(), value_errorCtorSignature.c_str(), jnivalue_error, value_error);

            jclass actionFailedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ActionsClusterActionFailedEvent", actionFailedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ActionsClusterActionFailedEvent");
                return nullptr;
            }
            jmethodID actionFailedStructCtor = env->GetMethodID(
                actionFailedStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;)V");
            if (actionFailedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$ActionsClusterActionFailedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(actionFailedStructClass, actionFailedStructCtor, value_actionID, value_invokeID,
                                           value_newState, value_error);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::BasicInformation::Id: {
        using namespace app::Clusters::BasicInformation;
        switch (aPath.mEventId)
        {
        case Events::StartUp::Id: {
            Events::StartUp::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_softwareVersion;
            std::string value_softwareVersionClassName     = "java/lang/Long";
            std::string value_softwareVersionCtorSignature = "(J)V";
            jlong jnivalue_softwareVersion                 = static_cast<jlong>(cppValue.softwareVersion);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_softwareVersionClassName.c_str(),
                                                                        value_softwareVersionCtorSignature.c_str(),
                                                                        jnivalue_softwareVersion, value_softwareVersion);

            jclass startUpStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterStartUpEvent", startUpStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterStartUpEvent");
                return nullptr;
            }
            jmethodID startUpStructCtor = env->GetMethodID(startUpStructClass, "<init>", "(Ljava/lang/Long;)V");
            if (startUpStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicInformationClusterStartUpEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(startUpStructClass, startUpStructCtor, value_softwareVersion);

            return value;
        }
        case Events::ShutDown::Id: {
            Events::ShutDown::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass shutDownStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterShutDownEvent", shutDownStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterShutDownEvent");
                return nullptr;
            }
            jmethodID shutDownStructCtor = env->GetMethodID(shutDownStructClass, "<init>", "()V");
            if (shutDownStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicInformationClusterShutDownEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(shutDownStructClass, shutDownStructCtor);

            return value;
        }
        case Events::Leave::Id: {
            Events::Leave::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass leaveStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterLeaveEvent", leaveStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterLeaveEvent");
                return nullptr;
            }
            jmethodID leaveStructCtor = env->GetMethodID(leaveStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (leaveStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicInformationClusterLeaveEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(leaveStructClass, leaveStructCtor, value_fabricIndex);

            return value;
        }
        case Events::ReachableChanged::Id: {
            Events::ReachableChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_reachableNewValue;
            std::string value_reachableNewValueClassName     = "java/lang/Boolean";
            std::string value_reachableNewValueCtorSignature = "(Z)V";
            jboolean jnivalue_reachableNewValue              = static_cast<jboolean>(cppValue.reachableNewValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(value_reachableNewValueClassName.c_str(),
                                                                           value_reachableNewValueCtorSignature.c_str(),
                                                                           jnivalue_reachableNewValue, value_reachableNewValue);

            jclass reachableChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterReachableChangedEvent",
                reachableChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterReachableChangedEvent");
                return nullptr;
            }
            jmethodID reachableChangedStructCtor =
                env->GetMethodID(reachableChangedStructClass, "<init>", "(Ljava/lang/Boolean;)V");
            if (reachableChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicInformationClusterReachableChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(reachableChangedStructClass, reachableChangedStructCtor, value_reachableNewValue);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OtaSoftwareUpdateProvider::Id: {
        using namespace app::Clusters::OtaSoftwareUpdateProvider;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OtaSoftwareUpdateRequestor::Id: {
        using namespace app::Clusters::OtaSoftwareUpdateRequestor;
        switch (aPath.mEventId)
        {
        case Events::StateTransition::Id: {
            Events::StateTransition::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_previousState;
            std::string value_previousStateClassName     = "java/lang/Integer";
            std::string value_previousStateCtorSignature = "(I)V";
            jint jnivalue_previousState                  = static_cast<jint>(cppValue.previousState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_previousStateClassName.c_str(),
                                                                       value_previousStateCtorSignature.c_str(),
                                                                       jnivalue_previousState, value_previousState);

            jobject value_newState;
            std::string value_newStateClassName     = "java/lang/Integer";
            std::string value_newStateCtorSignature = "(I)V";
            jint jnivalue_newState                  = static_cast<jint>(cppValue.newState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_newStateClassName.c_str(), value_newStateCtorSignature.c_str(), jnivalue_newState, value_newState);

            jobject value_reason;
            std::string value_reasonClassName     = "java/lang/Integer";
            std::string value_reasonCtorSignature = "(I)V";
            jint jnivalue_reason                  = static_cast<jint>(cppValue.reason);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_reasonClassName.c_str(), value_reasonCtorSignature.c_str(), jnivalue_reason, value_reason);

            jobject value_targetSoftwareVersion;
            if (cppValue.targetSoftwareVersion.IsNull())
            {
                value_targetSoftwareVersion = nullptr;
            }
            else
            {
                std::string value_targetSoftwareVersionClassName     = "java/lang/Long";
                std::string value_targetSoftwareVersionCtorSignature = "(J)V";
                jlong jnivalue_targetSoftwareVersion                 = static_cast<jlong>(cppValue.targetSoftwareVersion.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_targetSoftwareVersionClassName.c_str(), value_targetSoftwareVersionCtorSignature.c_str(),
                    jnivalue_targetSoftwareVersion, value_targetSoftwareVersion);
            }

            jclass stateTransitionStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OtaSoftwareUpdateRequestorClusterStateTransitionEvent",
                stateTransitionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OtaSoftwareUpdateRequestorClusterStateTransitionEvent");
                return nullptr;
            }
            jmethodID stateTransitionStructCtor =
                env->GetMethodID(stateTransitionStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Long;)V");
            if (stateTransitionStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$OtaSoftwareUpdateRequestorClusterStateTransitionEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(stateTransitionStructClass, stateTransitionStructCtor, value_previousState,
                                           value_newState, value_reason, value_targetSoftwareVersion);

            return value;
        }
        case Events::VersionApplied::Id: {
            Events::VersionApplied::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_softwareVersion;
            std::string value_softwareVersionClassName     = "java/lang/Long";
            std::string value_softwareVersionCtorSignature = "(J)V";
            jlong jnivalue_softwareVersion                 = static_cast<jlong>(cppValue.softwareVersion);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_softwareVersionClassName.c_str(),
                                                                        value_softwareVersionCtorSignature.c_str(),
                                                                        jnivalue_softwareVersion, value_softwareVersion);

            jobject value_productID;
            std::string value_productIDClassName     = "java/lang/Integer";
            std::string value_productIDCtorSignature = "(I)V";
            jint jnivalue_productID                  = static_cast<jint>(cppValue.productID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_productIDClassName.c_str(), value_productIDCtorSignature.c_str(), jnivalue_productID, value_productID);

            jclass versionAppliedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OtaSoftwareUpdateRequestorClusterVersionAppliedEvent",
                versionAppliedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OtaSoftwareUpdateRequestorClusterVersionAppliedEvent");
                return nullptr;
            }
            jmethodID versionAppliedStructCtor =
                env->GetMethodID(versionAppliedStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Integer;)V");
            if (versionAppliedStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$OtaSoftwareUpdateRequestorClusterVersionAppliedEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(versionAppliedStructClass, versionAppliedStructCtor, value_softwareVersion, value_productID);

            return value;
        }
        case Events::DownloadError::Id: {
            Events::DownloadError::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_softwareVersion;
            std::string value_softwareVersionClassName     = "java/lang/Long";
            std::string value_softwareVersionCtorSignature = "(J)V";
            jlong jnivalue_softwareVersion                 = static_cast<jlong>(cppValue.softwareVersion);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_softwareVersionClassName.c_str(),
                                                                        value_softwareVersionCtorSignature.c_str(),
                                                                        jnivalue_softwareVersion, value_softwareVersion);

            jobject value_bytesDownloaded;
            std::string value_bytesDownloadedClassName     = "java/lang/Long";
            std::string value_bytesDownloadedCtorSignature = "(J)V";
            jlong jnivalue_bytesDownloaded                 = static_cast<jlong>(cppValue.bytesDownloaded);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_bytesDownloadedClassName.c_str(),
                                                                        value_bytesDownloadedCtorSignature.c_str(),
                                                                        jnivalue_bytesDownloaded, value_bytesDownloaded);

            jobject value_progressPercent;
            if (cppValue.progressPercent.IsNull())
            {
                value_progressPercent = nullptr;
            }
            else
            {
                std::string value_progressPercentClassName     = "java/lang/Integer";
                std::string value_progressPercentCtorSignature = "(I)V";
                jint jnivalue_progressPercent                  = static_cast<jint>(cppValue.progressPercent.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_progressPercentClassName.c_str(),
                                                                           value_progressPercentCtorSignature.c_str(),
                                                                           jnivalue_progressPercent, value_progressPercent);
            }

            jobject value_platformCode;
            if (cppValue.platformCode.IsNull())
            {
                value_platformCode = nullptr;
            }
            else
            {
                std::string value_platformCodeClassName     = "java/lang/Long";
                std::string value_platformCodeCtorSignature = "(J)V";
                jlong jnivalue_platformCode                 = static_cast<jlong>(cppValue.platformCode.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_platformCodeClassName.c_str(),
                                                                            value_platformCodeCtorSignature.c_str(),
                                                                            jnivalue_platformCode, value_platformCode);
            }

            jclass downloadErrorStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OtaSoftwareUpdateRequestorClusterDownloadErrorEvent",
                downloadErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OtaSoftwareUpdateRequestorClusterDownloadErrorEvent");
                return nullptr;
            }
            jmethodID downloadErrorStructCtor = env->GetMethodID(
                downloadErrorStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;)V");
            if (downloadErrorStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$OtaSoftwareUpdateRequestorClusterDownloadErrorEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(downloadErrorStructClass, downloadErrorStructCtor, value_softwareVersion,
                                           value_bytesDownloaded, value_progressPercent, value_platformCode);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::LocalizationConfiguration::Id: {
        using namespace app::Clusters::LocalizationConfiguration;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TimeFormatLocalization::Id: {
        using namespace app::Clusters::TimeFormatLocalization;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::UnitLocalization::Id: {
        using namespace app::Clusters::UnitLocalization;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PowerSourceConfiguration::Id: {
        using namespace app::Clusters::PowerSourceConfiguration;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PowerSource::Id: {
        using namespace app::Clusters::PowerSource;
        switch (aPath.mEventId)
        {
        case Events::WiredFaultChange::Id: {
            Events::WiredFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass wiredFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PowerSourceClusterWiredFaultChangeEvent", wiredFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PowerSourceClusterWiredFaultChangeEvent");
                return nullptr;
            }
            jmethodID wiredFaultChangeStructCtor =
                env->GetMethodID(wiredFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (wiredFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PowerSourceClusterWiredFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(wiredFaultChangeStructClass, wiredFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        case Events::BatFaultChange::Id: {
            Events::BatFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass batFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PowerSourceClusterBatFaultChangeEvent", batFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PowerSourceClusterBatFaultChangeEvent");
                return nullptr;
            }
            jmethodID batFaultChangeStructCtor =
                env->GetMethodID(batFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (batFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PowerSourceClusterBatFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(batFaultChangeStructClass, batFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        case Events::BatChargeFaultChange::Id: {
            Events::BatChargeFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass batChargeFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PowerSourceClusterBatChargeFaultChangeEvent",
                batChargeFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PowerSourceClusterBatChargeFaultChangeEvent");
                return nullptr;
            }
            jmethodID batChargeFaultChangeStructCtor =
                env->GetMethodID(batChargeFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (batChargeFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PowerSourceClusterBatChargeFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(batChargeFaultChangeStructClass, batChargeFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::GeneralCommissioning::Id: {
        using namespace app::Clusters::GeneralCommissioning;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::NetworkCommissioning::Id: {
        using namespace app::Clusters::NetworkCommissioning;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DiagnosticLogs::Id: {
        using namespace app::Clusters::DiagnosticLogs;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::GeneralDiagnostics::Id: {
        using namespace app::Clusters::GeneralDiagnostics;
        switch (aPath.mEventId)
        {
        case Events::HardwareFaultChange::Id: {
            Events::HardwareFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass hardwareFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterHardwareFaultChangeEvent",
                hardwareFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterHardwareFaultChangeEvent");
                return nullptr;
            }
            jmethodID hardwareFaultChangeStructCtor =
                env->GetMethodID(hardwareFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (hardwareFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$GeneralDiagnosticsClusterHardwareFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(hardwareFaultChangeStructClass, hardwareFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        case Events::RadioFaultChange::Id: {
            Events::RadioFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass radioFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterRadioFaultChangeEvent",
                radioFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterRadioFaultChangeEvent");
                return nullptr;
            }
            jmethodID radioFaultChangeStructCtor =
                env->GetMethodID(radioFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (radioFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$GeneralDiagnosticsClusterRadioFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(radioFaultChangeStructClass, radioFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        case Events::NetworkFaultChange::Id: {
            Events::NetworkFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass networkFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterNetworkFaultChangeEvent",
                networkFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterNetworkFaultChangeEvent");
                return nullptr;
            }
            jmethodID networkFaultChangeStructCtor =
                env->GetMethodID(networkFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (networkFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$GeneralDiagnosticsClusterNetworkFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(networkFaultChangeStructClass, networkFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        case Events::BootReason::Id: {
            Events::BootReason::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_bootReason;
            std::string value_bootReasonClassName     = "java/lang/Integer";
            std::string value_bootReasonCtorSignature = "(I)V";
            jint jnivalue_bootReason                  = static_cast<jint>(cppValue.bootReason);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_bootReasonClassName.c_str(), value_bootReasonCtorSignature.c_str(), jnivalue_bootReason, value_bootReason);

            jclass bootReasonStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterBootReasonEvent", bootReasonStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterBootReasonEvent");
                return nullptr;
            }
            jmethodID bootReasonStructCtor = env->GetMethodID(bootReasonStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (bootReasonStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$GeneralDiagnosticsClusterBootReasonEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(bootReasonStructClass, bootReasonStructCtor, value_bootReason);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::SoftwareDiagnostics::Id: {
        using namespace app::Clusters::SoftwareDiagnostics;
        switch (aPath.mEventId)
        {
        case Events::SoftwareFault::Id: {
            Events::SoftwareFault::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_id;
            std::string value_idClassName     = "java/lang/Long";
            std::string value_idCtorSignature = "(J)V";
            jlong jnivalue_id                 = static_cast<jlong>(cppValue.id);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_idClassName.c_str(), value_idCtorSignature.c_str(),
                                                                        jnivalue_id, value_id);

            jobject value_name;
            if (!cppValue.name.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_name);
            }
            else
            {
                jobject value_nameInsideOptional;
                LogErrorOnFailure(
                    chip::JniReferences::GetInstance().CharToStringUTF(cppValue.name.Value(), value_nameInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_nameInsideOptional, value_name);
            }

            jobject value_faultRecording;
            if (!cppValue.faultRecording.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_faultRecording);
            }
            else
            {
                jobject value_faultRecordingInsideOptional;
                jbyteArray value_faultRecordingInsideOptionalByteArray =
                    env->NewByteArray(static_cast<jsize>(cppValue.faultRecording.Value().size()));
                env->SetByteArrayRegion(value_faultRecordingInsideOptionalByteArray, 0,
                                        static_cast<jsize>(cppValue.faultRecording.Value().size()),
                                        reinterpret_cast<const jbyte *>(cppValue.faultRecording.Value().data()));
                value_faultRecordingInsideOptional = value_faultRecordingInsideOptionalByteArray;
                chip::JniReferences::GetInstance().CreateOptional(value_faultRecordingInsideOptional, value_faultRecording);
            }

            jclass softwareFaultStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SoftwareDiagnosticsClusterSoftwareFaultEvent",
                softwareFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SoftwareDiagnosticsClusterSoftwareFaultEvent");
                return nullptr;
            }
            jmethodID softwareFaultStructCtor =
                env->GetMethodID(softwareFaultStructClass, "<init>", "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;)V");
            if (softwareFaultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SoftwareDiagnosticsClusterSoftwareFaultEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(softwareFaultStructClass, softwareFaultStructCtor, value_id, value_name, value_faultRecording);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ThreadNetworkDiagnostics::Id: {
        using namespace app::Clusters::ThreadNetworkDiagnostics;
        switch (aPath.mEventId)
        {
        case Events::ConnectionStatus::Id: {
            Events::ConnectionStatus::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_connectionStatus;
            std::string value_connectionStatusClassName     = "java/lang/Integer";
            std::string value_connectionStatusCtorSignature = "(I)V";
            jint jnivalue_connectionStatus                  = static_cast<jint>(cppValue.connectionStatus);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_connectionStatusClassName.c_str(),
                                                                       value_connectionStatusCtorSignature.c_str(),
                                                                       jnivalue_connectionStatus, value_connectionStatus);

            jclass connectionStatusStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ThreadNetworkDiagnosticsClusterConnectionStatusEvent",
                connectionStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ThreadNetworkDiagnosticsClusterConnectionStatusEvent");
                return nullptr;
            }
            jmethodID connectionStatusStructCtor =
                env->GetMethodID(connectionStatusStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (connectionStatusStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$ThreadNetworkDiagnosticsClusterConnectionStatusEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(connectionStatusStructClass, connectionStatusStructCtor, value_connectionStatus);

            return value;
        }
        case Events::NetworkFaultChange::Id: {
            Events::NetworkFaultChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_current;
            chip::JniReferences::GetInstance().CreateArrayList(value_current);

            auto iter_value_current_0 = cppValue.current.begin();
            while (iter_value_current_0.Next())
            {
                auto & entry_0 = iter_value_current_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_current, newElement_0);
            }

            jobject value_previous;
            chip::JniReferences::GetInstance().CreateArrayList(value_previous);

            auto iter_value_previous_0 = cppValue.previous.begin();
            while (iter_value_previous_0.Next())
            {
                auto & entry_0 = iter_value_previous_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_previous, newElement_0);
            }

            jclass networkFaultChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent",
                networkFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent");
                return nullptr;
            }
            jmethodID networkFaultChangeStructCtor =
                env->GetMethodID(networkFaultChangeStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (networkFaultChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(networkFaultChangeStructClass, networkFaultChangeStructCtor, value_current, value_previous);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::WiFiNetworkDiagnostics::Id: {
        using namespace app::Clusters::WiFiNetworkDiagnostics;
        switch (aPath.mEventId)
        {
        case Events::Disconnection::Id: {
            Events::Disconnection::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_reasonCode;
            std::string value_reasonCodeClassName     = "java/lang/Integer";
            std::string value_reasonCodeCtorSignature = "(I)V";
            jint jnivalue_reasonCode                  = static_cast<jint>(cppValue.reasonCode);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_reasonCodeClassName.c_str(), value_reasonCodeCtorSignature.c_str(), jnivalue_reasonCode, value_reasonCode);

            jclass disconnectionStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WiFiNetworkDiagnosticsClusterDisconnectionEvent",
                disconnectionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WiFiNetworkDiagnosticsClusterDisconnectionEvent");
                return nullptr;
            }
            jmethodID disconnectionStructCtor = env->GetMethodID(disconnectionStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (disconnectionStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$WiFiNetworkDiagnosticsClusterDisconnectionEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(disconnectionStructClass, disconnectionStructCtor, value_reasonCode);

            return value;
        }
        case Events::AssociationFailure::Id: {
            Events::AssociationFailure::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_associationFailure;
            std::string value_associationFailureClassName     = "java/lang/Integer";
            std::string value_associationFailureCtorSignature = "(I)V";
            jint jnivalue_associationFailure                  = static_cast<jint>(cppValue.associationFailure);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_associationFailureClassName.c_str(),
                                                                       value_associationFailureCtorSignature.c_str(),
                                                                       jnivalue_associationFailure, value_associationFailure);

            jobject value_status;
            std::string value_statusClassName     = "java/lang/Integer";
            std::string value_statusCtorSignature = "(I)V";
            jint jnivalue_status                  = static_cast<jint>(cppValue.status);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_statusClassName.c_str(), value_statusCtorSignature.c_str(), jnivalue_status, value_status);

            jclass associationFailureStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WiFiNetworkDiagnosticsClusterAssociationFailureEvent",
                associationFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WiFiNetworkDiagnosticsClusterAssociationFailureEvent");
                return nullptr;
            }
            jmethodID associationFailureStructCtor =
                env->GetMethodID(associationFailureStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
            if (associationFailureStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$WiFiNetworkDiagnosticsClusterAssociationFailureEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(associationFailureStructClass, associationFailureStructCtor, value_associationFailure, value_status);

            return value;
        }
        case Events::ConnectionStatus::Id: {
            Events::ConnectionStatus::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_connectionStatus;
            std::string value_connectionStatusClassName     = "java/lang/Integer";
            std::string value_connectionStatusCtorSignature = "(I)V";
            jint jnivalue_connectionStatus                  = static_cast<jint>(cppValue.connectionStatus);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_connectionStatusClassName.c_str(),
                                                                       value_connectionStatusCtorSignature.c_str(),
                                                                       jnivalue_connectionStatus, value_connectionStatus);

            jclass connectionStatusStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WiFiNetworkDiagnosticsClusterConnectionStatusEvent",
                connectionStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WiFiNetworkDiagnosticsClusterConnectionStatusEvent");
                return nullptr;
            }
            jmethodID connectionStatusStructCtor =
                env->GetMethodID(connectionStatusStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (connectionStatusStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$WiFiNetworkDiagnosticsClusterConnectionStatusEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(connectionStatusStructClass, connectionStatusStructCtor, value_connectionStatus);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::EthernetNetworkDiagnostics::Id: {
        using namespace app::Clusters::EthernetNetworkDiagnostics;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TimeSynchronization::Id: {
        using namespace app::Clusters::TimeSynchronization;
        switch (aPath.mEventId)
        {
        case Events::DSTTableEmpty::Id: {
            Events::DSTTableEmpty::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass DSTTableEmptyStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterDSTTableEmptyEvent",
                DSTTableEmptyStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterDSTTableEmptyEvent");
                return nullptr;
            }
            jmethodID DSTTableEmptyStructCtor = env->GetMethodID(DSTTableEmptyStructClass, "<init>", "()V");
            if (DSTTableEmptyStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TimeSynchronizationClusterDSTTableEmptyEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(DSTTableEmptyStructClass, DSTTableEmptyStructCtor);

            return value;
        }
        case Events::DSTStatus::Id: {
            Events::DSTStatus::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_DSTOffsetActive;
            std::string value_DSTOffsetActiveClassName     = "java/lang/Boolean";
            std::string value_DSTOffsetActiveCtorSignature = "(Z)V";
            jboolean jnivalue_DSTOffsetActive              = static_cast<jboolean>(cppValue.DSTOffsetActive);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(value_DSTOffsetActiveClassName.c_str(),
                                                                           value_DSTOffsetActiveCtorSignature.c_str(),
                                                                           jnivalue_DSTOffsetActive, value_DSTOffsetActive);

            jclass DSTStatusStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterDSTStatusEvent", DSTStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterDSTStatusEvent");
                return nullptr;
            }
            jmethodID DSTStatusStructCtor = env->GetMethodID(DSTStatusStructClass, "<init>", "(Ljava/lang/Boolean;)V");
            if (DSTStatusStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TimeSynchronizationClusterDSTStatusEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(DSTStatusStructClass, DSTStatusStructCtor, value_DSTOffsetActive);

            return value;
        }
        case Events::TimeZoneStatus::Id: {
            Events::TimeZoneStatus::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_offset;
            std::string value_offsetClassName     = "java/lang/Long";
            std::string value_offsetCtorSignature = "(J)V";
            jlong jnivalue_offset                 = static_cast<jlong>(cppValue.offset);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_offsetClassName.c_str(), value_offsetCtorSignature.c_str(), jnivalue_offset, value_offset);

            jobject value_name;
            if (!cppValue.name.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_name);
            }
            else
            {
                jobject value_nameInsideOptional;
                LogErrorOnFailure(
                    chip::JniReferences::GetInstance().CharToStringUTF(cppValue.name.Value(), value_nameInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_nameInsideOptional, value_name);
            }

            jclass timeZoneStatusStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterTimeZoneStatusEvent",
                timeZoneStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterTimeZoneStatusEvent");
                return nullptr;
            }
            jmethodID timeZoneStatusStructCtor =
                env->GetMethodID(timeZoneStatusStructClass, "<init>", "(Ljava/lang/Long;Ljava/util/Optional;)V");
            if (timeZoneStatusStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TimeSynchronizationClusterTimeZoneStatusEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(timeZoneStatusStructClass, timeZoneStatusStructCtor, value_offset, value_name);

            return value;
        }
        case Events::TimeFailure::Id: {
            Events::TimeFailure::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass timeFailureStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterTimeFailureEvent", timeFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterTimeFailureEvent");
                return nullptr;
            }
            jmethodID timeFailureStructCtor = env->GetMethodID(timeFailureStructClass, "<init>", "()V");
            if (timeFailureStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TimeSynchronizationClusterTimeFailureEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(timeFailureStructClass, timeFailureStructCtor);

            return value;
        }
        case Events::MissingTrustedTimeSource::Id: {
            Events::MissingTrustedTimeSource::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass missingTrustedTimeSourceStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterMissingTrustedTimeSourceEvent",
                missingTrustedTimeSourceStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterMissingTrustedTimeSourceEvent");
                return nullptr;
            }
            jmethodID missingTrustedTimeSourceStructCtor = env->GetMethodID(missingTrustedTimeSourceStructClass, "<init>", "()V");
            if (missingTrustedTimeSourceStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$TimeSynchronizationClusterMissingTrustedTimeSourceEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(missingTrustedTimeSourceStructClass, missingTrustedTimeSourceStructCtor);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::BridgedDeviceBasicInformation::Id: {
        using namespace app::Clusters::BridgedDeviceBasicInformation;
        switch (aPath.mEventId)
        {
        case Events::StartUp::Id: {
            Events::StartUp::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_softwareVersion;
            std::string value_softwareVersionClassName     = "java/lang/Long";
            std::string value_softwareVersionCtorSignature = "(J)V";
            jlong jnivalue_softwareVersion                 = static_cast<jlong>(cppValue.softwareVersion);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_softwareVersionClassName.c_str(),
                                                                        value_softwareVersionCtorSignature.c_str(),
                                                                        jnivalue_softwareVersion, value_softwareVersion);

            jclass startUpStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterStartUpEvent", startUpStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterStartUpEvent");
                return nullptr;
            }
            jmethodID startUpStructCtor = env->GetMethodID(startUpStructClass, "<init>", "(Ljava/lang/Long;)V");
            if (startUpStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicInformationClusterStartUpEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(startUpStructClass, startUpStructCtor, value_softwareVersion);

            return value;
        }
        case Events::ShutDown::Id: {
            Events::ShutDown::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass shutDownStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterShutDownEvent",
                shutDownStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterShutDownEvent");
                return nullptr;
            }
            jmethodID shutDownStructCtor = env->GetMethodID(shutDownStructClass, "<init>", "()V");
            if (shutDownStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicInformationClusterShutDownEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(shutDownStructClass, shutDownStructCtor);

            return value;
        }
        case Events::Leave::Id: {
            Events::Leave::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass leaveStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterLeaveEvent", leaveStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterLeaveEvent");
                return nullptr;
            }
            jmethodID leaveStructCtor = env->GetMethodID(leaveStructClass, "<init>", "()V");
            if (leaveStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicInformationClusterLeaveEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(leaveStructClass, leaveStructCtor);

            return value;
        }
        case Events::ReachableChanged::Id: {
            Events::ReachableChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_reachableNewValue;
            std::string value_reachableNewValueClassName     = "java/lang/Boolean";
            std::string value_reachableNewValueCtorSignature = "(Z)V";
            jboolean jnivalue_reachableNewValue              = static_cast<jboolean>(cppValue.reachableNewValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(value_reachableNewValueClassName.c_str(),
                                                                           value_reachableNewValueCtorSignature.c_str(),
                                                                           jnivalue_reachableNewValue, value_reachableNewValue);

            jclass reachableChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterReachableChangedEvent",
                reachableChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterReachableChangedEvent");
                return nullptr;
            }
            jmethodID reachableChangedStructCtor =
                env->GetMethodID(reachableChangedStructClass, "<init>", "(Ljava/lang/Boolean;)V");
            if (reachableChangedStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicInformationClusterReachableChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(reachableChangedStructClass, reachableChangedStructCtor, value_reachableNewValue);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Switch::Id: {
        using namespace app::Clusters::Switch;
        switch (aPath.mEventId)
        {
        case Events::SwitchLatched::Id: {
            Events::SwitchLatched::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_newPosition;
            std::string value_newPositionClassName     = "java/lang/Integer";
            std::string value_newPositionCtorSignature = "(I)V";
            jint jnivalue_newPosition                  = static_cast<jint>(cppValue.newPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_newPositionClassName.c_str(),
                                                                       value_newPositionCtorSignature.c_str(), jnivalue_newPosition,
                                                                       value_newPosition);

            jclass switchLatchedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterSwitchLatchedEvent", switchLatchedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterSwitchLatchedEvent");
                return nullptr;
            }
            jmethodID switchLatchedStructCtor = env->GetMethodID(switchLatchedStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (switchLatchedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterSwitchLatchedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(switchLatchedStructClass, switchLatchedStructCtor, value_newPosition);

            return value;
        }
        case Events::InitialPress::Id: {
            Events::InitialPress::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_newPosition;
            std::string value_newPositionClassName     = "java/lang/Integer";
            std::string value_newPositionCtorSignature = "(I)V";
            jint jnivalue_newPosition                  = static_cast<jint>(cppValue.newPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_newPositionClassName.c_str(),
                                                                       value_newPositionCtorSignature.c_str(), jnivalue_newPosition,
                                                                       value_newPosition);

            jclass initialPressStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterInitialPressEvent", initialPressStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterInitialPressEvent");
                return nullptr;
            }
            jmethodID initialPressStructCtor = env->GetMethodID(initialPressStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (initialPressStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterInitialPressEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(initialPressStructClass, initialPressStructCtor, value_newPosition);

            return value;
        }
        case Events::LongPress::Id: {
            Events::LongPress::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_newPosition;
            std::string value_newPositionClassName     = "java/lang/Integer";
            std::string value_newPositionCtorSignature = "(I)V";
            jint jnivalue_newPosition                  = static_cast<jint>(cppValue.newPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_newPositionClassName.c_str(),
                                                                       value_newPositionCtorSignature.c_str(), jnivalue_newPosition,
                                                                       value_newPosition);

            jclass longPressStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterLongPressEvent", longPressStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterLongPressEvent");
                return nullptr;
            }
            jmethodID longPressStructCtor = env->GetMethodID(longPressStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (longPressStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterLongPressEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(longPressStructClass, longPressStructCtor, value_newPosition);

            return value;
        }
        case Events::ShortRelease::Id: {
            Events::ShortRelease::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_previousPosition;
            std::string value_previousPositionClassName     = "java/lang/Integer";
            std::string value_previousPositionCtorSignature = "(I)V";
            jint jnivalue_previousPosition                  = static_cast<jint>(cppValue.previousPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_previousPositionClassName.c_str(),
                                                                       value_previousPositionCtorSignature.c_str(),
                                                                       jnivalue_previousPosition, value_previousPosition);

            jclass shortReleaseStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterShortReleaseEvent", shortReleaseStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterShortReleaseEvent");
                return nullptr;
            }
            jmethodID shortReleaseStructCtor = env->GetMethodID(shortReleaseStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (shortReleaseStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterShortReleaseEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(shortReleaseStructClass, shortReleaseStructCtor, value_previousPosition);

            return value;
        }
        case Events::LongRelease::Id: {
            Events::LongRelease::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_previousPosition;
            std::string value_previousPositionClassName     = "java/lang/Integer";
            std::string value_previousPositionCtorSignature = "(I)V";
            jint jnivalue_previousPosition                  = static_cast<jint>(cppValue.previousPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_previousPositionClassName.c_str(),
                                                                       value_previousPositionCtorSignature.c_str(),
                                                                       jnivalue_previousPosition, value_previousPosition);

            jclass longReleaseStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterLongReleaseEvent", longReleaseStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterLongReleaseEvent");
                return nullptr;
            }
            jmethodID longReleaseStructCtor = env->GetMethodID(longReleaseStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (longReleaseStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterLongReleaseEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(longReleaseStructClass, longReleaseStructCtor, value_previousPosition);

            return value;
        }
        case Events::MultiPressOngoing::Id: {
            Events::MultiPressOngoing::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_newPosition;
            std::string value_newPositionClassName     = "java/lang/Integer";
            std::string value_newPositionCtorSignature = "(I)V";
            jint jnivalue_newPosition                  = static_cast<jint>(cppValue.newPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_newPositionClassName.c_str(),
                                                                       value_newPositionCtorSignature.c_str(), jnivalue_newPosition,
                                                                       value_newPosition);

            jobject value_currentNumberOfPressesCounted;
            std::string value_currentNumberOfPressesCountedClassName     = "java/lang/Integer";
            std::string value_currentNumberOfPressesCountedCtorSignature = "(I)V";
            jint jnivalue_currentNumberOfPressesCounted = static_cast<jint>(cppValue.currentNumberOfPressesCounted);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_currentNumberOfPressesCountedClassName.c_str(), value_currentNumberOfPressesCountedCtorSignature.c_str(),
                jnivalue_currentNumberOfPressesCounted, value_currentNumberOfPressesCounted);

            jclass multiPressOngoingStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterMultiPressOngoingEvent", multiPressOngoingStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterMultiPressOngoingEvent");
                return nullptr;
            }
            jmethodID multiPressOngoingStructCtor =
                env->GetMethodID(multiPressOngoingStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
            if (multiPressOngoingStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterMultiPressOngoingEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(multiPressOngoingStructClass, multiPressOngoingStructCtor, value_newPosition,
                                           value_currentNumberOfPressesCounted);

            return value;
        }
        case Events::MultiPressComplete::Id: {
            Events::MultiPressComplete::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_previousPosition;
            std::string value_previousPositionClassName     = "java/lang/Integer";
            std::string value_previousPositionCtorSignature = "(I)V";
            jint jnivalue_previousPosition                  = static_cast<jint>(cppValue.previousPosition);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_previousPositionClassName.c_str(),
                                                                       value_previousPositionCtorSignature.c_str(),
                                                                       jnivalue_previousPosition, value_previousPosition);

            jobject value_totalNumberOfPressesCounted;
            std::string value_totalNumberOfPressesCountedClassName     = "java/lang/Integer";
            std::string value_totalNumberOfPressesCountedCtorSignature = "(I)V";
            jint jnivalue_totalNumberOfPressesCounted                  = static_cast<jint>(cppValue.totalNumberOfPressesCounted);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_totalNumberOfPressesCountedClassName.c_str(), value_totalNumberOfPressesCountedCtorSignature.c_str(),
                jnivalue_totalNumberOfPressesCounted, value_totalNumberOfPressesCounted);

            jclass multiPressCompleteStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterMultiPressCompleteEvent", multiPressCompleteStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterMultiPressCompleteEvent");
                return nullptr;
            }
            jmethodID multiPressCompleteStructCtor =
                env->GetMethodID(multiPressCompleteStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
            if (multiPressCompleteStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SwitchClusterMultiPressCompleteEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(multiPressCompleteStructClass, multiPressCompleteStructCtor, value_previousPosition,
                                           value_totalNumberOfPressesCounted);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::AdministratorCommissioning::Id: {
        using namespace app::Clusters::AdministratorCommissioning;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OperationalCredentials::Id: {
        using namespace app::Clusters::OperationalCredentials;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::GroupKeyManagement::Id: {
        using namespace app::Clusters::GroupKeyManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::FixedLabel::Id: {
        using namespace app::Clusters::FixedLabel;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::UserLabel::Id: {
        using namespace app::Clusters::UserLabel;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ProxyConfiguration::Id: {
        using namespace app::Clusters::ProxyConfiguration;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ProxyDiscovery::Id: {
        using namespace app::Clusters::ProxyDiscovery;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ProxyValid::Id: {
        using namespace app::Clusters::ProxyValid;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::BooleanState::Id: {
        using namespace app::Clusters::BooleanState;
        switch (aPath.mEventId)
        {
        case Events::StateChange::Id: {
            Events::StateChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_stateValue;
            std::string value_stateValueClassName     = "java/lang/Boolean";
            std::string value_stateValueCtorSignature = "(Z)V";
            jboolean jnivalue_stateValue              = static_cast<jboolean>(cppValue.stateValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                value_stateValueClassName.c_str(), value_stateValueCtorSignature.c_str(), jnivalue_stateValue, value_stateValue);

            jclass stateChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BooleanStateClusterStateChangeEvent", stateChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BooleanStateClusterStateChangeEvent");
                return nullptr;
            }
            jmethodID stateChangeStructCtor = env->GetMethodID(stateChangeStructClass, "<init>", "(Ljava/lang/Boolean;)V");
            if (stateChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BooleanStateClusterStateChangeEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(stateChangeStructClass, stateChangeStructCtor, value_stateValue);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::IcdManagement::Id: {
        using namespace app::Clusters::IcdManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ModeSelect::Id: {
        using namespace app::Clusters::ModeSelect;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::LaundryWasherMode::Id: {
        using namespace app::Clusters::LaundryWasherMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id: {
        using namespace app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::LaundryWasherControls::Id: {
        using namespace app::Clusters::LaundryWasherControls;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RvcRunMode::Id: {
        using namespace app::Clusters::RvcRunMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RvcCleanMode::Id: {
        using namespace app::Clusters::RvcCleanMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TemperatureControl::Id: {
        using namespace app::Clusters::TemperatureControl;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RefrigeratorAlarm::Id: {
        using namespace app::Clusters::RefrigeratorAlarm;
        switch (aPath.mEventId)
        {
        case Events::Notify::Id: {
            Events::Notify::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_active;
            std::string value_activeClassName     = "java/lang/Long";
            std::string value_activeCtorSignature = "(J)V";
            jlong jnivalue_active                 = static_cast<jlong>(cppValue.active.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_activeClassName.c_str(), value_activeCtorSignature.c_str(), jnivalue_active, value_active);

            jobject value_inactive;
            std::string value_inactiveClassName     = "java/lang/Long";
            std::string value_inactiveCtorSignature = "(J)V";
            jlong jnivalue_inactive                 = static_cast<jlong>(cppValue.inactive.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_inactiveClassName.c_str(), value_inactiveCtorSignature.c_str(), jnivalue_inactive, value_inactive);

            jobject value_state;
            std::string value_stateClassName     = "java/lang/Long";
            std::string value_stateCtorSignature = "(J)V";
            jlong jnivalue_state                 = static_cast<jlong>(cppValue.state.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_stateClassName.c_str(), value_stateCtorSignature.c_str(), jnivalue_state, value_state);

            jobject value_mask;
            std::string value_maskClassName     = "java/lang/Long";
            std::string value_maskCtorSignature = "(J)V";
            jlong jnivalue_mask                 = static_cast<jlong>(cppValue.mask.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_maskClassName.c_str(),
                                                                        value_maskCtorSignature.c_str(), jnivalue_mask, value_mask);

            jclass notifyStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$RefrigeratorAlarmClusterNotifyEvent", notifyStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$RefrigeratorAlarmClusterNotifyEvent");
                return nullptr;
            }
            jmethodID notifyStructCtor = env->GetMethodID(notifyStructClass, "<init>",
                                                          "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;)V");
            if (notifyStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$RefrigeratorAlarmClusterNotifyEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(notifyStructClass, notifyStructCtor, value_active, value_inactive, value_state, value_mask);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DishwasherMode::Id: {
        using namespace app::Clusters::DishwasherMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::AirQuality::Id: {
        using namespace app::Clusters::AirQuality;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::SmokeCoAlarm::Id: {
        using namespace app::Clusters::SmokeCoAlarm;
        switch (aPath.mEventId)
        {
        case Events::SmokeAlarm::Id: {
            Events::SmokeAlarm::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmSeverityLevel;
            std::string value_alarmSeverityLevelClassName     = "java/lang/Integer";
            std::string value_alarmSeverityLevelCtorSignature = "(I)V";
            jint jnivalue_alarmSeverityLevel                  = static_cast<jint>(cppValue.alarmSeverityLevel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_alarmSeverityLevelClassName.c_str(),
                                                                       value_alarmSeverityLevelCtorSignature.c_str(),
                                                                       jnivalue_alarmSeverityLevel, value_alarmSeverityLevel);

            jclass smokeAlarmStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterSmokeAlarmEvent", smokeAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterSmokeAlarmEvent");
                return nullptr;
            }
            jmethodID smokeAlarmStructCtor = env->GetMethodID(smokeAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (smokeAlarmStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterSmokeAlarmEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(smokeAlarmStructClass, smokeAlarmStructCtor, value_alarmSeverityLevel);

            return value;
        }
        case Events::COAlarm::Id: {
            Events::COAlarm::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmSeverityLevel;
            std::string value_alarmSeverityLevelClassName     = "java/lang/Integer";
            std::string value_alarmSeverityLevelCtorSignature = "(I)V";
            jint jnivalue_alarmSeverityLevel                  = static_cast<jint>(cppValue.alarmSeverityLevel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_alarmSeverityLevelClassName.c_str(),
                                                                       value_alarmSeverityLevelCtorSignature.c_str(),
                                                                       jnivalue_alarmSeverityLevel, value_alarmSeverityLevel);

            jclass COAlarmStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterCOAlarmEvent", COAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterCOAlarmEvent");
                return nullptr;
            }
            jmethodID COAlarmStructCtor = env->GetMethodID(COAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (COAlarmStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterCOAlarmEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(COAlarmStructClass, COAlarmStructCtor, value_alarmSeverityLevel);

            return value;
        }
        case Events::LowBattery::Id: {
            Events::LowBattery::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmSeverityLevel;
            std::string value_alarmSeverityLevelClassName     = "java/lang/Integer";
            std::string value_alarmSeverityLevelCtorSignature = "(I)V";
            jint jnivalue_alarmSeverityLevel                  = static_cast<jint>(cppValue.alarmSeverityLevel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_alarmSeverityLevelClassName.c_str(),
                                                                       value_alarmSeverityLevelCtorSignature.c_str(),
                                                                       jnivalue_alarmSeverityLevel, value_alarmSeverityLevel);

            jclass lowBatteryStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterLowBatteryEvent", lowBatteryStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterLowBatteryEvent");
                return nullptr;
            }
            jmethodID lowBatteryStructCtor = env->GetMethodID(lowBatteryStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (lowBatteryStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterLowBatteryEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(lowBatteryStructClass, lowBatteryStructCtor, value_alarmSeverityLevel);

            return value;
        }
        case Events::HardwareFault::Id: {
            Events::HardwareFault::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass hardwareFaultStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterHardwareFaultEvent", hardwareFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterHardwareFaultEvent");
                return nullptr;
            }
            jmethodID hardwareFaultStructCtor = env->GetMethodID(hardwareFaultStructClass, "<init>", "()V");
            if (hardwareFaultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterHardwareFaultEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(hardwareFaultStructClass, hardwareFaultStructCtor);

            return value;
        }
        case Events::EndOfService::Id: {
            Events::EndOfService::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass endOfServiceStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterEndOfServiceEvent", endOfServiceStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterEndOfServiceEvent");
                return nullptr;
            }
            jmethodID endOfServiceStructCtor = env->GetMethodID(endOfServiceStructClass, "<init>", "()V");
            if (endOfServiceStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterEndOfServiceEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(endOfServiceStructClass, endOfServiceStructCtor);

            return value;
        }
        case Events::SelfTestComplete::Id: {
            Events::SelfTestComplete::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass selfTestCompleteStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterSelfTestCompleteEvent",
                selfTestCompleteStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterSelfTestCompleteEvent");
                return nullptr;
            }
            jmethodID selfTestCompleteStructCtor = env->GetMethodID(selfTestCompleteStructClass, "<init>", "()V");
            if (selfTestCompleteStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterSelfTestCompleteEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(selfTestCompleteStructClass, selfTestCompleteStructCtor);

            return value;
        }
        case Events::AlarmMuted::Id: {
            Events::AlarmMuted::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass alarmMutedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterAlarmMutedEvent", alarmMutedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterAlarmMutedEvent");
                return nullptr;
            }
            jmethodID alarmMutedStructCtor = env->GetMethodID(alarmMutedStructClass, "<init>", "()V");
            if (alarmMutedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterAlarmMutedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(alarmMutedStructClass, alarmMutedStructCtor);

            return value;
        }
        case Events::MuteEnded::Id: {
            Events::MuteEnded::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass muteEndedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterMuteEndedEvent", muteEndedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterMuteEndedEvent");
                return nullptr;
            }
            jmethodID muteEndedStructCtor = env->GetMethodID(muteEndedStructClass, "<init>", "()V");
            if (muteEndedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterMuteEndedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(muteEndedStructClass, muteEndedStructCtor);

            return value;
        }
        case Events::InterconnectSmokeAlarm::Id: {
            Events::InterconnectSmokeAlarm::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmSeverityLevel;
            std::string value_alarmSeverityLevelClassName     = "java/lang/Integer";
            std::string value_alarmSeverityLevelCtorSignature = "(I)V";
            jint jnivalue_alarmSeverityLevel                  = static_cast<jint>(cppValue.alarmSeverityLevel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_alarmSeverityLevelClassName.c_str(),
                                                                       value_alarmSeverityLevelCtorSignature.c_str(),
                                                                       jnivalue_alarmSeverityLevel, value_alarmSeverityLevel);

            jclass interconnectSmokeAlarmStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterInterconnectSmokeAlarmEvent",
                interconnectSmokeAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterInterconnectSmokeAlarmEvent");
                return nullptr;
            }
            jmethodID interconnectSmokeAlarmStructCtor =
                env->GetMethodID(interconnectSmokeAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (interconnectSmokeAlarmStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterInterconnectSmokeAlarmEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(interconnectSmokeAlarmStructClass, interconnectSmokeAlarmStructCtor, value_alarmSeverityLevel);

            return value;
        }
        case Events::InterconnectCOAlarm::Id: {
            Events::InterconnectCOAlarm::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmSeverityLevel;
            std::string value_alarmSeverityLevelClassName     = "java/lang/Integer";
            std::string value_alarmSeverityLevelCtorSignature = "(I)V";
            jint jnivalue_alarmSeverityLevel                  = static_cast<jint>(cppValue.alarmSeverityLevel);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_alarmSeverityLevelClassName.c_str(),
                                                                       value_alarmSeverityLevelCtorSignature.c_str(),
                                                                       jnivalue_alarmSeverityLevel, value_alarmSeverityLevel);

            jclass interconnectCOAlarmStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterInterconnectCOAlarmEvent",
                interconnectCOAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterInterconnectCOAlarmEvent");
                return nullptr;
            }
            jmethodID interconnectCOAlarmStructCtor =
                env->GetMethodID(interconnectCOAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (interconnectCOAlarmStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterInterconnectCOAlarmEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(interconnectCOAlarmStructClass, interconnectCOAlarmStructCtor, value_alarmSeverityLevel);

            return value;
        }
        case Events::AllClear::Id: {
            Events::AllClear::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass allClearStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterAllClearEvent", allClearStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterAllClearEvent");
                return nullptr;
            }
            jmethodID allClearStructCtor = env->GetMethodID(allClearStructClass, "<init>", "()V");
            if (allClearStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SmokeCoAlarmClusterAllClearEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(allClearStructClass, allClearStructCtor);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DishwasherAlarm::Id: {
        using namespace app::Clusters::DishwasherAlarm;
        switch (aPath.mEventId)
        {
        case Events::Notify::Id: {
            Events::Notify::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_active;
            std::string value_activeClassName     = "java/lang/Long";
            std::string value_activeCtorSignature = "(J)V";
            jlong jnivalue_active                 = static_cast<jlong>(cppValue.active.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_activeClassName.c_str(), value_activeCtorSignature.c_str(), jnivalue_active, value_active);

            jobject value_inactive;
            std::string value_inactiveClassName     = "java/lang/Long";
            std::string value_inactiveCtorSignature = "(J)V";
            jlong jnivalue_inactive                 = static_cast<jlong>(cppValue.inactive.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_inactiveClassName.c_str(), value_inactiveCtorSignature.c_str(), jnivalue_inactive, value_inactive);

            jobject value_state;
            std::string value_stateClassName     = "java/lang/Long";
            std::string value_stateCtorSignature = "(J)V";
            jlong jnivalue_state                 = static_cast<jlong>(cppValue.state.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_stateClassName.c_str(), value_stateCtorSignature.c_str(), jnivalue_state, value_state);

            jobject value_mask;
            std::string value_maskClassName     = "java/lang/Long";
            std::string value_maskCtorSignature = "(J)V";
            jlong jnivalue_mask                 = static_cast<jlong>(cppValue.mask.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_maskClassName.c_str(),
                                                                        value_maskCtorSignature.c_str(), jnivalue_mask, value_mask);

            jclass notifyStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DishwasherAlarmClusterNotifyEvent", notifyStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DishwasherAlarmClusterNotifyEvent");
                return nullptr;
            }
            jmethodID notifyStructCtor = env->GetMethodID(notifyStructClass, "<init>",
                                                          "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;)V");
            if (notifyStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DishwasherAlarmClusterNotifyEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(notifyStructClass, notifyStructCtor, value_active, value_inactive, value_state, value_mask);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OperationalState::Id: {
        using namespace app::Clusters::OperationalState;
        switch (aPath.mEventId)
        {
        case Events::OperationalError::Id: {
            Events::OperationalError::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_errorState;
            jobject value_errorState_errorStateID;
            std::string value_errorState_errorStateIDClassName     = "java/lang/Integer";
            std::string value_errorState_errorStateIDCtorSignature = "(I)V";
            jint jnivalue_errorState_errorStateID                  = static_cast<jint>(cppValue.errorState.errorStateID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_errorState_errorStateIDClassName.c_str(), value_errorState_errorStateIDCtorSignature.c_str(),
                jnivalue_errorState_errorStateID, value_errorState_errorStateID);
            jobject value_errorState_errorStateLabel;
            if (!cppValue.errorState.errorStateLabel.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_errorState_errorStateLabel);
            }
            else
            {
                jobject value_errorState_errorStateLabelInsideOptional;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                    cppValue.errorState.errorStateLabel.Value(), value_errorState_errorStateLabelInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_errorState_errorStateLabelInsideOptional,
                                                                  value_errorState_errorStateLabel);
            }
            jobject value_errorState_errorStateDetails;
            if (!cppValue.errorState.errorStateDetails.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_errorState_errorStateDetails);
            }
            else
            {
                jobject value_errorState_errorStateDetailsInsideOptional;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                    cppValue.errorState.errorStateDetails.Value(), value_errorState_errorStateDetailsInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_errorState_errorStateDetailsInsideOptional,
                                                                  value_errorState_errorStateDetails);
            }

            jclass errorStateStructStructClass_0;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$OperationalStateClusterErrorStateStruct", errorStateStructStructClass_0);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$OperationalStateClusterErrorStateStruct");
                return nullptr;
            }
            jmethodID errorStateStructStructCtor_0 = env->GetMethodID(
                errorStateStructStructClass_0, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V");
            if (errorStateStructStructCtor_0 == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$OperationalStateClusterErrorStateStruct constructor");
                return nullptr;
            }

            value_errorState =
                env->NewObject(errorStateStructStructClass_0, errorStateStructStructCtor_0, value_errorState_errorStateID,
                               value_errorState_errorStateLabel, value_errorState_errorStateDetails);

            jclass operationalErrorStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OperationalStateClusterOperationalErrorEvent",
                operationalErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OperationalStateClusterOperationalErrorEvent");
                return nullptr;
            }
            jmethodID operationalErrorStructCtor =
                env->GetMethodID(operationalErrorStructClass, "<init>",
                                 "(Lchip/devicecontroller/ChipStructs$OperationalStateClusterErrorStateStruct;)V");
            if (operationalErrorStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$OperationalStateClusterOperationalErrorEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(operationalErrorStructClass, operationalErrorStructCtor, value_errorState);

            return value;
        }
        case Events::OperationCompletion::Id: {
            Events::OperationCompletion::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_completionErrorCode;
            std::string value_completionErrorCodeClassName     = "java/lang/Integer";
            std::string value_completionErrorCodeCtorSignature = "(I)V";
            jint jnivalue_completionErrorCode                  = static_cast<jint>(cppValue.completionErrorCode);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_completionErrorCodeClassName.c_str(),
                                                                       value_completionErrorCodeCtorSignature.c_str(),
                                                                       jnivalue_completionErrorCode, value_completionErrorCode);

            jobject value_totalOperationalTime;
            if (!cppValue.totalOperationalTime.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_totalOperationalTime);
            }
            else
            {
                jobject value_totalOperationalTimeInsideOptional;
                if (cppValue.totalOperationalTime.Value().IsNull())
                {
                    value_totalOperationalTimeInsideOptional = nullptr;
                }
                else
                {
                    std::string value_totalOperationalTimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_totalOperationalTimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_totalOperationalTimeInsideOptional =
                        static_cast<jlong>(cppValue.totalOperationalTime.Value().Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_totalOperationalTimeInsideOptionalClassName.c_str(),
                        value_totalOperationalTimeInsideOptionalCtorSignature.c_str(), jnivalue_totalOperationalTimeInsideOptional,
                        value_totalOperationalTimeInsideOptional);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_totalOperationalTimeInsideOptional,
                                                                  value_totalOperationalTime);
            }

            jobject value_pausedTime;
            if (!cppValue.pausedTime.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_pausedTime);
            }
            else
            {
                jobject value_pausedTimeInsideOptional;
                if (cppValue.pausedTime.Value().IsNull())
                {
                    value_pausedTimeInsideOptional = nullptr;
                }
                else
                {
                    std::string value_pausedTimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_pausedTimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_pausedTimeInsideOptional = static_cast<jlong>(cppValue.pausedTime.Value().Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_pausedTimeInsideOptionalClassName.c_str(), value_pausedTimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_pausedTimeInsideOptional, value_pausedTimeInsideOptional);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_pausedTimeInsideOptional, value_pausedTime);
            }

            jclass operationCompletionStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OperationalStateClusterOperationCompletionEvent",
                operationCompletionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OperationalStateClusterOperationCompletionEvent");
                return nullptr;
            }
            jmethodID operationCompletionStructCtor = env->GetMethodID(
                operationCompletionStructClass, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V");
            if (operationCompletionStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$OperationalStateClusterOperationCompletionEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(operationCompletionStructClass, operationCompletionStructCtor, value_completionErrorCode,
                                           value_totalOperationalTime, value_pausedTime);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RvcOperationalState::Id: {
        using namespace app::Clusters::RvcOperationalState;
        switch (aPath.mEventId)
        {
        case Events::OperationalError::Id: {
            Events::OperationalError::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_errorState;
            jobject value_errorState_errorStateID;
            std::string value_errorState_errorStateIDClassName     = "java/lang/Integer";
            std::string value_errorState_errorStateIDCtorSignature = "(I)V";
            jint jnivalue_errorState_errorStateID                  = static_cast<jint>(cppValue.errorState.errorStateID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_errorState_errorStateIDClassName.c_str(), value_errorState_errorStateIDCtorSignature.c_str(),
                jnivalue_errorState_errorStateID, value_errorState_errorStateID);
            jobject value_errorState_errorStateLabel;
            if (!cppValue.errorState.errorStateLabel.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_errorState_errorStateLabel);
            }
            else
            {
                jobject value_errorState_errorStateLabelInsideOptional;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                    cppValue.errorState.errorStateLabel.Value(), value_errorState_errorStateLabelInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_errorState_errorStateLabelInsideOptional,
                                                                  value_errorState_errorStateLabel);
            }
            jobject value_errorState_errorStateDetails;
            if (!cppValue.errorState.errorStateDetails.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_errorState_errorStateDetails);
            }
            else
            {
                jobject value_errorState_errorStateDetailsInsideOptional;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                    cppValue.errorState.errorStateDetails.Value(), value_errorState_errorStateDetailsInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_errorState_errorStateDetailsInsideOptional,
                                                                  value_errorState_errorStateDetails);
            }

            jclass errorStateStructStructClass_0;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$RvcOperationalStateClusterErrorStateStruct", errorStateStructStructClass_0);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$RvcOperationalStateClusterErrorStateStruct");
                return nullptr;
            }
            jmethodID errorStateStructStructCtor_0 = env->GetMethodID(
                errorStateStructStructClass_0, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V");
            if (errorStateStructStructCtor_0 == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$RvcOperationalStateClusterErrorStateStruct constructor");
                return nullptr;
            }

            value_errorState =
                env->NewObject(errorStateStructStructClass_0, errorStateStructStructCtor_0, value_errorState_errorStateID,
                               value_errorState_errorStateLabel, value_errorState_errorStateDetails);

            jclass operationalErrorStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$RvcOperationalStateClusterOperationalErrorEvent",
                operationalErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$RvcOperationalStateClusterOperationalErrorEvent");
                return nullptr;
            }
            jmethodID operationalErrorStructCtor =
                env->GetMethodID(operationalErrorStructClass, "<init>",
                                 "(Lchip/devicecontroller/ChipStructs$RvcOperationalStateClusterErrorStateStruct;)V");
            if (operationalErrorStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$RvcOperationalStateClusterOperationalErrorEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(operationalErrorStructClass, operationalErrorStructCtor, value_errorState);

            return value;
        }
        case Events::OperationCompletion::Id: {
            Events::OperationCompletion::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_completionErrorCode;
            std::string value_completionErrorCodeClassName     = "java/lang/Integer";
            std::string value_completionErrorCodeCtorSignature = "(I)V";
            jint jnivalue_completionErrorCode                  = static_cast<jint>(cppValue.completionErrorCode);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_completionErrorCodeClassName.c_str(),
                                                                       value_completionErrorCodeCtorSignature.c_str(),
                                                                       jnivalue_completionErrorCode, value_completionErrorCode);

            jobject value_totalOperationalTime;
            if (!cppValue.totalOperationalTime.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_totalOperationalTime);
            }
            else
            {
                jobject value_totalOperationalTimeInsideOptional;
                if (cppValue.totalOperationalTime.Value().IsNull())
                {
                    value_totalOperationalTimeInsideOptional = nullptr;
                }
                else
                {
                    std::string value_totalOperationalTimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_totalOperationalTimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_totalOperationalTimeInsideOptional =
                        static_cast<jlong>(cppValue.totalOperationalTime.Value().Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_totalOperationalTimeInsideOptionalClassName.c_str(),
                        value_totalOperationalTimeInsideOptionalCtorSignature.c_str(), jnivalue_totalOperationalTimeInsideOptional,
                        value_totalOperationalTimeInsideOptional);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_totalOperationalTimeInsideOptional,
                                                                  value_totalOperationalTime);
            }

            jobject value_pausedTime;
            if (!cppValue.pausedTime.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_pausedTime);
            }
            else
            {
                jobject value_pausedTimeInsideOptional;
                if (cppValue.pausedTime.Value().IsNull())
                {
                    value_pausedTimeInsideOptional = nullptr;
                }
                else
                {
                    std::string value_pausedTimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_pausedTimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_pausedTimeInsideOptional = static_cast<jlong>(cppValue.pausedTime.Value().Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_pausedTimeInsideOptionalClassName.c_str(), value_pausedTimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_pausedTimeInsideOptional, value_pausedTimeInsideOptional);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_pausedTimeInsideOptional, value_pausedTime);
            }

            jclass operationCompletionStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$RvcOperationalStateClusterOperationCompletionEvent",
                operationCompletionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$RvcOperationalStateClusterOperationCompletionEvent");
                return nullptr;
            }
            jmethodID operationCompletionStructCtor = env->GetMethodID(
                operationCompletionStructClass, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V");
            if (operationCompletionStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$RvcOperationalStateClusterOperationCompletionEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(operationCompletionStructClass, operationCompletionStructCtor, value_completionErrorCode,
                                           value_totalOperationalTime, value_pausedTime);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::HepaFilterMonitoring::Id: {
        using namespace app::Clusters::HepaFilterMonitoring;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ActivatedCarbonFilterMonitoring::Id: {
        using namespace app::Clusters::ActivatedCarbonFilterMonitoring;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DoorLock::Id: {
        using namespace app::Clusters::DoorLock;
        switch (aPath.mEventId)
        {
        case Events::DoorLockAlarm::Id: {
            Events::DoorLockAlarm::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmCode;
            std::string value_alarmCodeClassName     = "java/lang/Integer";
            std::string value_alarmCodeCtorSignature = "(I)V";
            jint jnivalue_alarmCode                  = static_cast<jint>(cppValue.alarmCode);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_alarmCodeClassName.c_str(), value_alarmCodeCtorSignature.c_str(), jnivalue_alarmCode, value_alarmCode);

            jclass doorLockAlarmStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterDoorLockAlarmEvent", doorLockAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterDoorLockAlarmEvent");
                return nullptr;
            }
            jmethodID doorLockAlarmStructCtor = env->GetMethodID(doorLockAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (doorLockAlarmStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DoorLockClusterDoorLockAlarmEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(doorLockAlarmStructClass, doorLockAlarmStructCtor, value_alarmCode);

            return value;
        }
        case Events::DoorStateChange::Id: {
            Events::DoorStateChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_doorState;
            std::string value_doorStateClassName     = "java/lang/Integer";
            std::string value_doorStateCtorSignature = "(I)V";
            jint jnivalue_doorState                  = static_cast<jint>(cppValue.doorState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_doorStateClassName.c_str(), value_doorStateCtorSignature.c_str(), jnivalue_doorState, value_doorState);

            jclass doorStateChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterDoorStateChangeEvent", doorStateChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterDoorStateChangeEvent");
                return nullptr;
            }
            jmethodID doorStateChangeStructCtor = env->GetMethodID(doorStateChangeStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (doorStateChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DoorLockClusterDoorStateChangeEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(doorStateChangeStructClass, doorStateChangeStructCtor, value_doorState);

            return value;
        }
        case Events::LockOperation::Id: {
            Events::LockOperation::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_lockOperationType;
            std::string value_lockOperationTypeClassName     = "java/lang/Integer";
            std::string value_lockOperationTypeCtorSignature = "(I)V";
            jint jnivalue_lockOperationType                  = static_cast<jint>(cppValue.lockOperationType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_lockOperationTypeClassName.c_str(),
                                                                       value_lockOperationTypeCtorSignature.c_str(),
                                                                       jnivalue_lockOperationType, value_lockOperationType);

            jobject value_operationSource;
            std::string value_operationSourceClassName     = "java/lang/Integer";
            std::string value_operationSourceCtorSignature = "(I)V";
            jint jnivalue_operationSource                  = static_cast<jint>(cppValue.operationSource);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_operationSourceClassName.c_str(),
                                                                       value_operationSourceCtorSignature.c_str(),
                                                                       jnivalue_operationSource, value_operationSource);

            jobject value_userIndex;
            if (cppValue.userIndex.IsNull())
            {
                value_userIndex = nullptr;
            }
            else
            {
                std::string value_userIndexClassName     = "java/lang/Integer";
                std::string value_userIndexCtorSignature = "(I)V";
                jint jnivalue_userIndex                  = static_cast<jint>(cppValue.userIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_userIndexClassName.c_str(), value_userIndexCtorSignature.c_str(), jnivalue_userIndex, value_userIndex);
            }

            jobject value_fabricIndex;
            if (cppValue.fabricIndex.IsNull())
            {
                value_fabricIndex = nullptr;
            }
            else
            {
                std::string value_fabricIndexClassName     = "java/lang/Integer";
                std::string value_fabricIndexCtorSignature = "(I)V";
                jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                           value_fabricIndexCtorSignature.c_str(),
                                                                           jnivalue_fabricIndex, value_fabricIndex);
            }

            jobject value_sourceNode;
            if (cppValue.sourceNode.IsNull())
            {
                value_sourceNode = nullptr;
            }
            else
            {
                std::string value_sourceNodeClassName     = "java/lang/Long";
                std::string value_sourceNodeCtorSignature = "(J)V";
                jlong jnivalue_sourceNode                 = static_cast<jlong>(cppValue.sourceNode.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_sourceNodeClassName.c_str(),
                                                                            value_sourceNodeCtorSignature.c_str(),
                                                                            jnivalue_sourceNode, value_sourceNode);
            }

            jobject value_credentials;
            if (!cppValue.credentials.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_credentials);
            }
            else
            {
                jobject value_credentialsInsideOptional;
                if (cppValue.credentials.Value().IsNull())
                {
                    value_credentialsInsideOptional = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(value_credentialsInsideOptional);

                    auto iter_value_credentialsInsideOptional_2 = cppValue.credentials.Value().Value().begin();
                    while (iter_value_credentialsInsideOptional_2.Next())
                    {
                        auto & entry_2 = iter_value_credentialsInsideOptional_2.GetValue();
                        jobject newElement_2;
                        jobject newElement_2_credentialType;
                        std::string newElement_2_credentialTypeClassName     = "java/lang/Integer";
                        std::string newElement_2_credentialTypeCtorSignature = "(I)V";
                        jint jninewElement_2_credentialType                  = static_cast<jint>(entry_2.credentialType);
                        chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                            newElement_2_credentialTypeClassName.c_str(), newElement_2_credentialTypeCtorSignature.c_str(),
                            jninewElement_2_credentialType, newElement_2_credentialType);
                        jobject newElement_2_credentialIndex;
                        std::string newElement_2_credentialIndexClassName     = "java/lang/Integer";
                        std::string newElement_2_credentialIndexCtorSignature = "(I)V";
                        jint jninewElement_2_credentialIndex                  = static_cast<jint>(entry_2.credentialIndex);
                        chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                            newElement_2_credentialIndexClassName.c_str(), newElement_2_credentialIndexCtorSignature.c_str(),
                            jninewElement_2_credentialIndex, newElement_2_credentialIndex);

                        jclass credentialStructStructClass_3;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$DoorLockClusterCredentialStruct",
                            credentialStructStructClass_3);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$DoorLockClusterCredentialStruct");
                            return nullptr;
                        }
                        jmethodID credentialStructStructCtor_3 =
                            env->GetMethodID(credentialStructStructClass_3, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
                        if (credentialStructStructCtor_3 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$DoorLockClusterCredentialStruct constructor");
                            return nullptr;
                        }

                        newElement_2 = env->NewObject(credentialStructStructClass_3, credentialStructStructCtor_3,
                                                      newElement_2_credentialType, newElement_2_credentialIndex);
                        chip::JniReferences::GetInstance().AddToList(value_credentialsInsideOptional, newElement_2);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_credentialsInsideOptional, value_credentials);
            }

            jclass lockOperationStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterLockOperationEvent", lockOperationStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterLockOperationEvent");
                return nullptr;
            }
            jmethodID lockOperationStructCtor = env->GetMethodID(lockOperationStructClass, "<init>",
                                                                 "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/"
                                                                 "lang/Integer;Ljava/lang/Long;Ljava/util/Optional;)V");
            if (lockOperationStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DoorLockClusterLockOperationEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(lockOperationStructClass, lockOperationStructCtor, value_lockOperationType, value_operationSource,
                               value_userIndex, value_fabricIndex, value_sourceNode, value_credentials);

            return value;
        }
        case Events::LockOperationError::Id: {
            Events::LockOperationError::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_lockOperationType;
            std::string value_lockOperationTypeClassName     = "java/lang/Integer";
            std::string value_lockOperationTypeCtorSignature = "(I)V";
            jint jnivalue_lockOperationType                  = static_cast<jint>(cppValue.lockOperationType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_lockOperationTypeClassName.c_str(),
                                                                       value_lockOperationTypeCtorSignature.c_str(),
                                                                       jnivalue_lockOperationType, value_lockOperationType);

            jobject value_operationSource;
            std::string value_operationSourceClassName     = "java/lang/Integer";
            std::string value_operationSourceCtorSignature = "(I)V";
            jint jnivalue_operationSource                  = static_cast<jint>(cppValue.operationSource);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_operationSourceClassName.c_str(),
                                                                       value_operationSourceCtorSignature.c_str(),
                                                                       jnivalue_operationSource, value_operationSource);

            jobject value_operationError;
            std::string value_operationErrorClassName     = "java/lang/Integer";
            std::string value_operationErrorCtorSignature = "(I)V";
            jint jnivalue_operationError                  = static_cast<jint>(cppValue.operationError);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_operationErrorClassName.c_str(),
                                                                       value_operationErrorCtorSignature.c_str(),
                                                                       jnivalue_operationError, value_operationError);

            jobject value_userIndex;
            if (cppValue.userIndex.IsNull())
            {
                value_userIndex = nullptr;
            }
            else
            {
                std::string value_userIndexClassName     = "java/lang/Integer";
                std::string value_userIndexCtorSignature = "(I)V";
                jint jnivalue_userIndex                  = static_cast<jint>(cppValue.userIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_userIndexClassName.c_str(), value_userIndexCtorSignature.c_str(), jnivalue_userIndex, value_userIndex);
            }

            jobject value_fabricIndex;
            if (cppValue.fabricIndex.IsNull())
            {
                value_fabricIndex = nullptr;
            }
            else
            {
                std::string value_fabricIndexClassName     = "java/lang/Integer";
                std::string value_fabricIndexCtorSignature = "(I)V";
                jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                           value_fabricIndexCtorSignature.c_str(),
                                                                           jnivalue_fabricIndex, value_fabricIndex);
            }

            jobject value_sourceNode;
            if (cppValue.sourceNode.IsNull())
            {
                value_sourceNode = nullptr;
            }
            else
            {
                std::string value_sourceNodeClassName     = "java/lang/Long";
                std::string value_sourceNodeCtorSignature = "(J)V";
                jlong jnivalue_sourceNode                 = static_cast<jlong>(cppValue.sourceNode.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_sourceNodeClassName.c_str(),
                                                                            value_sourceNodeCtorSignature.c_str(),
                                                                            jnivalue_sourceNode, value_sourceNode);
            }

            jobject value_credentials;
            if (!cppValue.credentials.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_credentials);
            }
            else
            {
                jobject value_credentialsInsideOptional;
                if (cppValue.credentials.Value().IsNull())
                {
                    value_credentialsInsideOptional = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(value_credentialsInsideOptional);

                    auto iter_value_credentialsInsideOptional_2 = cppValue.credentials.Value().Value().begin();
                    while (iter_value_credentialsInsideOptional_2.Next())
                    {
                        auto & entry_2 = iter_value_credentialsInsideOptional_2.GetValue();
                        jobject newElement_2;
                        jobject newElement_2_credentialType;
                        std::string newElement_2_credentialTypeClassName     = "java/lang/Integer";
                        std::string newElement_2_credentialTypeCtorSignature = "(I)V";
                        jint jninewElement_2_credentialType                  = static_cast<jint>(entry_2.credentialType);
                        chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                            newElement_2_credentialTypeClassName.c_str(), newElement_2_credentialTypeCtorSignature.c_str(),
                            jninewElement_2_credentialType, newElement_2_credentialType);
                        jobject newElement_2_credentialIndex;
                        std::string newElement_2_credentialIndexClassName     = "java/lang/Integer";
                        std::string newElement_2_credentialIndexCtorSignature = "(I)V";
                        jint jninewElement_2_credentialIndex                  = static_cast<jint>(entry_2.credentialIndex);
                        chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                            newElement_2_credentialIndexClassName.c_str(), newElement_2_credentialIndexCtorSignature.c_str(),
                            jninewElement_2_credentialIndex, newElement_2_credentialIndex);

                        jclass credentialStructStructClass_3;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$DoorLockClusterCredentialStruct",
                            credentialStructStructClass_3);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$DoorLockClusterCredentialStruct");
                            return nullptr;
                        }
                        jmethodID credentialStructStructCtor_3 =
                            env->GetMethodID(credentialStructStructClass_3, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
                        if (credentialStructStructCtor_3 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$DoorLockClusterCredentialStruct constructor");
                            return nullptr;
                        }

                        newElement_2 = env->NewObject(credentialStructStructClass_3, credentialStructStructCtor_3,
                                                      newElement_2_credentialType, newElement_2_credentialIndex);
                        chip::JniReferences::GetInstance().AddToList(value_credentialsInsideOptional, newElement_2);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_credentialsInsideOptional, value_credentials);
            }

            jclass lockOperationErrorStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterLockOperationErrorEvent",
                lockOperationErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterLockOperationErrorEvent");
                return nullptr;
            }
            jmethodID lockOperationErrorStructCtor =
                env->GetMethodID(lockOperationErrorStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                                 "Integer;Ljava/lang/Long;Ljava/util/Optional;)V");
            if (lockOperationErrorStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DoorLockClusterLockOperationErrorEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(lockOperationErrorStructClass, lockOperationErrorStructCtor, value_lockOperationType,
                                           value_operationSource, value_operationError, value_userIndex, value_fabricIndex,
                                           value_sourceNode, value_credentials);

            return value;
        }
        case Events::LockUserChange::Id: {
            Events::LockUserChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_lockDataType;
            std::string value_lockDataTypeClassName     = "java/lang/Integer";
            std::string value_lockDataTypeCtorSignature = "(I)V";
            jint jnivalue_lockDataType                  = static_cast<jint>(cppValue.lockDataType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_lockDataTypeClassName.c_str(),
                                                                       value_lockDataTypeCtorSignature.c_str(),
                                                                       jnivalue_lockDataType, value_lockDataType);

            jobject value_dataOperationType;
            std::string value_dataOperationTypeClassName     = "java/lang/Integer";
            std::string value_dataOperationTypeCtorSignature = "(I)V";
            jint jnivalue_dataOperationType                  = static_cast<jint>(cppValue.dataOperationType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_dataOperationTypeClassName.c_str(),
                                                                       value_dataOperationTypeCtorSignature.c_str(),
                                                                       jnivalue_dataOperationType, value_dataOperationType);

            jobject value_operationSource;
            std::string value_operationSourceClassName     = "java/lang/Integer";
            std::string value_operationSourceCtorSignature = "(I)V";
            jint jnivalue_operationSource                  = static_cast<jint>(cppValue.operationSource);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_operationSourceClassName.c_str(),
                                                                       value_operationSourceCtorSignature.c_str(),
                                                                       jnivalue_operationSource, value_operationSource);

            jobject value_userIndex;
            if (cppValue.userIndex.IsNull())
            {
                value_userIndex = nullptr;
            }
            else
            {
                std::string value_userIndexClassName     = "java/lang/Integer";
                std::string value_userIndexCtorSignature = "(I)V";
                jint jnivalue_userIndex                  = static_cast<jint>(cppValue.userIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_userIndexClassName.c_str(), value_userIndexCtorSignature.c_str(), jnivalue_userIndex, value_userIndex);
            }

            jobject value_fabricIndex;
            if (cppValue.fabricIndex.IsNull())
            {
                value_fabricIndex = nullptr;
            }
            else
            {
                std::string value_fabricIndexClassName     = "java/lang/Integer";
                std::string value_fabricIndexCtorSignature = "(I)V";
                jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                           value_fabricIndexCtorSignature.c_str(),
                                                                           jnivalue_fabricIndex, value_fabricIndex);
            }

            jobject value_sourceNode;
            if (cppValue.sourceNode.IsNull())
            {
                value_sourceNode = nullptr;
            }
            else
            {
                std::string value_sourceNodeClassName     = "java/lang/Long";
                std::string value_sourceNodeCtorSignature = "(J)V";
                jlong jnivalue_sourceNode                 = static_cast<jlong>(cppValue.sourceNode.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_sourceNodeClassName.c_str(),
                                                                            value_sourceNodeCtorSignature.c_str(),
                                                                            jnivalue_sourceNode, value_sourceNode);
            }

            jobject value_dataIndex;
            if (cppValue.dataIndex.IsNull())
            {
                value_dataIndex = nullptr;
            }
            else
            {
                std::string value_dataIndexClassName     = "java/lang/Integer";
                std::string value_dataIndexCtorSignature = "(I)V";
                jint jnivalue_dataIndex                  = static_cast<jint>(cppValue.dataIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_dataIndexClassName.c_str(), value_dataIndexCtorSignature.c_str(), jnivalue_dataIndex, value_dataIndex);
            }

            jclass lockUserChangeStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterLockUserChangeEvent", lockUserChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterLockUserChangeEvent");
                return nullptr;
            }
            jmethodID lockUserChangeStructCtor =
                env->GetMethodID(lockUserChangeStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                                 "Integer;Ljava/lang/Long;Ljava/lang/Integer;)V");
            if (lockUserChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DoorLockClusterLockUserChangeEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(lockUserChangeStructClass, lockUserChangeStructCtor, value_lockDataType, value_dataOperationType,
                               value_operationSource, value_userIndex, value_fabricIndex, value_sourceNode, value_dataIndex);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::WindowCovering::Id: {
        using namespace app::Clusters::WindowCovering;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::BarrierControl::Id: {
        using namespace app::Clusters::BarrierControl;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PumpConfigurationAndControl::Id: {
        using namespace app::Clusters::PumpConfigurationAndControl;
        switch (aPath.mEventId)
        {
        case Events::SupplyVoltageLow::Id: {
            Events::SupplyVoltageLow::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass supplyVoltageLowStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageLowEvent",
                supplyVoltageLowStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageLowEvent");
                return nullptr;
            }
            jmethodID supplyVoltageLowStructCtor = env->GetMethodID(supplyVoltageLowStructClass, "<init>", "()V");
            if (supplyVoltageLowStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageLowEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(supplyVoltageLowStructClass, supplyVoltageLowStructCtor);

            return value;
        }
        case Events::SupplyVoltageHigh::Id: {
            Events::SupplyVoltageHigh::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass supplyVoltageHighStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageHighEvent",
                supplyVoltageHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageHighEvent");
                return nullptr;
            }
            jmethodID supplyVoltageHighStructCtor = env->GetMethodID(supplyVoltageHighStructClass, "<init>", "()V");
            if (supplyVoltageHighStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageHighEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(supplyVoltageHighStructClass, supplyVoltageHighStructCtor);

            return value;
        }
        case Events::PowerMissingPhase::Id: {
            Events::PowerMissingPhase::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass powerMissingPhaseStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterPowerMissingPhaseEvent",
                powerMissingPhaseStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterPowerMissingPhaseEvent");
                return nullptr;
            }
            jmethodID powerMissingPhaseStructCtor = env->GetMethodID(powerMissingPhaseStructClass, "<init>", "()V");
            if (powerMissingPhaseStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterPowerMissingPhaseEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(powerMissingPhaseStructClass, powerMissingPhaseStructCtor);

            return value;
        }
        case Events::SystemPressureLow::Id: {
            Events::SystemPressureLow::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass systemPressureLowStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureLowEvent",
                systemPressureLowStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureLowEvent");
                return nullptr;
            }
            jmethodID systemPressureLowStructCtor = env->GetMethodID(systemPressureLowStructClass, "<init>", "()V");
            if (systemPressureLowStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureLowEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(systemPressureLowStructClass, systemPressureLowStructCtor);

            return value;
        }
        case Events::SystemPressureHigh::Id: {
            Events::SystemPressureHigh::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass systemPressureHighStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureHighEvent",
                systemPressureHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureHighEvent");
                return nullptr;
            }
            jmethodID systemPressureHighStructCtor = env->GetMethodID(systemPressureHighStructClass, "<init>", "()V");
            if (systemPressureHighStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureHighEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(systemPressureHighStructClass, systemPressureHighStructCtor);

            return value;
        }
        case Events::DryRunning::Id: {
            Events::DryRunning::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass dryRunningStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterDryRunningEvent",
                dryRunningStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterDryRunningEvent");
                return nullptr;
            }
            jmethodID dryRunningStructCtor = env->GetMethodID(dryRunningStructClass, "<init>", "()V");
            if (dryRunningStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterDryRunningEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(dryRunningStructClass, dryRunningStructCtor);

            return value;
        }
        case Events::MotorTemperatureHigh::Id: {
            Events::MotorTemperatureHigh::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass motorTemperatureHighStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterMotorTemperatureHighEvent",
                motorTemperatureHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterMotorTemperatureHighEvent");
                return nullptr;
            }
            jmethodID motorTemperatureHighStructCtor = env->GetMethodID(motorTemperatureHighStructClass, "<init>", "()V");
            if (motorTemperatureHighStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterMotorTemperatureHighEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(motorTemperatureHighStructClass, motorTemperatureHighStructCtor);

            return value;
        }
        case Events::PumpMotorFatalFailure::Id: {
            Events::PumpMotorFatalFailure::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass pumpMotorFatalFailureStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent",
                pumpMotorFatalFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent");
                return nullptr;
            }
            jmethodID pumpMotorFatalFailureStructCtor = env->GetMethodID(pumpMotorFatalFailureStructClass, "<init>", "()V");
            if (pumpMotorFatalFailureStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(pumpMotorFatalFailureStructClass, pumpMotorFatalFailureStructCtor);

            return value;
        }
        case Events::ElectronicTemperatureHigh::Id: {
            Events::ElectronicTemperatureHigh::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass electronicTemperatureHighStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterElectronicTemperatureHighEvent",
                electronicTemperatureHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterElectronicTemperatureHighEvent");
                return nullptr;
            }
            jmethodID electronicTemperatureHighStructCtor = env->GetMethodID(electronicTemperatureHighStructClass, "<init>", "()V");
            if (electronicTemperatureHighStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$PumpConfigurationAndControlClusterElectronicTemperatureHighEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(electronicTemperatureHighStructClass, electronicTemperatureHighStructCtor);

            return value;
        }
        case Events::PumpBlocked::Id: {
            Events::PumpBlocked::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass pumpBlockedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterPumpBlockedEvent",
                pumpBlockedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterPumpBlockedEvent");
                return nullptr;
            }
            jmethodID pumpBlockedStructCtor = env->GetMethodID(pumpBlockedStructClass, "<init>", "()V");
            if (pumpBlockedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterPumpBlockedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(pumpBlockedStructClass, pumpBlockedStructCtor);

            return value;
        }
        case Events::SensorFailure::Id: {
            Events::SensorFailure::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass sensorFailureStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSensorFailureEvent",
                sensorFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSensorFailureEvent");
                return nullptr;
            }
            jmethodID sensorFailureStructCtor = env->GetMethodID(sensorFailureStructClass, "<init>", "()V");
            if (sensorFailureStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$PumpConfigurationAndControlClusterSensorFailureEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(sensorFailureStructClass, sensorFailureStructCtor);

            return value;
        }
        case Events::ElectronicNonFatalFailure::Id: {
            Events::ElectronicNonFatalFailure::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass electronicNonFatalFailureStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent",
                electronicNonFatalFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent");
                return nullptr;
            }
            jmethodID electronicNonFatalFailureStructCtor = env->GetMethodID(electronicNonFatalFailureStructClass, "<init>", "()V");
            if (electronicNonFatalFailureStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(electronicNonFatalFailureStructClass, electronicNonFatalFailureStructCtor);

            return value;
        }
        case Events::ElectronicFatalFailure::Id: {
            Events::ElectronicFatalFailure::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass electronicFatalFailureStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterElectronicFatalFailureEvent",
                electronicFatalFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterElectronicFatalFailureEvent");
                return nullptr;
            }
            jmethodID electronicFatalFailureStructCtor = env->GetMethodID(electronicFatalFailureStructClass, "<init>", "()V");
            if (electronicFatalFailureStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$PumpConfigurationAndControlClusterElectronicFatalFailureEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(electronicFatalFailureStructClass, electronicFatalFailureStructCtor);

            return value;
        }
        case Events::GeneralFault::Id: {
            Events::GeneralFault::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass generalFaultStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterGeneralFaultEvent",
                generalFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterGeneralFaultEvent");
                return nullptr;
            }
            jmethodID generalFaultStructCtor = env->GetMethodID(generalFaultStructClass, "<init>", "()V");
            if (generalFaultStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$PumpConfigurationAndControlClusterGeneralFaultEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(generalFaultStructClass, generalFaultStructCtor);

            return value;
        }
        case Events::Leakage::Id: {
            Events::Leakage::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass leakageStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterLeakageEvent", leakageStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterLeakageEvent");
                return nullptr;
            }
            jmethodID leakageStructCtor = env->GetMethodID(leakageStructClass, "<init>", "()V");
            if (leakageStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PumpConfigurationAndControlClusterLeakageEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(leakageStructClass, leakageStructCtor);

            return value;
        }
        case Events::AirDetection::Id: {
            Events::AirDetection::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass airDetectionStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterAirDetectionEvent",
                airDetectionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterAirDetectionEvent");
                return nullptr;
            }
            jmethodID airDetectionStructCtor = env->GetMethodID(airDetectionStructClass, "<init>", "()V");
            if (airDetectionStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$PumpConfigurationAndControlClusterAirDetectionEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(airDetectionStructClass, airDetectionStructCtor);

            return value;
        }
        case Events::TurbineOperation::Id: {
            Events::TurbineOperation::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass turbineOperationStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterTurbineOperationEvent",
                turbineOperationStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterTurbineOperationEvent");
                return nullptr;
            }
            jmethodID turbineOperationStructCtor = env->GetMethodID(turbineOperationStructClass, "<init>", "()V");
            if (turbineOperationStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$PumpConfigurationAndControlClusterTurbineOperationEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(turbineOperationStructClass, turbineOperationStructCtor);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Thermostat::Id: {
        using namespace app::Clusters::Thermostat;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::FanControl::Id: {
        using namespace app::Clusters::FanControl;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ThermostatUserInterfaceConfiguration::Id: {
        using namespace app::Clusters::ThermostatUserInterfaceConfiguration;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ColorControl::Id: {
        using namespace app::Clusters::ColorControl;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::BallastConfiguration::Id: {
        using namespace app::Clusters::BallastConfiguration;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::IlluminanceMeasurement::Id: {
        using namespace app::Clusters::IlluminanceMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TemperatureMeasurement::Id: {
        using namespace app::Clusters::TemperatureMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PressureMeasurement::Id: {
        using namespace app::Clusters::PressureMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::FlowMeasurement::Id: {
        using namespace app::Clusters::FlowMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RelativeHumidityMeasurement::Id: {
        using namespace app::Clusters::RelativeHumidityMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OccupancySensing::Id: {
        using namespace app::Clusters::OccupancySensing;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::CarbonMonoxideConcentrationMeasurement::Id: {
        using namespace app::Clusters::CarbonMonoxideConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::CarbonDioxideConcentrationMeasurement::Id: {
        using namespace app::Clusters::CarbonDioxideConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::NitrogenDioxideConcentrationMeasurement::Id: {
        using namespace app::Clusters::NitrogenDioxideConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OzoneConcentrationMeasurement::Id: {
        using namespace app::Clusters::OzoneConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Pm25ConcentrationMeasurement::Id: {
        using namespace app::Clusters::Pm25ConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::FormaldehydeConcentrationMeasurement::Id: {
        using namespace app::Clusters::FormaldehydeConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Pm1ConcentrationMeasurement::Id: {
        using namespace app::Clusters::Pm1ConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Pm10ConcentrationMeasurement::Id: {
        using namespace app::Clusters::Pm10ConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id: {
        using namespace app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::RadonConcentrationMeasurement::Id: {
        using namespace app::Clusters::RadonConcentrationMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
    case app::Clusters::TargetNavigator::Id: {
        using namespace app::Clusters::TargetNavigator;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::MediaPlayback::Id: {
        using namespace app::Clusters::MediaPlayback;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
    case app::Clusters::AccountLogin::Id: {
        using namespace app::Clusters::AccountLogin;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ElectricalMeasurement::Id: {
        using namespace app::Clusters::ElectricalMeasurement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::UnitTesting::Id: {
        using namespace app::Clusters::UnitTesting;
        switch (aPath.mEventId)
        {
        case Events::TestEvent::Id: {
            Events::TestEvent::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_arg1;
            std::string value_arg1ClassName     = "java/lang/Integer";
            std::string value_arg1CtorSignature = "(I)V";
            jint jnivalue_arg1                  = static_cast<jint>(cppValue.arg1);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_arg1ClassName.c_str(), value_arg1CtorSignature.c_str(),
                                                                       jnivalue_arg1, value_arg1);

            jobject value_arg2;
            std::string value_arg2ClassName     = "java/lang/Integer";
            std::string value_arg2CtorSignature = "(I)V";
            jint jnivalue_arg2                  = static_cast<jint>(cppValue.arg2);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_arg2ClassName.c_str(), value_arg2CtorSignature.c_str(),
                                                                       jnivalue_arg2, value_arg2);

            jobject value_arg3;
            std::string value_arg3ClassName     = "java/lang/Boolean";
            std::string value_arg3CtorSignature = "(Z)V";
            jboolean jnivalue_arg3              = static_cast<jboolean>(cppValue.arg3);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                value_arg3ClassName.c_str(), value_arg3CtorSignature.c_str(), jnivalue_arg3, value_arg3);

            jobject value_arg4;
            jobject value_arg4_a;
            std::string value_arg4_aClassName     = "java/lang/Integer";
            std::string value_arg4_aCtorSignature = "(I)V";
            jint jnivalue_arg4_a                  = static_cast<jint>(cppValue.arg4.a);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_arg4_aClassName.c_str(), value_arg4_aCtorSignature.c_str(), jnivalue_arg4_a, value_arg4_a);
            jobject value_arg4_b;
            std::string value_arg4_bClassName     = "java/lang/Boolean";
            std::string value_arg4_bCtorSignature = "(Z)V";
            jboolean jnivalue_arg4_b              = static_cast<jboolean>(cppValue.arg4.b);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                value_arg4_bClassName.c_str(), value_arg4_bCtorSignature.c_str(), jnivalue_arg4_b, value_arg4_b);
            jobject value_arg4_c;
            std::string value_arg4_cClassName     = "java/lang/Integer";
            std::string value_arg4_cCtorSignature = "(I)V";
            jint jnivalue_arg4_c                  = static_cast<jint>(cppValue.arg4.c);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_arg4_cClassName.c_str(), value_arg4_cCtorSignature.c_str(), jnivalue_arg4_c, value_arg4_c);
            jobject value_arg4_d;
            jbyteArray value_arg4_dByteArray = env->NewByteArray(static_cast<jsize>(cppValue.arg4.d.size()));
            env->SetByteArrayRegion(value_arg4_dByteArray, 0, static_cast<jsize>(cppValue.arg4.d.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.arg4.d.data()));
            value_arg4_d = value_arg4_dByteArray;
            jobject value_arg4_e;
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.arg4.e, value_arg4_e));
            jobject value_arg4_f;
            std::string value_arg4_fClassName     = "java/lang/Integer";
            std::string value_arg4_fCtorSignature = "(I)V";
            jint jnivalue_arg4_f                  = static_cast<jint>(cppValue.arg4.f.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_arg4_fClassName.c_str(), value_arg4_fCtorSignature.c_str(), jnivalue_arg4_f, value_arg4_f);
            jobject value_arg4_g;
            std::string value_arg4_gClassName     = "java/lang/Float";
            std::string value_arg4_gCtorSignature = "(F)V";
            jfloat jnivalue_arg4_g                = static_cast<jfloat>(cppValue.arg4.g);
            chip::JniReferences::GetInstance().CreateBoxedObject<jfloat>(
                value_arg4_gClassName.c_str(), value_arg4_gCtorSignature.c_str(), jnivalue_arg4_g, value_arg4_g);
            jobject value_arg4_h;
            std::string value_arg4_hClassName     = "java/lang/Double";
            std::string value_arg4_hCtorSignature = "(D)V";
            jdouble jnivalue_arg4_h               = static_cast<jdouble>(cppValue.arg4.h);
            chip::JniReferences::GetInstance().CreateBoxedObject<jdouble>(
                value_arg4_hClassName.c_str(), value_arg4_hCtorSignature.c_str(), jnivalue_arg4_h, value_arg4_h);

            jclass simpleStructStructClass_0;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$UnitTestingClusterSimpleStruct", simpleStructStructClass_0);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$UnitTestingClusterSimpleStruct");
                return nullptr;
            }
            jmethodID simpleStructStructCtor_0 =
                env->GetMethodID(simpleStructStructClass_0, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                 "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
            if (simpleStructStructCtor_0 == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$UnitTestingClusterSimpleStruct constructor");
                return nullptr;
            }

            value_arg4 = env->NewObject(simpleStructStructClass_0, simpleStructStructCtor_0, value_arg4_a, value_arg4_b,
                                        value_arg4_c, value_arg4_d, value_arg4_e, value_arg4_f, value_arg4_g, value_arg4_h);

            jobject value_arg5;
            chip::JniReferences::GetInstance().CreateArrayList(value_arg5);

            auto iter_value_arg5_0 = cppValue.arg5.begin();
            while (iter_value_arg5_0.Next())
            {
                auto & entry_0 = iter_value_arg5_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_a;
                std::string newElement_0_aClassName     = "java/lang/Integer";
                std::string newElement_0_aCtorSignature = "(I)V";
                jint jninewElement_0_a                  = static_cast<jint>(entry_0.a);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0_aClassName.c_str(), newElement_0_aCtorSignature.c_str(), jninewElement_0_a, newElement_0_a);
                jobject newElement_0_b;
                std::string newElement_0_bClassName     = "java/lang/Boolean";
                std::string newElement_0_bCtorSignature = "(Z)V";
                jboolean jninewElement_0_b              = static_cast<jboolean>(entry_0.b);
                chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                    newElement_0_bClassName.c_str(), newElement_0_bCtorSignature.c_str(), jninewElement_0_b, newElement_0_b);
                jobject newElement_0_c;
                std::string newElement_0_cClassName     = "java/lang/Integer";
                std::string newElement_0_cCtorSignature = "(I)V";
                jint jninewElement_0_c                  = static_cast<jint>(entry_0.c);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0_cClassName.c_str(), newElement_0_cCtorSignature.c_str(), jninewElement_0_c, newElement_0_c);
                jobject newElement_0_d;
                jbyteArray newElement_0_dByteArray = env->NewByteArray(static_cast<jsize>(entry_0.d.size()));
                env->SetByteArrayRegion(newElement_0_dByteArray, 0, static_cast<jsize>(entry_0.d.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.d.data()));
                newElement_0_d = newElement_0_dByteArray;
                jobject newElement_0_e;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.e, newElement_0_e));
                jobject newElement_0_f;
                std::string newElement_0_fClassName     = "java/lang/Integer";
                std::string newElement_0_fCtorSignature = "(I)V";
                jint jninewElement_0_f                  = static_cast<jint>(entry_0.f.Raw());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0_fClassName.c_str(), newElement_0_fCtorSignature.c_str(), jninewElement_0_f, newElement_0_f);
                jobject newElement_0_g;
                std::string newElement_0_gClassName     = "java/lang/Float";
                std::string newElement_0_gCtorSignature = "(F)V";
                jfloat jninewElement_0_g                = static_cast<jfloat>(entry_0.g);
                chip::JniReferences::GetInstance().CreateBoxedObject<jfloat>(
                    newElement_0_gClassName.c_str(), newElement_0_gCtorSignature.c_str(), jninewElement_0_g, newElement_0_g);
                jobject newElement_0_h;
                std::string newElement_0_hClassName     = "java/lang/Double";
                std::string newElement_0_hCtorSignature = "(D)V";
                jdouble jninewElement_0_h               = static_cast<jdouble>(entry_0.h);
                chip::JniReferences::GetInstance().CreateBoxedObject<jdouble>(
                    newElement_0_hClassName.c_str(), newElement_0_hCtorSignature.c_str(), jninewElement_0_h, newElement_0_h);

                jclass simpleStructStructClass_1;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$UnitTestingClusterSimpleStruct", simpleStructStructClass_1);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$UnitTestingClusterSimpleStruct");
                    return nullptr;
                }
                jmethodID simpleStructStructCtor_1 =
                    env->GetMethodID(simpleStructStructClass_1, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                     "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
                if (simpleStructStructCtor_1 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$UnitTestingClusterSimpleStruct constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(simpleStructStructClass_1, simpleStructStructCtor_1, newElement_0_a, newElement_0_b,
                                   newElement_0_c, newElement_0_d, newElement_0_e, newElement_0_f, newElement_0_g, newElement_0_h);
                chip::JniReferences::GetInstance().AddToList(value_arg5, newElement_0);
            }

            jobject value_arg6;
            chip::JniReferences::GetInstance().CreateArrayList(value_arg6);

            auto iter_value_arg6_0 = cppValue.arg6.begin();
            while (iter_value_arg6_0.Next())
            {
                auto & entry_0 = iter_value_arg6_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_arg6, newElement_0);
            }

            jclass testEventStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$UnitTestingClusterTestEventEvent", testEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$UnitTestingClusterTestEventEvent");
                return nullptr;
            }
            jmethodID testEventStructCtor =
                env->GetMethodID(testEventStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Boolean;Lchip/devicecontroller/"
                                 "ChipStructs$UnitTestingClusterSimpleStruct;Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (testEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$UnitTestingClusterTestEventEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(testEventStructClass, testEventStructCtor, value_arg1, value_arg2, value_arg3,
                                           value_arg4, value_arg5, value_arg6);

            return value;
        }
        case Events::TestFabricScopedEvent::Id: {
            Events::TestFabricScopedEvent::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass testFabricScopedEventStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$UnitTestingClusterTestFabricScopedEventEvent",
                testFabricScopedEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$UnitTestingClusterTestFabricScopedEventEvent");
                return nullptr;
            }
            jmethodID testFabricScopedEventStructCtor =
                env->GetMethodID(testFabricScopedEventStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (testFabricScopedEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$UnitTestingClusterTestFabricScopedEventEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(testFabricScopedEventStructClass, testFabricScopedEventStructCtor, value_fabricIndex);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::FaultInjection::Id: {
        using namespace app::Clusters::FaultInjection;
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
