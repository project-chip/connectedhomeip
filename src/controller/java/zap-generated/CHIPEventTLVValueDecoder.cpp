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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_adminNodeIDClassName.c_str(),
                                                                               value_adminNodeIDCtorSignature.c_str(),
                                                                               cppValue.adminNodeID.Value(), value_adminNodeID);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    value_adminPasscodeIDClassName.c_str(), value_adminPasscodeIDCtorSignature.c_str(),
                    cppValue.adminPasscodeID.Value(), value_adminPasscodeID);
            }

            jobject value_changeType;
            std::string value_changeTypeClassName     = "java/lang/Integer";
            std::string value_changeTypeCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_changeTypeClassName.c_str(), value_changeTypeCtorSignature.c_str(), static_cast<uint8_t>(cppValue.changeType),
                value_changeType);

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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_latestValue_privilegeClassName.c_str(), value_latestValue_privilegeCtorSignature.c_str(),
                    static_cast<uint8_t>(cppValue.latestValue.Value().privilege), value_latestValue_privilege);
                jobject value_latestValue_authMode;
                std::string value_latestValue_authModeClassName     = "java/lang/Integer";
                std::string value_latestValue_authModeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_latestValue_authModeClassName.c_str(), value_latestValue_authModeCtorSignature.c_str(),
                    static_cast<uint8_t>(cppValue.latestValue.Value().authMode), value_latestValue_authMode);
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
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(
                            newElement_3ClassName.c_str(), newElement_3CtorSignature.c_str(), entry_3, newElement_3);
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
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                                newElement_3_clusterClassName.c_str(), newElement_3_clusterCtorSignature.c_str(),
                                entry_3.cluster.Value(), newElement_3_cluster);
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
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                                newElement_3_endpointClassName.c_str(), newElement_3_endpointCtorSignature.c_str(),
                                entry_3.endpoint.Value(), newElement_3_endpoint);
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
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                                newElement_3_deviceTypeClassName.c_str(), newElement_3_deviceTypeCtorSignature.c_str(),
                                entry_3.deviceType.Value(), newElement_3_deviceType);
                        }

                        jclass targetStructClass_4;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$AccessControlClusterTarget", targetStructClass_4);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterTarget");
                            return nullptr;
                        }
                        jmethodID targetStructCtor_4 = env->GetMethodID(targetStructClass_4, "<init>",
                                                                        "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;)V");
                        if (targetStructCtor_4 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterTarget constructor");
                            return nullptr;
                        }

                        newElement_3 = env->NewObject(targetStructClass_4, targetStructCtor_4, newElement_3_cluster,
                                                      newElement_3_endpoint, newElement_3_deviceType);
                        chip::JniReferences::GetInstance().AddToList(value_latestValue_targets, newElement_3);
                    }
                }
                jobject value_latestValue_fabricIndex;
                std::string value_latestValue_fabricIndexClassName     = "java/lang/Integer";
                std::string value_latestValue_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_latestValue_fabricIndexClassName.c_str(), value_latestValue_fabricIndexCtorSignature.c_str(),
                    cppValue.latestValue.Value().fabricIndex, value_latestValue_fabricIndex);

                jclass accessControlEntryStructClass_1;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlEntry",
                    accessControlEntryStructClass_1);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlEntry");
                    return nullptr;
                }
                jmethodID accessControlEntryStructCtor_1 = env->GetMethodID(
                    accessControlEntryStructClass_1, "<init>",
                    "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/ArrayList;Ljava/util/ArrayList;Ljava/lang/Integer;)V");
                if (accessControlEntryStructCtor_1 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterAccessControlEntry constructor");
                    return nullptr;
                }

                value_latestValue =
                    env->NewObject(accessControlEntryStructClass_1, accessControlEntryStructCtor_1, value_latestValue_privilege,
                                   value_latestValue_authMode, value_latestValue_subjects, value_latestValue_targets,
                                   value_latestValue_fabricIndex);
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                          value_fabricIndexCtorSignature.c_str(),
                                                                          cppValue.fabricIndex, value_fabricIndex);

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
                                 "ChipStructs$AccessControlClusterAccessControlEntry;Ljava/lang/Integer;)V");
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_adminNodeIDClassName.c_str(),
                                                                               value_adminNodeIDCtorSignature.c_str(),
                                                                               cppValue.adminNodeID.Value(), value_adminNodeID);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    value_adminPasscodeIDClassName.c_str(), value_adminPasscodeIDCtorSignature.c_str(),
                    cppValue.adminPasscodeID.Value(), value_adminPasscodeID);
            }

            jobject value_changeType;
            std::string value_changeTypeClassName     = "java/lang/Integer";
            std::string value_changeTypeCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_changeTypeClassName.c_str(), value_changeTypeCtorSignature.c_str(), static_cast<uint8_t>(cppValue.changeType),
                value_changeType);

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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_latestValue_fabricIndexClassName.c_str(), value_latestValue_fabricIndexCtorSignature.c_str(),
                    cppValue.latestValue.Value().fabricIndex, value_latestValue_fabricIndex);

                jclass extensionEntryStructClass_1;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AccessControlClusterExtensionEntry", extensionEntryStructClass_1);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterExtensionEntry");
                    return nullptr;
                }
                jmethodID extensionEntryStructCtor_1 =
                    env->GetMethodID(extensionEntryStructClass_1, "<init>", "([BLjava/lang/Integer;)V");
                if (extensionEntryStructCtor_1 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterExtensionEntry constructor");
                    return nullptr;
                }

                value_latestValue = env->NewObject(extensionEntryStructClass_1, extensionEntryStructCtor_1, value_latestValue_data,
                                                   value_latestValue_fabricIndex);
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                          value_fabricIndexCtorSignature.c_str(),
                                                                          cppValue.fabricIndex, value_fabricIndex);

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
                                 "ChipStructs$AccessControlClusterExtensionEntry;Ljava/lang/Integer;)V");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                value_actionIDClassName.c_str(), value_actionIDCtorSignature.c_str(), cppValue.actionID, value_actionID);

            jobject value_invokeID;
            std::string value_invokeIDClassName     = "java/lang/Long";
            std::string value_invokeIDCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                value_invokeIDClassName.c_str(), value_invokeIDCtorSignature.c_str(), cppValue.invokeID, value_invokeID);

            jobject value_newState;
            std::string value_newStateClassName     = "java/lang/Integer";
            std::string value_newStateCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newStateClassName.c_str(),
                                                                          value_newStateCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue.newState), value_newState);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                value_actionIDClassName.c_str(), value_actionIDCtorSignature.c_str(), cppValue.actionID, value_actionID);

            jobject value_invokeID;
            std::string value_invokeIDClassName     = "java/lang/Long";
            std::string value_invokeIDCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                value_invokeIDClassName.c_str(), value_invokeIDCtorSignature.c_str(), cppValue.invokeID, value_invokeID);

            jobject value_newState;
            std::string value_newStateClassName     = "java/lang/Integer";
            std::string value_newStateCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newStateClassName.c_str(),
                                                                          value_newStateCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue.newState), value_newState);

            jobject value_error;
            std::string value_errorClassName     = "java/lang/Integer";
            std::string value_errorCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_errorClassName.c_str(), value_errorCtorSignature.c_str(), static_cast<uint8_t>(cppValue.error), value_error);

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
    case app::Clusters::Basic::Id: {
        using namespace app::Clusters::Basic;
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(value_softwareVersionClassName.c_str(),
                                                                           value_softwareVersionCtorSignature.c_str(),
                                                                           cppValue.softwareVersion, value_softwareVersion);

            jclass startUpStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicClusterStartUpEvent", startUpStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicClusterStartUpEvent");
                return nullptr;
            }
            jmethodID startUpStructCtor = env->GetMethodID(startUpStructClass, "<init>", "(Ljava/lang/Long;)V");
            if (startUpStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicClusterStartUpEvent constructor");
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
                env, "chip/devicecontroller/ChipEventStructs$BasicClusterShutDownEvent", shutDownStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicClusterShutDownEvent");
                return nullptr;
            }
            jmethodID shutDownStructCtor = env->GetMethodID(shutDownStructClass, "<init>", "()V");
            if (shutDownStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicClusterShutDownEvent constructor");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                          value_fabricIndexCtorSignature.c_str(),
                                                                          cppValue.fabricIndex, value_fabricIndex);

            jclass leaveStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicClusterLeaveEvent", leaveStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicClusterLeaveEvent");
                return nullptr;
            }
            jmethodID leaveStructCtor = env->GetMethodID(leaveStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (leaveStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicClusterLeaveEvent constructor");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(value_reachableNewValueClassName.c_str(),
                                                                       value_reachableNewValueCtorSignature.c_str(),
                                                                       cppValue.reachableNewValue, value_reachableNewValue);

            jclass reachableChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicClusterReachableChangedEvent", reachableChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicClusterReachableChangedEvent");
                return nullptr;
            }
            jmethodID reachableChangedStructCtor =
                env->GetMethodID(reachableChangedStructClass, "<init>", "(Ljava/lang/Boolean;)V");
            if (reachableChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BasicClusterReachableChangedEvent constructor");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_previousStateClassName.c_str(), value_previousStateCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.previousState), value_previousState);

            jobject value_newState;
            std::string value_newStateClassName     = "java/lang/Integer";
            std::string value_newStateCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newStateClassName.c_str(),
                                                                          value_newStateCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue.newState), value_newState);

            jobject value_reason;
            std::string value_reasonClassName     = "java/lang/Integer";
            std::string value_reasonCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_reasonClassName.c_str(),
                                                                          value_reasonCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue.reason), value_reason);

            jobject value_targetSoftwareVersion;
            if (cppValue.targetSoftwareVersion.IsNull())
            {
                value_targetSoftwareVersion = nullptr;
            }
            else
            {
                std::string value_targetSoftwareVersionClassName     = "java/lang/Long";
                std::string value_targetSoftwareVersionCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    value_targetSoftwareVersionClassName.c_str(), value_targetSoftwareVersionCtorSignature.c_str(),
                    cppValue.targetSoftwareVersion.Value(), value_targetSoftwareVersion);
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(value_softwareVersionClassName.c_str(),
                                                                           value_softwareVersionCtorSignature.c_str(),
                                                                           cppValue.softwareVersion, value_softwareVersion);

            jobject value_productID;
            std::string value_productIDClassName     = "java/lang/Integer";
            std::string value_productIDCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                value_productIDClassName.c_str(), value_productIDCtorSignature.c_str(), cppValue.productID, value_productID);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(value_softwareVersionClassName.c_str(),
                                                                           value_softwareVersionCtorSignature.c_str(),
                                                                           cppValue.softwareVersion, value_softwareVersion);

            jobject value_bytesDownloaded;
            std::string value_bytesDownloadedClassName     = "java/lang/Long";
            std::string value_bytesDownloadedCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_bytesDownloadedClassName.c_str(),
                                                                           value_bytesDownloadedCtorSignature.c_str(),
                                                                           cppValue.bytesDownloaded, value_bytesDownloaded);

            jobject value_progressPercent;
            if (cppValue.progressPercent.IsNull())
            {
                value_progressPercent = nullptr;
            }
            else
            {
                std::string value_progressPercentClassName     = "java/lang/Integer";
                std::string value_progressPercentCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_progressPercentClassName.c_str(), value_progressPercentCtorSignature.c_str(),
                    cppValue.progressPercent.Value(), value_progressPercent);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(value_platformCodeClassName.c_str(),
                                                                              value_platformCodeCtorSignature.c_str(),
                                                                              cppValue.platformCode.Value(), value_platformCode);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_bootReasonClassName.c_str(), value_bootReasonCtorSignature.c_str(), static_cast<uint8_t>(cppValue.bootReason),
                value_bootReason);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_idClassName.c_str(), value_idCtorSignature.c_str(),
                                                                           cppValue.id, value_id);

            jobject value_name;
            if (!cppValue.name.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_name);
            }
            else
            {
                jobject value_nameInsideOptional;
                value_nameInsideOptional =
                    env->NewStringUTF(std::string(cppValue.name.Value().data(), cppValue.name.Value().size()).c_str());
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_connectionStatusClassName.c_str(), value_connectionStatusCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.connectionStatus), value_connectionStatus);

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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                value_reasonCodeClassName.c_str(), value_reasonCodeCtorSignature.c_str(), cppValue.reasonCode, value_reasonCode);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_associationFailureClassName.c_str(), value_associationFailureCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.associationFailure), value_associationFailure);

            jobject value_status;
            std::string value_statusClassName     = "java/lang/Integer";
            std::string value_statusCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                value_statusClassName.c_str(), value_statusCtorSignature.c_str(), cppValue.status, value_status);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_connectionStatusClassName.c_str(), value_connectionStatusCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.connectionStatus), value_connectionStatus);

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
    case app::Clusters::BridgedDeviceBasic::Id: {
        using namespace app::Clusters::BridgedDeviceBasic;
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(value_softwareVersionClassName.c_str(),
                                                                           value_softwareVersionCtorSignature.c_str(),
                                                                           cppValue.softwareVersion, value_softwareVersion);

            jclass startUpStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicClusterStartUpEvent", startUpStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicClusterStartUpEvent");
                return nullptr;
            }
            jmethodID startUpStructCtor = env->GetMethodID(startUpStructClass, "<init>", "(Ljava/lang/Long;)V");
            if (startUpStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicClusterStartUpEvent constructor");
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
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicClusterShutDownEvent", shutDownStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicClusterShutDownEvent");
                return nullptr;
            }
            jmethodID shutDownStructCtor = env->GetMethodID(shutDownStructClass, "<init>", "()V");
            if (shutDownStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicClusterShutDownEvent constructor");
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
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicClusterLeaveEvent", leaveStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicClusterLeaveEvent");
                return nullptr;
            }
            jmethodID leaveStructCtor = env->GetMethodID(leaveStructClass, "<init>", "()V");
            if (leaveStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicClusterLeaveEvent constructor");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(value_reachableNewValueClassName.c_str(),
                                                                       value_reachableNewValueCtorSignature.c_str(),
                                                                       cppValue.reachableNewValue, value_reachableNewValue);

            jclass reachableChangedStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicClusterReachableChangedEvent",
                reachableChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicClusterReachableChangedEvent");
                return nullptr;
            }
            jmethodID reachableChangedStructCtor =
                env->GetMethodID(reachableChangedStructClass, "<init>", "(Ljava/lang/Boolean;)V");
            if (reachableChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicClusterReachableChangedEvent constructor");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newPositionClassName.c_str(),
                                                                          value_newPositionCtorSignature.c_str(),
                                                                          cppValue.newPosition, value_newPosition);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newPositionClassName.c_str(),
                                                                          value_newPositionCtorSignature.c_str(),
                                                                          cppValue.newPosition, value_newPosition);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newPositionClassName.c_str(),
                                                                          value_newPositionCtorSignature.c_str(),
                                                                          cppValue.newPosition, value_newPosition);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_previousPositionClassName.c_str(),
                                                                          value_previousPositionCtorSignature.c_str(),
                                                                          cppValue.previousPosition, value_previousPosition);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_previousPositionClassName.c_str(),
                                                                          value_previousPositionCtorSignature.c_str(),
                                                                          cppValue.previousPosition, value_previousPosition);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newPositionClassName.c_str(),
                                                                          value_newPositionCtorSignature.c_str(),
                                                                          cppValue.newPosition, value_newPosition);

            jobject value_currentNumberOfPressesCounted;
            std::string value_currentNumberOfPressesCountedClassName     = "java/lang/Integer";
            std::string value_currentNumberOfPressesCountedCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_currentNumberOfPressesCountedClassName.c_str(), value_currentNumberOfPressesCountedCtorSignature.c_str(),
                cppValue.currentNumberOfPressesCounted, value_currentNumberOfPressesCounted);

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
            jobject value_newPosition;
            std::string value_newPositionClassName     = "java/lang/Integer";
            std::string value_newPositionCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_newPositionClassName.c_str(),
                                                                          value_newPositionCtorSignature.c_str(),
                                                                          cppValue.newPosition, value_newPosition);

            jobject value_totalNumberOfPressesCounted;
            std::string value_totalNumberOfPressesCountedClassName     = "java/lang/Integer";
            std::string value_totalNumberOfPressesCountedCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_totalNumberOfPressesCountedClassName.c_str(), value_totalNumberOfPressesCountedCtorSignature.c_str(),
                cppValue.totalNumberOfPressesCounted, value_totalNumberOfPressesCounted);

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

            jobject value = env->NewObject(multiPressCompleteStructClass, multiPressCompleteStructCtor, value_newPosition,
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
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                value_stateValueClassName.c_str(), value_stateValueCtorSignature.c_str(), cppValue.stateValue, value_stateValue);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_alarmCodeClassName.c_str(), value_alarmCodeCtorSignature.c_str(), static_cast<uint8_t>(cppValue.alarmCode),
                value_alarmCode);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_doorStateClassName.c_str(), value_doorStateCtorSignature.c_str(), static_cast<uint8_t>(cppValue.doorState),
                value_doorState);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_lockOperationTypeClassName.c_str(), value_lockOperationTypeCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.lockOperationType), value_lockOperationType);

            jobject value_operationSource;
            std::string value_operationSourceClassName     = "java/lang/Integer";
            std::string value_operationSourceCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_operationSourceClassName.c_str(), value_operationSourceCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.operationSource), value_operationSource);

            jobject value_userIndex;
            if (cppValue.userIndex.IsNull())
            {
                value_userIndex = nullptr;
            }
            else
            {
                std::string value_userIndexClassName     = "java/lang/Integer";
                std::string value_userIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_userIndexClassName.c_str(),
                                                                               value_userIndexCtorSignature.c_str(),
                                                                               cppValue.userIndex.Value(), value_userIndex);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                              value_fabricIndexCtorSignature.c_str(),
                                                                              cppValue.fabricIndex.Value(), value_fabricIndex);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_sourceNodeClassName.c_str(),
                                                                               value_sourceNodeCtorSignature.c_str(),
                                                                               cppValue.sourceNode.Value(), value_sourceNode);
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
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_2_credentialTypeClassName.c_str(), newElement_2_credentialTypeCtorSignature.c_str(),
                            static_cast<uint8_t>(entry_2.credentialType), newElement_2_credentialType);
                        jobject newElement_2_credentialIndex;
                        std::string newElement_2_credentialIndexClassName     = "java/lang/Integer";
                        std::string newElement_2_credentialIndexCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                            newElement_2_credentialIndexClassName.c_str(), newElement_2_credentialIndexCtorSignature.c_str(),
                            entry_2.credentialIndex, newElement_2_credentialIndex);

                        jclass dlCredentialStructClass_3;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$DoorLockClusterDlCredential", dlCredentialStructClass_3);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$DoorLockClusterDlCredential");
                            return nullptr;
                        }
                        jmethodID dlCredentialStructCtor_3 =
                            env->GetMethodID(dlCredentialStructClass_3, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
                        if (dlCredentialStructCtor_3 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$DoorLockClusterDlCredential constructor");
                            return nullptr;
                        }

                        newElement_2 = env->NewObject(dlCredentialStructClass_3, dlCredentialStructCtor_3,
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_lockOperationTypeClassName.c_str(), value_lockOperationTypeCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.lockOperationType), value_lockOperationType);

            jobject value_operationSource;
            std::string value_operationSourceClassName     = "java/lang/Integer";
            std::string value_operationSourceCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_operationSourceClassName.c_str(), value_operationSourceCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.operationSource), value_operationSource);

            jobject value_operationError;
            std::string value_operationErrorClassName     = "java/lang/Integer";
            std::string value_operationErrorCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_operationErrorClassName.c_str(), value_operationErrorCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.operationError), value_operationError);

            jobject value_userIndex;
            if (cppValue.userIndex.IsNull())
            {
                value_userIndex = nullptr;
            }
            else
            {
                std::string value_userIndexClassName     = "java/lang/Integer";
                std::string value_userIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_userIndexClassName.c_str(),
                                                                               value_userIndexCtorSignature.c_str(),
                                                                               cppValue.userIndex.Value(), value_userIndex);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                              value_fabricIndexCtorSignature.c_str(),
                                                                              cppValue.fabricIndex.Value(), value_fabricIndex);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_sourceNodeClassName.c_str(),
                                                                               value_sourceNodeCtorSignature.c_str(),
                                                                               cppValue.sourceNode.Value(), value_sourceNode);
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
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_2_credentialTypeClassName.c_str(), newElement_2_credentialTypeCtorSignature.c_str(),
                            static_cast<uint8_t>(entry_2.credentialType), newElement_2_credentialType);
                        jobject newElement_2_credentialIndex;
                        std::string newElement_2_credentialIndexClassName     = "java/lang/Integer";
                        std::string newElement_2_credentialIndexCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                            newElement_2_credentialIndexClassName.c_str(), newElement_2_credentialIndexCtorSignature.c_str(),
                            entry_2.credentialIndex, newElement_2_credentialIndex);

                        jclass dlCredentialStructClass_3;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$DoorLockClusterDlCredential", dlCredentialStructClass_3);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$DoorLockClusterDlCredential");
                            return nullptr;
                        }
                        jmethodID dlCredentialStructCtor_3 =
                            env->GetMethodID(dlCredentialStructClass_3, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
                        if (dlCredentialStructCtor_3 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$DoorLockClusterDlCredential constructor");
                            return nullptr;
                        }

                        newElement_2 = env->NewObject(dlCredentialStructClass_3, dlCredentialStructCtor_3,
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_lockDataTypeClassName.c_str(), value_lockDataTypeCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.lockDataType), value_lockDataType);

            jobject value_dataOperationType;
            std::string value_dataOperationTypeClassName     = "java/lang/Integer";
            std::string value_dataOperationTypeCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_dataOperationTypeClassName.c_str(), value_dataOperationTypeCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.dataOperationType), value_dataOperationType);

            jobject value_operationSource;
            std::string value_operationSourceClassName     = "java/lang/Integer";
            std::string value_operationSourceCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_operationSourceClassName.c_str(), value_operationSourceCtorSignature.c_str(),
                static_cast<uint8_t>(cppValue.operationSource), value_operationSource);

            jobject value_userIndex;
            if (cppValue.userIndex.IsNull())
            {
                value_userIndex = nullptr;
            }
            else
            {
                std::string value_userIndexClassName     = "java/lang/Integer";
                std::string value_userIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_userIndexClassName.c_str(),
                                                                               value_userIndexCtorSignature.c_str(),
                                                                               cppValue.userIndex.Value(), value_userIndex);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                              value_fabricIndexCtorSignature.c_str(),
                                                                              cppValue.fabricIndex.Value(), value_fabricIndex);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_sourceNodeClassName.c_str(),
                                                                               value_sourceNodeCtorSignature.c_str(),
                                                                               cppValue.sourceNode.Value(), value_sourceNode);
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_dataIndexClassName.c_str(),
                                                                               value_dataIndexCtorSignature.c_str(),
                                                                               cppValue.dataIndex.Value(), value_dataIndex);
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
    case app::Clusters::TestCluster::Id: {
        using namespace app::Clusters::TestCluster;
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_arg1ClassName.c_str(), value_arg1CtorSignature.c_str(), cppValue.arg1, value_arg1);

            jobject value_arg2;
            std::string value_arg2ClassName     = "java/lang/Integer";
            std::string value_arg2CtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_arg2ClassName.c_str(), value_arg2CtorSignature.c_str(), static_cast<uint8_t>(cppValue.arg2), value_arg2);

            jobject value_arg3;
            std::string value_arg3ClassName     = "java/lang/Boolean";
            std::string value_arg3CtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(value_arg3ClassName.c_str(), value_arg3CtorSignature.c_str(),
                                                                       cppValue.arg3, value_arg3);

            jobject value_arg4;
            jobject value_arg4_a;
            std::string value_arg4_aClassName     = "java/lang/Integer";
            std::string value_arg4_aCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_arg4_aClassName.c_str(), value_arg4_aCtorSignature.c_str(), cppValue.arg4.a, value_arg4_a);
            jobject value_arg4_b;
            std::string value_arg4_bClassName     = "java/lang/Boolean";
            std::string value_arg4_bCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                value_arg4_bClassName.c_str(), value_arg4_bCtorSignature.c_str(), cppValue.arg4.b, value_arg4_b);
            jobject value_arg4_c;
            std::string value_arg4_cClassName     = "java/lang/Integer";
            std::string value_arg4_cCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_arg4_cClassName.c_str(),
                                                                          value_arg4_cCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue.arg4.c), value_arg4_c);
            jobject value_arg4_d;
            jbyteArray value_arg4_dByteArray = env->NewByteArray(static_cast<jsize>(cppValue.arg4.d.size()));
            env->SetByteArrayRegion(value_arg4_dByteArray, 0, static_cast<jsize>(cppValue.arg4.d.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.arg4.d.data()));
            value_arg4_d = value_arg4_dByteArray;
            jobject value_arg4_e;
            value_arg4_e = env->NewStringUTF(std::string(cppValue.arg4.e.data(), cppValue.arg4.e.size()).c_str());
            jobject value_arg4_f;
            std::string value_arg4_fClassName     = "java/lang/Integer";
            std::string value_arg4_fCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                value_arg4_fClassName.c_str(), value_arg4_fCtorSignature.c_str(), cppValue.arg4.f.Raw(), value_arg4_f);
            jobject value_arg4_g;
            std::string value_arg4_gClassName     = "java/lang/Float";
            std::string value_arg4_gCtorSignature = "(F)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<float>(
                value_arg4_gClassName.c_str(), value_arg4_gCtorSignature.c_str(), cppValue.arg4.g, value_arg4_g);
            jobject value_arg4_h;
            std::string value_arg4_hClassName     = "java/lang/Double";
            std::string value_arg4_hCtorSignature = "(D)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<double>(
                value_arg4_hClassName.c_str(), value_arg4_hCtorSignature.c_str(), cppValue.arg4.h, value_arg4_h);

            jclass simpleStructStructClass_0;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass_0);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                return nullptr;
            }
            jmethodID simpleStructStructCtor_0 =
                env->GetMethodID(simpleStructStructClass_0, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                 "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
            if (simpleStructStructCtor_0 == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_aClassName.c_str(), newElement_0_aCtorSignature.c_str(), entry_0.a, newElement_0_a);
                jobject newElement_0_b;
                std::string newElement_0_bClassName     = "java/lang/Boolean";
                std::string newElement_0_bCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_bClassName.c_str(), newElement_0_bCtorSignature.c_str(), entry_0.b, newElement_0_b);
                jobject newElement_0_c;
                std::string newElement_0_cClassName     = "java/lang/Integer";
                std::string newElement_0_cCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_cClassName.c_str(),
                                                                              newElement_0_cCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(entry_0.c), newElement_0_c);
                jobject newElement_0_d;
                jbyteArray newElement_0_dByteArray = env->NewByteArray(static_cast<jsize>(entry_0.d.size()));
                env->SetByteArrayRegion(newElement_0_dByteArray, 0, static_cast<jsize>(entry_0.d.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.d.data()));
                newElement_0_d = newElement_0_dByteArray;
                jobject newElement_0_e;
                newElement_0_e = env->NewStringUTF(std::string(entry_0.e.data(), entry_0.e.size()).c_str());
                jobject newElement_0_f;
                std::string newElement_0_fClassName     = "java/lang/Integer";
                std::string newElement_0_fCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_fClassName.c_str(), newElement_0_fCtorSignature.c_str(), entry_0.f.Raw(), newElement_0_f);
                jobject newElement_0_g;
                std::string newElement_0_gClassName     = "java/lang/Float";
                std::string newElement_0_gCtorSignature = "(F)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<float>(
                    newElement_0_gClassName.c_str(), newElement_0_gCtorSignature.c_str(), entry_0.g, newElement_0_g);
                jobject newElement_0_h;
                std::string newElement_0_hClassName     = "java/lang/Double";
                std::string newElement_0_hCtorSignature = "(D)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<double>(
                    newElement_0_hClassName.c_str(), newElement_0_hCtorSignature.c_str(), entry_0.h, newElement_0_h);

                jclass simpleStructStructClass_1;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass_1);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                    return nullptr;
                }
                jmethodID simpleStructStructCtor_1 =
                    env->GetMethodID(simpleStructStructClass_1, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                     "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
                if (simpleStructStructCtor_1 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
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
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_arg6, newElement_0);
            }

            jclass testEventStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TestClusterClusterTestEventEvent", testEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TestClusterClusterTestEventEvent");
                return nullptr;
            }
            jmethodID testEventStructCtor =
                env->GetMethodID(testEventStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Boolean;Lchip/devicecontroller/"
                                 "ChipStructs$TestClusterClusterSimpleStruct;Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
            if (testEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TestClusterClusterTestEventEvent constructor");
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
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fabricIndexClassName.c_str(),
                                                                          value_fabricIndexCtorSignature.c_str(),
                                                                          cppValue.fabricIndex, value_fabricIndex);

            jclass testFabricScopedEventStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TestClusterClusterTestFabricScopedEventEvent",
                testFabricScopedEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TestClusterClusterTestFabricScopedEventEvent");
                return nullptr;
            }
            jmethodID testFabricScopedEventStructCtor =
                env->GetMethodID(testFabricScopedEventStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (testFabricScopedEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$TestClusterClusterTestFabricScopedEventEvent constructor");
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
    default:
        *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
        break;
    }
    return nullptr;
}

} // namespace chip
