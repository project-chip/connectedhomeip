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

                        {
                            jclass accessControlTargetStructStructClass_4;
                            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                                env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlTargetStruct",
                                accessControlTargetStructStructClass_4);
                            if (err != CHIP_NO_ERROR)
                            {
                                ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlTargetStruct");
                                return nullptr;
                            }

                            jmethodID accessControlTargetStructStructCtor_4;
                            err = chip::JniReferences::GetInstance().FindMethod(
                                env, accessControlTargetStructStructClass_4, "<init>",
                                "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;)V", &accessControlTargetStructStructCtor_4);
                            if (err != CHIP_NO_ERROR || accessControlTargetStructStructCtor_4 == nullptr)
                            {
                                ChipLogError(
                                    Zcl, "Could not find ChipStructs$AccessControlClusterAccessControlTargetStruct constructor");
                                return nullptr;
                            }

                            newElement_3 =
                                env->NewObject(accessControlTargetStructStructClass_4, accessControlTargetStructStructCtor_4,
                                               newElement_3_cluster, newElement_3_endpoint, newElement_3_deviceType);
                        }
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

                {
                    jclass accessControlEntryStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlEntryStruct",
                        accessControlEntryStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlEntryStruct");
                        return nullptr;
                    }

                    jmethodID accessControlEntryStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, accessControlEntryStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/ArrayList;Ljava/util/ArrayList;Ljava/lang/Integer;)V",
                        &accessControlEntryStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || accessControlEntryStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterAccessControlEntryStruct constructor");
                        return nullptr;
                    }

                    value_latestValue =
                        env->NewObject(accessControlEntryStructStructClass_1, accessControlEntryStructStructCtor_1,
                                       value_latestValue_privilege, value_latestValue_authMode, value_latestValue_subjects,
                                       value_latestValue_targets, value_latestValue_fabricIndex);
                }
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass accessControlEntryChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccessControlClusterAccessControlEntryChangedEvent",
                accessControlEntryChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccessControlClusterAccessControlEntryChangedEvent");
                return nullptr;
            }

            jmethodID accessControlEntryChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, accessControlEntryChangedStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Lchip/devicecontroller/"
                "ChipStructs$AccessControlClusterAccessControlEntryStruct;Ljava/lang/Integer;)V",
                &accessControlEntryChangedStructCtor);
            if (err != CHIP_NO_ERROR || accessControlEntryChangedStructCtor == nullptr)
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

                {
                    jclass accessControlExtensionStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlExtensionStruct",
                        accessControlExtensionStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlExtensionStruct");
                        return nullptr;
                    }

                    jmethodID accessControlExtensionStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(env, accessControlExtensionStructStructClass_1, "<init>",
                                                                        "([BLjava/lang/Integer;)V",
                                                                        &accessControlExtensionStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || accessControlExtensionStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl,
                                     "Could not find ChipStructs$AccessControlClusterAccessControlExtensionStruct constructor");
                        return nullptr;
                    }

                    value_latestValue =
                        env->NewObject(accessControlExtensionStructStructClass_1, accessControlExtensionStructStructCtor_1,
                                       value_latestValue_data, value_latestValue_fabricIndex);
                }
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass accessControlExtensionChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccessControlClusterAccessControlExtensionChangedEvent",
                accessControlExtensionChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccessControlClusterAccessControlExtensionChangedEvent");
                return nullptr;
            }

            jmethodID accessControlExtensionChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, accessControlExtensionChangedStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Lchip/devicecontroller/"
                "ChipStructs$AccessControlClusterAccessControlExtensionStruct;Ljava/lang/Integer;)V",
                &accessControlExtensionChangedStructCtor);
            if (err != CHIP_NO_ERROR || accessControlExtensionChangedStructCtor == nullptr)
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
        case Events::FabricRestrictionReviewUpdate::Id: {
            Events::FabricRestrictionReviewUpdate::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_token;
            std::string value_tokenClassName     = "java/lang/Long";
            std::string value_tokenCtorSignature = "(J)V";
            jlong jnivalue_token                 = static_cast<jlong>(cppValue.token);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_tokenClassName.c_str(), value_tokenCtorSignature.c_str(), jnivalue_token, value_token);

            jobject value_instruction;
            if (!cppValue.instruction.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_instruction);
            }
            else
            {
                jobject value_instructionInsideOptional;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.instruction.Value(),
                                                                                     value_instructionInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_instructionInsideOptional, value_instruction);
            }

            jobject value_ARLRequestFlowUrl;
            if (!cppValue.ARLRequestFlowUrl.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_ARLRequestFlowUrl);
            }
            else
            {
                jobject value_ARLRequestFlowUrlInsideOptional;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.ARLRequestFlowUrl.Value(),
                                                                                     value_ARLRequestFlowUrlInsideOptional));
                chip::JniReferences::GetInstance().CreateOptional(value_ARLRequestFlowUrlInsideOptional, value_ARLRequestFlowUrl);
            }

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass fabricRestrictionReviewUpdateStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccessControlClusterFabricRestrictionReviewUpdateEvent",
                fabricRestrictionReviewUpdateStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccessControlClusterFabricRestrictionReviewUpdateEvent");
                return nullptr;
            }

            jmethodID fabricRestrictionReviewUpdateStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, fabricRestrictionReviewUpdateStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/lang/Integer;)V",
                &fabricRestrictionReviewUpdateStructCtor);
            if (err != CHIP_NO_ERROR || fabricRestrictionReviewUpdateStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$AccessControlClusterFabricRestrictionReviewUpdateEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(fabricRestrictionReviewUpdateStructClass, fabricRestrictionReviewUpdateStructCtor,
                                           value_token, value_instruction, value_ARLRequestFlowUrl, value_fabricIndex);

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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ActionsClusterStateChangedEvent", stateChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ActionsClusterStateChangedEvent");
                return nullptr;
            }

            jmethodID stateChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, stateChangedStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Integer;)V",
                                                                &stateChangedStructCtor);
            if (err != CHIP_NO_ERROR || stateChangedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ActionsClusterActionFailedEvent", actionFailedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ActionsClusterActionFailedEvent");
                return nullptr;
            }

            jmethodID actionFailedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, actionFailedStructClass, "<init>",
                "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;)V", &actionFailedStructCtor);
            if (err != CHIP_NO_ERROR || actionFailedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterStartUpEvent", startUpStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterStartUpEvent");
                return nullptr;
            }

            jmethodID startUpStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, startUpStructClass, "<init>", "(Ljava/lang/Long;)V",
                                                                &startUpStructCtor);
            if (err != CHIP_NO_ERROR || startUpStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterShutDownEvent", shutDownStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterShutDownEvent");
                return nullptr;
            }

            jmethodID shutDownStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, shutDownStructClass, "<init>", "()V", &shutDownStructCtor);
            if (err != CHIP_NO_ERROR || shutDownStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterLeaveEvent", leaveStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterLeaveEvent");
                return nullptr;
            }

            jmethodID leaveStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, leaveStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &leaveStructCtor);
            if (err != CHIP_NO_ERROR || leaveStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BasicInformationClusterReachableChangedEvent",
                reachableChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BasicInformationClusterReachableChangedEvent");
                return nullptr;
            }

            jmethodID reachableChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, reachableChangedStructClass, "<init>",
                                                                "(Ljava/lang/Boolean;)V", &reachableChangedStructCtor);
            if (err != CHIP_NO_ERROR || reachableChangedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OtaSoftwareUpdateRequestorClusterStateTransitionEvent",
                stateTransitionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OtaSoftwareUpdateRequestorClusterStateTransitionEvent");
                return nullptr;
            }

            jmethodID stateTransitionStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, stateTransitionStructClass, "<init>",
                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Long;)V", &stateTransitionStructCtor);
            if (err != CHIP_NO_ERROR || stateTransitionStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OtaSoftwareUpdateRequestorClusterVersionAppliedEvent",
                versionAppliedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OtaSoftwareUpdateRequestorClusterVersionAppliedEvent");
                return nullptr;
            }

            jmethodID versionAppliedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, versionAppliedStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Integer;)V", &versionAppliedStructCtor);
            if (err != CHIP_NO_ERROR || versionAppliedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OtaSoftwareUpdateRequestorClusterDownloadErrorEvent",
                downloadErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OtaSoftwareUpdateRequestorClusterDownloadErrorEvent");
                return nullptr;
            }

            jmethodID downloadErrorStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, downloadErrorStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;)V",
                &downloadErrorStructCtor);
            if (err != CHIP_NO_ERROR || downloadErrorStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PowerSourceClusterWiredFaultChangeEvent", wiredFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PowerSourceClusterWiredFaultChangeEvent");
                return nullptr;
            }

            jmethodID wiredFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, wiredFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &wiredFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || wiredFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PowerSourceClusterBatFaultChangeEvent", batFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PowerSourceClusterBatFaultChangeEvent");
                return nullptr;
            }

            jmethodID batFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, batFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &batFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || batFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PowerSourceClusterBatChargeFaultChangeEvent",
                batChargeFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PowerSourceClusterBatChargeFaultChangeEvent");
                return nullptr;
            }

            jmethodID batChargeFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, batChargeFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &batChargeFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || batChargeFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterHardwareFaultChangeEvent",
                hardwareFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterHardwareFaultChangeEvent");
                return nullptr;
            }

            jmethodID hardwareFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, hardwareFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &hardwareFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || hardwareFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterRadioFaultChangeEvent",
                radioFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterRadioFaultChangeEvent");
                return nullptr;
            }

            jmethodID radioFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, radioFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &radioFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || radioFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterNetworkFaultChangeEvent",
                networkFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterNetworkFaultChangeEvent");
                return nullptr;
            }

            jmethodID networkFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, networkFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &networkFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || networkFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$GeneralDiagnosticsClusterBootReasonEvent", bootReasonStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$GeneralDiagnosticsClusterBootReasonEvent");
                return nullptr;
            }

            jmethodID bootReasonStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, bootReasonStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &bootReasonStructCtor);
            if (err != CHIP_NO_ERROR || bootReasonStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SoftwareDiagnosticsClusterSoftwareFaultEvent",
                softwareFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SoftwareDiagnosticsClusterSoftwareFaultEvent");
                return nullptr;
            }

            jmethodID softwareFaultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, softwareFaultStructClass, "<init>",
                                                                "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                &softwareFaultStructCtor);
            if (err != CHIP_NO_ERROR || softwareFaultStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ThreadNetworkDiagnosticsClusterConnectionStatusEvent",
                connectionStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ThreadNetworkDiagnosticsClusterConnectionStatusEvent");
                return nullptr;
            }

            jmethodID connectionStatusStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, connectionStatusStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &connectionStatusStructCtor);
            if (err != CHIP_NO_ERROR || connectionStatusStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent",
                networkFaultChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent");
                return nullptr;
            }

            jmethodID networkFaultChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, networkFaultChangeStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                                                                &networkFaultChangeStructCtor);
            if (err != CHIP_NO_ERROR || networkFaultChangeStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WiFiNetworkDiagnosticsClusterDisconnectionEvent",
                disconnectionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WiFiNetworkDiagnosticsClusterDisconnectionEvent");
                return nullptr;
            }

            jmethodID disconnectionStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, disconnectionStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &disconnectionStructCtor);
            if (err != CHIP_NO_ERROR || disconnectionStructCtor == nullptr)
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
            jobject value_associationFailureCause;
            std::string value_associationFailureCauseClassName     = "java/lang/Integer";
            std::string value_associationFailureCauseCtorSignature = "(I)V";
            jint jnivalue_associationFailureCause                  = static_cast<jint>(cppValue.associationFailureCause);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_associationFailureCauseClassName.c_str(), value_associationFailureCauseCtorSignature.c_str(),
                jnivalue_associationFailureCause, value_associationFailureCause);

            jobject value_status;
            std::string value_statusClassName     = "java/lang/Integer";
            std::string value_statusCtorSignature = "(I)V";
            jint jnivalue_status                  = static_cast<jint>(cppValue.status);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_statusClassName.c_str(), value_statusCtorSignature.c_str(), jnivalue_status, value_status);

            jclass associationFailureStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WiFiNetworkDiagnosticsClusterAssociationFailureEvent",
                associationFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WiFiNetworkDiagnosticsClusterAssociationFailureEvent");
                return nullptr;
            }

            jmethodID associationFailureStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, associationFailureStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;)V",
                                                                &associationFailureStructCtor);
            if (err != CHIP_NO_ERROR || associationFailureStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$WiFiNetworkDiagnosticsClusterAssociationFailureEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(associationFailureStructClass, associationFailureStructCtor,
                                           value_associationFailureCause, value_status);

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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WiFiNetworkDiagnosticsClusterConnectionStatusEvent",
                connectionStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WiFiNetworkDiagnosticsClusterConnectionStatusEvent");
                return nullptr;
            }

            jmethodID connectionStatusStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, connectionStatusStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &connectionStatusStructCtor);
            if (err != CHIP_NO_ERROR || connectionStatusStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterDSTTableEmptyEvent",
                DSTTableEmptyStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterDSTTableEmptyEvent");
                return nullptr;
            }

            jmethodID DSTTableEmptyStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, DSTTableEmptyStructClass, "<init>", "()V",
                                                                &DSTTableEmptyStructCtor);
            if (err != CHIP_NO_ERROR || DSTTableEmptyStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterDSTStatusEvent", DSTStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterDSTStatusEvent");
                return nullptr;
            }

            jmethodID DSTStatusStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, DSTStatusStructClass, "<init>", "(Ljava/lang/Boolean;)V",
                                                                &DSTStatusStructCtor);
            if (err != CHIP_NO_ERROR || DSTStatusStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterTimeZoneStatusEvent",
                timeZoneStatusStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterTimeZoneStatusEvent");
                return nullptr;
            }

            jmethodID timeZoneStatusStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, timeZoneStatusStructClass, "<init>", "(Ljava/lang/Long;Ljava/util/Optional;)V", &timeZoneStatusStructCtor);
            if (err != CHIP_NO_ERROR || timeZoneStatusStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterTimeFailureEvent", timeFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterTimeFailureEvent");
                return nullptr;
            }

            jmethodID timeFailureStructCtor;
            err =
                chip::JniReferences::GetInstance().FindMethod(env, timeFailureStructClass, "<init>", "()V", &timeFailureStructCtor);
            if (err != CHIP_NO_ERROR || timeFailureStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$TimeSynchronizationClusterMissingTrustedTimeSourceEvent",
                missingTrustedTimeSourceStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$TimeSynchronizationClusterMissingTrustedTimeSourceEvent");
                return nullptr;
            }

            jmethodID missingTrustedTimeSourceStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, missingTrustedTimeSourceStructClass, "<init>", "()V",
                                                                &missingTrustedTimeSourceStructCtor);
            if (err != CHIP_NO_ERROR || missingTrustedTimeSourceStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterStartUpEvent", startUpStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterStartUpEvent");
                return nullptr;
            }

            jmethodID startUpStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, startUpStructClass, "<init>", "(Ljava/lang/Long;)V",
                                                                &startUpStructCtor);
            if (err != CHIP_NO_ERROR || startUpStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterShutDownEvent",
                shutDownStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterShutDownEvent");
                return nullptr;
            }

            jmethodID shutDownStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, shutDownStructClass, "<init>", "()V", &shutDownStructCtor);
            if (err != CHIP_NO_ERROR || shutDownStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterLeaveEvent", leaveStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterLeaveEvent");
                return nullptr;
            }

            jmethodID leaveStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, leaveStructClass, "<init>", "()V", &leaveStructCtor);
            if (err != CHIP_NO_ERROR || leaveStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterReachableChangedEvent",
                reachableChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterReachableChangedEvent");
                return nullptr;
            }

            jmethodID reachableChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, reachableChangedStructClass, "<init>",
                                                                "(Ljava/lang/Boolean;)V", &reachableChangedStructCtor);
            if (err != CHIP_NO_ERROR || reachableChangedStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$BridgedDeviceBasicInformationClusterReachableChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(reachableChangedStructClass, reachableChangedStructCtor, value_reachableNewValue);

            return value;
        }
        case Events::ActiveChanged::Id: {
            Events::ActiveChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_promisedActiveDuration;
            std::string value_promisedActiveDurationClassName     = "java/lang/Long";
            std::string value_promisedActiveDurationCtorSignature = "(J)V";
            jlong jnivalue_promisedActiveDuration                 = static_cast<jlong>(cppValue.promisedActiveDuration);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_promisedActiveDurationClassName.c_str(), value_promisedActiveDurationCtorSignature.c_str(),
                jnivalue_promisedActiveDuration, value_promisedActiveDuration);

            jclass activeChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BridgedDeviceBasicInformationClusterActiveChangedEvent",
                activeChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BridgedDeviceBasicInformationClusterActiveChangedEvent");
                return nullptr;
            }

            jmethodID activeChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, activeChangedStructClass, "<init>", "(Ljava/lang/Long;)V",
                                                                &activeChangedStructCtor);
            if (err != CHIP_NO_ERROR || activeChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$BridgedDeviceBasicInformationClusterActiveChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(activeChangedStructClass, activeChangedStructCtor, value_promisedActiveDuration);

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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterSwitchLatchedEvent", switchLatchedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterSwitchLatchedEvent");
                return nullptr;
            }

            jmethodID switchLatchedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, switchLatchedStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &switchLatchedStructCtor);
            if (err != CHIP_NO_ERROR || switchLatchedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterInitialPressEvent", initialPressStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterInitialPressEvent");
                return nullptr;
            }

            jmethodID initialPressStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, initialPressStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &initialPressStructCtor);
            if (err != CHIP_NO_ERROR || initialPressStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterLongPressEvent", longPressStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterLongPressEvent");
                return nullptr;
            }

            jmethodID longPressStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, longPressStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &longPressStructCtor);
            if (err != CHIP_NO_ERROR || longPressStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterShortReleaseEvent", shortReleaseStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterShortReleaseEvent");
                return nullptr;
            }

            jmethodID shortReleaseStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, shortReleaseStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &shortReleaseStructCtor);
            if (err != CHIP_NO_ERROR || shortReleaseStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterLongReleaseEvent", longReleaseStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterLongReleaseEvent");
                return nullptr;
            }

            jmethodID longReleaseStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, longReleaseStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &longReleaseStructCtor);
            if (err != CHIP_NO_ERROR || longReleaseStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterMultiPressOngoingEvent", multiPressOngoingStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterMultiPressOngoingEvent");
                return nullptr;
            }

            jmethodID multiPressOngoingStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, multiPressOngoingStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;)V",
                                                                &multiPressOngoingStructCtor);
            if (err != CHIP_NO_ERROR || multiPressOngoingStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SwitchClusterMultiPressCompleteEvent", multiPressCompleteStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SwitchClusterMultiPressCompleteEvent");
                return nullptr;
            }

            jmethodID multiPressCompleteStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, multiPressCompleteStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;)V",
                                                                &multiPressCompleteStructCtor);
            if (err != CHIP_NO_ERROR || multiPressCompleteStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BooleanStateClusterStateChangeEvent", stateChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BooleanStateClusterStateChangeEvent");
                return nullptr;
            }

            jmethodID stateChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, stateChangeStructClass, "<init>", "(Ljava/lang/Boolean;)V",
                                                                &stateChangeStructCtor);
            if (err != CHIP_NO_ERROR || stateChangeStructCtor == nullptr)
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
    case app::Clusters::Timer::Id: {
        using namespace app::Clusters::Timer;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::OvenCavityOperationalState::Id: {
        using namespace app::Clusters::OvenCavityOperationalState;
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

            {
                jclass errorStateStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$OvenCavityOperationalStateClusterErrorStateStruct",
                    errorStateStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$OvenCavityOperationalStateClusterErrorStateStruct");
                    return nullptr;
                }

                jmethodID errorStateStructStructCtor_0;
                err = chip::JniReferences::GetInstance().FindMethod(
                    env, errorStateStructStructClass_0, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V",
                    &errorStateStructStructCtor_0);
                if (err != CHIP_NO_ERROR || errorStateStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$OvenCavityOperationalStateClusterErrorStateStruct constructor");
                    return nullptr;
                }

                value_errorState =
                    env->NewObject(errorStateStructStructClass_0, errorStateStructStructCtor_0, value_errorState_errorStateID,
                                   value_errorState_errorStateLabel, value_errorState_errorStateDetails);
            }

            jclass operationalErrorStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OvenCavityOperationalStateClusterOperationalErrorEvent",
                operationalErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OvenCavityOperationalStateClusterOperationalErrorEvent");
                return nullptr;
            }

            jmethodID operationalErrorStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, operationalErrorStructClass, "<init>",
                "(Lchip/devicecontroller/ChipStructs$OvenCavityOperationalStateClusterErrorStateStruct;)V",
                &operationalErrorStructCtor);
            if (err != CHIP_NO_ERROR || operationalErrorStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$OvenCavityOperationalStateClusterOperationalErrorEvent constructor");
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OvenCavityOperationalStateClusterOperationCompletionEvent",
                operationCompletionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$OvenCavityOperationalStateClusterOperationCompletionEvent");
                return nullptr;
            }

            jmethodID operationCompletionStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, operationCompletionStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                &operationCompletionStructCtor);
            if (err != CHIP_NO_ERROR || operationCompletionStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$OvenCavityOperationalStateClusterOperationCompletionEvent constructor");
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
    case app::Clusters::OvenMode::Id: {
        using namespace app::Clusters::OvenMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::LaundryDryerControls::Id: {
        using namespace app::Clusters::LaundryDryerControls;
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$RefrigeratorAlarmClusterNotifyEvent", notifyStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$RefrigeratorAlarmClusterNotifyEvent");
                return nullptr;
            }

            jmethodID notifyStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, notifyStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;)V",
                &notifyStructCtor);
            if (err != CHIP_NO_ERROR || notifyStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterSmokeAlarmEvent", smokeAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterSmokeAlarmEvent");
                return nullptr;
            }

            jmethodID smokeAlarmStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, smokeAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &smokeAlarmStructCtor);
            if (err != CHIP_NO_ERROR || smokeAlarmStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterCOAlarmEvent", COAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterCOAlarmEvent");
                return nullptr;
            }

            jmethodID COAlarmStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, COAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &COAlarmStructCtor);
            if (err != CHIP_NO_ERROR || COAlarmStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterLowBatteryEvent", lowBatteryStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterLowBatteryEvent");
                return nullptr;
            }

            jmethodID lowBatteryStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, lowBatteryStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &lowBatteryStructCtor);
            if (err != CHIP_NO_ERROR || lowBatteryStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterHardwareFaultEvent", hardwareFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterHardwareFaultEvent");
                return nullptr;
            }

            jmethodID hardwareFaultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, hardwareFaultStructClass, "<init>", "()V",
                                                                &hardwareFaultStructCtor);
            if (err != CHIP_NO_ERROR || hardwareFaultStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterEndOfServiceEvent", endOfServiceStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterEndOfServiceEvent");
                return nullptr;
            }

            jmethodID endOfServiceStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, endOfServiceStructClass, "<init>", "()V",
                                                                &endOfServiceStructCtor);
            if (err != CHIP_NO_ERROR || endOfServiceStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterSelfTestCompleteEvent",
                selfTestCompleteStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterSelfTestCompleteEvent");
                return nullptr;
            }

            jmethodID selfTestCompleteStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, selfTestCompleteStructClass, "<init>", "()V",
                                                                &selfTestCompleteStructCtor);
            if (err != CHIP_NO_ERROR || selfTestCompleteStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterAlarmMutedEvent", alarmMutedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterAlarmMutedEvent");
                return nullptr;
            }

            jmethodID alarmMutedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, alarmMutedStructClass, "<init>", "()V", &alarmMutedStructCtor);
            if (err != CHIP_NO_ERROR || alarmMutedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterMuteEndedEvent", muteEndedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterMuteEndedEvent");
                return nullptr;
            }

            jmethodID muteEndedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, muteEndedStructClass, "<init>", "()V", &muteEndedStructCtor);
            if (err != CHIP_NO_ERROR || muteEndedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterInterconnectSmokeAlarmEvent",
                interconnectSmokeAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterInterconnectSmokeAlarmEvent");
                return nullptr;
            }

            jmethodID interconnectSmokeAlarmStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, interconnectSmokeAlarmStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &interconnectSmokeAlarmStructCtor);
            if (err != CHIP_NO_ERROR || interconnectSmokeAlarmStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterInterconnectCOAlarmEvent",
                interconnectCOAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterInterconnectCOAlarmEvent");
                return nullptr;
            }

            jmethodID interconnectCOAlarmStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, interconnectCOAlarmStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &interconnectCOAlarmStructCtor);
            if (err != CHIP_NO_ERROR || interconnectCOAlarmStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SmokeCoAlarmClusterAllClearEvent", allClearStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SmokeCoAlarmClusterAllClearEvent");
                return nullptr;
            }

            jmethodID allClearStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, allClearStructClass, "<init>", "()V", &allClearStructCtor);
            if (err != CHIP_NO_ERROR || allClearStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DishwasherAlarmClusterNotifyEvent", notifyStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DishwasherAlarmClusterNotifyEvent");
                return nullptr;
            }

            jmethodID notifyStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, notifyStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;)V",
                &notifyStructCtor);
            if (err != CHIP_NO_ERROR || notifyStructCtor == nullptr)
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
    case app::Clusters::MicrowaveOvenMode::Id: {
        using namespace app::Clusters::MicrowaveOvenMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::MicrowaveOvenControl::Id: {
        using namespace app::Clusters::MicrowaveOvenControl;
        switch (aPath.mEventId)
        {
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

            {
                jclass errorStateStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$OperationalStateClusterErrorStateStruct",
                    errorStateStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$OperationalStateClusterErrorStateStruct");
                    return nullptr;
                }

                jmethodID errorStateStructStructCtor_0;
                err = chip::JniReferences::GetInstance().FindMethod(
                    env, errorStateStructStructClass_0, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V",
                    &errorStateStructStructCtor_0);
                if (err != CHIP_NO_ERROR || errorStateStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$OperationalStateClusterErrorStateStruct constructor");
                    return nullptr;
                }

                value_errorState =
                    env->NewObject(errorStateStructStructClass_0, errorStateStructStructCtor_0, value_errorState_errorStateID,
                                   value_errorState_errorStateLabel, value_errorState_errorStateDetails);
            }

            jclass operationalErrorStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OperationalStateClusterOperationalErrorEvent",
                operationalErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OperationalStateClusterOperationalErrorEvent");
                return nullptr;
            }

            jmethodID operationalErrorStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, operationalErrorStructClass, "<init>",
                "(Lchip/devicecontroller/ChipStructs$OperationalStateClusterErrorStateStruct;)V", &operationalErrorStructCtor);
            if (err != CHIP_NO_ERROR || operationalErrorStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OperationalStateClusterOperationCompletionEvent",
                operationCompletionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OperationalStateClusterOperationCompletionEvent");
                return nullptr;
            }

            jmethodID operationCompletionStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, operationCompletionStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                &operationCompletionStructCtor);
            if (err != CHIP_NO_ERROR || operationCompletionStructCtor == nullptr)
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

            {
                jclass errorStateStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$RvcOperationalStateClusterErrorStateStruct",
                    errorStateStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$RvcOperationalStateClusterErrorStateStruct");
                    return nullptr;
                }

                jmethodID errorStateStructStructCtor_0;
                err = chip::JniReferences::GetInstance().FindMethod(
                    env, errorStateStructStructClass_0, "<init>", "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V",
                    &errorStateStructStructCtor_0);
                if (err != CHIP_NO_ERROR || errorStateStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$RvcOperationalStateClusterErrorStateStruct constructor");
                    return nullptr;
                }

                value_errorState =
                    env->NewObject(errorStateStructStructClass_0, errorStateStructStructCtor_0, value_errorState_errorStateID,
                                   value_errorState_errorStateLabel, value_errorState_errorStateDetails);
            }

            jclass operationalErrorStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$RvcOperationalStateClusterOperationalErrorEvent",
                operationalErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$RvcOperationalStateClusterOperationalErrorEvent");
                return nullptr;
            }

            jmethodID operationalErrorStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, operationalErrorStructClass, "<init>",
                "(Lchip/devicecontroller/ChipStructs$RvcOperationalStateClusterErrorStateStruct;)V", &operationalErrorStructCtor);
            if (err != CHIP_NO_ERROR || operationalErrorStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$RvcOperationalStateClusterOperationCompletionEvent",
                operationCompletionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$RvcOperationalStateClusterOperationCompletionEvent");
                return nullptr;
            }

            jmethodID operationCompletionStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, operationCompletionStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                &operationCompletionStructCtor);
            if (err != CHIP_NO_ERROR || operationCompletionStructCtor == nullptr)
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
    case app::Clusters::ScenesManagement::Id: {
        using namespace app::Clusters::ScenesManagement;
        switch (aPath.mEventId)
        {
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
    case app::Clusters::BooleanStateConfiguration::Id: {
        using namespace app::Clusters::BooleanStateConfiguration;
        switch (aPath.mEventId)
        {
        case Events::AlarmsStateChanged::Id: {
            Events::AlarmsStateChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_alarmsActive;
            std::string value_alarmsActiveClassName     = "java/lang/Integer";
            std::string value_alarmsActiveCtorSignature = "(I)V";
            jint jnivalue_alarmsActive                  = static_cast<jint>(cppValue.alarmsActive.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_alarmsActiveClassName.c_str(),
                                                                       value_alarmsActiveCtorSignature.c_str(),
                                                                       jnivalue_alarmsActive, value_alarmsActive);

            jobject value_alarmsSuppressed;
            if (!cppValue.alarmsSuppressed.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_alarmsSuppressed);
            }
            else
            {
                jobject value_alarmsSuppressedInsideOptional;
                std::string value_alarmsSuppressedInsideOptionalClassName     = "java/lang/Integer";
                std::string value_alarmsSuppressedInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_alarmsSuppressedInsideOptional = static_cast<jint>(cppValue.alarmsSuppressed.Value().Raw());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_alarmsSuppressedInsideOptionalClassName.c_str(),
                    value_alarmsSuppressedInsideOptionalCtorSignature.c_str(), jnivalue_alarmsSuppressedInsideOptional,
                    value_alarmsSuppressedInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_alarmsSuppressedInsideOptional, value_alarmsSuppressed);
            }

            jclass alarmsStateChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BooleanStateConfigurationClusterAlarmsStateChangedEvent",
                alarmsStateChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BooleanStateConfigurationClusterAlarmsStateChangedEvent");
                return nullptr;
            }

            jmethodID alarmsStateChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, alarmsStateChangedStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/util/Optional;)V",
                                                                &alarmsStateChangedStructCtor);
            if (err != CHIP_NO_ERROR || alarmsStateChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$BooleanStateConfigurationClusterAlarmsStateChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(alarmsStateChangedStructClass, alarmsStateChangedStructCtor, value_alarmsActive,
                                           value_alarmsSuppressed);

            return value;
        }
        case Events::SensorFault::Id: {
            Events::SensorFault::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_sensorFault;
            std::string value_sensorFaultClassName     = "java/lang/Integer";
            std::string value_sensorFaultCtorSignature = "(I)V";
            jint jnivalue_sensorFault                  = static_cast<jint>(cppValue.sensorFault.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_sensorFaultClassName.c_str(),
                                                                       value_sensorFaultCtorSignature.c_str(), jnivalue_sensorFault,
                                                                       value_sensorFault);

            jclass sensorFaultStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$BooleanStateConfigurationClusterSensorFaultEvent",
                sensorFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$BooleanStateConfigurationClusterSensorFaultEvent");
                return nullptr;
            }

            jmethodID sensorFaultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, sensorFaultStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &sensorFaultStructCtor);
            if (err != CHIP_NO_ERROR || sensorFaultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$BooleanStateConfigurationClusterSensorFaultEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(sensorFaultStructClass, sensorFaultStructCtor, value_sensorFault);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ValveConfigurationAndControl::Id: {
        using namespace app::Clusters::ValveConfigurationAndControl;
        switch (aPath.mEventId)
        {
        case Events::ValveStateChanged::Id: {
            Events::ValveStateChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_valveState;
            std::string value_valveStateClassName     = "java/lang/Integer";
            std::string value_valveStateCtorSignature = "(I)V";
            jint jnivalue_valveState                  = static_cast<jint>(cppValue.valveState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_valveStateClassName.c_str(), value_valveStateCtorSignature.c_str(), jnivalue_valveState, value_valveState);

            jobject value_valveLevel;
            if (!cppValue.valveLevel.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_valveLevel);
            }
            else
            {
                jobject value_valveLevelInsideOptional;
                std::string value_valveLevelInsideOptionalClassName     = "java/lang/Integer";
                std::string value_valveLevelInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_valveLevelInsideOptional                  = static_cast<jint>(cppValue.valveLevel.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_valveLevelInsideOptionalClassName.c_str(), value_valveLevelInsideOptionalCtorSignature.c_str(),
                    jnivalue_valveLevelInsideOptional, value_valveLevelInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_valveLevelInsideOptional, value_valveLevel);
            }

            jclass valveStateChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ValveConfigurationAndControlClusterValveStateChangedEvent",
                valveStateChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$ValveConfigurationAndControlClusterValveStateChangedEvent");
                return nullptr;
            }

            jmethodID valveStateChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, valveStateChangedStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/util/Optional;)V",
                                                                &valveStateChangedStructCtor);
            if (err != CHIP_NO_ERROR || valveStateChangedStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$ValveConfigurationAndControlClusterValveStateChangedEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(valveStateChangedStructClass, valveStateChangedStructCtor, value_valveState, value_valveLevel);

            return value;
        }
        case Events::ValveFault::Id: {
            Events::ValveFault::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_valveFault;
            std::string value_valveFaultClassName     = "java/lang/Integer";
            std::string value_valveFaultCtorSignature = "(I)V";
            jint jnivalue_valveFault                  = static_cast<jint>(cppValue.valveFault.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_valveFaultClassName.c_str(), value_valveFaultCtorSignature.c_str(), jnivalue_valveFault, value_valveFault);

            jclass valveFaultStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ValveConfigurationAndControlClusterValveFaultEvent",
                valveFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ValveConfigurationAndControlClusterValveFaultEvent");
                return nullptr;
            }

            jmethodID valveFaultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, valveFaultStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &valveFaultStructCtor);
            if (err != CHIP_NO_ERROR || valveFaultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$ValveConfigurationAndControlClusterValveFaultEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(valveFaultStructClass, valveFaultStructCtor, value_valveFault);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ElectricalPowerMeasurement::Id: {
        using namespace app::Clusters::ElectricalPowerMeasurement;
        switch (aPath.mEventId)
        {
        case Events::MeasurementPeriodRanges::Id: {
            Events::MeasurementPeriodRanges::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_ranges;
            chip::JniReferences::GetInstance().CreateArrayList(value_ranges);

            auto iter_value_ranges_0 = cppValue.ranges.begin();
            while (iter_value_ranges_0.Next())
            {
                auto & entry_0 = iter_value_ranges_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_measurementType;
                std::string newElement_0_measurementTypeClassName     = "java/lang/Integer";
                std::string newElement_0_measurementTypeCtorSignature = "(I)V";
                jint jninewElement_0_measurementType                  = static_cast<jint>(entry_0.measurementType);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0_measurementTypeClassName.c_str(), newElement_0_measurementTypeCtorSignature.c_str(),
                    jninewElement_0_measurementType, newElement_0_measurementType);
                jobject newElement_0_min;
                std::string newElement_0_minClassName     = "java/lang/Long";
                std::string newElement_0_minCtorSignature = "(J)V";
                jlong jninewElement_0_min                 = static_cast<jlong>(entry_0.min);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(newElement_0_minClassName.c_str(),
                                                                            newElement_0_minCtorSignature.c_str(),
                                                                            jninewElement_0_min, newElement_0_min);
                jobject newElement_0_max;
                std::string newElement_0_maxClassName     = "java/lang/Long";
                std::string newElement_0_maxCtorSignature = "(J)V";
                jlong jninewElement_0_max                 = static_cast<jlong>(entry_0.max);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(newElement_0_maxClassName.c_str(),
                                                                            newElement_0_maxCtorSignature.c_str(),
                                                                            jninewElement_0_max, newElement_0_max);
                jobject newElement_0_startTimestamp;
                if (!entry_0.startTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_startTimestamp);
                }
                else
                {
                    jobject newElement_0_startTimestampInsideOptional;
                    std::string newElement_0_startTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_startTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_startTimestampInsideOptional = static_cast<jlong>(entry_0.startTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_startTimestampInsideOptionalClassName.c_str(),
                        newElement_0_startTimestampInsideOptionalCtorSignature.c_str(),
                        jninewElement_0_startTimestampInsideOptional, newElement_0_startTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_startTimestampInsideOptional,
                                                                      newElement_0_startTimestamp);
                }
                jobject newElement_0_endTimestamp;
                if (!entry_0.endTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_endTimestamp);
                }
                else
                {
                    jobject newElement_0_endTimestampInsideOptional;
                    std::string newElement_0_endTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_endTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_endTimestampInsideOptional = static_cast<jlong>(entry_0.endTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_endTimestampInsideOptionalClassName.c_str(),
                        newElement_0_endTimestampInsideOptionalCtorSignature.c_str(), jninewElement_0_endTimestampInsideOptional,
                        newElement_0_endTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_endTimestampInsideOptional,
                                                                      newElement_0_endTimestamp);
                }
                jobject newElement_0_minTimestamp;
                if (!entry_0.minTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_minTimestamp);
                }
                else
                {
                    jobject newElement_0_minTimestampInsideOptional;
                    std::string newElement_0_minTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_minTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_minTimestampInsideOptional = static_cast<jlong>(entry_0.minTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_minTimestampInsideOptionalClassName.c_str(),
                        newElement_0_minTimestampInsideOptionalCtorSignature.c_str(), jninewElement_0_minTimestampInsideOptional,
                        newElement_0_minTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_minTimestampInsideOptional,
                                                                      newElement_0_minTimestamp);
                }
                jobject newElement_0_maxTimestamp;
                if (!entry_0.maxTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_maxTimestamp);
                }
                else
                {
                    jobject newElement_0_maxTimestampInsideOptional;
                    std::string newElement_0_maxTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_maxTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_maxTimestampInsideOptional = static_cast<jlong>(entry_0.maxTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_maxTimestampInsideOptionalClassName.c_str(),
                        newElement_0_maxTimestampInsideOptionalCtorSignature.c_str(), jninewElement_0_maxTimestampInsideOptional,
                        newElement_0_maxTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_maxTimestampInsideOptional,
                                                                      newElement_0_maxTimestamp);
                }
                jobject newElement_0_startSystime;
                if (!entry_0.startSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_startSystime);
                }
                else
                {
                    jobject newElement_0_startSystimeInsideOptional;
                    std::string newElement_0_startSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_startSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_startSystimeInsideOptional = static_cast<jlong>(entry_0.startSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_startSystimeInsideOptionalClassName.c_str(),
                        newElement_0_startSystimeInsideOptionalCtorSignature.c_str(), jninewElement_0_startSystimeInsideOptional,
                        newElement_0_startSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_startSystimeInsideOptional,
                                                                      newElement_0_startSystime);
                }
                jobject newElement_0_endSystime;
                if (!entry_0.endSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_endSystime);
                }
                else
                {
                    jobject newElement_0_endSystimeInsideOptional;
                    std::string newElement_0_endSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_endSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_endSystimeInsideOptional                 = static_cast<jlong>(entry_0.endSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_endSystimeInsideOptionalClassName.c_str(),
                        newElement_0_endSystimeInsideOptionalCtorSignature.c_str(), jninewElement_0_endSystimeInsideOptional,
                        newElement_0_endSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_endSystimeInsideOptional,
                                                                      newElement_0_endSystime);
                }
                jobject newElement_0_minSystime;
                if (!entry_0.minSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_minSystime);
                }
                else
                {
                    jobject newElement_0_minSystimeInsideOptional;
                    std::string newElement_0_minSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_minSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_minSystimeInsideOptional                 = static_cast<jlong>(entry_0.minSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_minSystimeInsideOptionalClassName.c_str(),
                        newElement_0_minSystimeInsideOptionalCtorSignature.c_str(), jninewElement_0_minSystimeInsideOptional,
                        newElement_0_minSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_minSystimeInsideOptional,
                                                                      newElement_0_minSystime);
                }
                jobject newElement_0_maxSystime;
                if (!entry_0.maxSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_maxSystime);
                }
                else
                {
                    jobject newElement_0_maxSystimeInsideOptional;
                    std::string newElement_0_maxSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_maxSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_maxSystimeInsideOptional                 = static_cast<jlong>(entry_0.maxSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_maxSystimeInsideOptionalClassName.c_str(),
                        newElement_0_maxSystimeInsideOptionalCtorSignature.c_str(), jninewElement_0_maxSystimeInsideOptional,
                        newElement_0_maxSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_maxSystimeInsideOptional,
                                                                      newElement_0_maxSystime);
                }

                {
                    jclass measurementRangeStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ElectricalPowerMeasurementClusterMeasurementRangeStruct",
                        measurementRangeStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl,
                                     "Could not find class ChipStructs$ElectricalPowerMeasurementClusterMeasurementRangeStruct");
                        return nullptr;
                    }

                    jmethodID measurementRangeStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, measurementRangeStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/"
                        "Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/"
                        "Optional;)V",
                        &measurementRangeStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || measurementRangeStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(
                            Zcl, "Could not find ChipStructs$ElectricalPowerMeasurementClusterMeasurementRangeStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(measurementRangeStructStructClass_1, measurementRangeStructStructCtor_1,
                                                  newElement_0_measurementType, newElement_0_min, newElement_0_max,
                                                  newElement_0_startTimestamp, newElement_0_endTimestamp, newElement_0_minTimestamp,
                                                  newElement_0_maxTimestamp, newElement_0_startSystime, newElement_0_endSystime,
                                                  newElement_0_minSystime, newElement_0_maxSystime);
                }
                chip::JniReferences::GetInstance().AddToList(value_ranges, newElement_0);
            }

            jclass measurementPeriodRangesStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent",
                measurementPeriodRangesStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent");
                return nullptr;
            }

            jmethodID measurementPeriodRangesStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, measurementPeriodRangesStructClass, "<init>",
                                                                "(Ljava/util/ArrayList;)V", &measurementPeriodRangesStructCtor);
            if (err != CHIP_NO_ERROR || measurementPeriodRangesStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(measurementPeriodRangesStructClass, measurementPeriodRangesStructCtor, value_ranges);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ElectricalEnergyMeasurement::Id: {
        using namespace app::Clusters::ElectricalEnergyMeasurement;
        switch (aPath.mEventId)
        {
        case Events::CumulativeEnergyMeasured::Id: {
            Events::CumulativeEnergyMeasured::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_energyImported;
            if (!cppValue.energyImported.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImported);
            }
            else
            {
                jobject value_energyImportedInsideOptional;
                jobject value_energyImportedInsideOptional_energy;
                std::string value_energyImportedInsideOptional_energyClassName     = "java/lang/Long";
                std::string value_energyImportedInsideOptional_energyCtorSignature = "(J)V";
                jlong jnivalue_energyImportedInsideOptional_energy = static_cast<jlong>(cppValue.energyImported.Value().energy);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_energyImportedInsideOptional_energyClassName.c_str(),
                    value_energyImportedInsideOptional_energyCtorSignature.c_str(), jnivalue_energyImportedInsideOptional_energy,
                    value_energyImportedInsideOptional_energy);
                jobject value_energyImportedInsideOptional_startTimestamp;
                if (!cppValue.energyImported.Value().startTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_startTimestamp);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_startTimestampInsideOptional;
                    std::string value_energyImportedInsideOptional_startTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_startTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_startTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().startTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_startTimestampInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_startTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_startTimestampInsideOptional,
                        value_energyImportedInsideOptional_startTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(
                        value_energyImportedInsideOptional_startTimestampInsideOptional,
                        value_energyImportedInsideOptional_startTimestamp);
                }
                jobject value_energyImportedInsideOptional_endTimestamp;
                if (!cppValue.energyImported.Value().endTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_endTimestamp);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_endTimestampInsideOptional;
                    std::string value_energyImportedInsideOptional_endTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_endTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_endTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().endTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_endTimestampInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_endTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_endTimestampInsideOptional,
                        value_energyImportedInsideOptional_endTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional_endTimestampInsideOptional,
                                                                      value_energyImportedInsideOptional_endTimestamp);
                }
                jobject value_energyImportedInsideOptional_startSystime;
                if (!cppValue.energyImported.Value().startSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_startSystime);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_startSystimeInsideOptional;
                    std::string value_energyImportedInsideOptional_startSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_startSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_startSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().startSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_startSystimeInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_startSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_startSystimeInsideOptional,
                        value_energyImportedInsideOptional_startSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional_startSystimeInsideOptional,
                                                                      value_energyImportedInsideOptional_startSystime);
                }
                jobject value_energyImportedInsideOptional_endSystime;
                if (!cppValue.energyImported.Value().endSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_endSystime);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_endSystimeInsideOptional;
                    std::string value_energyImportedInsideOptional_endSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_endSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_endSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().endSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_endSystimeInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_endSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_endSystimeInsideOptional,
                        value_energyImportedInsideOptional_endSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional_endSystimeInsideOptional,
                                                                      value_energyImportedInsideOptional_endSystime);
                }

                {
                    jclass energyMeasurementStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct",
                        energyMeasurementStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl,
                                     "Could not find class ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct");
                        return nullptr;
                    }

                    jmethodID energyMeasurementStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, energyMeasurementStructStructClass_1, "<init>",
                        "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                        &energyMeasurementStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || energyMeasurementStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(
                            Zcl,
                            "Could not find ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct constructor");
                        return nullptr;
                    }

                    value_energyImportedInsideOptional = env->NewObject(
                        energyMeasurementStructStructClass_1, energyMeasurementStructStructCtor_1,
                        value_energyImportedInsideOptional_energy, value_energyImportedInsideOptional_startTimestamp,
                        value_energyImportedInsideOptional_endTimestamp, value_energyImportedInsideOptional_startSystime,
                        value_energyImportedInsideOptional_endSystime);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional, value_energyImported);
            }

            jobject value_energyExported;
            if (!cppValue.energyExported.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExported);
            }
            else
            {
                jobject value_energyExportedInsideOptional;
                jobject value_energyExportedInsideOptional_energy;
                std::string value_energyExportedInsideOptional_energyClassName     = "java/lang/Long";
                std::string value_energyExportedInsideOptional_energyCtorSignature = "(J)V";
                jlong jnivalue_energyExportedInsideOptional_energy = static_cast<jlong>(cppValue.energyExported.Value().energy);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_energyExportedInsideOptional_energyClassName.c_str(),
                    value_energyExportedInsideOptional_energyCtorSignature.c_str(), jnivalue_energyExportedInsideOptional_energy,
                    value_energyExportedInsideOptional_energy);
                jobject value_energyExportedInsideOptional_startTimestamp;
                if (!cppValue.energyExported.Value().startTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_startTimestamp);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_startTimestampInsideOptional;
                    std::string value_energyExportedInsideOptional_startTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_startTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_startTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().startTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_startTimestampInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_startTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_startTimestampInsideOptional,
                        value_energyExportedInsideOptional_startTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(
                        value_energyExportedInsideOptional_startTimestampInsideOptional,
                        value_energyExportedInsideOptional_startTimestamp);
                }
                jobject value_energyExportedInsideOptional_endTimestamp;
                if (!cppValue.energyExported.Value().endTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_endTimestamp);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_endTimestampInsideOptional;
                    std::string value_energyExportedInsideOptional_endTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_endTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_endTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().endTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_endTimestampInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_endTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_endTimestampInsideOptional,
                        value_energyExportedInsideOptional_endTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional_endTimestampInsideOptional,
                                                                      value_energyExportedInsideOptional_endTimestamp);
                }
                jobject value_energyExportedInsideOptional_startSystime;
                if (!cppValue.energyExported.Value().startSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_startSystime);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_startSystimeInsideOptional;
                    std::string value_energyExportedInsideOptional_startSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_startSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_startSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().startSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_startSystimeInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_startSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_startSystimeInsideOptional,
                        value_energyExportedInsideOptional_startSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional_startSystimeInsideOptional,
                                                                      value_energyExportedInsideOptional_startSystime);
                }
                jobject value_energyExportedInsideOptional_endSystime;
                if (!cppValue.energyExported.Value().endSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_endSystime);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_endSystimeInsideOptional;
                    std::string value_energyExportedInsideOptional_endSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_endSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_endSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().endSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_endSystimeInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_endSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_endSystimeInsideOptional,
                        value_energyExportedInsideOptional_endSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional_endSystimeInsideOptional,
                                                                      value_energyExportedInsideOptional_endSystime);
                }

                {
                    jclass energyMeasurementStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct",
                        energyMeasurementStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl,
                                     "Could not find class ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct");
                        return nullptr;
                    }

                    jmethodID energyMeasurementStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, energyMeasurementStructStructClass_1, "<init>",
                        "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                        &energyMeasurementStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || energyMeasurementStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(
                            Zcl,
                            "Could not find ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct constructor");
                        return nullptr;
                    }

                    value_energyExportedInsideOptional = env->NewObject(
                        energyMeasurementStructStructClass_1, energyMeasurementStructStructCtor_1,
                        value_energyExportedInsideOptional_energy, value_energyExportedInsideOptional_startTimestamp,
                        value_energyExportedInsideOptional_endTimestamp, value_energyExportedInsideOptional_startSystime,
                        value_energyExportedInsideOptional_endSystime);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional, value_energyExported);
            }

            jclass cumulativeEnergyMeasuredStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent",
                cumulativeEnergyMeasuredStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    Zcl, "Could not find class ChipEventStructs$ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent");
                return nullptr;
            }

            jmethodID cumulativeEnergyMeasuredStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, cumulativeEnergyMeasuredStructClass, "<init>",
                                                                "(Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                &cumulativeEnergyMeasuredStructCtor);
            if (err != CHIP_NO_ERROR || cumulativeEnergyMeasuredStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(cumulativeEnergyMeasuredStructClass, cumulativeEnergyMeasuredStructCtor,
                                           value_energyImported, value_energyExported);

            return value;
        }
        case Events::PeriodicEnergyMeasured::Id: {
            Events::PeriodicEnergyMeasured::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_energyImported;
            if (!cppValue.energyImported.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImported);
            }
            else
            {
                jobject value_energyImportedInsideOptional;
                jobject value_energyImportedInsideOptional_energy;
                std::string value_energyImportedInsideOptional_energyClassName     = "java/lang/Long";
                std::string value_energyImportedInsideOptional_energyCtorSignature = "(J)V";
                jlong jnivalue_energyImportedInsideOptional_energy = static_cast<jlong>(cppValue.energyImported.Value().energy);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_energyImportedInsideOptional_energyClassName.c_str(),
                    value_energyImportedInsideOptional_energyCtorSignature.c_str(), jnivalue_energyImportedInsideOptional_energy,
                    value_energyImportedInsideOptional_energy);
                jobject value_energyImportedInsideOptional_startTimestamp;
                if (!cppValue.energyImported.Value().startTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_startTimestamp);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_startTimestampInsideOptional;
                    std::string value_energyImportedInsideOptional_startTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_startTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_startTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().startTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_startTimestampInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_startTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_startTimestampInsideOptional,
                        value_energyImportedInsideOptional_startTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(
                        value_energyImportedInsideOptional_startTimestampInsideOptional,
                        value_energyImportedInsideOptional_startTimestamp);
                }
                jobject value_energyImportedInsideOptional_endTimestamp;
                if (!cppValue.energyImported.Value().endTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_endTimestamp);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_endTimestampInsideOptional;
                    std::string value_energyImportedInsideOptional_endTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_endTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_endTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().endTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_endTimestampInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_endTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_endTimestampInsideOptional,
                        value_energyImportedInsideOptional_endTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional_endTimestampInsideOptional,
                                                                      value_energyImportedInsideOptional_endTimestamp);
                }
                jobject value_energyImportedInsideOptional_startSystime;
                if (!cppValue.energyImported.Value().startSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_startSystime);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_startSystimeInsideOptional;
                    std::string value_energyImportedInsideOptional_startSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_startSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_startSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().startSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_startSystimeInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_startSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_startSystimeInsideOptional,
                        value_energyImportedInsideOptional_startSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional_startSystimeInsideOptional,
                                                                      value_energyImportedInsideOptional_startSystime);
                }
                jobject value_energyImportedInsideOptional_endSystime;
                if (!cppValue.energyImported.Value().endSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyImportedInsideOptional_endSystime);
                }
                else
                {
                    jobject value_energyImportedInsideOptional_endSystimeInsideOptional;
                    std::string value_energyImportedInsideOptional_endSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyImportedInsideOptional_endSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyImportedInsideOptional_endSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyImported.Value().endSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyImportedInsideOptional_endSystimeInsideOptionalClassName.c_str(),
                        value_energyImportedInsideOptional_endSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyImportedInsideOptional_endSystimeInsideOptional,
                        value_energyImportedInsideOptional_endSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional_endSystimeInsideOptional,
                                                                      value_energyImportedInsideOptional_endSystime);
                }

                {
                    jclass energyMeasurementStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct",
                        energyMeasurementStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl,
                                     "Could not find class ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct");
                        return nullptr;
                    }

                    jmethodID energyMeasurementStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, energyMeasurementStructStructClass_1, "<init>",
                        "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                        &energyMeasurementStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || energyMeasurementStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(
                            Zcl,
                            "Could not find ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct constructor");
                        return nullptr;
                    }

                    value_energyImportedInsideOptional = env->NewObject(
                        energyMeasurementStructStructClass_1, energyMeasurementStructStructCtor_1,
                        value_energyImportedInsideOptional_energy, value_energyImportedInsideOptional_startTimestamp,
                        value_energyImportedInsideOptional_endTimestamp, value_energyImportedInsideOptional_startSystime,
                        value_energyImportedInsideOptional_endSystime);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_energyImportedInsideOptional, value_energyImported);
            }

            jobject value_energyExported;
            if (!cppValue.energyExported.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExported);
            }
            else
            {
                jobject value_energyExportedInsideOptional;
                jobject value_energyExportedInsideOptional_energy;
                std::string value_energyExportedInsideOptional_energyClassName     = "java/lang/Long";
                std::string value_energyExportedInsideOptional_energyCtorSignature = "(J)V";
                jlong jnivalue_energyExportedInsideOptional_energy = static_cast<jlong>(cppValue.energyExported.Value().energy);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_energyExportedInsideOptional_energyClassName.c_str(),
                    value_energyExportedInsideOptional_energyCtorSignature.c_str(), jnivalue_energyExportedInsideOptional_energy,
                    value_energyExportedInsideOptional_energy);
                jobject value_energyExportedInsideOptional_startTimestamp;
                if (!cppValue.energyExported.Value().startTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_startTimestamp);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_startTimestampInsideOptional;
                    std::string value_energyExportedInsideOptional_startTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_startTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_startTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().startTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_startTimestampInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_startTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_startTimestampInsideOptional,
                        value_energyExportedInsideOptional_startTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(
                        value_energyExportedInsideOptional_startTimestampInsideOptional,
                        value_energyExportedInsideOptional_startTimestamp);
                }
                jobject value_energyExportedInsideOptional_endTimestamp;
                if (!cppValue.energyExported.Value().endTimestamp.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_endTimestamp);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_endTimestampInsideOptional;
                    std::string value_energyExportedInsideOptional_endTimestampInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_endTimestampInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_endTimestampInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().endTimestamp.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_endTimestampInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_endTimestampInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_endTimestampInsideOptional,
                        value_energyExportedInsideOptional_endTimestampInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional_endTimestampInsideOptional,
                                                                      value_energyExportedInsideOptional_endTimestamp);
                }
                jobject value_energyExportedInsideOptional_startSystime;
                if (!cppValue.energyExported.Value().startSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_startSystime);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_startSystimeInsideOptional;
                    std::string value_energyExportedInsideOptional_startSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_startSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_startSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().startSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_startSystimeInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_startSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_startSystimeInsideOptional,
                        value_energyExportedInsideOptional_startSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional_startSystimeInsideOptional,
                                                                      value_energyExportedInsideOptional_startSystime);
                }
                jobject value_energyExportedInsideOptional_endSystime;
                if (!cppValue.energyExported.Value().endSystime.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyExportedInsideOptional_endSystime);
                }
                else
                {
                    jobject value_energyExportedInsideOptional_endSystimeInsideOptional;
                    std::string value_energyExportedInsideOptional_endSystimeInsideOptionalClassName     = "java/lang/Long";
                    std::string value_energyExportedInsideOptional_endSystimeInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_energyExportedInsideOptional_endSystimeInsideOptional =
                        static_cast<jlong>(cppValue.energyExported.Value().endSystime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_energyExportedInsideOptional_endSystimeInsideOptionalClassName.c_str(),
                        value_energyExportedInsideOptional_endSystimeInsideOptionalCtorSignature.c_str(),
                        jnivalue_energyExportedInsideOptional_endSystimeInsideOptional,
                        value_energyExportedInsideOptional_endSystimeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional_endSystimeInsideOptional,
                                                                      value_energyExportedInsideOptional_endSystime);
                }

                {
                    jclass energyMeasurementStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct",
                        energyMeasurementStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl,
                                     "Could not find class ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct");
                        return nullptr;
                    }

                    jmethodID energyMeasurementStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, energyMeasurementStructStructClass_1, "<init>",
                        "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                        &energyMeasurementStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || energyMeasurementStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(
                            Zcl,
                            "Could not find ChipStructs$ElectricalEnergyMeasurementClusterEnergyMeasurementStruct constructor");
                        return nullptr;
                    }

                    value_energyExportedInsideOptional = env->NewObject(
                        energyMeasurementStructStructClass_1, energyMeasurementStructStructCtor_1,
                        value_energyExportedInsideOptional_energy, value_energyExportedInsideOptional_startTimestamp,
                        value_energyExportedInsideOptional_endTimestamp, value_energyExportedInsideOptional_startSystime,
                        value_energyExportedInsideOptional_endSystime);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_energyExportedInsideOptional, value_energyExported);
            }

            jclass periodicEnergyMeasuredStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent",
                periodicEnergyMeasuredStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent");
                return nullptr;
            }

            jmethodID periodicEnergyMeasuredStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, periodicEnergyMeasuredStructClass, "<init>",
                                                                "(Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                &periodicEnergyMeasuredStructCtor);
            if (err != CHIP_NO_ERROR || periodicEnergyMeasuredStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl,
                    "Could not find ChipEventStructs$ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(periodicEnergyMeasuredStructClass, periodicEnergyMeasuredStructCtor,
                                           value_energyImported, value_energyExported);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::WaterHeaterManagement::Id: {
        using namespace app::Clusters::WaterHeaterManagement;
        switch (aPath.mEventId)
        {
        case Events::BoostStarted::Id: {
            Events::BoostStarted::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_boostInfo;
            jobject value_boostInfo_duration;
            std::string value_boostInfo_durationClassName     = "java/lang/Long";
            std::string value_boostInfo_durationCtorSignature = "(J)V";
            jlong jnivalue_boostInfo_duration                 = static_cast<jlong>(cppValue.boostInfo.duration);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_boostInfo_durationClassName.c_str(),
                                                                        value_boostInfo_durationCtorSignature.c_str(),
                                                                        jnivalue_boostInfo_duration, value_boostInfo_duration);
            jobject value_boostInfo_oneShot;
            if (!cppValue.boostInfo.oneShot.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_boostInfo_oneShot);
            }
            else
            {
                jobject value_boostInfo_oneShotInsideOptional;
                std::string value_boostInfo_oneShotInsideOptionalClassName     = "java/lang/Boolean";
                std::string value_boostInfo_oneShotInsideOptionalCtorSignature = "(Z)V";
                jboolean jnivalue_boostInfo_oneShotInsideOptional = static_cast<jboolean>(cppValue.boostInfo.oneShot.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                    value_boostInfo_oneShotInsideOptionalClassName.c_str(),
                    value_boostInfo_oneShotInsideOptionalCtorSignature.c_str(), jnivalue_boostInfo_oneShotInsideOptional,
                    value_boostInfo_oneShotInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_boostInfo_oneShotInsideOptional, value_boostInfo_oneShot);
            }
            jobject value_boostInfo_emergencyBoost;
            if (!cppValue.boostInfo.emergencyBoost.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_boostInfo_emergencyBoost);
            }
            else
            {
                jobject value_boostInfo_emergencyBoostInsideOptional;
                std::string value_boostInfo_emergencyBoostInsideOptionalClassName     = "java/lang/Boolean";
                std::string value_boostInfo_emergencyBoostInsideOptionalCtorSignature = "(Z)V";
                jboolean jnivalue_boostInfo_emergencyBoostInsideOptional =
                    static_cast<jboolean>(cppValue.boostInfo.emergencyBoost.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
                    value_boostInfo_emergencyBoostInsideOptionalClassName.c_str(),
                    value_boostInfo_emergencyBoostInsideOptionalCtorSignature.c_str(),
                    jnivalue_boostInfo_emergencyBoostInsideOptional, value_boostInfo_emergencyBoostInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_boostInfo_emergencyBoostInsideOptional,
                                                                  value_boostInfo_emergencyBoost);
            }
            jobject value_boostInfo_temporarySetpoint;
            if (!cppValue.boostInfo.temporarySetpoint.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_boostInfo_temporarySetpoint);
            }
            else
            {
                jobject value_boostInfo_temporarySetpointInsideOptional;
                std::string value_boostInfo_temporarySetpointInsideOptionalClassName     = "java/lang/Integer";
                std::string value_boostInfo_temporarySetpointInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_boostInfo_temporarySetpointInsideOptional =
                    static_cast<jint>(cppValue.boostInfo.temporarySetpoint.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_boostInfo_temporarySetpointInsideOptionalClassName.c_str(),
                    value_boostInfo_temporarySetpointInsideOptionalCtorSignature.c_str(),
                    jnivalue_boostInfo_temporarySetpointInsideOptional, value_boostInfo_temporarySetpointInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_boostInfo_temporarySetpointInsideOptional,
                                                                  value_boostInfo_temporarySetpoint);
            }
            jobject value_boostInfo_targetPercentage;
            if (!cppValue.boostInfo.targetPercentage.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_boostInfo_targetPercentage);
            }
            else
            {
                jobject value_boostInfo_targetPercentageInsideOptional;
                std::string value_boostInfo_targetPercentageInsideOptionalClassName     = "java/lang/Integer";
                std::string value_boostInfo_targetPercentageInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_boostInfo_targetPercentageInsideOptional =
                    static_cast<jint>(cppValue.boostInfo.targetPercentage.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_boostInfo_targetPercentageInsideOptionalClassName.c_str(),
                    value_boostInfo_targetPercentageInsideOptionalCtorSignature.c_str(),
                    jnivalue_boostInfo_targetPercentageInsideOptional, value_boostInfo_targetPercentageInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_boostInfo_targetPercentageInsideOptional,
                                                                  value_boostInfo_targetPercentage);
            }
            jobject value_boostInfo_targetReheat;
            if (!cppValue.boostInfo.targetReheat.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_boostInfo_targetReheat);
            }
            else
            {
                jobject value_boostInfo_targetReheatInsideOptional;
                std::string value_boostInfo_targetReheatInsideOptionalClassName     = "java/lang/Integer";
                std::string value_boostInfo_targetReheatInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_boostInfo_targetReheatInsideOptional = static_cast<jint>(cppValue.boostInfo.targetReheat.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_boostInfo_targetReheatInsideOptionalClassName.c_str(),
                    value_boostInfo_targetReheatInsideOptionalCtorSignature.c_str(), jnivalue_boostInfo_targetReheatInsideOptional,
                    value_boostInfo_targetReheatInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_boostInfo_targetReheatInsideOptional,
                                                                  value_boostInfo_targetReheat);
            }

            {
                jclass waterHeaterBoostInfoStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$WaterHeaterManagementClusterWaterHeaterBoostInfoStruct",
                    waterHeaterBoostInfoStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$WaterHeaterManagementClusterWaterHeaterBoostInfoStruct");
                    return nullptr;
                }

                jmethodID waterHeaterBoostInfoStructStructCtor_0;
                err =
                    chip::JniReferences::GetInstance().FindMethod(env, waterHeaterBoostInfoStructStructClass_0, "<init>",
                                                                  "(Ljava/lang/Long;Ljava/util/Optional;Ljava/util/Optional;Ljava/"
                                                                  "util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                                                                  &waterHeaterBoostInfoStructStructCtor_0);
                if (err != CHIP_NO_ERROR || waterHeaterBoostInfoStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl,
                                 "Could not find ChipStructs$WaterHeaterManagementClusterWaterHeaterBoostInfoStruct constructor");
                    return nullptr;
                }

                value_boostInfo = env->NewObject(waterHeaterBoostInfoStructStructClass_0, waterHeaterBoostInfoStructStructCtor_0,
                                                 value_boostInfo_duration, value_boostInfo_oneShot, value_boostInfo_emergencyBoost,
                                                 value_boostInfo_temporarySetpoint, value_boostInfo_targetPercentage,
                                                 value_boostInfo_targetReheat);
            }

            jclass boostStartedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WaterHeaterManagementClusterBoostStartedEvent",
                boostStartedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WaterHeaterManagementClusterBoostStartedEvent");
                return nullptr;
            }

            jmethodID boostStartedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, boostStartedStructClass, "<init>",
                "(Lchip/devicecontroller/ChipStructs$WaterHeaterManagementClusterWaterHeaterBoostInfoStruct;)V",
                &boostStartedStructCtor);
            if (err != CHIP_NO_ERROR || boostStartedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$WaterHeaterManagementClusterBoostStartedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(boostStartedStructClass, boostStartedStructCtor, value_boostInfo);

            return value;
        }
        case Events::BoostEnded::Id: {
            Events::BoostEnded::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass boostEndedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$WaterHeaterManagementClusterBoostEndedEvent", boostEndedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$WaterHeaterManagementClusterBoostEndedEvent");
                return nullptr;
            }

            jmethodID boostEndedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, boostEndedStructClass, "<init>", "()V", &boostEndedStructCtor);
            if (err != CHIP_NO_ERROR || boostEndedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$WaterHeaterManagementClusterBoostEndedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(boostEndedStructClass, boostEndedStructCtor);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DemandResponseLoadControl::Id: {
        using namespace app::Clusters::DemandResponseLoadControl;
        switch (aPath.mEventId)
        {
        case Events::LoadControlEventStatusChange::Id: {
            Events::LoadControlEventStatusChange::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_eventID;
            jbyteArray value_eventIDByteArray = env->NewByteArray(static_cast<jsize>(cppValue.eventID.size()));
            env->SetByteArrayRegion(value_eventIDByteArray, 0, static_cast<jsize>(cppValue.eventID.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.eventID.data()));
            value_eventID = value_eventIDByteArray;

            jobject value_transitionIndex;
            if (cppValue.transitionIndex.IsNull())
            {
                value_transitionIndex = nullptr;
            }
            else
            {
                std::string value_transitionIndexClassName     = "java/lang/Integer";
                std::string value_transitionIndexCtorSignature = "(I)V";
                jint jnivalue_transitionIndex                  = static_cast<jint>(cppValue.transitionIndex.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_transitionIndexClassName.c_str(),
                                                                           value_transitionIndexCtorSignature.c_str(),
                                                                           jnivalue_transitionIndex, value_transitionIndex);
            }

            jobject value_status;
            std::string value_statusClassName     = "java/lang/Integer";
            std::string value_statusCtorSignature = "(I)V";
            jint jnivalue_status                  = static_cast<jint>(cppValue.status);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_statusClassName.c_str(), value_statusCtorSignature.c_str(), jnivalue_status, value_status);

            jobject value_criticality;
            std::string value_criticalityClassName     = "java/lang/Integer";
            std::string value_criticalityCtorSignature = "(I)V";
            jint jnivalue_criticality                  = static_cast<jint>(cppValue.criticality);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_criticalityClassName.c_str(),
                                                                       value_criticalityCtorSignature.c_str(), jnivalue_criticality,
                                                                       value_criticality);

            jobject value_control;
            std::string value_controlClassName     = "java/lang/Integer";
            std::string value_controlCtorSignature = "(I)V";
            jint jnivalue_control                  = static_cast<jint>(cppValue.control.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_controlClassName.c_str(), value_controlCtorSignature.c_str(), jnivalue_control, value_control);

            jobject value_temperatureControl;
            if (!cppValue.temperatureControl.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_temperatureControl);
            }
            else
            {
                jobject value_temperatureControlInsideOptional;
                if (cppValue.temperatureControl.Value().IsNull())
                {
                    value_temperatureControlInsideOptional = nullptr;
                }
                else
                {
                    jobject value_temperatureControlInsideOptional_coolingTempOffset;
                    if (!cppValue.temperatureControl.Value().Value().coolingTempOffset.HasValue())
                    {
                        chip::JniReferences::GetInstance().CreateOptional(nullptr,
                                                                          value_temperatureControlInsideOptional_coolingTempOffset);
                    }
                    else
                    {
                        jobject value_temperatureControlInsideOptional_coolingTempOffsetInsideOptional;
                        if (cppValue.temperatureControl.Value().Value().coolingTempOffset.Value().IsNull())
                        {
                            value_temperatureControlInsideOptional_coolingTempOffsetInsideOptional = nullptr;
                        }
                        else
                        {
                            std::string value_temperatureControlInsideOptional_coolingTempOffsetInsideOptionalClassName =
                                "java/lang/Integer";
                            std::string value_temperatureControlInsideOptional_coolingTempOffsetInsideOptionalCtorSignature =
                                "(I)V";
                            jint jnivalue_temperatureControlInsideOptional_coolingTempOffsetInsideOptional =
                                static_cast<jint>(cppValue.temperatureControl.Value().Value().coolingTempOffset.Value().Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                                value_temperatureControlInsideOptional_coolingTempOffsetInsideOptionalClassName.c_str(),
                                value_temperatureControlInsideOptional_coolingTempOffsetInsideOptionalCtorSignature.c_str(),
                                jnivalue_temperatureControlInsideOptional_coolingTempOffsetInsideOptional,
                                value_temperatureControlInsideOptional_coolingTempOffsetInsideOptional);
                        }
                        chip::JniReferences::GetInstance().CreateOptional(
                            value_temperatureControlInsideOptional_coolingTempOffsetInsideOptional,
                            value_temperatureControlInsideOptional_coolingTempOffset);
                    }
                    jobject value_temperatureControlInsideOptional_heatingtTempOffset;
                    if (!cppValue.temperatureControl.Value().Value().heatingtTempOffset.HasValue())
                    {
                        chip::JniReferences::GetInstance().CreateOptional(
                            nullptr, value_temperatureControlInsideOptional_heatingtTempOffset);
                    }
                    else
                    {
                        jobject value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptional;
                        if (cppValue.temperatureControl.Value().Value().heatingtTempOffset.Value().IsNull())
                        {
                            value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptional = nullptr;
                        }
                        else
                        {
                            std::string value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptionalClassName =
                                "java/lang/Integer";
                            std::string value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptionalCtorSignature =
                                "(I)V";
                            jint jnivalue_temperatureControlInsideOptional_heatingtTempOffsetInsideOptional =
                                static_cast<jint>(cppValue.temperatureControl.Value().Value().heatingtTempOffset.Value().Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                                value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptionalClassName.c_str(),
                                value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptionalCtorSignature.c_str(),
                                jnivalue_temperatureControlInsideOptional_heatingtTempOffsetInsideOptional,
                                value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptional);
                        }
                        chip::JniReferences::GetInstance().CreateOptional(
                            value_temperatureControlInsideOptional_heatingtTempOffsetInsideOptional,
                            value_temperatureControlInsideOptional_heatingtTempOffset);
                    }
                    jobject value_temperatureControlInsideOptional_coolingTempSetpoint;
                    if (!cppValue.temperatureControl.Value().Value().coolingTempSetpoint.HasValue())
                    {
                        chip::JniReferences::GetInstance().CreateOptional(
                            nullptr, value_temperatureControlInsideOptional_coolingTempSetpoint);
                    }
                    else
                    {
                        jobject value_temperatureControlInsideOptional_coolingTempSetpointInsideOptional;
                        if (cppValue.temperatureControl.Value().Value().coolingTempSetpoint.Value().IsNull())
                        {
                            value_temperatureControlInsideOptional_coolingTempSetpointInsideOptional = nullptr;
                        }
                        else
                        {
                            std::string value_temperatureControlInsideOptional_coolingTempSetpointInsideOptionalClassName =
                                "java/lang/Integer";
                            std::string value_temperatureControlInsideOptional_coolingTempSetpointInsideOptionalCtorSignature =
                                "(I)V";
                            jint jnivalue_temperatureControlInsideOptional_coolingTempSetpointInsideOptional =
                                static_cast<jint>(cppValue.temperatureControl.Value().Value().coolingTempSetpoint.Value().Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                                value_temperatureControlInsideOptional_coolingTempSetpointInsideOptionalClassName.c_str(),
                                value_temperatureControlInsideOptional_coolingTempSetpointInsideOptionalCtorSignature.c_str(),
                                jnivalue_temperatureControlInsideOptional_coolingTempSetpointInsideOptional,
                                value_temperatureControlInsideOptional_coolingTempSetpointInsideOptional);
                        }
                        chip::JniReferences::GetInstance().CreateOptional(
                            value_temperatureControlInsideOptional_coolingTempSetpointInsideOptional,
                            value_temperatureControlInsideOptional_coolingTempSetpoint);
                    }
                    jobject value_temperatureControlInsideOptional_heatingTempSetpoint;
                    if (!cppValue.temperatureControl.Value().Value().heatingTempSetpoint.HasValue())
                    {
                        chip::JniReferences::GetInstance().CreateOptional(
                            nullptr, value_temperatureControlInsideOptional_heatingTempSetpoint);
                    }
                    else
                    {
                        jobject value_temperatureControlInsideOptional_heatingTempSetpointInsideOptional;
                        if (cppValue.temperatureControl.Value().Value().heatingTempSetpoint.Value().IsNull())
                        {
                            value_temperatureControlInsideOptional_heatingTempSetpointInsideOptional = nullptr;
                        }
                        else
                        {
                            std::string value_temperatureControlInsideOptional_heatingTempSetpointInsideOptionalClassName =
                                "java/lang/Integer";
                            std::string value_temperatureControlInsideOptional_heatingTempSetpointInsideOptionalCtorSignature =
                                "(I)V";
                            jint jnivalue_temperatureControlInsideOptional_heatingTempSetpointInsideOptional =
                                static_cast<jint>(cppValue.temperatureControl.Value().Value().heatingTempSetpoint.Value().Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                                value_temperatureControlInsideOptional_heatingTempSetpointInsideOptionalClassName.c_str(),
                                value_temperatureControlInsideOptional_heatingTempSetpointInsideOptionalCtorSignature.c_str(),
                                jnivalue_temperatureControlInsideOptional_heatingTempSetpointInsideOptional,
                                value_temperatureControlInsideOptional_heatingTempSetpointInsideOptional);
                        }
                        chip::JniReferences::GetInstance().CreateOptional(
                            value_temperatureControlInsideOptional_heatingTempSetpointInsideOptional,
                            value_temperatureControlInsideOptional_heatingTempSetpoint);
                    }

                    {
                        jclass temperatureControlStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$DemandResponseLoadControlClusterTemperatureControlStruct",
                            temperatureControlStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(
                                Zcl, "Could not find class ChipStructs$DemandResponseLoadControlClusterTemperatureControlStruct");
                            return nullptr;
                        }

                        jmethodID temperatureControlStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(
                            env, temperatureControlStructStructClass_2, "<init>",
                            "(Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                            &temperatureControlStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || temperatureControlStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(
                                Zcl,
                                "Could not find ChipStructs$DemandResponseLoadControlClusterTemperatureControlStruct constructor");
                            return nullptr;
                        }

                        value_temperatureControlInsideOptional =
                            env->NewObject(temperatureControlStructStructClass_2, temperatureControlStructStructCtor_2,
                                           value_temperatureControlInsideOptional_coolingTempOffset,
                                           value_temperatureControlInsideOptional_heatingtTempOffset,
                                           value_temperatureControlInsideOptional_coolingTempSetpoint,
                                           value_temperatureControlInsideOptional_heatingTempSetpoint);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_temperatureControlInsideOptional, value_temperatureControl);
            }

            jobject value_averageLoadControl;
            if (!cppValue.averageLoadControl.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_averageLoadControl);
            }
            else
            {
                jobject value_averageLoadControlInsideOptional;
                if (cppValue.averageLoadControl.Value().IsNull())
                {
                    value_averageLoadControlInsideOptional = nullptr;
                }
                else
                {
                    jobject value_averageLoadControlInsideOptional_loadAdjustment;
                    std::string value_averageLoadControlInsideOptional_loadAdjustmentClassName     = "java/lang/Integer";
                    std::string value_averageLoadControlInsideOptional_loadAdjustmentCtorSignature = "(I)V";
                    jint jnivalue_averageLoadControlInsideOptional_loadAdjustment =
                        static_cast<jint>(cppValue.averageLoadControl.Value().Value().loadAdjustment);
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        value_averageLoadControlInsideOptional_loadAdjustmentClassName.c_str(),
                        value_averageLoadControlInsideOptional_loadAdjustmentCtorSignature.c_str(),
                        jnivalue_averageLoadControlInsideOptional_loadAdjustment,
                        value_averageLoadControlInsideOptional_loadAdjustment);

                    {
                        jclass averageLoadControlStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$DemandResponseLoadControlClusterAverageLoadControlStruct",
                            averageLoadControlStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(
                                Zcl, "Could not find class ChipStructs$DemandResponseLoadControlClusterAverageLoadControlStruct");
                            return nullptr;
                        }

                        jmethodID averageLoadControlStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(env, averageLoadControlStructStructClass_2, "<init>",
                                                                            "(Ljava/lang/Integer;)V",
                                                                            &averageLoadControlStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || averageLoadControlStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(
                                Zcl,
                                "Could not find ChipStructs$DemandResponseLoadControlClusterAverageLoadControlStruct constructor");
                            return nullptr;
                        }

                        value_averageLoadControlInsideOptional =
                            env->NewObject(averageLoadControlStructStructClass_2, averageLoadControlStructStructCtor_2,
                                           value_averageLoadControlInsideOptional_loadAdjustment);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_averageLoadControlInsideOptional, value_averageLoadControl);
            }

            jobject value_dutyCycleControl;
            if (!cppValue.dutyCycleControl.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_dutyCycleControl);
            }
            else
            {
                jobject value_dutyCycleControlInsideOptional;
                if (cppValue.dutyCycleControl.Value().IsNull())
                {
                    value_dutyCycleControlInsideOptional = nullptr;
                }
                else
                {
                    jobject value_dutyCycleControlInsideOptional_dutyCycle;
                    std::string value_dutyCycleControlInsideOptional_dutyCycleClassName     = "java/lang/Integer";
                    std::string value_dutyCycleControlInsideOptional_dutyCycleCtorSignature = "(I)V";
                    jint jnivalue_dutyCycleControlInsideOptional_dutyCycle =
                        static_cast<jint>(cppValue.dutyCycleControl.Value().Value().dutyCycle);
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        value_dutyCycleControlInsideOptional_dutyCycleClassName.c_str(),
                        value_dutyCycleControlInsideOptional_dutyCycleCtorSignature.c_str(),
                        jnivalue_dutyCycleControlInsideOptional_dutyCycle, value_dutyCycleControlInsideOptional_dutyCycle);

                    {
                        jclass dutyCycleControlStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$DemandResponseLoadControlClusterDutyCycleControlStruct",
                            dutyCycleControlStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl,
                                         "Could not find class ChipStructs$DemandResponseLoadControlClusterDutyCycleControlStruct");
                            return nullptr;
                        }

                        jmethodID dutyCycleControlStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(env, dutyCycleControlStructStructClass_2, "<init>",
                                                                            "(Ljava/lang/Integer;)V",
                                                                            &dutyCycleControlStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || dutyCycleControlStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(
                                Zcl,
                                "Could not find ChipStructs$DemandResponseLoadControlClusterDutyCycleControlStruct constructor");
                            return nullptr;
                        }

                        value_dutyCycleControlInsideOptional =
                            env->NewObject(dutyCycleControlStructStructClass_2, dutyCycleControlStructStructCtor_2,
                                           value_dutyCycleControlInsideOptional_dutyCycle);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_dutyCycleControlInsideOptional, value_dutyCycleControl);
            }

            jobject value_powerSavingsControl;
            if (!cppValue.powerSavingsControl.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_powerSavingsControl);
            }
            else
            {
                jobject value_powerSavingsControlInsideOptional;
                if (cppValue.powerSavingsControl.Value().IsNull())
                {
                    value_powerSavingsControlInsideOptional = nullptr;
                }
                else
                {
                    jobject value_powerSavingsControlInsideOptional_powerSavings;
                    std::string value_powerSavingsControlInsideOptional_powerSavingsClassName     = "java/lang/Integer";
                    std::string value_powerSavingsControlInsideOptional_powerSavingsCtorSignature = "(I)V";
                    jint jnivalue_powerSavingsControlInsideOptional_powerSavings =
                        static_cast<jint>(cppValue.powerSavingsControl.Value().Value().powerSavings);
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        value_powerSavingsControlInsideOptional_powerSavingsClassName.c_str(),
                        value_powerSavingsControlInsideOptional_powerSavingsCtorSignature.c_str(),
                        jnivalue_powerSavingsControlInsideOptional_powerSavings,
                        value_powerSavingsControlInsideOptional_powerSavings);

                    {
                        jclass powerSavingsControlStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$DemandResponseLoadControlClusterPowerSavingsControlStruct",
                            powerSavingsControlStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(
                                Zcl, "Could not find class ChipStructs$DemandResponseLoadControlClusterPowerSavingsControlStruct");
                            return nullptr;
                        }

                        jmethodID powerSavingsControlStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(env, powerSavingsControlStructStructClass_2, "<init>",
                                                                            "(Ljava/lang/Integer;)V",
                                                                            &powerSavingsControlStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || powerSavingsControlStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(
                                Zcl,
                                "Could not find ChipStructs$DemandResponseLoadControlClusterPowerSavingsControlStruct constructor");
                            return nullptr;
                        }

                        value_powerSavingsControlInsideOptional =
                            env->NewObject(powerSavingsControlStructStructClass_2, powerSavingsControlStructStructCtor_2,
                                           value_powerSavingsControlInsideOptional_powerSavings);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_powerSavingsControlInsideOptional,
                                                                  value_powerSavingsControl);
            }

            jobject value_heatingSourceControl;
            if (!cppValue.heatingSourceControl.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_heatingSourceControl);
            }
            else
            {
                jobject value_heatingSourceControlInsideOptional;
                if (cppValue.heatingSourceControl.Value().IsNull())
                {
                    value_heatingSourceControlInsideOptional = nullptr;
                }
                else
                {
                    jobject value_heatingSourceControlInsideOptional_heatingSource;
                    std::string value_heatingSourceControlInsideOptional_heatingSourceClassName     = "java/lang/Integer";
                    std::string value_heatingSourceControlInsideOptional_heatingSourceCtorSignature = "(I)V";
                    jint jnivalue_heatingSourceControlInsideOptional_heatingSource =
                        static_cast<jint>(cppValue.heatingSourceControl.Value().Value().heatingSource);
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        value_heatingSourceControlInsideOptional_heatingSourceClassName.c_str(),
                        value_heatingSourceControlInsideOptional_heatingSourceCtorSignature.c_str(),
                        jnivalue_heatingSourceControlInsideOptional_heatingSource,
                        value_heatingSourceControlInsideOptional_heatingSource);

                    {
                        jclass heatingSourceControlStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$DemandResponseLoadControlClusterHeatingSourceControlStruct",
                            heatingSourceControlStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(
                                Zcl, "Could not find class ChipStructs$DemandResponseLoadControlClusterHeatingSourceControlStruct");
                            return nullptr;
                        }

                        jmethodID heatingSourceControlStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(env, heatingSourceControlStructStructClass_2, "<init>",
                                                                            "(Ljava/lang/Integer;)V",
                                                                            &heatingSourceControlStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || heatingSourceControlStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(Zcl,
                                         "Could not find ChipStructs$DemandResponseLoadControlClusterHeatingSourceControlStruct "
                                         "constructor");
                            return nullptr;
                        }

                        value_heatingSourceControlInsideOptional =
                            env->NewObject(heatingSourceControlStructStructClass_2, heatingSourceControlStructStructCtor_2,
                                           value_heatingSourceControlInsideOptional_heatingSource);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_heatingSourceControlInsideOptional,
                                                                  value_heatingSourceControl);
            }

            jclass loadControlEventStatusChangeStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent",
                loadControlEventStatusChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    Zcl, "Could not find class ChipEventStructs$DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent");
                return nullptr;
            }

            jmethodID loadControlEventStatusChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, loadControlEventStatusChangeStructClass, "<init>",
                "([BLjava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/"
                "Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V",
                &loadControlEventStatusChangeStructCtor);
            if (err != CHIP_NO_ERROR || loadControlEventStatusChangeStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent "
                             "constructor");
                return nullptr;
            }

            jobject value = env->NewObject(loadControlEventStatusChangeStructClass, loadControlEventStatusChangeStructCtor,
                                           value_eventID, value_transitionIndex, value_status, value_criticality, value_control,
                                           value_temperatureControl, value_averageLoadControl, value_dutyCycleControl,
                                           value_powerSavingsControl, value_heatingSourceControl);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Messages::Id: {
        using namespace app::Clusters::Messages;
        switch (aPath.mEventId)
        {
        case Events::MessageQueued::Id: {
            Events::MessageQueued::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_messageID;
            jbyteArray value_messageIDByteArray = env->NewByteArray(static_cast<jsize>(cppValue.messageID.size()));
            env->SetByteArrayRegion(value_messageIDByteArray, 0, static_cast<jsize>(cppValue.messageID.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.messageID.data()));
            value_messageID = value_messageIDByteArray;

            jclass messageQueuedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$MessagesClusterMessageQueuedEvent", messageQueuedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$MessagesClusterMessageQueuedEvent");
                return nullptr;
            }

            jmethodID messageQueuedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, messageQueuedStructClass, "<init>", "([B)V",
                                                                &messageQueuedStructCtor);
            if (err != CHIP_NO_ERROR || messageQueuedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$MessagesClusterMessageQueuedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(messageQueuedStructClass, messageQueuedStructCtor, value_messageID);

            return value;
        }
        case Events::MessagePresented::Id: {
            Events::MessagePresented::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_messageID;
            jbyteArray value_messageIDByteArray = env->NewByteArray(static_cast<jsize>(cppValue.messageID.size()));
            env->SetByteArrayRegion(value_messageIDByteArray, 0, static_cast<jsize>(cppValue.messageID.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.messageID.data()));
            value_messageID = value_messageIDByteArray;

            jclass messagePresentedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$MessagesClusterMessagePresentedEvent", messagePresentedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$MessagesClusterMessagePresentedEvent");
                return nullptr;
            }

            jmethodID messagePresentedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, messagePresentedStructClass, "<init>", "([B)V",
                                                                &messagePresentedStructCtor);
            if (err != CHIP_NO_ERROR || messagePresentedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$MessagesClusterMessagePresentedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(messagePresentedStructClass, messagePresentedStructCtor, value_messageID);

            return value;
        }
        case Events::MessageComplete::Id: {
            Events::MessageComplete::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_messageID;
            jbyteArray value_messageIDByteArray = env->NewByteArray(static_cast<jsize>(cppValue.messageID.size()));
            env->SetByteArrayRegion(value_messageIDByteArray, 0, static_cast<jsize>(cppValue.messageID.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.messageID.data()));
            value_messageID = value_messageIDByteArray;

            jobject value_responseID;
            if (!cppValue.responseID.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_responseID);
            }
            else
            {
                jobject value_responseIDInsideOptional;
                if (cppValue.responseID.Value().IsNull())
                {
                    value_responseIDInsideOptional = nullptr;
                }
                else
                {
                    std::string value_responseIDInsideOptionalClassName     = "java/lang/Long";
                    std::string value_responseIDInsideOptionalCtorSignature = "(J)V";
                    jlong jnivalue_responseIDInsideOptional = static_cast<jlong>(cppValue.responseID.Value().Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_responseIDInsideOptionalClassName.c_str(), value_responseIDInsideOptionalCtorSignature.c_str(),
                        jnivalue_responseIDInsideOptional, value_responseIDInsideOptional);
                }
                chip::JniReferences::GetInstance().CreateOptional(value_responseIDInsideOptional, value_responseID);
            }

            jobject value_reply;
            if (!cppValue.reply.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_reply);
            }
            else
            {
                jobject value_replyInsideOptional;
                if (cppValue.reply.Value().IsNull())
                {
                    value_replyInsideOptional = nullptr;
                }
                else
                {
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.reply.Value().Value(),
                                                                                         value_replyInsideOptional));
                }
                chip::JniReferences::GetInstance().CreateOptional(value_replyInsideOptional, value_reply);
            }

            jobject value_futureMessagesPreference;
            if (cppValue.futureMessagesPreference.IsNull())
            {
                value_futureMessagesPreference = nullptr;
            }
            else
            {
                std::string value_futureMessagesPreferenceClassName     = "java/lang/Integer";
                std::string value_futureMessagesPreferenceCtorSignature = "(I)V";
                jint jnivalue_futureMessagesPreference = static_cast<jint>(cppValue.futureMessagesPreference.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_futureMessagesPreferenceClassName.c_str(), value_futureMessagesPreferenceCtorSignature.c_str(),
                    jnivalue_futureMessagesPreference, value_futureMessagesPreference);
            }

            jclass messageCompleteStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$MessagesClusterMessageCompleteEvent", messageCompleteStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$MessagesClusterMessageCompleteEvent");
                return nullptr;
            }

            jmethodID messageCompleteStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, messageCompleteStructClass, "<init>",
                                                                "([BLjava/util/Optional;Ljava/util/Optional;Ljava/lang/Integer;)V",
                                                                &messageCompleteStructCtor);
            if (err != CHIP_NO_ERROR || messageCompleteStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$MessagesClusterMessageCompleteEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(messageCompleteStructClass, messageCompleteStructCtor, value_messageID, value_responseID,
                                           value_reply, value_futureMessagesPreference);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DeviceEnergyManagement::Id: {
        using namespace app::Clusters::DeviceEnergyManagement;
        switch (aPath.mEventId)
        {
        case Events::PowerAdjustStart::Id: {
            Events::PowerAdjustStart::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass powerAdjustStartStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DeviceEnergyManagementClusterPowerAdjustStartEvent",
                powerAdjustStartStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DeviceEnergyManagementClusterPowerAdjustStartEvent");
                return nullptr;
            }

            jmethodID powerAdjustStartStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, powerAdjustStartStructClass, "<init>", "()V",
                                                                &powerAdjustStartStructCtor);
            if (err != CHIP_NO_ERROR || powerAdjustStartStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DeviceEnergyManagementClusterPowerAdjustStartEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(powerAdjustStartStructClass, powerAdjustStartStructCtor);

            return value;
        }
        case Events::PowerAdjustEnd::Id: {
            Events::PowerAdjustEnd::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_cause;
            std::string value_causeClassName     = "java/lang/Integer";
            std::string value_causeCtorSignature = "(I)V";
            jint jnivalue_cause                  = static_cast<jint>(cppValue.cause);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_causeClassName.c_str(), value_causeCtorSignature.c_str(), jnivalue_cause, value_cause);

            jobject value_duration;
            std::string value_durationClassName     = "java/lang/Long";
            std::string value_durationCtorSignature = "(J)V";
            jlong jnivalue_duration                 = static_cast<jlong>(cppValue.duration);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_durationClassName.c_str(), value_durationCtorSignature.c_str(), jnivalue_duration, value_duration);

            jobject value_energyUse;
            std::string value_energyUseClassName     = "java/lang/Long";
            std::string value_energyUseCtorSignature = "(J)V";
            jlong jnivalue_energyUse                 = static_cast<jlong>(cppValue.energyUse);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_energyUseClassName.c_str(), value_energyUseCtorSignature.c_str(), jnivalue_energyUse, value_energyUse);

            jclass powerAdjustEndStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DeviceEnergyManagementClusterPowerAdjustEndEvent",
                powerAdjustEndStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DeviceEnergyManagementClusterPowerAdjustEndEvent");
                return nullptr;
            }

            jmethodID powerAdjustEndStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, powerAdjustEndStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;)V",
                                                                &powerAdjustEndStructCtor);
            if (err != CHIP_NO_ERROR || powerAdjustEndStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DeviceEnergyManagementClusterPowerAdjustEndEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(powerAdjustEndStructClass, powerAdjustEndStructCtor, value_cause, value_duration, value_energyUse);

            return value;
        }
        case Events::Paused::Id: {
            Events::Paused::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jclass pausedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DeviceEnergyManagementClusterPausedEvent", pausedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DeviceEnergyManagementClusterPausedEvent");
                return nullptr;
            }

            jmethodID pausedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, pausedStructClass, "<init>", "()V", &pausedStructCtor);
            if (err != CHIP_NO_ERROR || pausedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DeviceEnergyManagementClusterPausedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(pausedStructClass, pausedStructCtor);

            return value;
        }
        case Events::Resumed::Id: {
            Events::Resumed::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_cause;
            std::string value_causeClassName     = "java/lang/Integer";
            std::string value_causeCtorSignature = "(I)V";
            jint jnivalue_cause                  = static_cast<jint>(cppValue.cause);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_causeClassName.c_str(), value_causeCtorSignature.c_str(), jnivalue_cause, value_cause);

            jclass resumedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DeviceEnergyManagementClusterResumedEvent", resumedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DeviceEnergyManagementClusterResumedEvent");
                return nullptr;
            }

            jmethodID resumedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, resumedStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &resumedStructCtor);
            if (err != CHIP_NO_ERROR || resumedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$DeviceEnergyManagementClusterResumedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(resumedStructClass, resumedStructCtor, value_cause);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::EnergyEvse::Id: {
        using namespace app::Clusters::EnergyEvse;
        switch (aPath.mEventId)
        {
        case Events::EVConnected::Id: {
            Events::EVConnected::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_sessionID;
            std::string value_sessionIDClassName     = "java/lang/Long";
            std::string value_sessionIDCtorSignature = "(J)V";
            jlong jnivalue_sessionID                 = static_cast<jlong>(cppValue.sessionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_sessionIDClassName.c_str(), value_sessionIDCtorSignature.c_str(), jnivalue_sessionID, value_sessionID);

            jclass EVConnectedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$EnergyEvseClusterEVConnectedEvent", EVConnectedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$EnergyEvseClusterEVConnectedEvent");
                return nullptr;
            }

            jmethodID EVConnectedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, EVConnectedStructClass, "<init>", "(Ljava/lang/Long;)V",
                                                                &EVConnectedStructCtor);
            if (err != CHIP_NO_ERROR || EVConnectedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$EnergyEvseClusterEVConnectedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(EVConnectedStructClass, EVConnectedStructCtor, value_sessionID);

            return value;
        }
        case Events::EVNotDetected::Id: {
            Events::EVNotDetected::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_sessionID;
            std::string value_sessionIDClassName     = "java/lang/Long";
            std::string value_sessionIDCtorSignature = "(J)V";
            jlong jnivalue_sessionID                 = static_cast<jlong>(cppValue.sessionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_sessionIDClassName.c_str(), value_sessionIDCtorSignature.c_str(), jnivalue_sessionID, value_sessionID);

            jobject value_state;
            std::string value_stateClassName     = "java/lang/Integer";
            std::string value_stateCtorSignature = "(I)V";
            jint jnivalue_state                  = static_cast<jint>(cppValue.state);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_stateClassName.c_str(), value_stateCtorSignature.c_str(), jnivalue_state, value_state);

            jobject value_sessionDuration;
            std::string value_sessionDurationClassName     = "java/lang/Long";
            std::string value_sessionDurationCtorSignature = "(J)V";
            jlong jnivalue_sessionDuration                 = static_cast<jlong>(cppValue.sessionDuration);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_sessionDurationClassName.c_str(),
                                                                        value_sessionDurationCtorSignature.c_str(),
                                                                        jnivalue_sessionDuration, value_sessionDuration);

            jobject value_sessionEnergyCharged;
            std::string value_sessionEnergyChargedClassName     = "java/lang/Long";
            std::string value_sessionEnergyChargedCtorSignature = "(J)V";
            jlong jnivalue_sessionEnergyCharged                 = static_cast<jlong>(cppValue.sessionEnergyCharged);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_sessionEnergyChargedClassName.c_str(),
                                                                        value_sessionEnergyChargedCtorSignature.c_str(),
                                                                        jnivalue_sessionEnergyCharged, value_sessionEnergyCharged);

            jobject value_sessionEnergyDischarged;
            if (!cppValue.sessionEnergyDischarged.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_sessionEnergyDischarged);
            }
            else
            {
                jobject value_sessionEnergyDischargedInsideOptional;
                std::string value_sessionEnergyDischargedInsideOptionalClassName     = "java/lang/Long";
                std::string value_sessionEnergyDischargedInsideOptionalCtorSignature = "(J)V";
                jlong jnivalue_sessionEnergyDischargedInsideOptional = static_cast<jlong>(cppValue.sessionEnergyDischarged.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_sessionEnergyDischargedInsideOptionalClassName.c_str(),
                    value_sessionEnergyDischargedInsideOptionalCtorSignature.c_str(),
                    jnivalue_sessionEnergyDischargedInsideOptional, value_sessionEnergyDischargedInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_sessionEnergyDischargedInsideOptional,
                                                                  value_sessionEnergyDischarged);
            }

            jclass EVNotDetectedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$EnergyEvseClusterEVNotDetectedEvent", EVNotDetectedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$EnergyEvseClusterEVNotDetectedEvent");
                return nullptr;
            }

            jmethodID EVNotDetectedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, EVNotDetectedStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;Ljava/util/Optional;)V",
                &EVNotDetectedStructCtor);
            if (err != CHIP_NO_ERROR || EVNotDetectedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$EnergyEvseClusterEVNotDetectedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(EVNotDetectedStructClass, EVNotDetectedStructCtor, value_sessionID, value_state,
                                           value_sessionDuration, value_sessionEnergyCharged, value_sessionEnergyDischarged);

            return value;
        }
        case Events::EnergyTransferStarted::Id: {
            Events::EnergyTransferStarted::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_sessionID;
            std::string value_sessionIDClassName     = "java/lang/Long";
            std::string value_sessionIDCtorSignature = "(J)V";
            jlong jnivalue_sessionID                 = static_cast<jlong>(cppValue.sessionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_sessionIDClassName.c_str(), value_sessionIDCtorSignature.c_str(), jnivalue_sessionID, value_sessionID);

            jobject value_state;
            std::string value_stateClassName     = "java/lang/Integer";
            std::string value_stateCtorSignature = "(I)V";
            jint jnivalue_state                  = static_cast<jint>(cppValue.state);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_stateClassName.c_str(), value_stateCtorSignature.c_str(), jnivalue_state, value_state);

            jobject value_maximumCurrent;
            std::string value_maximumCurrentClassName     = "java/lang/Long";
            std::string value_maximumCurrentCtorSignature = "(J)V";
            jlong jnivalue_maximumCurrent                 = static_cast<jlong>(cppValue.maximumCurrent);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_maximumCurrentClassName.c_str(),
                                                                        value_maximumCurrentCtorSignature.c_str(),
                                                                        jnivalue_maximumCurrent, value_maximumCurrent);

            jobject value_maximumDischargeCurrent;
            if (!cppValue.maximumDischargeCurrent.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_maximumDischargeCurrent);
            }
            else
            {
                jobject value_maximumDischargeCurrentInsideOptional;
                std::string value_maximumDischargeCurrentInsideOptionalClassName     = "java/lang/Long";
                std::string value_maximumDischargeCurrentInsideOptionalCtorSignature = "(J)V";
                jlong jnivalue_maximumDischargeCurrentInsideOptional = static_cast<jlong>(cppValue.maximumDischargeCurrent.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_maximumDischargeCurrentInsideOptionalClassName.c_str(),
                    value_maximumDischargeCurrentInsideOptionalCtorSignature.c_str(),
                    jnivalue_maximumDischargeCurrentInsideOptional, value_maximumDischargeCurrentInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_maximumDischargeCurrentInsideOptional,
                                                                  value_maximumDischargeCurrent);
            }

            jclass energyTransferStartedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$EnergyEvseClusterEnergyTransferStartedEvent",
                energyTransferStartedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$EnergyEvseClusterEnergyTransferStartedEvent");
                return nullptr;
            }

            jmethodID energyTransferStartedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, energyTransferStartedStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;Ljava/util/Optional;)V", &energyTransferStartedStructCtor);
            if (err != CHIP_NO_ERROR || energyTransferStartedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$EnergyEvseClusterEnergyTransferStartedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(energyTransferStartedStructClass, energyTransferStartedStructCtor, value_sessionID,
                                           value_state, value_maximumCurrent, value_maximumDischargeCurrent);

            return value;
        }
        case Events::EnergyTransferStopped::Id: {
            Events::EnergyTransferStopped::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_sessionID;
            std::string value_sessionIDClassName     = "java/lang/Long";
            std::string value_sessionIDCtorSignature = "(J)V";
            jlong jnivalue_sessionID                 = static_cast<jlong>(cppValue.sessionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_sessionIDClassName.c_str(), value_sessionIDCtorSignature.c_str(), jnivalue_sessionID, value_sessionID);

            jobject value_state;
            std::string value_stateClassName     = "java/lang/Integer";
            std::string value_stateCtorSignature = "(I)V";
            jint jnivalue_state                  = static_cast<jint>(cppValue.state);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_stateClassName.c_str(), value_stateCtorSignature.c_str(), jnivalue_state, value_state);

            jobject value_reason;
            std::string value_reasonClassName     = "java/lang/Integer";
            std::string value_reasonCtorSignature = "(I)V";
            jint jnivalue_reason                  = static_cast<jint>(cppValue.reason);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_reasonClassName.c_str(), value_reasonCtorSignature.c_str(), jnivalue_reason, value_reason);

            jobject value_energyTransferred;
            std::string value_energyTransferredClassName     = "java/lang/Long";
            std::string value_energyTransferredCtorSignature = "(J)V";
            jlong jnivalue_energyTransferred                 = static_cast<jlong>(cppValue.energyTransferred);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_energyTransferredClassName.c_str(),
                                                                        value_energyTransferredCtorSignature.c_str(),
                                                                        jnivalue_energyTransferred, value_energyTransferred);

            jobject value_energyDischarged;
            if (!cppValue.energyDischarged.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_energyDischarged);
            }
            else
            {
                jobject value_energyDischargedInsideOptional;
                std::string value_energyDischargedInsideOptionalClassName     = "java/lang/Long";
                std::string value_energyDischargedInsideOptionalCtorSignature = "(J)V";
                jlong jnivalue_energyDischargedInsideOptional = static_cast<jlong>(cppValue.energyDischarged.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_energyDischargedInsideOptionalClassName.c_str(),
                    value_energyDischargedInsideOptionalCtorSignature.c_str(), jnivalue_energyDischargedInsideOptional,
                    value_energyDischargedInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_energyDischargedInsideOptional, value_energyDischarged);
            }

            jclass energyTransferStoppedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$EnergyEvseClusterEnergyTransferStoppedEvent",
                energyTransferStoppedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$EnergyEvseClusterEnergyTransferStoppedEvent");
                return nullptr;
            }

            jmethodID energyTransferStoppedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, energyTransferStoppedStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Long;Ljava/util/Optional;)V",
                &energyTransferStoppedStructCtor);
            if (err != CHIP_NO_ERROR || energyTransferStoppedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$EnergyEvseClusterEnergyTransferStoppedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(energyTransferStoppedStructClass, energyTransferStoppedStructCtor, value_sessionID,
                                           value_state, value_reason, value_energyTransferred, value_energyDischarged);

            return value;
        }
        case Events::Fault::Id: {
            Events::Fault::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_sessionID;
            if (cppValue.sessionID.IsNull())
            {
                value_sessionID = nullptr;
            }
            else
            {
                std::string value_sessionIDClassName     = "java/lang/Long";
                std::string value_sessionIDCtorSignature = "(J)V";
                jlong jnivalue_sessionID                 = static_cast<jlong>(cppValue.sessionID.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_sessionIDClassName.c_str(), value_sessionIDCtorSignature.c_str(), jnivalue_sessionID, value_sessionID);
            }

            jobject value_state;
            std::string value_stateClassName     = "java/lang/Integer";
            std::string value_stateCtorSignature = "(I)V";
            jint jnivalue_state                  = static_cast<jint>(cppValue.state);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_stateClassName.c_str(), value_stateCtorSignature.c_str(), jnivalue_state, value_state);

            jobject value_faultStatePreviousState;
            std::string value_faultStatePreviousStateClassName     = "java/lang/Integer";
            std::string value_faultStatePreviousStateCtorSignature = "(I)V";
            jint jnivalue_faultStatePreviousState                  = static_cast<jint>(cppValue.faultStatePreviousState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_faultStatePreviousStateClassName.c_str(), value_faultStatePreviousStateCtorSignature.c_str(),
                jnivalue_faultStatePreviousState, value_faultStatePreviousState);

            jobject value_faultStateCurrentState;
            std::string value_faultStateCurrentStateClassName     = "java/lang/Integer";
            std::string value_faultStateCurrentStateCtorSignature = "(I)V";
            jint jnivalue_faultStateCurrentState                  = static_cast<jint>(cppValue.faultStateCurrentState);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_faultStateCurrentStateClassName.c_str(), value_faultStateCurrentStateCtorSignature.c_str(),
                jnivalue_faultStateCurrentState, value_faultStateCurrentState);

            jclass faultStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$EnergyEvseClusterFaultEvent", faultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$EnergyEvseClusterFaultEvent");
                return nullptr;
            }

            jmethodID faultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, faultStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;)V",
                &faultStructCtor);
            if (err != CHIP_NO_ERROR || faultStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$EnergyEvseClusterFaultEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(faultStructClass, faultStructCtor, value_sessionID, value_state,
                                           value_faultStatePreviousState, value_faultStateCurrentState);

            return value;
        }
        case Events::Rfid::Id: {
            Events::Rfid::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_uid;
            jbyteArray value_uidByteArray = env->NewByteArray(static_cast<jsize>(cppValue.uid.size()));
            env->SetByteArrayRegion(value_uidByteArray, 0, static_cast<jsize>(cppValue.uid.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.uid.data()));
            value_uid = value_uidByteArray;

            jclass rfidStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$EnergyEvseClusterRfidEvent", rfidStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$EnergyEvseClusterRfidEvent");
                return nullptr;
            }

            jmethodID rfidStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, rfidStructClass, "<init>", "([B)V", &rfidStructCtor);
            if (err != CHIP_NO_ERROR || rfidStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$EnergyEvseClusterRfidEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(rfidStructClass, rfidStructCtor, value_uid);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::EnergyPreference::Id: {
        using namespace app::Clusters::EnergyPreference;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PowerTopology::Id: {
        using namespace app::Clusters::PowerTopology;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::EnergyEvseMode::Id: {
        using namespace app::Clusters::EnergyEvseMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::WaterHeaterMode::Id: {
        using namespace app::Clusters::WaterHeaterMode;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::DeviceEnergyManagementMode::Id: {
        using namespace app::Clusters::DeviceEnergyManagementMode;
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterDoorLockAlarmEvent", doorLockAlarmStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterDoorLockAlarmEvent");
                return nullptr;
            }

            jmethodID doorLockAlarmStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, doorLockAlarmStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &doorLockAlarmStructCtor);
            if (err != CHIP_NO_ERROR || doorLockAlarmStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterDoorStateChangeEvent", doorStateChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterDoorStateChangeEvent");
                return nullptr;
            }

            jmethodID doorStateChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, doorStateChangeStructClass, "<init>", "(Ljava/lang/Integer;)V",
                                                                &doorStateChangeStructCtor);
            if (err != CHIP_NO_ERROR || doorStateChangeStructCtor == nullptr)
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

                        {
                            jclass credentialStructStructClass_3;
                            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                                env, "chip/devicecontroller/ChipStructs$DoorLockClusterCredentialStruct",
                                credentialStructStructClass_3);
                            if (err != CHIP_NO_ERROR)
                            {
                                ChipLogError(Zcl, "Could not find class ChipStructs$DoorLockClusterCredentialStruct");
                                return nullptr;
                            }

                            jmethodID credentialStructStructCtor_3;
                            err = chip::JniReferences::GetInstance().FindMethod(env, credentialStructStructClass_3, "<init>",
                                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;)V",
                                                                                &credentialStructStructCtor_3);
                            if (err != CHIP_NO_ERROR || credentialStructStructCtor_3 == nullptr)
                            {
                                ChipLogError(Zcl, "Could not find ChipStructs$DoorLockClusterCredentialStruct constructor");
                                return nullptr;
                            }

                            newElement_2 = env->NewObject(credentialStructStructClass_3, credentialStructStructCtor_3,
                                                          newElement_2_credentialType, newElement_2_credentialIndex);
                        }
                        chip::JniReferences::GetInstance().AddToList(value_credentialsInsideOptional, newElement_2);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_credentialsInsideOptional, value_credentials);
            }

            jclass lockOperationStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterLockOperationEvent", lockOperationStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterLockOperationEvent");
                return nullptr;
            }

            jmethodID lockOperationStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, lockOperationStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/"
                                                                "lang/Integer;Ljava/lang/Long;Ljava/util/Optional;)V",
                                                                &lockOperationStructCtor);
            if (err != CHIP_NO_ERROR || lockOperationStructCtor == nullptr)
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

                        {
                            jclass credentialStructStructClass_3;
                            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                                env, "chip/devicecontroller/ChipStructs$DoorLockClusterCredentialStruct",
                                credentialStructStructClass_3);
                            if (err != CHIP_NO_ERROR)
                            {
                                ChipLogError(Zcl, "Could not find class ChipStructs$DoorLockClusterCredentialStruct");
                                return nullptr;
                            }

                            jmethodID credentialStructStructCtor_3;
                            err = chip::JniReferences::GetInstance().FindMethod(env, credentialStructStructClass_3, "<init>",
                                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;)V",
                                                                                &credentialStructStructCtor_3);
                            if (err != CHIP_NO_ERROR || credentialStructStructCtor_3 == nullptr)
                            {
                                ChipLogError(Zcl, "Could not find ChipStructs$DoorLockClusterCredentialStruct constructor");
                                return nullptr;
                            }

                            newElement_2 = env->NewObject(credentialStructStructClass_3, credentialStructStructCtor_3,
                                                          newElement_2_credentialType, newElement_2_credentialIndex);
                        }
                        chip::JniReferences::GetInstance().AddToList(value_credentialsInsideOptional, newElement_2);
                    }
                }
                chip::JniReferences::GetInstance().CreateOptional(value_credentialsInsideOptional, value_credentials);
            }

            jclass lockOperationErrorStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterLockOperationErrorEvent",
                lockOperationErrorStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterLockOperationErrorEvent");
                return nullptr;
            }

            jmethodID lockOperationErrorStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, lockOperationErrorStructClass, "<init>",
                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                "Long;Ljava/util/Optional;)V",
                &lockOperationErrorStructCtor);
            if (err != CHIP_NO_ERROR || lockOperationErrorStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$DoorLockClusterLockUserChangeEvent", lockUserChangeStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$DoorLockClusterLockUserChangeEvent");
                return nullptr;
            }

            jmethodID lockUserChangeStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, lockUserChangeStructClass, "<init>",
                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                "Long;Ljava/lang/Integer;)V",
                &lockUserChangeStructCtor);
            if (err != CHIP_NO_ERROR || lockUserChangeStructCtor == nullptr)
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
    case app::Clusters::ClosureControl::Id: {
        using namespace app::Clusters::ClosureControl;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ServiceArea::Id: {
        using namespace app::Clusters::ServiceArea;
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageLowEvent",
                supplyVoltageLowStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageLowEvent");
                return nullptr;
            }

            jmethodID supplyVoltageLowStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, supplyVoltageLowStructClass, "<init>", "()V",
                                                                &supplyVoltageLowStructCtor);
            if (err != CHIP_NO_ERROR || supplyVoltageLowStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageHighEvent",
                supplyVoltageHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSupplyVoltageHighEvent");
                return nullptr;
            }

            jmethodID supplyVoltageHighStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, supplyVoltageHighStructClass, "<init>", "()V",
                                                                &supplyVoltageHighStructCtor);
            if (err != CHIP_NO_ERROR || supplyVoltageHighStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterPowerMissingPhaseEvent",
                powerMissingPhaseStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterPowerMissingPhaseEvent");
                return nullptr;
            }

            jmethodID powerMissingPhaseStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, powerMissingPhaseStructClass, "<init>", "()V",
                                                                &powerMissingPhaseStructCtor);
            if (err != CHIP_NO_ERROR || powerMissingPhaseStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureLowEvent",
                systemPressureLowStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureLowEvent");
                return nullptr;
            }

            jmethodID systemPressureLowStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, systemPressureLowStructClass, "<init>", "()V",
                                                                &systemPressureLowStructCtor);
            if (err != CHIP_NO_ERROR || systemPressureLowStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureHighEvent",
                systemPressureHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSystemPressureHighEvent");
                return nullptr;
            }

            jmethodID systemPressureHighStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, systemPressureHighStructClass, "<init>", "()V",
                                                                &systemPressureHighStructCtor);
            if (err != CHIP_NO_ERROR || systemPressureHighStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterDryRunningEvent",
                dryRunningStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterDryRunningEvent");
                return nullptr;
            }

            jmethodID dryRunningStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, dryRunningStructClass, "<init>", "()V", &dryRunningStructCtor);
            if (err != CHIP_NO_ERROR || dryRunningStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterMotorTemperatureHighEvent",
                motorTemperatureHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterMotorTemperatureHighEvent");
                return nullptr;
            }

            jmethodID motorTemperatureHighStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, motorTemperatureHighStructClass, "<init>", "()V",
                                                                &motorTemperatureHighStructCtor);
            if (err != CHIP_NO_ERROR || motorTemperatureHighStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent",
                pumpMotorFatalFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent");
                return nullptr;
            }

            jmethodID pumpMotorFatalFailureStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, pumpMotorFatalFailureStructClass, "<init>", "()V",
                                                                &pumpMotorFatalFailureStructCtor);
            if (err != CHIP_NO_ERROR || pumpMotorFatalFailureStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterElectronicTemperatureHighEvent",
                electronicTemperatureHighStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterElectronicTemperatureHighEvent");
                return nullptr;
            }

            jmethodID electronicTemperatureHighStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, electronicTemperatureHighStructClass, "<init>", "()V",
                                                                &electronicTemperatureHighStructCtor);
            if (err != CHIP_NO_ERROR || electronicTemperatureHighStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterPumpBlockedEvent",
                pumpBlockedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterPumpBlockedEvent");
                return nullptr;
            }

            jmethodID pumpBlockedStructCtor;
            err =
                chip::JniReferences::GetInstance().FindMethod(env, pumpBlockedStructClass, "<init>", "()V", &pumpBlockedStructCtor);
            if (err != CHIP_NO_ERROR || pumpBlockedStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterSensorFailureEvent",
                sensorFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterSensorFailureEvent");
                return nullptr;
            }

            jmethodID sensorFailureStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, sensorFailureStructClass, "<init>", "()V",
                                                                &sensorFailureStructCtor);
            if (err != CHIP_NO_ERROR || sensorFailureStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent",
                electronicNonFatalFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(
                    Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent");
                return nullptr;
            }

            jmethodID electronicNonFatalFailureStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, electronicNonFatalFailureStructClass, "<init>", "()V",
                                                                &electronicNonFatalFailureStructCtor);
            if (err != CHIP_NO_ERROR || electronicNonFatalFailureStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterElectronicFatalFailureEvent",
                electronicFatalFailureStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterElectronicFatalFailureEvent");
                return nullptr;
            }

            jmethodID electronicFatalFailureStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, electronicFatalFailureStructClass, "<init>", "()V",
                                                                &electronicFatalFailureStructCtor);
            if (err != CHIP_NO_ERROR || electronicFatalFailureStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterGeneralFaultEvent",
                generalFaultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterGeneralFaultEvent");
                return nullptr;
            }

            jmethodID generalFaultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, generalFaultStructClass, "<init>", "()V",
                                                                &generalFaultStructCtor);
            if (err != CHIP_NO_ERROR || generalFaultStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterLeakageEvent", leakageStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterLeakageEvent");
                return nullptr;
            }

            jmethodID leakageStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, leakageStructClass, "<init>", "()V", &leakageStructCtor);
            if (err != CHIP_NO_ERROR || leakageStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterAirDetectionEvent",
                airDetectionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterAirDetectionEvent");
                return nullptr;
            }

            jmethodID airDetectionStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, airDetectionStructClass, "<init>", "()V",
                                                                &airDetectionStructCtor);
            if (err != CHIP_NO_ERROR || airDetectionStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PumpConfigurationAndControlClusterTurbineOperationEvent",
                turbineOperationStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PumpConfigurationAndControlClusterTurbineOperationEvent");
                return nullptr;
            }

            jmethodID turbineOperationStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, turbineOperationStructClass, "<init>", "()V",
                                                                &turbineOperationStructCtor);
            if (err != CHIP_NO_ERROR || turbineOperationStructCtor == nullptr)
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
        case Events::OccupancyChanged::Id: {
            Events::OccupancyChanged::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_occupancy;
            std::string value_occupancyClassName     = "java/lang/Integer";
            std::string value_occupancyCtorSignature = "(I)V";
            jint jnivalue_occupancy                  = static_cast<jint>(cppValue.occupancy.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_occupancyClassName.c_str(), value_occupancyCtorSignature.c_str(), jnivalue_occupancy, value_occupancy);

            jclass occupancyChangedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$OccupancySensingClusterOccupancyChangedEvent",
                occupancyChangedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$OccupancySensingClusterOccupancyChangedEvent");
                return nullptr;
            }

            jmethodID occupancyChangedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, occupancyChangedStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &occupancyChangedStructCtor);
            if (err != CHIP_NO_ERROR || occupancyChangedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$OccupancySensingClusterOccupancyChangedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(occupancyChangedStructClass, occupancyChangedStructCtor, value_occupancy);

            return value;
        }
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
    case app::Clusters::WiFiNetworkManagement::Id: {
        using namespace app::Clusters::WiFiNetworkManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ThreadBorderRouterManagement::Id: {
        using namespace app::Clusters::ThreadBorderRouterManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::ThreadNetworkDirectory::Id: {
        using namespace app::Clusters::ThreadNetworkDirectory;
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
        case Events::TargetUpdated::Id: {
            Events::TargetUpdated::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_targetList;
            chip::JniReferences::GetInstance().CreateArrayList(value_targetList);

            auto iter_value_targetList_0 = cppValue.targetList.begin();
            while (iter_value_targetList_0.Next())
            {
                auto & entry_0 = iter_value_targetList_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_identifier;
                std::string newElement_0_identifierClassName     = "java/lang/Integer";
                std::string newElement_0_identifierCtorSignature = "(I)V";
                jint jninewElement_0_identifier                  = static_cast<jint>(entry_0.identifier);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_identifierClassName.c_str(),
                                                                           newElement_0_identifierCtorSignature.c_str(),
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
                chip::JniReferences::GetInstance().AddToList(value_targetList, newElement_0);
            }

            jobject value_currentTarget;
            std::string value_currentTargetClassName     = "java/lang/Integer";
            std::string value_currentTargetCtorSignature = "(I)V";
            jint jnivalue_currentTarget                  = static_cast<jint>(cppValue.currentTarget);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_currentTargetClassName.c_str(),
                                                                       value_currentTargetCtorSignature.c_str(),
                                                                       jnivalue_currentTarget, value_currentTarget);

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
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_currentStateClassName.c_str(),
                                                                       value_currentStateCtorSignature.c_str(),
                                                                       jnivalue_currentState, value_currentState);

            jobject value_startTime;
            std::string value_startTimeClassName     = "java/lang/Long";
            std::string value_startTimeCtorSignature = "(J)V";
            jlong jnivalue_startTime                 = static_cast<jlong>(cppValue.startTime);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_startTimeClassName.c_str(), value_startTimeCtorSignature.c_str(), jnivalue_startTime, value_startTime);

            jobject value_duration;
            std::string value_durationClassName     = "java/lang/Long";
            std::string value_durationCtorSignature = "(J)V";
            jlong jnivalue_duration                 = static_cast<jlong>(cppValue.duration);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_durationClassName.c_str(), value_durationCtorSignature.c_str(), jnivalue_duration, value_duration);

            jobject value_sampledPosition;
            jobject value_sampledPosition_updatedAt;
            std::string value_sampledPosition_updatedAtClassName     = "java/lang/Long";
            std::string value_sampledPosition_updatedAtCtorSignature = "(J)V";
            jlong jnivalue_sampledPosition_updatedAt                 = static_cast<jlong>(cppValue.sampledPosition.updatedAt);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
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
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
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
            chip::JniReferences::GetInstance().CreateBoxedObject<jfloat>(value_playbackSpeedClassName.c_str(),
                                                                         value_playbackSpeedCtorSignature.c_str(),
                                                                         jnivalue_playbackSpeed, value_playbackSpeed);

            jobject value_seekRangeEnd;
            std::string value_seekRangeEndClassName     = "java/lang/Long";
            std::string value_seekRangeEndCtorSignature = "(J)V";
            jlong jnivalue_seekRangeEnd                 = static_cast<jlong>(cppValue.seekRangeEnd);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_seekRangeEndClassName.c_str(),
                                                                        value_seekRangeEndCtorSignature.c_str(),
                                                                        jnivalue_seekRangeEnd, value_seekRangeEnd);

            jobject value_seekRangeStart;
            std::string value_seekRangeStartClassName     = "java/lang/Long";
            std::string value_seekRangeStartCtorSignature = "(J)V";
            jlong jnivalue_seekRangeStart                 = static_cast<jlong>(cppValue.seekRangeStart);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_seekRangeStartClassName.c_str(),
                                                                        value_seekRangeStartCtorSignature.c_str(),
                                                                        jnivalue_seekRangeStart, value_seekRangeStart);

            jobject value_data;
            if (!cppValue.data.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_data);
            }
            else
            {
                jobject value_dataInsideOptional;
                jbyteArray value_dataInsideOptionalByteArray = env->NewByteArray(static_cast<jsize>(cppValue.data.Value().size()));
                env->SetByteArrayRegion(value_dataInsideOptionalByteArray, 0, static_cast<jsize>(cppValue.data.Value().size()),
                                        reinterpret_cast<const jbyte *>(cppValue.data.Value().data()));
                value_dataInsideOptional = value_dataInsideOptionalByteArray;
                chip::JniReferences::GetInstance().CreateOptional(value_dataInsideOptional, value_data);
            }

            jobject value_audioAdvanceUnmuted;
            std::string value_audioAdvanceUnmutedClassName     = "java/lang/Boolean";
            std::string value_audioAdvanceUnmutedCtorSignature = "(Z)V";
            jboolean jnivalue_audioAdvanceUnmuted              = static_cast<jboolean>(cppValue.audioAdvanceUnmuted);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(value_audioAdvanceUnmutedClassName.c_str(),
                                                                           value_audioAdvanceUnmutedCtorSignature.c_str(),
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
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_node);
            }
            else
            {
                jobject value_nodeInsideOptional;
                std::string value_nodeInsideOptionalClassName     = "java/lang/Long";
                std::string value_nodeInsideOptionalCtorSignature = "(J)V";
                jlong jnivalue_nodeInsideOptional                 = static_cast<jlong>(cppValue.node.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_nodeInsideOptionalClassName.c_str(),
                                                                            value_nodeInsideOptionalCtorSignature.c_str(),
                                                                            jnivalue_nodeInsideOptional, value_nodeInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_nodeInsideOptional, value_node);
            }

            jclass loggedOutStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$AccountLoginClusterLoggedOutEvent", loggedOutStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$AccountLoginClusterLoggedOutEvent");
                return nullptr;
            }

            jmethodID loggedOutStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, loggedOutStructClass, "<init>", "(Ljava/util/Optional;)V",
                                                                &loggedOutStructCtor);
            if (err != CHIP_NO_ERROR || loggedOutStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$AccountLoginClusterLoggedOutEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(loggedOutStructClass, loggedOutStructCtor, value_node);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
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
    case app::Clusters::ZoneManagement::Id: {
        using namespace app::Clusters::ZoneManagement;
        switch (aPath.mEventId)
        {
        case Events::ZoneTriggered::Id: {
            Events::ZoneTriggered::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_zones;
            chip::JniReferences::GetInstance().CreateArrayList(value_zones);

            auto iter_value_zones_0 = cppValue.zones.begin();
            while (iter_value_zones_0.Next())
            {
                auto & entry_0 = iter_value_zones_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_zones, newElement_0);
            }

            jobject value_reason;
            std::string value_reasonClassName     = "java/lang/Integer";
            std::string value_reasonCtorSignature = "(I)V";
            jint jnivalue_reason                  = static_cast<jint>(cppValue.reason);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_reasonClassName.c_str(), value_reasonCtorSignature.c_str(), jnivalue_reason, value_reason);

            jclass zoneTriggeredStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ZoneManagementClusterZoneTriggeredEvent", zoneTriggeredStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ZoneManagementClusterZoneTriggeredEvent");
                return nullptr;
            }

            jmethodID zoneTriggeredStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, zoneTriggeredStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/lang/Integer;)V", &zoneTriggeredStructCtor);
            if (err != CHIP_NO_ERROR || zoneTriggeredStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$ZoneManagementClusterZoneTriggeredEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(zoneTriggeredStructClass, zoneTriggeredStructCtor, value_zones, value_reason);

            return value;
        }
        case Events::ZoneStopped::Id: {
            Events::ZoneStopped::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_zones;
            chip::JniReferences::GetInstance().CreateArrayList(value_zones);

            auto iter_value_zones_0 = cppValue.zones.begin();
            while (iter_value_zones_0.Next())
            {
                auto & entry_0 = iter_value_zones_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value_zones, newElement_0);
            }

            jobject value_reason;
            std::string value_reasonClassName     = "java/lang/Integer";
            std::string value_reasonCtorSignature = "(I)V";
            jint jnivalue_reason                  = static_cast<jint>(cppValue.reason);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_reasonClassName.c_str(), value_reasonCtorSignature.c_str(), jnivalue_reason, value_reason);

            jclass zoneStoppedStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$ZoneManagementClusterZoneStoppedEvent", zoneStoppedStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$ZoneManagementClusterZoneStoppedEvent");
                return nullptr;
            }

            jmethodID zoneStoppedStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, zoneStoppedStructClass, "<init>", "(Ljava/util/ArrayList;Ljava/lang/Integer;)V", &zoneStoppedStructCtor);
            if (err != CHIP_NO_ERROR || zoneStoppedStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$ZoneManagementClusterZoneStoppedEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(zoneStoppedStructClass, zoneStoppedStructCtor, value_zones, value_reason);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::CameraAvStreamManagement::Id: {
        using namespace app::Clusters::CameraAvStreamManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::CameraAvSettingsUserLevelManagement::Id: {
        using namespace app::Clusters::CameraAvSettingsUserLevelManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::WebRTCTransportProvider::Id: {
        using namespace app::Clusters::WebRTCTransportProvider;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::WebRTCTransportRequestor::Id: {
        using namespace app::Clusters::WebRTCTransportRequestor;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::PushAvStreamTransport::Id: {
        using namespace app::Clusters::PushAvStreamTransport;
        switch (aPath.mEventId)
        {
        case Events::PushTransportBegin::Id: {
            Events::PushTransportBegin::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_connectionID;
            std::string value_connectionIDClassName     = "java/lang/Integer";
            std::string value_connectionIDCtorSignature = "(I)V";
            jint jnivalue_connectionID                  = static_cast<jint>(cppValue.connectionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_connectionIDClassName.c_str(),
                                                                       value_connectionIDCtorSignature.c_str(),
                                                                       jnivalue_connectionID, value_connectionID);

            jobject value_triggerType;
            std::string value_triggerTypeClassName     = "java/lang/Integer";
            std::string value_triggerTypeCtorSignature = "(I)V";
            jint jnivalue_triggerType                  = static_cast<jint>(cppValue.triggerType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_triggerTypeClassName.c_str(),
                                                                       value_triggerTypeCtorSignature.c_str(), jnivalue_triggerType,
                                                                       value_triggerType);

            jobject value_activationReason;
            if (!cppValue.activationReason.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_activationReason);
            }
            else
            {
                jobject value_activationReasonInsideOptional;
                std::string value_activationReasonInsideOptionalClassName     = "java/lang/Integer";
                std::string value_activationReasonInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_activationReasonInsideOptional = static_cast<jint>(cppValue.activationReason.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_activationReasonInsideOptionalClassName.c_str(),
                    value_activationReasonInsideOptionalCtorSignature.c_str(), jnivalue_activationReasonInsideOptional,
                    value_activationReasonInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_activationReasonInsideOptional, value_activationReason);
            }

            jclass pushTransportBeginStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PushAvStreamTransportClusterPushTransportBeginEvent",
                pushTransportBeginStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PushAvStreamTransportClusterPushTransportBeginEvent");
                return nullptr;
            }

            jmethodID pushTransportBeginStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, pushTransportBeginStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;)V",
                                                                &pushTransportBeginStructCtor);
            if (err != CHIP_NO_ERROR || pushTransportBeginStructCtor == nullptr)
            {
                ChipLogError(Zcl,
                             "Could not find ChipEventStructs$PushAvStreamTransportClusterPushTransportBeginEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(pushTransportBeginStructClass, pushTransportBeginStructCtor, value_connectionID,
                                           value_triggerType, value_activationReason);

            return value;
        }
        case Events::PushTransportEnd::Id: {
            Events::PushTransportEnd::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_connectionID;
            std::string value_connectionIDClassName     = "java/lang/Integer";
            std::string value_connectionIDCtorSignature = "(I)V";
            jint jnivalue_connectionID                  = static_cast<jint>(cppValue.connectionID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_connectionIDClassName.c_str(),
                                                                       value_connectionIDCtorSignature.c_str(),
                                                                       jnivalue_connectionID, value_connectionID);

            jobject value_triggerType;
            std::string value_triggerTypeClassName     = "java/lang/Integer";
            std::string value_triggerTypeCtorSignature = "(I)V";
            jint jnivalue_triggerType                  = static_cast<jint>(cppValue.triggerType);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_triggerTypeClassName.c_str(),
                                                                       value_triggerTypeCtorSignature.c_str(), jnivalue_triggerType,
                                                                       value_triggerType);

            jobject value_activationReason;
            if (!cppValue.activationReason.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_activationReason);
            }
            else
            {
                jobject value_activationReasonInsideOptional;
                std::string value_activationReasonInsideOptionalClassName     = "java/lang/Integer";
                std::string value_activationReasonInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_activationReasonInsideOptional = static_cast<jint>(cppValue.activationReason.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_activationReasonInsideOptionalClassName.c_str(),
                    value_activationReasonInsideOptionalCtorSignature.c_str(), jnivalue_activationReasonInsideOptional,
                    value_activationReasonInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_activationReasonInsideOptional, value_activationReason);
            }

            jclass pushTransportEndStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$PushAvStreamTransportClusterPushTransportEndEvent",
                pushTransportEndStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$PushAvStreamTransportClusterPushTransportEndEvent");
                return nullptr;
            }

            jmethodID pushTransportEndStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, pushTransportEndStructClass, "<init>",
                                                                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;)V",
                                                                &pushTransportEndStructCtor);
            if (err != CHIP_NO_ERROR || pushTransportEndStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$PushAvStreamTransportClusterPushTransportEndEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(pushTransportEndStructClass, pushTransportEndStructCtor, value_connectionID,
                                           value_triggerType, value_activationReason);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::Chime::Id: {
        using namespace app::Clusters::Chime;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::EcosystemInformation::Id: {
        using namespace app::Clusters::EcosystemInformation;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::CommissionerControl::Id: {
        using namespace app::Clusters::CommissionerControl;
        switch (aPath.mEventId)
        {
        case Events::CommissioningRequestResult::Id: {
            Events::CommissioningRequestResult::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_requestID;
            std::string value_requestIDClassName     = "java/lang/Long";
            std::string value_requestIDCtorSignature = "(J)V";
            jlong jnivalue_requestID                 = static_cast<jlong>(cppValue.requestID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_requestIDClassName.c_str(), value_requestIDCtorSignature.c_str(), jnivalue_requestID, value_requestID);

            jobject value_clientNodeID;
            std::string value_clientNodeIDClassName     = "java/lang/Long";
            std::string value_clientNodeIDCtorSignature = "(J)V";
            jlong jnivalue_clientNodeID                 = static_cast<jlong>(cppValue.clientNodeID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(value_clientNodeIDClassName.c_str(),
                                                                        value_clientNodeIDCtorSignature.c_str(),
                                                                        jnivalue_clientNodeID, value_clientNodeID);

            jobject value_statusCode;
            std::string value_statusCodeClassName     = "java/lang/Integer";
            std::string value_statusCodeCtorSignature = "(I)V";
            jint jnivalue_statusCode                  = static_cast<jint>(cppValue.statusCode);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                value_statusCodeClassName.c_str(), value_statusCodeCtorSignature.c_str(), jnivalue_statusCode, value_statusCode);

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass commissioningRequestResultStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$CommissionerControlClusterCommissioningRequestResultEvent",
                commissioningRequestResultStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl,
                             "Could not find class ChipEventStructs$CommissionerControlClusterCommissioningRequestResultEvent");
                return nullptr;
            }

            jmethodID commissioningRequestResultStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, commissioningRequestResultStructClass, "<init>",
                "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;)V", &commissioningRequestResultStructCtor);
            if (err != CHIP_NO_ERROR || commissioningRequestResultStructCtor == nullptr)
            {
                ChipLogError(
                    Zcl, "Could not find ChipEventStructs$CommissionerControlClusterCommissioningRequestResultEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(commissioningRequestResultStructClass, commissioningRequestResultStructCtor,
                                           value_requestID, value_clientNodeID, value_statusCode, value_fabricIndex);

            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TlsCertificateManagement::Id: {
        using namespace app::Clusters::TlsCertificateManagement;
        switch (aPath.mEventId)
        {
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case app::Clusters::TlsClientManagement::Id: {
        using namespace app::Clusters::TlsClientManagement;
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
            jobject value_arg4_i;
            if (!cppValue.arg4.i.HasValue())
            {
                chip::JniReferences::GetInstance().CreateOptional(nullptr, value_arg4_i);
            }
            else
            {
                jobject value_arg4_iInsideOptional;
                std::string value_arg4_iInsideOptionalClassName     = "java/lang/Integer";
                std::string value_arg4_iInsideOptionalCtorSignature = "(I)V";
                jint jnivalue_arg4_iInsideOptional                  = static_cast<jint>(cppValue.arg4.i.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_arg4_iInsideOptionalClassName.c_str(), value_arg4_iInsideOptionalCtorSignature.c_str(),
                    jnivalue_arg4_iInsideOptional, value_arg4_iInsideOptional);
                chip::JniReferences::GetInstance().CreateOptional(value_arg4_iInsideOptional, value_arg4_i);
            }

            {
                jclass simpleStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$UnitTestingClusterSimpleStruct", simpleStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$UnitTestingClusterSimpleStruct");
                    return nullptr;
                }

                jmethodID simpleStructStructCtor_0;
                err = chip::JniReferences::GetInstance().FindMethod(
                    env, simpleStructStructClass_0, "<init>",
                    "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/Integer;Ljava/lang/"
                    "Float;Ljava/lang/Double;Ljava/util/Optional;)V",
                    &simpleStructStructCtor_0);
                if (err != CHIP_NO_ERROR || simpleStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$UnitTestingClusterSimpleStruct constructor");
                    return nullptr;
                }

                value_arg4 =
                    env->NewObject(simpleStructStructClass_0, simpleStructStructCtor_0, value_arg4_a, value_arg4_b, value_arg4_c,
                                   value_arg4_d, value_arg4_e, value_arg4_f, value_arg4_g, value_arg4_h, value_arg4_i);
            }

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
                jobject newElement_0_i;
                if (!entry_0.i.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_i);
                }
                else
                {
                    jobject newElement_0_iInsideOptional;
                    std::string newElement_0_iInsideOptionalClassName     = "java/lang/Integer";
                    std::string newElement_0_iInsideOptionalCtorSignature = "(I)V";
                    jint jninewElement_0_iInsideOptional                  = static_cast<jint>(entry_0.i.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        newElement_0_iInsideOptionalClassName.c_str(), newElement_0_iInsideOptionalCtorSignature.c_str(),
                        jninewElement_0_iInsideOptional, newElement_0_iInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_iInsideOptional, newElement_0_i);
                }

                {
                    jclass simpleStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$UnitTestingClusterSimpleStruct", simpleStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$UnitTestingClusterSimpleStruct");
                        return nullptr;
                    }

                    jmethodID simpleStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, simpleStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/Integer;Ljava/"
                        "lang/Float;Ljava/lang/Double;Ljava/util/Optional;)V",
                        &simpleStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || simpleStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$UnitTestingClusterSimpleStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(simpleStructStructClass_1, simpleStructStructCtor_1, newElement_0_a,
                                                  newElement_0_b, newElement_0_c, newElement_0_d, newElement_0_e, newElement_0_f,
                                                  newElement_0_g, newElement_0_h, newElement_0_i);
                }
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$UnitTestingClusterTestEventEvent", testEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$UnitTestingClusterTestEventEvent");
                return nullptr;
            }

            jmethodID testEventStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, testEventStructClass, "<init>",
                "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Boolean;Lchip/devicecontroller/"
                "ChipStructs$UnitTestingClusterSimpleStruct;Ljava/util/ArrayList;Ljava/util/ArrayList;)V",
                &testEventStructCtor);
            if (err != CHIP_NO_ERROR || testEventStructCtor == nullptr)
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
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$UnitTestingClusterTestFabricScopedEventEvent",
                testFabricScopedEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$UnitTestingClusterTestFabricScopedEventEvent");
                return nullptr;
            }

            jmethodID testFabricScopedEventStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, testFabricScopedEventStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &testFabricScopedEventStructCtor);
            if (err != CHIP_NO_ERROR || testFabricScopedEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$UnitTestingClusterTestFabricScopedEventEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(testFabricScopedEventStructClass, testFabricScopedEventStructCtor, value_fabricIndex);

            return value;
        }
        case Events::TestDifferentVendorMeiEvent::Id: {
            Events::TestDifferentVendorMeiEvent::DecodableType cppValue;
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

            jclass testDifferentVendorMeiEventStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$UnitTestingClusterTestDifferentVendorMeiEventEvent",
                testDifferentVendorMeiEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$UnitTestingClusterTestDifferentVendorMeiEventEvent");
                return nullptr;
            }

            jmethodID testDifferentVendorMeiEventStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(env, testDifferentVendorMeiEventStructClass, "<init>",
                                                                "(Ljava/lang/Integer;)V", &testDifferentVendorMeiEventStructCtor);
            if (err != CHIP_NO_ERROR || testDifferentVendorMeiEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$UnitTestingClusterTestDifferentVendorMeiEventEvent constructor");
                return nullptr;
            }

            jobject value =
                env->NewObject(testDifferentVendorMeiEventStructClass, testDifferentVendorMeiEventStructCtor, value_arg1);

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
    case app::Clusters::SampleMei::Id: {
        using namespace app::Clusters::SampleMei;
        switch (aPath.mEventId)
        {
        case Events::PingCountEvent::Id: {
            Events::PingCountEvent::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value_count;
            std::string value_countClassName     = "java/lang/Long";
            std::string value_countCtorSignature = "(J)V";
            jlong jnivalue_count                 = static_cast<jlong>(cppValue.count);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                value_countClassName.c_str(), value_countCtorSignature.c_str(), jnivalue_count, value_count);

            jobject value_fabricIndex;
            std::string value_fabricIndexClassName     = "java/lang/Integer";
            std::string value_fabricIndexCtorSignature = "(I)V";
            jint jnivalue_fabricIndex                  = static_cast<jint>(cppValue.fabricIndex);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_fabricIndexClassName.c_str(),
                                                                       value_fabricIndexCtorSignature.c_str(), jnivalue_fabricIndex,
                                                                       value_fabricIndex);

            jclass pingCountEventStructClass;
            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                env, "chip/devicecontroller/ChipEventStructs$SampleMeiClusterPingCountEventEvent", pingCountEventStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipEventStructs$SampleMeiClusterPingCountEventEvent");
                return nullptr;
            }

            jmethodID pingCountEventStructCtor;
            err = chip::JniReferences::GetInstance().FindMethod(
                env, pingCountEventStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Integer;)V", &pingCountEventStructCtor);
            if (err != CHIP_NO_ERROR || pingCountEventStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipEventStructs$SampleMeiClusterPingCountEventEvent constructor");
                return nullptr;
            }

            jobject value = env->NewObject(pingCountEventStructClass, pingCountEventStructCtor, value_count, value_fabricIndex);

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
