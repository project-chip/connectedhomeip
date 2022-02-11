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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/TypeTraits.h>

namespace chip {

jobject DecodeAttributeValue(const app::ConcreteAttributePath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aPath.mClusterId)
    {
    case app::Clusters::AccessControl::Id: {
        using namespace app::Clusters::AccessControl;
        switch (aPath.mAttributeId)
        {
        case Attributes::Acl::Id: {
            using TypeInfo = Attributes::Acl::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_privilege;
                std::string newElement_0_privilegeClassName     = "java/lang/Integer";
                std::string newElement_0_privilegeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_privilegeClassName.c_str(), newElement_0_privilegeCtorSignature.c_str(),
                    static_cast<uint8_t>(entry_0.privilege), newElement_0_privilege);
                jobject newElement_0_authMode;
                std::string newElement_0_authModeClassName     = "java/lang/Integer";
                std::string newElement_0_authModeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_authModeClassName.c_str(), newElement_0_authModeCtorSignature.c_str(),
                    static_cast<uint8_t>(entry_0.authMode), newElement_0_authMode);
                jobject newElement_0_subjects;
                if (entry_0.subjects.IsNull())
                {
                    newElement_0_subjects = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(newElement_0_subjects);

                    auto iter_newElement_0_subjects_NaN = entry_0.subjects.Value().begin();
                    while (iter_newElement_0_subjects_NaN.Next())
                    {
                        auto & entry_NaN = iter_newElement_0_subjects_NaN.GetValue();
                        jobject newElement_NaN;
                        std::string newElement_NaNClassName     = "java/lang/Long";
                        std::string newElement_NaNCtorSignature = "(J)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(
                            newElement_NaNClassName.c_str(), newElement_NaNCtorSignature.c_str(), entry_NaN, newElement_NaN);
                        chip::JniReferences::GetInstance().AddToArrayList(newElement_0_subjects, newElement_NaN);
                    }
                }
                jobject newElement_0_targets;
                if (entry_0.targets.IsNull())
                {
                    newElement_0_targets = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(newElement_0_targets);

                    auto iter_newElement_0_targets_NaN = entry_0.targets.Value().begin();
                    while (iter_newElement_0_targets_NaN.Next())
                    {
                        auto & entry_NaN = iter_newElement_0_targets_NaN.GetValue();
                        jobject newElement_NaN;
                        jobject newElement_NaN_cluster;
                        if (entry_NaN.cluster.IsNull())
                        {
                            newElement_NaN_cluster = nullptr;
                        }
                        else
                        {
                            std::string newElement_NaN_clusterClassName     = "java/lang/Long";
                            std::string newElement_NaN_clusterCtorSignature = "(J)V";
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                                newElement_NaN_clusterClassName.c_str(), newElement_NaN_clusterCtorSignature.c_str(),
                                entry_NaN.cluster.Value(), newElement_NaN_cluster);
                        }
                        jobject newElement_NaN_endpoint;
                        if (entry_NaN.endpoint.IsNull())
                        {
                            newElement_NaN_endpoint = nullptr;
                        }
                        else
                        {
                            std::string newElement_NaN_endpointClassName     = "java/lang/Integer";
                            std::string newElement_NaN_endpointCtorSignature = "(I)V";
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                                newElement_NaN_endpointClassName.c_str(), newElement_NaN_endpointCtorSignature.c_str(),
                                entry_NaN.endpoint.Value(), newElement_NaN_endpoint);
                        }
                        jobject newElement_NaN_deviceType;
                        if (entry_NaN.deviceType.IsNull())
                        {
                            newElement_NaN_deviceType = nullptr;
                        }
                        else
                        {
                            std::string newElement_NaN_deviceTypeClassName     = "java/lang/Long";
                            std::string newElement_NaN_deviceTypeCtorSignature = "(J)V";
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                                newElement_NaN_deviceTypeClassName.c_str(), newElement_NaN_deviceTypeCtorSignature.c_str(),
                                entry_NaN.deviceType.Value(), newElement_NaN_deviceType);
                        }

                        jclass targetStructClass;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$AccessControlClusterTarget", targetStructClass);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterTarget");
                            return nullptr;
                        }
                        jmethodID targetStructCtor =
                            env->GetMethodID(targetStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;)V");
                        if (targetStructCtor == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterTarget constructor");
                            return nullptr;
                        }

                        newElement_NaN = env->NewObject(targetStructClass, targetStructCtor, newElement_NaN_cluster,
                                                        newElement_NaN_endpoint, newElement_NaN_deviceType);
                        chip::JniReferences::GetInstance().AddToArrayList(newElement_0_targets, newElement_NaN);
                    }
                }

                jclass accessControlEntryStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AccessControlClusterAccessControlEntry", accessControlEntryStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterAccessControlEntry");
                    return nullptr;
                }
                jmethodID accessControlEntryStructCtor = env->GetMethodID(
                    accessControlEntryStructClass, "<init>",
                    "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/ArrayList;Ljava/util/ArrayList;)V");
                if (accessControlEntryStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterAccessControlEntry constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(accessControlEntryStructClass, accessControlEntryStructCtor, newElement_0_fabricIndex,
                                   newElement_0_privilege, newElement_0_authMode, newElement_0_subjects, newElement_0_targets);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::Extension::Id: {
            using TypeInfo = Attributes::Extension::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_data;
                jbyteArray newElement_0_dataByteArray = env->NewByteArray(static_cast<jsize>(entry_0.data.size()));
                env->SetByteArrayRegion(newElement_0_dataByteArray, 0, static_cast<jsize>(entry_0.data.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.data.data()));
                newElement_0_data = newElement_0_dataByteArray;

                jclass extensionEntryStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AccessControlClusterExtensionEntry", extensionEntryStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AccessControlClusterExtensionEntry");
                    return nullptr;
                }
                jmethodID extensionEntryStructCtor =
                    env->GetMethodID(extensionEntryStructClass, "<init>", "(Ljava/lang/Integer;[B)V");
                if (extensionEntryStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AccessControlClusterExtensionEntry constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(extensionEntryStructClass, extensionEntryStructCtor, newElement_0_fabricIndex,
                                              newElement_0_data);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::AccountLogin::Id: {
        using namespace app::Clusters::AccountLogin;
        switch (aPath.mAttributeId)
        {
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::AdministratorCommissioning::Id: {
        using namespace app::Clusters::AdministratorCommissioning;
        switch (aPath.mAttributeId)
        {
        case Attributes::WindowStatus::Id: {
            using TypeInfo = Attributes::WindowStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AdminFabricIndex::Id: {
            using TypeInfo = Attributes::AdminFabricIndex::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AdminVendorId::Id: {
            using TypeInfo = Attributes::AdminVendorId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ApplicationBasic::Id: {
        using namespace app::Clusters::ApplicationBasic;
        switch (aPath.mAttributeId)
        {
        case Attributes::VendorName::Id: {
            using TypeInfo = Attributes::VendorName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::VendorId::Id: {
            using TypeInfo = Attributes::VendorId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ApplicationName::Id: {
            using TypeInfo = Attributes::ApplicationName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ProductId::Id: {
            using TypeInfo = Attributes::ProductId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ApplicationApp::Id: {
            using TypeInfo = Attributes::ApplicationApp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jobject value_catalogVendorId;
            std::string value_catalogVendorIdClassName     = "java/lang/Integer";
            std::string value_catalogVendorIdCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_catalogVendorIdClassName.c_str(),
                                                                           value_catalogVendorIdCtorSignature.c_str(),
                                                                           cppValue.catalogVendorId, value_catalogVendorId);
            jobject value_applicationId;
            value_applicationId =
                env->NewStringUTF(std::string(cppValue.applicationId.data(), cppValue.applicationId.size()).c_str());

            jclass applicationBasicApplicationStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$ApplicationBasicClusterApplicationBasicApplication",
                applicationBasicApplicationStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$ApplicationBasicClusterApplicationBasicApplication");
                return nullptr;
            }
            jmethodID applicationBasicApplicationStructCtor =
                env->GetMethodID(applicationBasicApplicationStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/String;)V");
            if (applicationBasicApplicationStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$ApplicationBasicClusterApplicationBasicApplication constructor");
                return nullptr;
            }

            value = env->NewObject(applicationBasicApplicationStructClass, applicationBasicApplicationStructCtor,
                                   value_catalogVendorId, value_applicationId);
            return value;
        }
        case Attributes::ApplicationStatus::Id: {
            using TypeInfo = Attributes::ApplicationStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::ApplicationVersion::Id: {
            using TypeInfo = Attributes::ApplicationVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::AllowedVendorList::Id: {
            using TypeInfo = Attributes::AllowedVendorList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint16_t>(entry_0), newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ApplicationLauncher::Id: {
        using namespace app::Clusters::ApplicationLauncher;
        switch (aPath.mAttributeId)
        {
        case Attributes::ApplicationLauncherList::Id: {
            using TypeInfo = Attributes::ApplicationLauncherList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ApplicationLauncherApp::Id: {
            using TypeInfo = Attributes::ApplicationLauncherApp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                jobject value_application;
                jobject value_application_catalogVendorId;
                std::string value_application_catalogVendorIdClassName     = "java/lang/Integer";
                std::string value_application_catalogVendorIdCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    value_application_catalogVendorIdClassName.c_str(), value_application_catalogVendorIdCtorSignature.c_str(),
                    cppValue.Value().application.catalogVendorId, value_application_catalogVendorId);
                jobject value_application_applicationId;
                value_application_applicationId = env->NewStringUTF(std::string(cppValue.Value().application.applicationId.data(),
                                                                                cppValue.Value().application.applicationId.size())
                                                                        .c_str());

                jclass applicationStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ApplicationLauncherClusterApplication", applicationStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ApplicationLauncherClusterApplication");
                    return nullptr;
                }
                jmethodID applicationStructCtor =
                    env->GetMethodID(applicationStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/String;)V");
                if (applicationStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ApplicationLauncherClusterApplication constructor");
                    return nullptr;
                }

                value_application = env->NewObject(applicationStructClass, applicationStructCtor, value_application_catalogVendorId,
                                                   value_application_applicationId);
                jobject value_endpoint;
                if (!cppValue.Value().endpoint.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_endpoint);
                }
                else
                {
                    std::string value_endpointClassName     = "java/lang/Integer";
                    std::string value_endpointCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                        value_endpointClassName.c_str(), value_endpointCtorSignature.c_str(), cppValue.Value().endpoint.Value(),
                        value_endpoint);
                }

                jclass applicationEPStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ApplicationLauncherClusterApplicationEP", applicationEPStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ApplicationLauncherClusterApplicationEP");
                    return nullptr;
                }
                jmethodID applicationEPStructCtor = env->GetMethodID(
                    applicationEPStructClass, "<init>",
                    "(Lchip/devicecontroller/ChipStructs$ApplicationLauncherClusterApplication;Ljava/util/Optional;)V");
                if (applicationEPStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ApplicationLauncherClusterApplicationEP constructor");
                    return nullptr;
                }

                value = env->NewObject(applicationEPStructClass, applicationEPStructCtor, value_application, value_endpoint);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::AudioOutput::Id: {
        using namespace app::Clusters::AudioOutput;
        switch (aPath.mAttributeId)
        {
        case Attributes::AudioOutputList::Id: {
            using TypeInfo = Attributes::AudioOutputList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_index;
                std::string newElement_0_indexClassName     = "java/lang/Integer";
                std::string newElement_0_indexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_indexClassName.c_str(),
                                                                              newElement_0_indexCtorSignature.c_str(),
                                                                              entry_0.index, newElement_0_index);
                jobject newElement_0_outputType;
                std::string newElement_0_outputTypeClassName     = "java/lang/Integer";
                std::string newElement_0_outputTypeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_outputTypeClassName.c_str(), newElement_0_outputTypeCtorSignature.c_str(),
                    static_cast<uint8_t>(entry_0.outputType), newElement_0_outputType);
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());

                jclass outputInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$AudioOutputClusterOutputInfo", outputInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$AudioOutputClusterOutputInfo");
                    return nullptr;
                }
                jmethodID outputInfoStructCtor = env->GetMethodID(outputInfoStructClass, "<init>",
                                                                  "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/String;)V");
                if (outputInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$AudioOutputClusterOutputInfo constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(outputInfoStructClass, outputInfoStructCtor, newElement_0_index,
                                              newElement_0_outputType, newElement_0_name);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentAudioOutput::Id: {
            using TypeInfo = Attributes::CurrentAudioOutput::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::BarrierControl::Id: {
        using namespace app::Clusters::BarrierControl;
        switch (aPath.mAttributeId)
        {
        case Attributes::BarrierMovingState::Id: {
            using TypeInfo = Attributes::BarrierMovingState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::BarrierSafetyStatus::Id: {
            using TypeInfo = Attributes::BarrierSafetyStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BarrierCapabilities::Id: {
            using TypeInfo = Attributes::BarrierCapabilities::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::BarrierPosition::Id: {
            using TypeInfo = Attributes::BarrierPosition::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Basic::Id: {
        using namespace app::Clusters::Basic;
        switch (aPath.mAttributeId)
        {
        case Attributes::DataModelRevision::Id: {
            using TypeInfo = Attributes::DataModelRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::VendorName::Id: {
            using TypeInfo = Attributes::VendorName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::VendorID::Id: {
            using TypeInfo = Attributes::VendorID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           static_cast<uint16_t>(cppValue), value);
            return value;
        }
        case Attributes::ProductName::Id: {
            using TypeInfo = Attributes::ProductName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ProductID::Id: {
            using TypeInfo = Attributes::ProductID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::NodeLabel::Id: {
            using TypeInfo = Attributes::NodeLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::Location::Id: {
            using TypeInfo = Attributes::Location::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::HardwareVersion::Id: {
            using TypeInfo = Attributes::HardwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::HardwareVersionString::Id: {
            using TypeInfo = Attributes::HardwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::SoftwareVersion::Id: {
            using TypeInfo = Attributes::SoftwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::SoftwareVersionString::Id: {
            using TypeInfo = Attributes::SoftwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ManufacturingDate::Id: {
            using TypeInfo = Attributes::ManufacturingDate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::PartNumber::Id: {
            using TypeInfo = Attributes::PartNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ProductURL::Id: {
            using TypeInfo = Attributes::ProductURL::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ProductLabel::Id: {
            using TypeInfo = Attributes::ProductLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::SerialNumber::Id: {
            using TypeInfo = Attributes::SerialNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::LocalConfigDisabled::Id: {
            using TypeInfo = Attributes::LocalConfigDisabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::Reachable::Id: {
            using TypeInfo = Attributes::Reachable::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::UniqueID::Id: {
            using TypeInfo = Attributes::UniqueID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::BinaryInputBasic::Id: {
        using namespace app::Clusters::BinaryInputBasic;
        switch (aPath.mAttributeId)
        {
        case Attributes::OutOfService::Id: {
            using TypeInfo = Attributes::OutOfService::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::PresentValue::Id: {
            using TypeInfo = Attributes::PresentValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::StatusFlags::Id: {
            using TypeInfo = Attributes::StatusFlags::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Binding::Id: {
        using namespace app::Clusters::Binding;
        switch (aPath.mAttributeId)
        {
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::BooleanState::Id: {
        using namespace app::Clusters::BooleanState;
        switch (aPath.mAttributeId)
        {
        case Attributes::StateValue::Id: {
            using TypeInfo = Attributes::StateValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::BridgedActions::Id: {
        using namespace app::Clusters::BridgedActions;
        switch (aPath.mAttributeId)
        {
        case Attributes::ActionList::Id: {
            using TypeInfo = Attributes::ActionList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_actionID;
                std::string newElement_0_actionIDClassName     = "java/lang/Integer";
                std::string newElement_0_actionIDCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_actionIDClassName.c_str(),
                                                                               newElement_0_actionIDCtorSignature.c_str(),
                                                                               entry_0.actionID, newElement_0_actionID);
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());
                jobject newElement_0_type;
                std::string newElement_0_typeClassName     = "java/lang/Integer";
                std::string newElement_0_typeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_typeClassName.c_str(), newElement_0_typeCtorSignature.c_str(), static_cast<uint8_t>(entry_0.type),
                    newElement_0_type);
                jobject newElement_0_endpointListID;
                std::string newElement_0_endpointListIDClassName     = "java/lang/Integer";
                std::string newElement_0_endpointListIDCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_endpointListIDClassName.c_str(),
                                                                               newElement_0_endpointListIDCtorSignature.c_str(),
                                                                               entry_0.endpointListID, newElement_0_endpointListID);
                jobject newElement_0_supportedCommands;
                std::string newElement_0_supportedCommandsClassName     = "java/lang/Integer";
                std::string newElement_0_supportedCommandsCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    newElement_0_supportedCommandsClassName.c_str(), newElement_0_supportedCommandsCtorSignature.c_str(),
                    entry_0.supportedCommands, newElement_0_supportedCommands);
                jobject newElement_0_status;
                std::string newElement_0_statusClassName     = "java/lang/Integer";
                std::string newElement_0_statusCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_statusClassName.c_str(), newElement_0_statusCtorSignature.c_str(),
                    static_cast<uint8_t>(entry_0.status), newElement_0_status);

                jclass actionStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$BridgedActionsClusterActionStruct", actionStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$BridgedActionsClusterActionStruct");
                    return nullptr;
                }
                jmethodID actionStructStructCtor =
                    env->GetMethodID(actionStructStructClass, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/String;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                                     "Integer;Ljava/lang/Integer;)V");
                if (actionStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$BridgedActionsClusterActionStruct constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(actionStructStructClass, actionStructStructCtor, newElement_0_actionID,
                                              newElement_0_name, newElement_0_type, newElement_0_endpointListID,
                                              newElement_0_supportedCommands, newElement_0_status);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::EndpointList::Id: {
            using TypeInfo = Attributes::EndpointList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_endpointListID;
                std::string newElement_0_endpointListIDClassName     = "java/lang/Integer";
                std::string newElement_0_endpointListIDCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_endpointListIDClassName.c_str(),
                                                                               newElement_0_endpointListIDCtorSignature.c_str(),
                                                                               entry_0.endpointListID, newElement_0_endpointListID);
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());
                jobject newElement_0_type;
                std::string newElement_0_typeClassName     = "java/lang/Integer";
                std::string newElement_0_typeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_typeClassName.c_str(), newElement_0_typeCtorSignature.c_str(), static_cast<uint8_t>(entry_0.type),
                    newElement_0_type);
                jobject newElement_0_endpoints;
                chip::JniReferences::GetInstance().CreateArrayList(newElement_0_endpoints);

                auto iter_newElement_0_endpoints_NaN = entry_0.endpoints.begin();
                while (iter_newElement_0_endpoints_NaN.Next())
                {
                    auto & entry_NaN = iter_newElement_0_endpoints_NaN.GetValue();
                    jobject newElement_NaN;
                    std::string newElement_NaNClassName     = "java/lang/Integer";
                    std::string newElement_NaNCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                        newElement_NaNClassName.c_str(), newElement_NaNCtorSignature.c_str(), entry_NaN, newElement_NaN);
                    chip::JniReferences::GetInstance().AddToArrayList(newElement_0_endpoints, newElement_NaN);
                }

                jclass endpointListStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$BridgedActionsClusterEndpointListStruct",
                    endpointListStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$BridgedActionsClusterEndpointListStruct");
                    return nullptr;
                }
                jmethodID endpointListStructStructCtor =
                    env->GetMethodID(endpointListStructStructClass, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/String;Ljava/lang/Integer;Ljava/util/ArrayList;)V");
                if (endpointListStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$BridgedActionsClusterEndpointListStruct constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(endpointListStructStructClass, endpointListStructStructCtor, newElement_0_endpointListID,
                                   newElement_0_name, newElement_0_type, newElement_0_endpoints);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::SetupUrl::Id: {
            using TypeInfo = Attributes::SetupUrl::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::BridgedDeviceBasic::Id: {
        using namespace app::Clusters::BridgedDeviceBasic;
        switch (aPath.mAttributeId)
        {
        case Attributes::VendorName::Id: {
            using TypeInfo = Attributes::VendorName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::VendorID::Id: {
            using TypeInfo = Attributes::VendorID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ProductName::Id: {
            using TypeInfo = Attributes::ProductName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::NodeLabel::Id: {
            using TypeInfo = Attributes::NodeLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::HardwareVersion::Id: {
            using TypeInfo = Attributes::HardwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::HardwareVersionString::Id: {
            using TypeInfo = Attributes::HardwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::SoftwareVersion::Id: {
            using TypeInfo = Attributes::SoftwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::SoftwareVersionString::Id: {
            using TypeInfo = Attributes::SoftwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ManufacturingDate::Id: {
            using TypeInfo = Attributes::ManufacturingDate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::PartNumber::Id: {
            using TypeInfo = Attributes::PartNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ProductURL::Id: {
            using TypeInfo = Attributes::ProductURL::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ProductLabel::Id: {
            using TypeInfo = Attributes::ProductLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::SerialNumber::Id: {
            using TypeInfo = Attributes::SerialNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::Reachable::Id: {
            using TypeInfo = Attributes::Reachable::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Channel::Id: {
        using namespace app::Clusters::Channel;
        switch (aPath.mAttributeId)
        {
        case Attributes::ChannelList::Id: {
            using TypeInfo = Attributes::ChannelList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_majorNumber;
                std::string newElement_0_majorNumberClassName     = "java/lang/Integer";
                std::string newElement_0_majorNumberCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_majorNumberClassName.c_str(),
                                                                               newElement_0_majorNumberCtorSignature.c_str(),
                                                                               entry_0.majorNumber, newElement_0_majorNumber);
                jobject newElement_0_minorNumber;
                std::string newElement_0_minorNumberClassName     = "java/lang/Integer";
                std::string newElement_0_minorNumberCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_minorNumberClassName.c_str(),
                                                                               newElement_0_minorNumberCtorSignature.c_str(),
                                                                               entry_0.minorNumber, newElement_0_minorNumber);
                jobject newElement_0_name;
                if (!entry_0.name.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_name);
                }
                else
                {
                    newElement_0_name =
                        env->NewStringUTF(std::string(entry_0.name.Value().data(), entry_0.name.Value().size()).c_str());
                }
                jobject newElement_0_callSign;
                if (!entry_0.callSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_callSign);
                }
                else
                {
                    newElement_0_callSign =
                        env->NewStringUTF(std::string(entry_0.callSign.Value().data(), entry_0.callSign.Value().size()).c_str());
                }
                jobject newElement_0_affiliateCallSign;
                if (!entry_0.affiliateCallSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_affiliateCallSign);
                }
                else
                {
                    newElement_0_affiliateCallSign = env->NewStringUTF(
                        std::string(entry_0.affiliateCallSign.Value().data(), entry_0.affiliateCallSign.Value().size()).c_str());
                }

                jclass channelInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ChannelClusterChannelInfo", channelInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ChannelClusterChannelInfo");
                    return nullptr;
                }
                jmethodID channelInfoStructCtor = env->GetMethodID(
                    channelInfoStructClass, "<init>",
                    "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V");
                if (channelInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ChannelClusterChannelInfo constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(channelInfoStructClass, channelInfoStructCtor, newElement_0_majorNumber,
                                              newElement_0_minorNumber, newElement_0_name, newElement_0_callSign,
                                              newElement_0_affiliateCallSign);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ChannelLineup::Id: {
            using TypeInfo = Attributes::ChannelLineup::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                jobject value_operatorName;
                value_operatorName = env->NewStringUTF(
                    std::string(cppValue.Value().operatorName.data(), cppValue.Value().operatorName.size()).c_str());
                jobject value_lineupName;
                if (!cppValue.Value().lineupName.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_lineupName);
                }
                else
                {
                    value_lineupName = env->NewStringUTF(
                        std::string(cppValue.Value().lineupName.Value().data(), cppValue.Value().lineupName.Value().size())
                            .c_str());
                }
                jobject value_postalCode;
                if (!cppValue.Value().postalCode.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_postalCode);
                }
                else
                {
                    value_postalCode = env->NewStringUTF(
                        std::string(cppValue.Value().postalCode.Value().data(), cppValue.Value().postalCode.Value().size())
                            .c_str());
                }
                jobject value_lineupInfoType;
                std::string value_lineupInfoTypeClassName     = "java/lang/Integer";
                std::string value_lineupInfoTypeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_lineupInfoTypeClassName.c_str(), value_lineupInfoTypeCtorSignature.c_str(),
                    static_cast<uint8_t>(cppValue.Value().lineupInfoType), value_lineupInfoType);

                jclass lineupInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ChannelClusterLineupInfo", lineupInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ChannelClusterLineupInfo");
                    return nullptr;
                }
                jmethodID lineupInfoStructCtor =
                    env->GetMethodID(lineupInfoStructClass, "<init>",
                                     "(Ljava/lang/String;Ljava/util/Optional;Ljava/util/Optional;Ljava/lang/Integer;)V");
                if (lineupInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ChannelClusterLineupInfo constructor");
                    return nullptr;
                }

                value = env->NewObject(lineupInfoStructClass, lineupInfoStructCtor, value_operatorName, value_lineupName,
                                       value_postalCode, value_lineupInfoType);
            }
            return value;
        }
        case Attributes::CurrentChannel::Id: {
            using TypeInfo = Attributes::CurrentChannel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                jobject value_majorNumber;
                std::string value_majorNumberClassName     = "java/lang/Integer";
                std::string value_majorNumberCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_majorNumberClassName.c_str(),
                                                                               value_majorNumberCtorSignature.c_str(),
                                                                               cppValue.Value().majorNumber, value_majorNumber);
                jobject value_minorNumber;
                std::string value_minorNumberClassName     = "java/lang/Integer";
                std::string value_minorNumberCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(value_minorNumberClassName.c_str(),
                                                                               value_minorNumberCtorSignature.c_str(),
                                                                               cppValue.Value().minorNumber, value_minorNumber);
                jobject value_name;
                if (!cppValue.Value().name.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_name);
                }
                else
                {
                    value_name = env->NewStringUTF(
                        std::string(cppValue.Value().name.Value().data(), cppValue.Value().name.Value().size()).c_str());
                }
                jobject value_callSign;
                if (!cppValue.Value().callSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_callSign);
                }
                else
                {
                    value_callSign = env->NewStringUTF(
                        std::string(cppValue.Value().callSign.Value().data(), cppValue.Value().callSign.Value().size()).c_str());
                }
                jobject value_affiliateCallSign;
                if (!cppValue.Value().affiliateCallSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_affiliateCallSign);
                }
                else
                {
                    value_affiliateCallSign = env->NewStringUTF(std::string(cppValue.Value().affiliateCallSign.Value().data(),
                                                                            cppValue.Value().affiliateCallSign.Value().size())
                                                                    .c_str());
                }

                jclass channelInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ChannelClusterChannelInfo", channelInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ChannelClusterChannelInfo");
                    return nullptr;
                }
                jmethodID channelInfoStructCtor = env->GetMethodID(
                    channelInfoStructClass, "<init>",
                    "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;)V");
                if (channelInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ChannelClusterChannelInfo constructor");
                    return nullptr;
                }

                value = env->NewObject(channelInfoStructClass, channelInfoStructCtor, value_majorNumber, value_minorNumber,
                                       value_name, value_callSign, value_affiliateCallSign);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ColorControl::Id: {
        using namespace app::Clusters::ColorControl;
        switch (aPath.mAttributeId)
        {
        case Attributes::CurrentHue::Id: {
            using TypeInfo = Attributes::CurrentHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentSaturation::Id: {
            using TypeInfo = Attributes::CurrentSaturation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::RemainingTime::Id: {
            using TypeInfo = Attributes::RemainingTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CurrentX::Id: {
            using TypeInfo = Attributes::CurrentX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CurrentY::Id: {
            using TypeInfo = Attributes::CurrentY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::DriftCompensation::Id: {
            using TypeInfo = Attributes::DriftCompensation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CompensationText::Id: {
            using TypeInfo = Attributes::CompensationText::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ColorTemperature::Id: {
            using TypeInfo = Attributes::ColorTemperature::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorMode::Id: {
            using TypeInfo = Attributes::ColorMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ColorControlOptions::Id: {
            using TypeInfo = Attributes::ColorControlOptions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::NumberOfPrimaries::Id: {
            using TypeInfo = Attributes::NumberOfPrimaries::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Primary1X::Id: {
            using TypeInfo = Attributes::Primary1X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary1Y::Id: {
            using TypeInfo = Attributes::Primary1Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary1Intensity::Id: {
            using TypeInfo = Attributes::Primary1Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Primary2X::Id: {
            using TypeInfo = Attributes::Primary2X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary2Y::Id: {
            using TypeInfo = Attributes::Primary2Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary2Intensity::Id: {
            using TypeInfo = Attributes::Primary2Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Primary3X::Id: {
            using TypeInfo = Attributes::Primary3X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary3Y::Id: {
            using TypeInfo = Attributes::Primary3Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary3Intensity::Id: {
            using TypeInfo = Attributes::Primary3Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Primary4X::Id: {
            using TypeInfo = Attributes::Primary4X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary4Y::Id: {
            using TypeInfo = Attributes::Primary4Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary4Intensity::Id: {
            using TypeInfo = Attributes::Primary4Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Primary5X::Id: {
            using TypeInfo = Attributes::Primary5X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary5Y::Id: {
            using TypeInfo = Attributes::Primary5Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary5Intensity::Id: {
            using TypeInfo = Attributes::Primary5Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Primary6X::Id: {
            using TypeInfo = Attributes::Primary6X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary6Y::Id: {
            using TypeInfo = Attributes::Primary6Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Primary6Intensity::Id: {
            using TypeInfo = Attributes::Primary6Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::WhitePointX::Id: {
            using TypeInfo = Attributes::WhitePointX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::WhitePointY::Id: {
            using TypeInfo = Attributes::WhitePointY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointRX::Id: {
            using TypeInfo = Attributes::ColorPointRX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointRY::Id: {
            using TypeInfo = Attributes::ColorPointRY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointRIntensity::Id: {
            using TypeInfo = Attributes::ColorPointRIntensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ColorPointGX::Id: {
            using TypeInfo = Attributes::ColorPointGX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointGY::Id: {
            using TypeInfo = Attributes::ColorPointGY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointGIntensity::Id: {
            using TypeInfo = Attributes::ColorPointGIntensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ColorPointBX::Id: {
            using TypeInfo = Attributes::ColorPointBX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointBY::Id: {
            using TypeInfo = Attributes::ColorPointBY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorPointBIntensity::Id: {
            using TypeInfo = Attributes::ColorPointBIntensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::EnhancedCurrentHue::Id: {
            using TypeInfo = Attributes::EnhancedCurrentHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::EnhancedColorMode::Id: {
            using TypeInfo = Attributes::EnhancedColorMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ColorLoopActive::Id: {
            using TypeInfo = Attributes::ColorLoopActive::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ColorLoopDirection::Id: {
            using TypeInfo = Attributes::ColorLoopDirection::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ColorLoopTime::Id: {
            using TypeInfo = Attributes::ColorLoopTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorLoopStartEnhancedHue::Id: {
            using TypeInfo = Attributes::ColorLoopStartEnhancedHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorLoopStoredEnhancedHue::Id: {
            using TypeInfo = Attributes::ColorLoopStoredEnhancedHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorCapabilities::Id: {
            using TypeInfo = Attributes::ColorCapabilities::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorTempPhysicalMin::Id: {
            using TypeInfo = Attributes::ColorTempPhysicalMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ColorTempPhysicalMax::Id: {
            using TypeInfo = Attributes::ColorTempPhysicalMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CoupleColorTempToLevelMinMireds::Id: {
            using TypeInfo = Attributes::CoupleColorTempToLevelMinMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::StartUpColorTemperatureMireds::Id: {
            using TypeInfo = Attributes::StartUpColorTemperatureMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ContentLauncher::Id: {
        using namespace app::Clusters::ContentLauncher;
        switch (aPath.mAttributeId)
        {
        case Attributes::AcceptHeaderList::Id: {
            using TypeInfo = Attributes::AcceptHeaderList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                newElement_0 = env->NewStringUTF(std::string(entry_0.data(), entry_0.size()).c_str());
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::SupportedStreamingProtocols::Id: {
            using TypeInfo = Attributes::SupportedStreamingProtocols::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Descriptor::Id: {
        using namespace app::Clusters::Descriptor;
        switch (aPath.mAttributeId)
        {
        case Attributes::DeviceList::Id: {
            using TypeInfo = Attributes::DeviceList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_type;
                std::string newElement_0_typeClassName     = "java/lang/Long";
                std::string newElement_0_typeCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0_typeClassName.c_str(), newElement_0_typeCtorSignature.c_str(), entry_0.type, newElement_0_type);
                jobject newElement_0_revision;
                std::string newElement_0_revisionClassName     = "java/lang/Integer";
                std::string newElement_0_revisionCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_revisionClassName.c_str(),
                                                                               newElement_0_revisionCtorSignature.c_str(),
                                                                               entry_0.revision, newElement_0_revision);

                jclass deviceTypeStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$DescriptorClusterDeviceType", deviceTypeStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$DescriptorClusterDeviceType");
                    return nullptr;
                }
                jmethodID deviceTypeStructCtor =
                    env->GetMethodID(deviceTypeStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Integer;)V");
                if (deviceTypeStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$DescriptorClusterDeviceType constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(deviceTypeStructClass, deviceTypeStructCtor, newElement_0_type, newElement_0_revision);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerList::Id: {
            using TypeInfo = Attributes::ServerList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientList::Id: {
            using TypeInfo = Attributes::ClientList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::PartsList::Id: {
            using TypeInfo = Attributes::PartsList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::DiagnosticLogs::Id: {
        using namespace app::Clusters::DiagnosticLogs;
        switch (aPath.mAttributeId)
        {
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::DoorLock::Id: {
        using namespace app::Clusters::DoorLock;
        switch (aPath.mAttributeId)
        {
        case Attributes::LockState::Id: {
            using TypeInfo = Attributes::LockState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(cppValue.Value()), value);
            }
            return value;
        }
        case Attributes::LockType::Id: {
            using TypeInfo = Attributes::LockType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::ActuatorEnabled::Id: {
            using TypeInfo = Attributes::ActuatorEnabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::DoorState::Id: {
            using TypeInfo = Attributes::DoorState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(cppValue.Value()), value);
            }
            return value;
        }
        case Attributes::NumberOfTotalUsersSupported::Id: {
            using TypeInfo = Attributes::NumberOfTotalUsersSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::NumberOfPINUsersSupported::Id: {
            using TypeInfo = Attributes::NumberOfPINUsersSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::NumberOfRFIDUsersSupported::Id: {
            using TypeInfo = Attributes::NumberOfRFIDUsersSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::NumberOfWeekDaySchedulesSupportedPerUser::Id: {
            using TypeInfo = Attributes::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::NumberOfYearDaySchedulesSupportedPerUser::Id: {
            using TypeInfo = Attributes::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxPINCodeLength::Id: {
            using TypeInfo = Attributes::MaxPINCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinPINCodeLength::Id: {
            using TypeInfo = Attributes::MinPINCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxRFIDCodeLength::Id: {
            using TypeInfo = Attributes::MaxRFIDCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinRFIDCodeLength::Id: {
            using TypeInfo = Attributes::MinRFIDCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Language::Id: {
            using TypeInfo = Attributes::Language::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::AutoRelockTime::Id: {
            using TypeInfo = Attributes::AutoRelockTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::SoundVolume::Id: {
            using TypeInfo = Attributes::SoundVolume::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::OperatingMode::Id: {
            using TypeInfo = Attributes::OperatingMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::SupportedOperatingModes::Id: {
            using TypeInfo = Attributes::SupportedOperatingModes::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::EnableOneTouchLocking::Id: {
            using TypeInfo = Attributes::EnableOneTouchLocking::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::EnablePrivacyModeButton::Id: {
            using TypeInfo = Attributes::EnablePrivacyModeButton::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::WrongCodeEntryLimit::Id: {
            using TypeInfo = Attributes::WrongCodeEntryLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ElectricalMeasurement::Id: {
        using namespace app::Clusters::ElectricalMeasurement;
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasurementType::Id: {
            using TypeInfo = Attributes::MeasurementType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TotalActivePower::Id: {
            using TypeInfo = Attributes::TotalActivePower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::RmsVoltage::Id: {
            using TypeInfo = Attributes::RmsVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RmsVoltageMin::Id: {
            using TypeInfo = Attributes::RmsVoltageMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RmsVoltageMax::Id: {
            using TypeInfo = Attributes::RmsVoltageMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RmsCurrent::Id: {
            using TypeInfo = Attributes::RmsCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RmsCurrentMin::Id: {
            using TypeInfo = Attributes::RmsCurrentMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RmsCurrentMax::Id: {
            using TypeInfo = Attributes::RmsCurrentMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ActivePower::Id: {
            using TypeInfo = Attributes::ActivePower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ActivePowerMin::Id: {
            using TypeInfo = Attributes::ActivePowerMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ActivePowerMax::Id: {
            using TypeInfo = Attributes::ActivePowerMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::EthernetNetworkDiagnostics::Id: {
        using namespace app::Clusters::EthernetNetworkDiagnostics;
        switch (aPath.mAttributeId)
        {
        case Attributes::PHYRate::Id: {
            using TypeInfo = Attributes::PHYRate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(cppValue.Value()), value);
            }
            return value;
        }
        case Attributes::FullDuplex::Id: {
            using TypeInfo = Attributes::FullDuplex::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Boolean";
                std::string valueCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::PacketRxCount::Id: {
            using TypeInfo = Attributes::PacketRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PacketTxCount::Id: {
            using TypeInfo = Attributes::PacketTxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxErrCount::Id: {
            using TypeInfo = Attributes::TxErrCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CollisionCount::Id: {
            using TypeInfo = Attributes::CollisionCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::OverrunCount::Id: {
            using TypeInfo = Attributes::OverrunCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CarrierDetect::Id: {
            using TypeInfo = Attributes::CarrierDetect::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Boolean";
                std::string valueCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::TimeSinceReset::Id: {
            using TypeInfo = Attributes::TimeSinceReset::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::FixedLabel::Id: {
        using namespace app::Clusters::FixedLabel;
        switch (aPath.mAttributeId)
        {
        case Attributes::LabelList::Id: {
            using TypeInfo = Attributes::LabelList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_label;
                newElement_0_label = env->NewStringUTF(std::string(entry_0.label.data(), entry_0.label.size()).c_str());
                jobject newElement_0_value;
                newElement_0_value = env->NewStringUTF(std::string(entry_0.value.data(), entry_0.value.size()).c_str());

                jclass labelStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$FixedLabelClusterLabelStruct", labelStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$FixedLabelClusterLabelStruct");
                    return nullptr;
                }
                jmethodID labelStructStructCtor =
                    env->GetMethodID(labelStructStructClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
                if (labelStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$FixedLabelClusterLabelStruct constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(labelStructStructClass, labelStructStructCtor, newElement_0_label, newElement_0_value);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::FlowMeasurement::Id: {
        using namespace app::Clusters::FlowMeasurement;
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::GeneralCommissioning::Id: {
        using namespace app::Clusters::GeneralCommissioning;
        switch (aPath.mAttributeId)
        {
        case Attributes::Breadcrumb::Id: {
            using TypeInfo = Attributes::Breadcrumb::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BasicCommissioningInfo::Id: {
            using TypeInfo = Attributes::BasicCommissioningInfo::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jobject value_failSafeExpiryLengthSeconds;
            std::string value_failSafeExpiryLengthSecondsClassName     = "java/lang/Integer";
            std::string value_failSafeExpiryLengthSecondsCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                value_failSafeExpiryLengthSecondsClassName.c_str(), value_failSafeExpiryLengthSecondsCtorSignature.c_str(),
                cppValue.failSafeExpiryLengthSeconds, value_failSafeExpiryLengthSeconds);

            jclass basicCommissioningInfoStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$GeneralCommissioningClusterBasicCommissioningInfo",
                basicCommissioningInfoStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$GeneralCommissioningClusterBasicCommissioningInfo");
                return nullptr;
            }
            jmethodID basicCommissioningInfoStructCtor =
                env->GetMethodID(basicCommissioningInfoStructClass, "<init>", "(Ljava/lang/Integer;)V");
            if (basicCommissioningInfoStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$GeneralCommissioningClusterBasicCommissioningInfo constructor");
                return nullptr;
            }

            value = env->NewObject(basicCommissioningInfoStructClass, basicCommissioningInfoStructCtor,
                                   value_failSafeExpiryLengthSeconds);
            return value;
        }
        case Attributes::RegulatoryConfig::Id: {
            using TypeInfo = Attributes::RegulatoryConfig::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::LocationCapability::Id: {
            using TypeInfo = Attributes::LocationCapability::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::GeneralDiagnostics::Id: {
        using namespace app::Clusters::GeneralDiagnostics;
        switch (aPath.mAttributeId)
        {
        case Attributes::NetworkInterfaces::Id: {
            using TypeInfo = Attributes::NetworkInterfaces::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());
                jobject newElement_0_fabricConnected;
                std::string newElement_0_fabricConnectedClassName     = "java/lang/Boolean";
                std::string newElement_0_fabricConnectedCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_fabricConnectedClassName.c_str(),
                                                                           newElement_0_fabricConnectedCtorSignature.c_str(),
                                                                           entry_0.fabricConnected, newElement_0_fabricConnected);
                jobject newElement_0_offPremiseServicesReachableIPv4;
                if (entry_0.offPremiseServicesReachableIPv4.IsNull())
                {
                    newElement_0_offPremiseServicesReachableIPv4 = nullptr;
                }
                else
                {
                    std::string newElement_0_offPremiseServicesReachableIPv4ClassName     = "java/lang/Boolean";
                    std::string newElement_0_offPremiseServicesReachableIPv4CtorSignature = "(Z)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                        newElement_0_offPremiseServicesReachableIPv4ClassName.c_str(),
                        newElement_0_offPremiseServicesReachableIPv4CtorSignature.c_str(),
                        entry_0.offPremiseServicesReachableIPv4.Value(), newElement_0_offPremiseServicesReachableIPv4);
                }
                jobject newElement_0_offPremiseServicesReachableIPv6;
                if (entry_0.offPremiseServicesReachableIPv6.IsNull())
                {
                    newElement_0_offPremiseServicesReachableIPv6 = nullptr;
                }
                else
                {
                    std::string newElement_0_offPremiseServicesReachableIPv6ClassName     = "java/lang/Boolean";
                    std::string newElement_0_offPremiseServicesReachableIPv6CtorSignature = "(Z)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                        newElement_0_offPremiseServicesReachableIPv6ClassName.c_str(),
                        newElement_0_offPremiseServicesReachableIPv6CtorSignature.c_str(),
                        entry_0.offPremiseServicesReachableIPv6.Value(), newElement_0_offPremiseServicesReachableIPv6);
                }
                jobject newElement_0_hardwareAddress;
                jbyteArray newElement_0_hardwareAddressByteArray =
                    env->NewByteArray(static_cast<jsize>(entry_0.hardwareAddress.size()));
                env->SetByteArrayRegion(newElement_0_hardwareAddressByteArray, 0,
                                        static_cast<jsize>(entry_0.hardwareAddress.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.hardwareAddress.data()));
                newElement_0_hardwareAddress = newElement_0_hardwareAddressByteArray;
                jobject newElement_0_type;
                std::string newElement_0_typeClassName     = "java/lang/Integer";
                std::string newElement_0_typeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_typeClassName.c_str(), newElement_0_typeCtorSignature.c_str(), static_cast<uint8_t>(entry_0.type),
                    newElement_0_type);

                jclass networkInterfaceTypeStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$GeneralDiagnosticsClusterNetworkInterfaceType",
                    networkInterfaceTypeStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$GeneralDiagnosticsClusterNetworkInterfaceType");
                    return nullptr;
                }
                jmethodID networkInterfaceTypeStructCtor = env->GetMethodID(
                    networkInterfaceTypeStructClass, "<init>",
                    "(Ljava/lang/String;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;[BLjava/lang/Integer;)V");
                if (networkInterfaceTypeStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$GeneralDiagnosticsClusterNetworkInterfaceType constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(networkInterfaceTypeStructClass, networkInterfaceTypeStructCtor, newElement_0_name,
                                   newElement_0_fabricConnected, newElement_0_offPremiseServicesReachableIPv4,
                                   newElement_0_offPremiseServicesReachableIPv6, newElement_0_hardwareAddress, newElement_0_type);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::RebootCount::Id: {
            using TypeInfo = Attributes::RebootCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::UpTime::Id: {
            using TypeInfo = Attributes::UpTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TotalOperationalHours::Id: {
            using TypeInfo = Attributes::TotalOperationalHours::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BootReasons::Id: {
            using TypeInfo = Attributes::BootReasons::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ActiveHardwareFaults::Id: {
            using TypeInfo = Attributes::ActiveHardwareFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ActiveRadioFaults::Id: {
            using TypeInfo = Attributes::ActiveRadioFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ActiveNetworkFaults::Id: {
            using TypeInfo = Attributes::ActiveNetworkFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::GroupKeyManagement::Id: {
        using namespace app::Clusters::GroupKeyManagement;
        switch (aPath.mAttributeId)
        {
        case Attributes::GroupKeyMap::Id: {
            using TypeInfo = Attributes::GroupKeyMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_groupId;
                std::string newElement_0_groupIdClassName     = "java/lang/Integer";
                std::string newElement_0_groupIdCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_groupIdClassName.c_str(),
                                                                               newElement_0_groupIdCtorSignature.c_str(),
                                                                               entry_0.groupId, newElement_0_groupId);
                jobject newElement_0_groupKeySetID;
                std::string newElement_0_groupKeySetIDClassName     = "java/lang/Integer";
                std::string newElement_0_groupKeySetIDCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_groupKeySetIDClassName.c_str(),
                                                                               newElement_0_groupKeySetIDCtorSignature.c_str(),
                                                                               entry_0.groupKeySetID, newElement_0_groupKeySetID);

                jclass groupKeyMapStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$GroupKeyManagementClusterGroupKeyMapStruct",
                    groupKeyMapStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$GroupKeyManagementClusterGroupKeyMapStruct");
                    return nullptr;
                }
                jmethodID groupKeyMapStructStructCtor = env->GetMethodID(
                    groupKeyMapStructStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;)V");
                if (groupKeyMapStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$GroupKeyManagementClusterGroupKeyMapStruct constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(groupKeyMapStructStructClass, groupKeyMapStructStructCtor, newElement_0_fabricIndex,
                                              newElement_0_groupId, newElement_0_groupKeySetID);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::GroupTable::Id: {
            using TypeInfo = Attributes::GroupTable::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_groupId;
                std::string newElement_0_groupIdClassName     = "java/lang/Integer";
                std::string newElement_0_groupIdCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_groupIdClassName.c_str(),
                                                                               newElement_0_groupIdCtorSignature.c_str(),
                                                                               entry_0.groupId, newElement_0_groupId);
                jobject newElement_0_endpoints;
                chip::JniReferences::GetInstance().CreateArrayList(newElement_0_endpoints);

                auto iter_newElement_0_endpoints_NaN = entry_0.endpoints.begin();
                while (iter_newElement_0_endpoints_NaN.Next())
                {
                    auto & entry_NaN = iter_newElement_0_endpoints_NaN.GetValue();
                    jobject newElement_NaN;
                    std::string newElement_NaNClassName     = "java/lang/Integer";
                    std::string newElement_NaNCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                        newElement_NaNClassName.c_str(), newElement_NaNCtorSignature.c_str(), entry_NaN, newElement_NaN);
                    chip::JniReferences::GetInstance().AddToArrayList(newElement_0_endpoints, newElement_NaN);
                }
                jobject newElement_0_groupName;
                if (!entry_0.groupName.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_groupName);
                }
                else
                {
                    newElement_0_groupName =
                        env->NewStringUTF(std::string(entry_0.groupName.Value().data(), entry_0.groupName.Value().size()).c_str());
                }

                jclass groupInfoMapStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$GroupKeyManagementClusterGroupInfoMapStruct",
                    groupInfoMapStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$GroupKeyManagementClusterGroupInfoMapStruct");
                    return nullptr;
                }
                jmethodID groupInfoMapStructStructCtor =
                    env->GetMethodID(groupInfoMapStructStructClass, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/ArrayList;Ljava/util/Optional;)V");
                if (groupInfoMapStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$GroupKeyManagementClusterGroupInfoMapStruct constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(groupInfoMapStructStructClass, groupInfoMapStructStructCtor, newElement_0_fabricIndex,
                                              newElement_0_groupId, newElement_0_endpoints, newElement_0_groupName);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::MaxGroupsPerFabric::Id: {
            using TypeInfo = Attributes::MaxGroupsPerFabric::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MaxGroupKeysPerFabric::Id: {
            using TypeInfo = Attributes::MaxGroupKeysPerFabric::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Groups::Id: {
        using namespace app::Clusters::Groups;
        switch (aPath.mAttributeId)
        {
        case Attributes::NameSupport::Id: {
            using TypeInfo = Attributes::NameSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Identify::Id: {
        using namespace app::Clusters::Identify;
        switch (aPath.mAttributeId)
        {
        case Attributes::IdentifyTime::Id: {
            using TypeInfo = Attributes::IdentifyTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::IdentifyType::Id: {
            using TypeInfo = Attributes::IdentifyType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::IlluminanceMeasurement::Id: {
        using namespace app::Clusters::IlluminanceMeasurement;
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::LightSensorType::Id: {
            using TypeInfo = Attributes::LightSensorType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::KeypadInput::Id: {
        using namespace app::Clusters::KeypadInput;
        switch (aPath.mAttributeId)
        {
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::LevelControl::Id: {
        using namespace app::Clusters::LevelControl;
        switch (aPath.mAttributeId)
        {
        case Attributes::CurrentLevel::Id: {
            using TypeInfo = Attributes::CurrentLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::RemainingTime::Id: {
            using TypeInfo = Attributes::RemainingTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MinLevel::Id: {
            using TypeInfo = Attributes::MinLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxLevel::Id: {
            using TypeInfo = Attributes::MaxLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentFrequency::Id: {
            using TypeInfo = Attributes::CurrentFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MinFrequency::Id: {
            using TypeInfo = Attributes::MinFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MaxFrequency::Id: {
            using TypeInfo = Attributes::MaxFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Options::Id: {
            using TypeInfo = Attributes::Options::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::OnOffTransitionTime::Id: {
            using TypeInfo = Attributes::OnOffTransitionTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::OnLevel::Id: {
            using TypeInfo = Attributes::OnLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::OnTransitionTime::Id: {
            using TypeInfo = Attributes::OnTransitionTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::OffTransitionTime::Id: {
            using TypeInfo = Attributes::OffTransitionTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::DefaultMoveRate::Id: {
            using TypeInfo = Attributes::DefaultMoveRate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::StartUpCurrentLevel::Id: {
            using TypeInfo = Attributes::StartUpCurrentLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::LocalizationConfiguration::Id: {
        using namespace app::Clusters::LocalizationConfiguration;
        switch (aPath.mAttributeId)
        {
        case Attributes::ActiveLocale::Id: {
            using TypeInfo = Attributes::ActiveLocale::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::SupportedLocales::Id: {
            using TypeInfo = Attributes::SupportedLocales::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                newElement_0 = env->NewStringUTF(std::string(entry_0.data(), entry_0.size()).c_str());
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::LowPower::Id: {
        using namespace app::Clusters::LowPower;
        switch (aPath.mAttributeId)
        {
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::MediaInput::Id: {
        using namespace app::Clusters::MediaInput;
        switch (aPath.mAttributeId)
        {
        case Attributes::MediaInputList::Id: {
            using TypeInfo = Attributes::MediaInputList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_index;
                std::string newElement_0_indexClassName     = "java/lang/Integer";
                std::string newElement_0_indexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_indexClassName.c_str(),
                                                                              newElement_0_indexCtorSignature.c_str(),
                                                                              entry_0.index, newElement_0_index);
                jobject newElement_0_inputType;
                std::string newElement_0_inputTypeClassName     = "java/lang/Integer";
                std::string newElement_0_inputTypeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_inputTypeClassName.c_str(), newElement_0_inputTypeCtorSignature.c_str(),
                    static_cast<uint8_t>(entry_0.inputType), newElement_0_inputType);
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());
                jobject newElement_0_description;
                newElement_0_description =
                    env->NewStringUTF(std::string(entry_0.description.data(), entry_0.description.size()).c_str());

                jclass inputInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$MediaInputClusterInputInfo", inputInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$MediaInputClusterInputInfo");
                    return nullptr;
                }
                jmethodID inputInfoStructCtor =
                    env->GetMethodID(inputInfoStructClass, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/String;Ljava/lang/String;)V");
                if (inputInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$MediaInputClusterInputInfo constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(inputInfoStructClass, inputInfoStructCtor, newElement_0_index, newElement_0_inputType,
                                              newElement_0_name, newElement_0_description);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentMediaInput::Id: {
            using TypeInfo = Attributes::CurrentMediaInput::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::MediaPlayback::Id: {
        using namespace app::Clusters::MediaPlayback;
        switch (aPath.mAttributeId)
        {
        case Attributes::PlaybackState::Id: {
            using TypeInfo = Attributes::PlaybackState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::StartTime::Id: {
            using TypeInfo = Attributes::StartTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::Duration::Id: {
            using TypeInfo = Attributes::Duration::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::Position::Id: {
            using TypeInfo = Attributes::Position::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jobject value_updatedAt;
            std::string value_updatedAtClassName     = "java/lang/Long";
            std::string value_updatedAtCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(
                value_updatedAtClassName.c_str(), value_updatedAtCtorSignature.c_str(), cppValue.updatedAt, value_updatedAt);
            jobject value_position;
            if (cppValue.position.IsNull())
            {
                value_position = nullptr;
            }
            else
            {
                std::string value_positionClassName     = "java/lang/Long";
                std::string value_positionCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(value_positionClassName.c_str(),
                                                                               value_positionCtorSignature.c_str(),
                                                                               cppValue.position.Value(), value_position);
            }

            jclass playbackPositionStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterPlaybackPosition", playbackPositionStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterPlaybackPosition");
                return nullptr;
            }
            jmethodID playbackPositionStructCtor =
                env->GetMethodID(playbackPositionStructClass, "<init>", "(Ljava/lang/Long;Ljava/lang/Long;)V");
            if (playbackPositionStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterPlaybackPosition constructor");
                return nullptr;
            }

            value = env->NewObject(playbackPositionStructClass, playbackPositionStructCtor, value_updatedAt, value_position);
            return value;
        }
        case Attributes::PlaybackSpeed::Id: {
            using TypeInfo = Attributes::PlaybackSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Float";
            std::string valueCtorSignature = "(F)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<float>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        cppValue, value);
            return value;
        }
        case Attributes::SeekRangeEnd::Id: {
            using TypeInfo = Attributes::SeekRangeEnd::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::SeekRangeStart::Id: {
            using TypeInfo = Attributes::SeekRangeStart::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ModeSelect::Id: {
        using namespace app::Clusters::ModeSelect;
        switch (aPath.mAttributeId)
        {
        case Attributes::CurrentMode::Id: {
            using TypeInfo = Attributes::CurrentMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::SupportedModes::Id: {
            using TypeInfo = Attributes::SupportedModes::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_label;
                newElement_0_label = env->NewStringUTF(std::string(entry_0.label.data(), entry_0.label.size()).c_str());
                jobject newElement_0_mode;
                std::string newElement_0_modeClassName     = "java/lang/Integer";
                std::string newElement_0_modeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_modeClassName.c_str(), newElement_0_modeCtorSignature.c_str(), entry_0.mode, newElement_0_mode);
                jobject newElement_0_semanticTag;
                std::string newElement_0_semanticTagClassName     = "java/lang/Long";
                std::string newElement_0_semanticTagCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(newElement_0_semanticTagClassName.c_str(),
                                                                               newElement_0_semanticTagCtorSignature.c_str(),
                                                                               entry_0.semanticTag, newElement_0_semanticTag);

                jclass modeOptionStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ModeSelectClusterModeOptionStruct", modeOptionStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ModeSelectClusterModeOptionStruct");
                    return nullptr;
                }
                jmethodID modeOptionStructStructCtor = env->GetMethodID(modeOptionStructStructClass, "<init>",
                                                                        "(Ljava/lang/String;Ljava/lang/Integer;Ljava/lang/Long;)V");
                if (modeOptionStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ModeSelectClusterModeOptionStruct constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(modeOptionStructStructClass, modeOptionStructStructCtor, newElement_0_label,
                                              newElement_0_mode, newElement_0_semanticTag);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::OnMode::Id: {
            using TypeInfo = Attributes::OnMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::StartUpMode::Id: {
            using TypeInfo = Attributes::StartUpMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Description::Id: {
            using TypeInfo = Attributes::Description::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::NetworkCommissioning::Id: {
        using namespace app::Clusters::NetworkCommissioning;
        switch (aPath.mAttributeId)
        {
        case Attributes::MaxNetworks::Id: {
            using TypeInfo = Attributes::MaxNetworks::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Networks::Id: {
            using TypeInfo = Attributes::Networks::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_networkID;
                jbyteArray newElement_0_networkIDByteArray = env->NewByteArray(static_cast<jsize>(entry_0.networkID.size()));
                env->SetByteArrayRegion(newElement_0_networkIDByteArray, 0, static_cast<jsize>(entry_0.networkID.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.networkID.data()));
                newElement_0_networkID = newElement_0_networkIDByteArray;
                jobject newElement_0_connected;
                std::string newElement_0_connectedClassName     = "java/lang/Boolean";
                std::string newElement_0_connectedCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_connectedClassName.c_str(),
                                                                           newElement_0_connectedCtorSignature.c_str(),
                                                                           entry_0.connected, newElement_0_connected);

                jclass networkInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$NetworkCommissioningClusterNetworkInfo", networkInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$NetworkCommissioningClusterNetworkInfo");
                    return nullptr;
                }
                jmethodID networkInfoStructCtor = env->GetMethodID(networkInfoStructClass, "<init>", "([BLjava/lang/Boolean;)V");
                if (networkInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$NetworkCommissioningClusterNetworkInfo constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(networkInfoStructClass, networkInfoStructCtor, newElement_0_networkID, newElement_0_connected);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ScanMaxTimeSeconds::Id: {
            using TypeInfo = Attributes::ScanMaxTimeSeconds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ConnectMaxTimeSeconds::Id: {
            using TypeInfo = Attributes::ConnectMaxTimeSeconds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::InterfaceEnabled::Id: {
            using TypeInfo = Attributes::InterfaceEnabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::LastNetworkingStatus::Id: {
            using TypeInfo = Attributes::LastNetworkingStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::LastNetworkID::Id: {
            using TypeInfo = Attributes::LastNetworkID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.size()));
            env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.data()));
            value = valueByteArray;
            return value;
        }
        case Attributes::LastConnectErrorValue::Id: {
            using TypeInfo = Attributes::LastConnectErrorValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::OtaSoftwareUpdateProvider::Id: {
        using namespace app::Clusters::OtaSoftwareUpdateProvider;
        switch (aPath.mAttributeId)
        {
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::OtaSoftwareUpdateRequestor::Id: {
        using namespace app::Clusters::OtaSoftwareUpdateRequestor;
        switch (aPath.mAttributeId)
        {
        case Attributes::DefaultOtaProviders::Id: {
            using TypeInfo = Attributes::DefaultOtaProviders::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_providerNodeID;
                std::string newElement_0_providerNodeIDClassName     = "java/lang/Long";
                std::string newElement_0_providerNodeIDCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(newElement_0_providerNodeIDClassName.c_str(),
                                                                               newElement_0_providerNodeIDCtorSignature.c_str(),
                                                                               entry_0.providerNodeID, newElement_0_providerNodeID);
                jobject newElement_0_endpoint;
                std::string newElement_0_endpointClassName     = "java/lang/Integer";
                std::string newElement_0_endpointCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_endpointClassName.c_str(),
                                                                               newElement_0_endpointCtorSignature.c_str(),
                                                                               entry_0.endpoint, newElement_0_endpoint);

                jclass providerLocationStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$OtaSoftwareUpdateRequestorClusterProviderLocation",
                    providerLocationStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$OtaSoftwareUpdateRequestorClusterProviderLocation");
                    return nullptr;
                }
                jmethodID providerLocationStructCtor = env->GetMethodID(
                    providerLocationStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Integer;)V");
                if (providerLocationStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$OtaSoftwareUpdateRequestorClusterProviderLocation constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(providerLocationStructClass, providerLocationStructCtor, newElement_0_fabricIndex,
                                              newElement_0_providerNodeID, newElement_0_endpoint);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::UpdatePossible::Id: {
            using TypeInfo = Attributes::UpdatePossible::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::UpdateState::Id: {
            using TypeInfo = Attributes::UpdateState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::UpdateStateProgress::Id: {
            using TypeInfo = Attributes::UpdateStateProgress::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::OccupancySensing::Id: {
        using namespace app::Clusters::OccupancySensing;
        switch (aPath.mAttributeId)
        {
        case Attributes::Occupancy::Id: {
            using TypeInfo = Attributes::Occupancy::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::OccupancySensorType::Id: {
            using TypeInfo = Attributes::OccupancySensorType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::OccupancySensorTypeBitmap::Id: {
            using TypeInfo = Attributes::OccupancySensorTypeBitmap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::OnOff::Id: {
        using namespace app::Clusters::OnOff;
        switch (aPath.mAttributeId)
        {
        case Attributes::OnOff::Id: {
            using TypeInfo = Attributes::OnOff::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::GlobalSceneControl::Id: {
            using TypeInfo = Attributes::GlobalSceneControl::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::OnTime::Id: {
            using TypeInfo = Attributes::OnTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::OffWaitTime::Id: {
            using TypeInfo = Attributes::OffWaitTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::StartUpOnOff::Id: {
            using TypeInfo = Attributes::StartUpOnOff::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::OnOffSwitchConfiguration::Id: {
        using namespace app::Clusters::OnOffSwitchConfiguration;
        switch (aPath.mAttributeId)
        {
        case Attributes::SwitchType::Id: {
            using TypeInfo = Attributes::SwitchType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::SwitchActions::Id: {
            using TypeInfo = Attributes::SwitchActions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::OperationalCredentials::Id: {
        using namespace app::Clusters::OperationalCredentials;
        switch (aPath.mAttributeId)
        {
        case Attributes::NOCs::Id: {
            using TypeInfo = Attributes::NOCs::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_noc;
                jbyteArray newElement_0_nocByteArray = env->NewByteArray(static_cast<jsize>(entry_0.noc.size()));
                env->SetByteArrayRegion(newElement_0_nocByteArray, 0, static_cast<jsize>(entry_0.noc.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.noc.data()));
                newElement_0_noc = newElement_0_nocByteArray;
                jobject newElement_0_icac;
                if (entry_0.icac.IsNull())
                {
                    newElement_0_icac = nullptr;
                }
                else
                {
                    jbyteArray newElement_0_icacByteArray = env->NewByteArray(static_cast<jsize>(entry_0.icac.Value().size()));
                    env->SetByteArrayRegion(newElement_0_icacByteArray, 0, static_cast<jsize>(entry_0.icac.Value().size()),
                                            reinterpret_cast<const jbyte *>(entry_0.icac.Value().data()));
                    newElement_0_icac = newElement_0_icacByteArray;
                }

                jclass NOCStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$OperationalCredentialsClusterNOCStruct", NOCStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$OperationalCredentialsClusterNOCStruct");
                    return nullptr;
                }
                jmethodID NOCStructStructCtor = env->GetMethodID(NOCStructStructClass, "<init>", "(Ljava/lang/Integer;[B[B)V");
                if (NOCStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$OperationalCredentialsClusterNOCStruct constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(NOCStructStructClass, NOCStructStructCtor, newElement_0_fabricIndex, newElement_0_noc,
                                              newElement_0_icac);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FabricsList::Id: {
            using TypeInfo = Attributes::FabricsList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                              newElement_0_fabricIndexCtorSignature.c_str(),
                                                                              entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_rootPublicKey;
                jbyteArray newElement_0_rootPublicKeyByteArray =
                    env->NewByteArray(static_cast<jsize>(entry_0.rootPublicKey.size()));
                env->SetByteArrayRegion(newElement_0_rootPublicKeyByteArray, 0, static_cast<jsize>(entry_0.rootPublicKey.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.rootPublicKey.data()));
                newElement_0_rootPublicKey = newElement_0_rootPublicKeyByteArray;
                jobject newElement_0_vendorId;
                std::string newElement_0_vendorIdClassName     = "java/lang/Integer";
                std::string newElement_0_vendorIdCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_vendorIdClassName.c_str(),
                                                                               newElement_0_vendorIdCtorSignature.c_str(),
                                                                               entry_0.vendorId, newElement_0_vendorId);
                jobject newElement_0_fabricId;
                std::string newElement_0_fabricIdClassName     = "java/lang/Long";
                std::string newElement_0_fabricIdCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(newElement_0_fabricIdClassName.c_str(),
                                                                               newElement_0_fabricIdCtorSignature.c_str(),
                                                                               entry_0.fabricId, newElement_0_fabricId);
                jobject newElement_0_nodeId;
                std::string newElement_0_nodeIdClassName     = "java/lang/Long";
                std::string newElement_0_nodeIdCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(newElement_0_nodeIdClassName.c_str(),
                                                                               newElement_0_nodeIdCtorSignature.c_str(),
                                                                               entry_0.nodeId, newElement_0_nodeId);
                jobject newElement_0_label;
                newElement_0_label = env->NewStringUTF(std::string(entry_0.label.data(), entry_0.label.size()).c_str());

                jclass fabricDescriptorStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$OperationalCredentialsClusterFabricDescriptor",
                    fabricDescriptorStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$OperationalCredentialsClusterFabricDescriptor");
                    return nullptr;
                }
                jmethodID fabricDescriptorStructCtor = env->GetMethodID(
                    fabricDescriptorStructClass, "<init>",
                    "(Ljava/lang/Integer;[BLjava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/String;)V");
                if (fabricDescriptorStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$OperationalCredentialsClusterFabricDescriptor constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(fabricDescriptorStructClass, fabricDescriptorStructCtor, newElement_0_fabricIndex,
                                              newElement_0_rootPublicKey, newElement_0_vendorId, newElement_0_fabricId,
                                              newElement_0_nodeId, newElement_0_label);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::SupportedFabrics::Id: {
            using TypeInfo = Attributes::SupportedFabrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CommissionedFabrics::Id: {
            using TypeInfo = Attributes::CommissionedFabrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::TrustedRootCertificates::Id: {
            using TypeInfo = Attributes::TrustedRootCertificates::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jbyteArray newElement_0ByteArray = env->NewByteArray(static_cast<jsize>(entry_0.size()));
                env->SetByteArrayRegion(newElement_0ByteArray, 0, static_cast<jsize>(entry_0.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.data()));
                newElement_0 = newElement_0ByteArray;
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentFabricIndex::Id: {
            using TypeInfo = Attributes::CurrentFabricIndex::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::PowerSource::Id: {
        using namespace app::Clusters::PowerSource;
        switch (aPath.mAttributeId)
        {
        case Attributes::Status::Id: {
            using TypeInfo = Attributes::Status::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Order::Id: {
            using TypeInfo = Attributes::Order::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Description::Id: {
            using TypeInfo = Attributes::Description::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::BatteryVoltage::Id: {
            using TypeInfo = Attributes::BatteryVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BatteryPercentRemaining::Id: {
            using TypeInfo = Attributes::BatteryPercentRemaining::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::BatteryTimeRemaining::Id: {
            using TypeInfo = Attributes::BatteryTimeRemaining::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BatteryChargeLevel::Id: {
            using TypeInfo = Attributes::BatteryChargeLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ActiveBatteryFaults::Id: {
            using TypeInfo = Attributes::ActiveBatteryFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::BatteryChargeState::Id: {
            using TypeInfo = Attributes::BatteryChargeState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::PowerSourceConfiguration::Id: {
        using namespace app::Clusters::PowerSourceConfiguration;
        switch (aPath.mAttributeId)
        {
        case Attributes::Sources::Id: {
            using TypeInfo = Attributes::Sources::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::PressureMeasurement::Id: {
        using namespace app::Clusters::PressureMeasurement;
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::PumpConfigurationAndControl::Id: {
        using namespace app::Clusters::PumpConfigurationAndControl;
        switch (aPath.mAttributeId)
        {
        case Attributes::MaxPressure::Id: {
            using TypeInfo = Attributes::MaxPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxSpeed::Id: {
            using TypeInfo = Attributes::MaxSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MaxFlow::Id: {
            using TypeInfo = Attributes::MaxFlow::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MinConstPressure::Id: {
            using TypeInfo = Attributes::MinConstPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxConstPressure::Id: {
            using TypeInfo = Attributes::MaxConstPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinCompPressure::Id: {
            using TypeInfo = Attributes::MinCompPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxCompPressure::Id: {
            using TypeInfo = Attributes::MaxCompPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinConstSpeed::Id: {
            using TypeInfo = Attributes::MinConstSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MaxConstSpeed::Id: {
            using TypeInfo = Attributes::MaxConstSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MinConstFlow::Id: {
            using TypeInfo = Attributes::MinConstFlow::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MaxConstFlow::Id: {
            using TypeInfo = Attributes::MaxConstFlow::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MinConstTemp::Id: {
            using TypeInfo = Attributes::MinConstTemp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxConstTemp::Id: {
            using TypeInfo = Attributes::MaxConstTemp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::PumpStatus::Id: {
            using TypeInfo = Attributes::PumpStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::EffectiveOperationMode::Id: {
            using TypeInfo = Attributes::EffectiveOperationMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::EffectiveControlMode::Id: {
            using TypeInfo = Attributes::EffectiveControlMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Capacity::Id: {
            using TypeInfo = Attributes::Capacity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Speed::Id: {
            using TypeInfo = Attributes::Speed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::LifetimeRunningHours::Id: {
            using TypeInfo = Attributes::LifetimeRunningHours::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::Power::Id: {
            using TypeInfo = Attributes::Power::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::LifetimeEnergyConsumed::Id: {
            using TypeInfo = Attributes::LifetimeEnergyConsumed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::OperationMode::Id: {
            using TypeInfo = Attributes::OperationMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ControlMode::Id: {
            using TypeInfo = Attributes::ControlMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AlarmMask::Id: {
            using TypeInfo = Attributes::AlarmMask::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::RelativeHumidityMeasurement::Id: {
        using namespace app::Clusters::RelativeHumidityMeasurement;
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Scenes::Id: {
        using namespace app::Clusters::Scenes;
        switch (aPath.mAttributeId)
        {
        case Attributes::SceneCount::Id: {
            using TypeInfo = Attributes::SceneCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentScene::Id: {
            using TypeInfo = Attributes::CurrentScene::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentGroup::Id: {
            using TypeInfo = Attributes::CurrentGroup::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::SceneValid::Id: {
            using TypeInfo = Attributes::SceneValid::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::NameSupport::Id: {
            using TypeInfo = Attributes::NameSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::SoftwareDiagnostics::Id: {
        using namespace app::Clusters::SoftwareDiagnostics;
        switch (aPath.mAttributeId)
        {
        case Attributes::ThreadMetrics::Id: {
            using TypeInfo = Attributes::ThreadMetrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_id;
                std::string newElement_0_idClassName     = "java/lang/Long";
                std::string newElement_0_idCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(
                    newElement_0_idClassName.c_str(), newElement_0_idCtorSignature.c_str(), entry_0.id, newElement_0_id);
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());
                jobject newElement_0_stackFreeCurrent;
                std::string newElement_0_stackFreeCurrentClassName     = "java/lang/Long";
                std::string newElement_0_stackFreeCurrentCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0_stackFreeCurrentClassName.c_str(), newElement_0_stackFreeCurrentCtorSignature.c_str(),
                    entry_0.stackFreeCurrent, newElement_0_stackFreeCurrent);
                jobject newElement_0_stackFreeMinimum;
                std::string newElement_0_stackFreeMinimumClassName     = "java/lang/Long";
                std::string newElement_0_stackFreeMinimumCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0_stackFreeMinimumClassName.c_str(), newElement_0_stackFreeMinimumCtorSignature.c_str(),
                    entry_0.stackFreeMinimum, newElement_0_stackFreeMinimum);
                jobject newElement_0_stackSize;
                std::string newElement_0_stackSizeClassName     = "java/lang/Long";
                std::string newElement_0_stackSizeCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(newElement_0_stackSizeClassName.c_str(),
                                                                               newElement_0_stackSizeCtorSignature.c_str(),
                                                                               entry_0.stackSize, newElement_0_stackSize);

                jclass threadMetricsStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$SoftwareDiagnosticsClusterThreadMetrics", threadMetricsStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$SoftwareDiagnosticsClusterThreadMetrics");
                    return nullptr;
                }
                jmethodID threadMetricsStructCtor =
                    env->GetMethodID(threadMetricsStructClass, "<init>",
                                     "(Ljava/lang/Long;Ljava/lang/String;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Long;)V");
                if (threadMetricsStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$SoftwareDiagnosticsClusterThreadMetrics constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(threadMetricsStructClass, threadMetricsStructCtor, newElement_0_id, newElement_0_name,
                                              newElement_0_stackFreeCurrent, newElement_0_stackFreeMinimum, newElement_0_stackSize);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentHeapFree::Id: {
            using TypeInfo = Attributes::CurrentHeapFree::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CurrentHeapUsed::Id: {
            using TypeInfo = Attributes::CurrentHeapUsed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CurrentHeapHighWatermark::Id: {
            using TypeInfo = Attributes::CurrentHeapHighWatermark::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Switch::Id: {
        using namespace app::Clusters::Switch;
        switch (aPath.mAttributeId)
        {
        case Attributes::NumberOfPositions::Id: {
            using TypeInfo = Attributes::NumberOfPositions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentPosition::Id: {
            using TypeInfo = Attributes::CurrentPosition::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MultiPressMax::Id: {
            using TypeInfo = Attributes::MultiPressMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::TargetNavigator::Id: {
        using namespace app::Clusters::TargetNavigator;
        switch (aPath.mAttributeId)
        {
        case Attributes::TargetNavigatorList::Id: {
            using TypeInfo = Attributes::TargetNavigatorList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_identifier;
                std::string newElement_0_identifierClassName     = "java/lang/Integer";
                std::string newElement_0_identifierCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_identifierClassName.c_str(),
                                                                              newElement_0_identifierCtorSignature.c_str(),
                                                                              entry_0.identifier, newElement_0_identifier);
                jobject newElement_0_name;
                newElement_0_name = env->NewStringUTF(std::string(entry_0.name.data(), entry_0.name.size()).c_str());

                jclass targetInfoStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$TargetNavigatorClusterTargetInfo", targetInfoStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$TargetNavigatorClusterTargetInfo");
                    return nullptr;
                }
                jmethodID targetInfoStructCtor =
                    env->GetMethodID(targetInfoStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/String;)V");
                if (targetInfoStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$TargetNavigatorClusterTargetInfo constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(targetInfoStructClass, targetInfoStructCtor, newElement_0_identifier, newElement_0_name);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentNavigatorTarget::Id: {
            using TypeInfo = Attributes::CurrentNavigatorTarget::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::TemperatureMeasurement::Id: {
        using namespace app::Clusters::TemperatureMeasurement;
        switch (aPath.mAttributeId)
        {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::TestCluster::Id: {
        using namespace app::Clusters::TestCluster;
        switch (aPath.mAttributeId)
        {
        case Attributes::Boolean::Id: {
            using TypeInfo = Attributes::Boolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::Bitmap8::Id: {
            using TypeInfo = Attributes::Bitmap8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Bitmap16::Id: {
            using TypeInfo = Attributes::Bitmap16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Bitmap32::Id: {
            using TypeInfo = Attributes::Bitmap32::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Bitmap64::Id: {
            using TypeInfo = Attributes::Bitmap64::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int8u::Id: {
            using TypeInfo = Attributes::Int8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int16u::Id: {
            using TypeInfo = Attributes::Int16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int24u::Id: {
            using TypeInfo = Attributes::Int24u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int32u::Id: {
            using TypeInfo = Attributes::Int32u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int40u::Id: {
            using TypeInfo = Attributes::Int40u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int48u::Id: {
            using TypeInfo = Attributes::Int48u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int56u::Id: {
            using TypeInfo = Attributes::Int56u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int64u::Id: {
            using TypeInfo = Attributes::Int64u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Int8s::Id: {
            using TypeInfo = Attributes::Int8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                         cppValue, value);
            return value;
        }
        case Attributes::Int16s::Id: {
            using TypeInfo = Attributes::Int16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int24s::Id: {
            using TypeInfo = Attributes::Int24s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int32s::Id: {
            using TypeInfo = Attributes::Int32s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int40s::Id: {
            using TypeInfo = Attributes::Int40s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int48s::Id: {
            using TypeInfo = Attributes::Int48s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int56s::Id: {
            using TypeInfo = Attributes::Int56s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Int64s::Id: {
            using TypeInfo = Attributes::Int64s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Enum8::Id: {
            using TypeInfo = Attributes::Enum8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::Enum16::Id: {
            using TypeInfo = Attributes::Enum16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::FloatSingle::Id: {
            using TypeInfo = Attributes::FloatSingle::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Float";
            std::string valueCtorSignature = "(F)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<float>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        cppValue, value);
            return value;
        }
        case Attributes::FloatDouble::Id: {
            using TypeInfo = Attributes::FloatDouble::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Double";
            std::string valueCtorSignature = "(D)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<double>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                         cppValue, value);
            return value;
        }
        case Attributes::OctetString::Id: {
            using TypeInfo = Attributes::OctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.size()));
            env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.data()));
            value = valueByteArray;
            return value;
        }
        case Attributes::ListInt8u::Id: {
            using TypeInfo = Attributes::ListInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ListOctetString::Id: {
            using TypeInfo = Attributes::ListOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jbyteArray newElement_0ByteArray = env->NewByteArray(static_cast<jsize>(entry_0.size()));
                env->SetByteArrayRegion(newElement_0ByteArray, 0, static_cast<jsize>(entry_0.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.data()));
                newElement_0 = newElement_0ByteArray;
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ListStructOctetString::Id: {
            using TypeInfo = Attributes::ListStructOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Long";
                std::string newElement_0_fabricIndexCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(newElement_0_fabricIndexClassName.c_str(),
                                                                               newElement_0_fabricIndexCtorSignature.c_str(),
                                                                               entry_0.fabricIndex, newElement_0_fabricIndex);
                jobject newElement_0_operationalCert;
                jbyteArray newElement_0_operationalCertByteArray =
                    env->NewByteArray(static_cast<jsize>(entry_0.operationalCert.size()));
                env->SetByteArrayRegion(newElement_0_operationalCertByteArray, 0,
                                        static_cast<jsize>(entry_0.operationalCert.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.operationalCert.data()));
                newElement_0_operationalCert = newElement_0_operationalCertByteArray;

                jclass testListStructOctetStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$TestClusterClusterTestListStructOctet", testListStructOctetStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterTestListStructOctet");
                    return nullptr;
                }
                jmethodID testListStructOctetStructCtor =
                    env->GetMethodID(testListStructOctetStructClass, "<init>", "(Ljava/lang/Long;[B)V");
                if (testListStructOctetStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterTestListStructOctet constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(testListStructOctetStructClass, testListStructOctetStructCtor,
                                              newElement_0_fabricIndex, newElement_0_operationalCert);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::LongOctetString::Id: {
            using TypeInfo = Attributes::LongOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.size()));
            env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.data()));
            value = valueByteArray;
            return value;
        }
        case Attributes::CharString::Id: {
            using TypeInfo = Attributes::CharString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::LongCharString::Id: {
            using TypeInfo = Attributes::LongCharString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::EpochUs::Id: {
            using TypeInfo = Attributes::EpochUs::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::EpochS::Id: {
            using TypeInfo = Attributes::EpochS::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::VendorId::Id: {
            using TypeInfo = Attributes::VendorId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           static_cast<uint16_t>(cppValue), value);
            return value;
        }
        case Attributes::ListNullablesAndOptionalsStruct::Id: {
            using TypeInfo = Attributes::ListNullablesAndOptionalsStruct::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_nullableInt;
                if (entry_0.nullableInt.IsNull())
                {
                    newElement_0_nullableInt = nullptr;
                }
                else
                {
                    std::string newElement_0_nullableIntClassName     = "java/lang/Integer";
                    std::string newElement_0_nullableIntCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                        newElement_0_nullableIntClassName.c_str(), newElement_0_nullableIntCtorSignature.c_str(),
                        entry_0.nullableInt.Value(), newElement_0_nullableInt);
                }
                jobject newElement_0_optionalInt;
                if (!entry_0.optionalInt.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_optionalInt);
                }
                else
                {
                    std::string newElement_0_optionalIntClassName     = "java/lang/Integer";
                    std::string newElement_0_optionalIntCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                        newElement_0_optionalIntClassName.c_str(), newElement_0_optionalIntCtorSignature.c_str(),
                        entry_0.optionalInt.Value(), newElement_0_optionalInt);
                }
                jobject newElement_0_nullableOptionalInt;
                if (!entry_0.nullableOptionalInt.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_nullableOptionalInt);
                }
                else
                {
                    if (entry_0.nullableOptionalInt.Value().IsNull())
                    {
                        newElement_0_nullableOptionalInt = nullptr;
                    }
                    else
                    {
                        std::string newElement_0_nullableOptionalIntClassName     = "java/lang/Integer";
                        std::string newElement_0_nullableOptionalIntCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(
                            newElement_0_nullableOptionalIntClassName.c_str(),
                            newElement_0_nullableOptionalIntCtorSignature.c_str(), entry_0.nullableOptionalInt.Value().Value(),
                            newElement_0_nullableOptionalInt);
                    }
                }
                jobject newElement_0_nullableString;
                if (entry_0.nullableString.IsNull())
                {
                    newElement_0_nullableString = nullptr;
                }
                else
                {
                    newElement_0_nullableString = env->NewStringUTF(
                        std::string(entry_0.nullableString.Value().data(), entry_0.nullableString.Value().size()).c_str());
                }
                jobject newElement_0_optionalString;
                if (!entry_0.optionalString.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_optionalString);
                }
                else
                {
                    newElement_0_optionalString = env->NewStringUTF(
                        std::string(entry_0.optionalString.Value().data(), entry_0.optionalString.Value().size()).c_str());
                }
                jobject newElement_0_nullableOptionalString;
                if (!entry_0.nullableOptionalString.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_nullableOptionalString);
                }
                else
                {
                    if (entry_0.nullableOptionalString.Value().IsNull())
                    {
                        newElement_0_nullableOptionalString = nullptr;
                    }
                    else
                    {
                        newElement_0_nullableOptionalString =
                            env->NewStringUTF(std::string(entry_0.nullableOptionalString.Value().Value().data(),
                                                          entry_0.nullableOptionalString.Value().Value().size())
                                                  .c_str());
                    }
                }
                jobject newElement_0_nullableStruct;
                if (entry_0.nullableStruct.IsNull())
                {
                    newElement_0_nullableStruct = nullptr;
                }
                else
                {
                    jobject newElement_0_nullableStruct_a;
                    std::string newElement_0_nullableStruct_aClassName     = "java/lang/Integer";
                    std::string newElement_0_nullableStruct_aCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                        newElement_0_nullableStruct_aClassName.c_str(), newElement_0_nullableStruct_aCtorSignature.c_str(),
                        entry_0.nullableStruct.Value().a, newElement_0_nullableStruct_a);
                    jobject newElement_0_nullableStruct_b;
                    std::string newElement_0_nullableStruct_bClassName     = "java/lang/Boolean";
                    std::string newElement_0_nullableStruct_bCtorSignature = "(Z)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                        newElement_0_nullableStruct_bClassName.c_str(), newElement_0_nullableStruct_bCtorSignature.c_str(),
                        entry_0.nullableStruct.Value().b, newElement_0_nullableStruct_b);
                    jobject newElement_0_nullableStruct_c;
                    std::string newElement_0_nullableStruct_cClassName     = "java/lang/Integer";
                    std::string newElement_0_nullableStruct_cCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                        newElement_0_nullableStruct_cClassName.c_str(), newElement_0_nullableStruct_cCtorSignature.c_str(),
                        static_cast<uint8_t>(entry_0.nullableStruct.Value().c), newElement_0_nullableStruct_c);
                    jobject newElement_0_nullableStruct_d;
                    jbyteArray newElement_0_nullableStruct_dByteArray =
                        env->NewByteArray(static_cast<jsize>(entry_0.nullableStruct.Value().d.size()));
                    env->SetByteArrayRegion(newElement_0_nullableStruct_dByteArray, 0,
                                            static_cast<jsize>(entry_0.nullableStruct.Value().d.size()),
                                            reinterpret_cast<const jbyte *>(entry_0.nullableStruct.Value().d.data()));
                    newElement_0_nullableStruct_d = newElement_0_nullableStruct_dByteArray;
                    jobject newElement_0_nullableStruct_e;
                    newElement_0_nullableStruct_e = env->NewStringUTF(
                        std::string(entry_0.nullableStruct.Value().e.data(), entry_0.nullableStruct.Value().e.size()).c_str());
                    jobject newElement_0_nullableStruct_f;
                    std::string newElement_0_nullableStruct_fClassName     = "java/lang/Integer";
                    std::string newElement_0_nullableStruct_fCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                        newElement_0_nullableStruct_fClassName.c_str(), newElement_0_nullableStruct_fCtorSignature.c_str(),
                        entry_0.nullableStruct.Value().f.Raw(), newElement_0_nullableStruct_f);
                    jobject newElement_0_nullableStruct_g;
                    std::string newElement_0_nullableStruct_gClassName     = "java/lang/Float";
                    std::string newElement_0_nullableStruct_gCtorSignature = "(F)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<float>(
                        newElement_0_nullableStruct_gClassName.c_str(), newElement_0_nullableStruct_gCtorSignature.c_str(),
                        entry_0.nullableStruct.Value().g, newElement_0_nullableStruct_g);
                    jobject newElement_0_nullableStruct_h;
                    std::string newElement_0_nullableStruct_hClassName     = "java/lang/Double";
                    std::string newElement_0_nullableStruct_hCtorSignature = "(D)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<double>(
                        newElement_0_nullableStruct_hClassName.c_str(), newElement_0_nullableStruct_hCtorSignature.c_str(),
                        entry_0.nullableStruct.Value().h, newElement_0_nullableStruct_h);

                    jclass simpleStructStructClass;
                    err = chip::JniReferences::GetInstance().GetClassRef(
                        env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                        return nullptr;
                    }
                    jmethodID simpleStructStructCtor =
                        env->GetMethodID(simpleStructStructClass, "<init>",
                                         "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                         "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
                    if (simpleStructStructCtor == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
                        return nullptr;
                    }

                    newElement_0_nullableStruct =
                        env->NewObject(simpleStructStructClass, simpleStructStructCtor, newElement_0_nullableStruct_a,
                                       newElement_0_nullableStruct_b, newElement_0_nullableStruct_c, newElement_0_nullableStruct_d,
                                       newElement_0_nullableStruct_e, newElement_0_nullableStruct_f, newElement_0_nullableStruct_g,
                                       newElement_0_nullableStruct_h);
                }
                jobject newElement_0_optionalStruct;
                if (!entry_0.optionalStruct.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_optionalStruct);
                }
                else
                {
                    jobject newElement_0_optionalStruct_a;
                    std::string newElement_0_optionalStruct_aClassName     = "java/lang/Integer";
                    std::string newElement_0_optionalStruct_aCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                        newElement_0_optionalStruct_aClassName.c_str(), newElement_0_optionalStruct_aCtorSignature.c_str(),
                        entry_0.optionalStruct.Value().a, newElement_0_optionalStruct_a);
                    jobject newElement_0_optionalStruct_b;
                    std::string newElement_0_optionalStruct_bClassName     = "java/lang/Boolean";
                    std::string newElement_0_optionalStruct_bCtorSignature = "(Z)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                        newElement_0_optionalStruct_bClassName.c_str(), newElement_0_optionalStruct_bCtorSignature.c_str(),
                        entry_0.optionalStruct.Value().b, newElement_0_optionalStruct_b);
                    jobject newElement_0_optionalStruct_c;
                    std::string newElement_0_optionalStruct_cClassName     = "java/lang/Integer";
                    std::string newElement_0_optionalStruct_cCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                        newElement_0_optionalStruct_cClassName.c_str(), newElement_0_optionalStruct_cCtorSignature.c_str(),
                        static_cast<uint8_t>(entry_0.optionalStruct.Value().c), newElement_0_optionalStruct_c);
                    jobject newElement_0_optionalStruct_d;
                    jbyteArray newElement_0_optionalStruct_dByteArray =
                        env->NewByteArray(static_cast<jsize>(entry_0.optionalStruct.Value().d.size()));
                    env->SetByteArrayRegion(newElement_0_optionalStruct_dByteArray, 0,
                                            static_cast<jsize>(entry_0.optionalStruct.Value().d.size()),
                                            reinterpret_cast<const jbyte *>(entry_0.optionalStruct.Value().d.data()));
                    newElement_0_optionalStruct_d = newElement_0_optionalStruct_dByteArray;
                    jobject newElement_0_optionalStruct_e;
                    newElement_0_optionalStruct_e = env->NewStringUTF(
                        std::string(entry_0.optionalStruct.Value().e.data(), entry_0.optionalStruct.Value().e.size()).c_str());
                    jobject newElement_0_optionalStruct_f;
                    std::string newElement_0_optionalStruct_fClassName     = "java/lang/Integer";
                    std::string newElement_0_optionalStruct_fCtorSignature = "(I)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                        newElement_0_optionalStruct_fClassName.c_str(), newElement_0_optionalStruct_fCtorSignature.c_str(),
                        entry_0.optionalStruct.Value().f.Raw(), newElement_0_optionalStruct_f);
                    jobject newElement_0_optionalStruct_g;
                    std::string newElement_0_optionalStruct_gClassName     = "java/lang/Float";
                    std::string newElement_0_optionalStruct_gCtorSignature = "(F)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<float>(
                        newElement_0_optionalStruct_gClassName.c_str(), newElement_0_optionalStruct_gCtorSignature.c_str(),
                        entry_0.optionalStruct.Value().g, newElement_0_optionalStruct_g);
                    jobject newElement_0_optionalStruct_h;
                    std::string newElement_0_optionalStruct_hClassName     = "java/lang/Double";
                    std::string newElement_0_optionalStruct_hCtorSignature = "(D)V";
                    chip::JniReferences::GetInstance().CreateBoxedObject<double>(
                        newElement_0_optionalStruct_hClassName.c_str(), newElement_0_optionalStruct_hCtorSignature.c_str(),
                        entry_0.optionalStruct.Value().h, newElement_0_optionalStruct_h);

                    jclass simpleStructStructClass;
                    err = chip::JniReferences::GetInstance().GetClassRef(
                        env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                        return nullptr;
                    }
                    jmethodID simpleStructStructCtor =
                        env->GetMethodID(simpleStructStructClass, "<init>",
                                         "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                         "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
                    if (simpleStructStructCtor == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
                        return nullptr;
                    }

                    newElement_0_optionalStruct =
                        env->NewObject(simpleStructStructClass, simpleStructStructCtor, newElement_0_optionalStruct_a,
                                       newElement_0_optionalStruct_b, newElement_0_optionalStruct_c, newElement_0_optionalStruct_d,
                                       newElement_0_optionalStruct_e, newElement_0_optionalStruct_f, newElement_0_optionalStruct_g,
                                       newElement_0_optionalStruct_h);
                }
                jobject newElement_0_nullableOptionalStruct;
                if (!entry_0.nullableOptionalStruct.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_nullableOptionalStruct);
                }
                else
                {
                    if (entry_0.nullableOptionalStruct.Value().IsNull())
                    {
                        newElement_0_nullableOptionalStruct = nullptr;
                    }
                    else
                    {
                        jobject newElement_0_nullableOptionalStruct_a;
                        std::string newElement_0_nullableOptionalStruct_aClassName     = "java/lang/Integer";
                        std::string newElement_0_nullableOptionalStruct_aCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_0_nullableOptionalStruct_aClassName.c_str(),
                            newElement_0_nullableOptionalStruct_aCtorSignature.c_str(),
                            entry_0.nullableOptionalStruct.Value().Value().a, newElement_0_nullableOptionalStruct_a);
                        jobject newElement_0_nullableOptionalStruct_b;
                        std::string newElement_0_nullableOptionalStruct_bClassName     = "java/lang/Boolean";
                        std::string newElement_0_nullableOptionalStruct_bCtorSignature = "(Z)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                            newElement_0_nullableOptionalStruct_bClassName.c_str(),
                            newElement_0_nullableOptionalStruct_bCtorSignature.c_str(),
                            entry_0.nullableOptionalStruct.Value().Value().b, newElement_0_nullableOptionalStruct_b);
                        jobject newElement_0_nullableOptionalStruct_c;
                        std::string newElement_0_nullableOptionalStruct_cClassName     = "java/lang/Integer";
                        std::string newElement_0_nullableOptionalStruct_cCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_0_nullableOptionalStruct_cClassName.c_str(),
                            newElement_0_nullableOptionalStruct_cCtorSignature.c_str(),
                            static_cast<uint8_t>(entry_0.nullableOptionalStruct.Value().Value().c),
                            newElement_0_nullableOptionalStruct_c);
                        jobject newElement_0_nullableOptionalStruct_d;
                        jbyteArray newElement_0_nullableOptionalStruct_dByteArray =
                            env->NewByteArray(static_cast<jsize>(entry_0.nullableOptionalStruct.Value().Value().d.size()));
                        env->SetByteArrayRegion(
                            newElement_0_nullableOptionalStruct_dByteArray, 0,
                            static_cast<jsize>(entry_0.nullableOptionalStruct.Value().Value().d.size()),
                            reinterpret_cast<const jbyte *>(entry_0.nullableOptionalStruct.Value().Value().d.data()));
                        newElement_0_nullableOptionalStruct_d = newElement_0_nullableOptionalStruct_dByteArray;
                        jobject newElement_0_nullableOptionalStruct_e;
                        newElement_0_nullableOptionalStruct_e =
                            env->NewStringUTF(std::string(entry_0.nullableOptionalStruct.Value().Value().e.data(),
                                                          entry_0.nullableOptionalStruct.Value().Value().e.size())
                                                  .c_str());
                        jobject newElement_0_nullableOptionalStruct_f;
                        std::string newElement_0_nullableOptionalStruct_fClassName     = "java/lang/Integer";
                        std::string newElement_0_nullableOptionalStruct_fCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_0_nullableOptionalStruct_fClassName.c_str(),
                            newElement_0_nullableOptionalStruct_fCtorSignature.c_str(),
                            entry_0.nullableOptionalStruct.Value().Value().f.Raw(), newElement_0_nullableOptionalStruct_f);
                        jobject newElement_0_nullableOptionalStruct_g;
                        std::string newElement_0_nullableOptionalStruct_gClassName     = "java/lang/Float";
                        std::string newElement_0_nullableOptionalStruct_gCtorSignature = "(F)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<float>(
                            newElement_0_nullableOptionalStruct_gClassName.c_str(),
                            newElement_0_nullableOptionalStruct_gCtorSignature.c_str(),
                            entry_0.nullableOptionalStruct.Value().Value().g, newElement_0_nullableOptionalStruct_g);
                        jobject newElement_0_nullableOptionalStruct_h;
                        std::string newElement_0_nullableOptionalStruct_hClassName     = "java/lang/Double";
                        std::string newElement_0_nullableOptionalStruct_hCtorSignature = "(D)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<double>(
                            newElement_0_nullableOptionalStruct_hClassName.c_str(),
                            newElement_0_nullableOptionalStruct_hCtorSignature.c_str(),
                            entry_0.nullableOptionalStruct.Value().Value().h, newElement_0_nullableOptionalStruct_h);

                        jclass simpleStructStructClass;
                        err = chip::JniReferences::GetInstance().GetClassRef(
                            env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                            return nullptr;
                        }
                        jmethodID simpleStructStructCtor =
                            env->GetMethodID(simpleStructStructClass, "<init>",
                                             "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/"
                                             "lang/Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
                        if (simpleStructStructCtor == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
                            return nullptr;
                        }

                        newElement_0_nullableOptionalStruct =
                            env->NewObject(simpleStructStructClass, simpleStructStructCtor, newElement_0_nullableOptionalStruct_a,
                                           newElement_0_nullableOptionalStruct_b, newElement_0_nullableOptionalStruct_c,
                                           newElement_0_nullableOptionalStruct_d, newElement_0_nullableOptionalStruct_e,
                                           newElement_0_nullableOptionalStruct_f, newElement_0_nullableOptionalStruct_g,
                                           newElement_0_nullableOptionalStruct_h);
                    }
                }
                jobject newElement_0_nullableList;
                if (entry_0.nullableList.IsNull())
                {
                    newElement_0_nullableList = nullptr;
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(newElement_0_nullableList);

                    auto iter_newElement_0_nullableList_NaN = entry_0.nullableList.Value().begin();
                    while (iter_newElement_0_nullableList_NaN.Next())
                    {
                        auto & entry_NaN = iter_newElement_0_nullableList_NaN.GetValue();
                        jobject newElement_NaN;
                        std::string newElement_NaNClassName     = "java/lang/Integer";
                        std::string newElement_NaNCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_NaNClassName.c_str(), newElement_NaNCtorSignature.c_str(), static_cast<uint8_t>(entry_NaN),
                            newElement_NaN);
                        chip::JniReferences::GetInstance().AddToArrayList(newElement_0_nullableList, newElement_NaN);
                    }
                }
                jobject newElement_0_optionalList;
                if (!entry_0.optionalList.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_optionalList);
                }
                else
                {
                    chip::JniReferences::GetInstance().CreateArrayList(newElement_0_optionalList);

                    auto iter_newElement_0_optionalList_NaN = entry_0.optionalList.Value().begin();
                    while (iter_newElement_0_optionalList_NaN.Next())
                    {
                        auto & entry_NaN = iter_newElement_0_optionalList_NaN.GetValue();
                        jobject newElement_NaN;
                        std::string newElement_NaNClassName     = "java/lang/Integer";
                        std::string newElement_NaNCtorSignature = "(I)V";
                        chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                            newElement_NaNClassName.c_str(), newElement_NaNCtorSignature.c_str(), static_cast<uint8_t>(entry_NaN),
                            newElement_NaN);
                        chip::JniReferences::GetInstance().AddToArrayList(newElement_0_optionalList, newElement_NaN);
                    }
                }
                jobject newElement_0_nullableOptionalList;
                if (!entry_0.nullableOptionalList.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_nullableOptionalList);
                }
                else
                {
                    if (entry_0.nullableOptionalList.Value().IsNull())
                    {
                        newElement_0_nullableOptionalList = nullptr;
                    }
                    else
                    {
                        chip::JniReferences::GetInstance().CreateArrayList(newElement_0_nullableOptionalList);

                        auto iter_newElement_0_nullableOptionalList_NaN = entry_0.nullableOptionalList.Value().Value().begin();
                        while (iter_newElement_0_nullableOptionalList_NaN.Next())
                        {
                            auto & entry_NaN = iter_newElement_0_nullableOptionalList_NaN.GetValue();
                            jobject newElement_NaN;
                            std::string newElement_NaNClassName     = "java/lang/Integer";
                            std::string newElement_NaNCtorSignature = "(I)V";
                            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                                newElement_NaNClassName.c_str(), newElement_NaNCtorSignature.c_str(),
                                static_cast<uint8_t>(entry_NaN), newElement_NaN);
                            chip::JniReferences::GetInstance().AddToArrayList(newElement_0_nullableOptionalList, newElement_NaN);
                        }
                    }
                }

                jclass nullablesAndOptionalsStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$TestClusterClusterNullablesAndOptionalsStruct",
                    nullablesAndOptionalsStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterNullablesAndOptionalsStruct");
                    return nullptr;
                }
                jmethodID nullablesAndOptionalsStructStructCtor = env->GetMethodID(
                    nullablesAndOptionalsStructStructClass, "<init>",
                    "(Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/lang/String;Ljava/util/Optional;Ljava/util/"
                    "Optional;Lchip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct;Ljava/util/Optional;Ljava/util/"
                    "Optional;Ljava/util/ArrayList;Ljava/util/Optional;Ljava/util/Optional;)V");
                if (nullablesAndOptionalsStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterNullablesAndOptionalsStruct constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(nullablesAndOptionalsStructStructClass, nullablesAndOptionalsStructStructCtor,
                                   newElement_0_nullableInt, newElement_0_optionalInt, newElement_0_nullableOptionalInt,
                                   newElement_0_nullableString, newElement_0_optionalString, newElement_0_nullableOptionalString,
                                   newElement_0_nullableStruct, newElement_0_optionalStruct, newElement_0_nullableOptionalStruct,
                                   newElement_0_nullableList, newElement_0_optionalList, newElement_0_nullableOptionalList);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::EnumAttr::Id: {
            using TypeInfo = Attributes::EnumAttr::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::StructAttr::Id: {
            using TypeInfo = Attributes::StructAttr::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jobject value_a;
            std::string value_aClassName     = "java/lang/Integer";
            std::string value_aCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_aClassName.c_str(), value_aCtorSignature.c_str(),
                                                                          cppValue.a, value_a);
            jobject value_b;
            std::string value_bClassName     = "java/lang/Boolean";
            std::string value_bCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(value_bClassName.c_str(), value_bCtorSignature.c_str(),
                                                                       cppValue.b, value_b);
            jobject value_c;
            std::string value_cClassName     = "java/lang/Integer";
            std::string value_cCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_cClassName.c_str(), value_cCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue.c), value_c);
            jobject value_d;
            jbyteArray value_dByteArray = env->NewByteArray(static_cast<jsize>(cppValue.d.size()));
            env->SetByteArrayRegion(value_dByteArray, 0, static_cast<jsize>(cppValue.d.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.d.data()));
            value_d = value_dByteArray;
            jobject value_e;
            value_e = env->NewStringUTF(std::string(cppValue.e.data(), cppValue.e.size()).c_str());
            jobject value_f;
            std::string value_fClassName     = "java/lang/Integer";
            std::string value_fCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(value_fClassName.c_str(), value_fCtorSignature.c_str(),
                                                                          cppValue.f.Raw(), value_f);
            jobject value_g;
            std::string value_gClassName     = "java/lang/Float";
            std::string value_gCtorSignature = "(F)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<float>(value_gClassName.c_str(), value_gCtorSignature.c_str(),
                                                                        cppValue.g, value_g);
            jobject value_h;
            std::string value_hClassName     = "java/lang/Double";
            std::string value_hCtorSignature = "(D)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<double>(value_hClassName.c_str(), value_hCtorSignature.c_str(),
                                                                         cppValue.h, value_h);

            jclass simpleStructStructClass;
            err = chip::JniReferences::GetInstance().GetClassRef(
                env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                return nullptr;
            }
            jmethodID simpleStructStructCtor =
                env->GetMethodID(simpleStructStructClass, "<init>",
                                 "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                 "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
            if (simpleStructStructCtor == nullptr)
            {
                ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
                return nullptr;
            }

            value = env->NewObject(simpleStructStructClass, simpleStructStructCtor, value_a, value_b, value_c, value_d, value_e,
                                   value_f, value_g, value_h);
            return value;
        }
        case Attributes::RangeRestrictedInt8u::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::RangeRestrictedInt8s::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                         cppValue, value);
            return value;
        }
        case Attributes::RangeRestrictedInt16u::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RangeRestrictedInt16s::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ListLongOctetString::Id: {
            using TypeInfo = Attributes::ListLongOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jbyteArray newElement_0ByteArray = env->NewByteArray(static_cast<jsize>(entry_0.size()));
                env->SetByteArrayRegion(newElement_0ByteArray, 0, static_cast<jsize>(entry_0.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.data()));
                newElement_0 = newElement_0ByteArray;
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::TimedWriteBoolean::Id: {
            using TypeInfo = Attributes::TimedWriteBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::GeneralErrorBoolean::Id: {
            using TypeInfo = Attributes::GeneralErrorBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::ClusterErrorBoolean::Id: {
            using TypeInfo = Attributes::ClusterErrorBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::Unsupported::Id: {
            using TypeInfo = Attributes::Unsupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(), cppValue,
                                                                       value);
            return value;
        }
        case Attributes::NullableBoolean::Id: {
            using TypeInfo = Attributes::NullableBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Boolean";
                std::string valueCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableBitmap8::Id: {
            using TypeInfo = Attributes::NullableBitmap8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableBitmap16::Id: {
            using TypeInfo = Attributes::NullableBitmap16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableBitmap32::Id: {
            using TypeInfo = Attributes::NullableBitmap32::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableBitmap64::Id: {
            using TypeInfo = Attributes::NullableBitmap64::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt8u::Id: {
            using TypeInfo = Attributes::NullableInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt16u::Id: {
            using TypeInfo = Attributes::NullableInt16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt24u::Id: {
            using TypeInfo = Attributes::NullableInt24u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt32u::Id: {
            using TypeInfo = Attributes::NullableInt32u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt40u::Id: {
            using TypeInfo = Attributes::NullableInt40u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt48u::Id: {
            using TypeInfo = Attributes::NullableInt48u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt56u::Id: {
            using TypeInfo = Attributes::NullableInt56u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt64u::Id: {
            using TypeInfo = Attributes::NullableInt64u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt8s::Id: {
            using TypeInfo = Attributes::NullableInt8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                             cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt16s::Id: {
            using TypeInfo = Attributes::NullableInt16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt24s::Id: {
            using TypeInfo = Attributes::NullableInt24s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt32s::Id: {
            using TypeInfo = Attributes::NullableInt32s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt40s::Id: {
            using TypeInfo = Attributes::NullableInt40s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt48s::Id: {
            using TypeInfo = Attributes::NullableInt48s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt56s::Id: {
            using TypeInfo = Attributes::NullableInt56s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableInt64s::Id: {
            using TypeInfo = Attributes::NullableInt64s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Long";
                std::string valueCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableEnum8::Id: {
            using TypeInfo = Attributes::NullableEnum8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableEnum16::Id: {
            using TypeInfo = Attributes::NullableEnum16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableFloatSingle::Id: {
            using TypeInfo = Attributes::NullableFloatSingle::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Float";
                std::string valueCtorSignature = "(F)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<float>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                            cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableFloatDouble::Id: {
            using TypeInfo = Attributes::NullableFloatDouble::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Double";
                std::string valueCtorSignature = "(D)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<double>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                             cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableOctetString::Id: {
            using TypeInfo = Attributes::NullableOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.Value().size()));
                env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.Value().size()),
                                        reinterpret_cast<const jbyte *>(cppValue.Value().data()));
                value = valueByteArray;
            }
            return value;
        }
        case Attributes::NullableCharString::Id: {
            using TypeInfo = Attributes::NullableCharString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                value = env->NewStringUTF(std::string(cppValue.Value().data(), cppValue.Value().size()).c_str());
            }
            return value;
        }
        case Attributes::NullableEnumAttr::Id: {
            using TypeInfo = Attributes::NullableEnumAttr::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(cppValue.Value()), value);
            }
            return value;
        }
        case Attributes::NullableStruct::Id: {
            using TypeInfo = Attributes::NullableStruct::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                jobject value_a;
                std::string value_aClassName     = "java/lang/Integer";
                std::string value_aCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_aClassName.c_str(), value_aCtorSignature.c_str(), cppValue.Value().a, value_a);
                jobject value_b;
                std::string value_bClassName     = "java/lang/Boolean";
                std::string value_bCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(value_bClassName.c_str(), value_bCtorSignature.c_str(),
                                                                           cppValue.Value().b, value_b);
                jobject value_c;
                std::string value_cClassName     = "java/lang/Integer";
                std::string value_cCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_cClassName.c_str(), value_cCtorSignature.c_str(), static_cast<uint8_t>(cppValue.Value().c), value_c);
                jobject value_d;
                jbyteArray value_dByteArray = env->NewByteArray(static_cast<jsize>(cppValue.Value().d.size()));
                env->SetByteArrayRegion(value_dByteArray, 0, static_cast<jsize>(cppValue.Value().d.size()),
                                        reinterpret_cast<const jbyte *>(cppValue.Value().d.data()));
                value_d = value_dByteArray;
                jobject value_e;
                value_e = env->NewStringUTF(std::string(cppValue.Value().e.data(), cppValue.Value().e.size()).c_str());
                jobject value_f;
                std::string value_fClassName     = "java/lang/Integer";
                std::string value_fCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    value_fClassName.c_str(), value_fCtorSignature.c_str(), cppValue.Value().f.Raw(), value_f);
                jobject value_g;
                std::string value_gClassName     = "java/lang/Float";
                std::string value_gCtorSignature = "(F)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<float>(value_gClassName.c_str(), value_gCtorSignature.c_str(),
                                                                            cppValue.Value().g, value_g);
                jobject value_h;
                std::string value_hClassName     = "java/lang/Double";
                std::string value_hCtorSignature = "(D)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<double>(value_hClassName.c_str(), value_hCtorSignature.c_str(),
                                                                             cppValue.Value().h, value_h);

                jclass simpleStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$TestClusterClusterSimpleStruct", simpleStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$TestClusterClusterSimpleStruct");
                    return nullptr;
                }
                jmethodID simpleStructStructCtor =
                    env->GetMethodID(simpleStructStructClass, "<init>",
                                     "(Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Integer;[BLjava/lang/String;Ljava/lang/"
                                     "Integer;Ljava/lang/Float;Ljava/lang/Double;)V");
                if (simpleStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$TestClusterClusterSimpleStruct constructor");
                    return nullptr;
                }

                value = env->NewObject(simpleStructStructClass, simpleStructStructCtor, value_a, value_b, value_c, value_d, value_e,
                                       value_f, value_g, value_h);
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt8u::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt8s::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                             cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt16u::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt16s::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::Thermostat::Id: {
        using namespace app::Clusters::Thermostat;
        switch (aPath.mAttributeId)
        {
        case Attributes::LocalTemperature::Id: {
            using TypeInfo = Attributes::LocalTemperature::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AbsMinHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMinHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AbsMaxHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMaxHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AbsMinCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMinCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AbsMaxCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMaxCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::OccupiedCoolingSetpoint::Id: {
            using TypeInfo = Attributes::OccupiedCoolingSetpoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::OccupiedHeatingSetpoint::Id: {
            using TypeInfo = Attributes::OccupiedHeatingSetpoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::MinHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::MaxHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::MinCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MaxCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::MaxCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::MinSetpointDeadBand::Id: {
            using TypeInfo = Attributes::MinSetpointDeadBand::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                         cppValue, value);
            return value;
        }
        case Attributes::ControlSequenceOfOperation::Id: {
            using TypeInfo = Attributes::ControlSequenceOfOperation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::SystemMode::Id: {
            using TypeInfo = Attributes::SystemMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::StartOfWeek::Id: {
            using TypeInfo = Attributes::StartOfWeek::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::NumberOfWeeklyTransitions::Id: {
            using TypeInfo = Attributes::NumberOfWeeklyTransitions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::NumberOfDailyTransitions::Id: {
            using TypeInfo = Attributes::NumberOfDailyTransitions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ThermostatUserInterfaceConfiguration::Id: {
        using namespace app::Clusters::ThermostatUserInterfaceConfiguration;
        switch (aPath.mAttributeId)
        {
        case Attributes::TemperatureDisplayMode::Id: {
            using TypeInfo = Attributes::TemperatureDisplayMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::KeypadLockout::Id: {
            using TypeInfo = Attributes::KeypadLockout::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ScheduleProgrammingVisibility::Id: {
            using TypeInfo = Attributes::ScheduleProgrammingVisibility::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::ThreadNetworkDiagnostics::Id: {
        using namespace app::Clusters::ThreadNetworkDiagnostics;
        switch (aPath.mAttributeId)
        {
        case Attributes::Channel::Id: {
            using TypeInfo = Attributes::Channel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RoutingRole::Id: {
            using TypeInfo = Attributes::RoutingRole::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::NetworkName::Id: {
            using TypeInfo = Attributes::NetworkName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::PanId::Id: {
            using TypeInfo = Attributes::PanId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ExtendedPanId::Id: {
            using TypeInfo = Attributes::ExtendedPanId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::MeshLocalPrefix::Id: {
            using TypeInfo = Attributes::MeshLocalPrefix::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.size()));
            env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.data()));
            value = valueByteArray;
            return value;
        }
        case Attributes::OverrunCount::Id: {
            using TypeInfo = Attributes::OverrunCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::NeighborTableList::Id: {
            using TypeInfo = Attributes::NeighborTableList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_extAddress;
                std::string newElement_0_extAddressClassName     = "java/lang/Long";
                std::string newElement_0_extAddressCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(newElement_0_extAddressClassName.c_str(),
                                                                               newElement_0_extAddressCtorSignature.c_str(),
                                                                               entry_0.extAddress, newElement_0_extAddress);
                jobject newElement_0_age;
                std::string newElement_0_ageClassName     = "java/lang/Long";
                std::string newElement_0_ageCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0_ageClassName.c_str(), newElement_0_ageCtorSignature.c_str(), entry_0.age, newElement_0_age);
                jobject newElement_0_rloc16;
                std::string newElement_0_rloc16ClassName     = "java/lang/Integer";
                std::string newElement_0_rloc16CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_rloc16ClassName.c_str(),
                                                                               newElement_0_rloc16CtorSignature.c_str(),
                                                                               entry_0.rloc16, newElement_0_rloc16);
                jobject newElement_0_linkFrameCounter;
                std::string newElement_0_linkFrameCounterClassName     = "java/lang/Long";
                std::string newElement_0_linkFrameCounterCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0_linkFrameCounterClassName.c_str(), newElement_0_linkFrameCounterCtorSignature.c_str(),
                    entry_0.linkFrameCounter, newElement_0_linkFrameCounter);
                jobject newElement_0_mleFrameCounter;
                std::string newElement_0_mleFrameCounterClassName     = "java/lang/Long";
                std::string newElement_0_mleFrameCounterCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0_mleFrameCounterClassName.c_str(), newElement_0_mleFrameCounterCtorSignature.c_str(),
                    entry_0.mleFrameCounter, newElement_0_mleFrameCounter);
                jobject newElement_0_lqi;
                std::string newElement_0_lqiClassName     = "java/lang/Integer";
                std::string newElement_0_lqiCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_lqiClassName.c_str(), newElement_0_lqiCtorSignature.c_str(), entry_0.lqi, newElement_0_lqi);
                jobject newElement_0_averageRssi;
                std::string newElement_0_averageRssiClassName     = "java/lang/Integer";
                std::string newElement_0_averageRssiCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(newElement_0_averageRssiClassName.c_str(),
                                                                             newElement_0_averageRssiCtorSignature.c_str(),
                                                                             entry_0.averageRssi, newElement_0_averageRssi);
                jobject newElement_0_lastRssi;
                std::string newElement_0_lastRssiClassName     = "java/lang/Integer";
                std::string newElement_0_lastRssiCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(newElement_0_lastRssiClassName.c_str(),
                                                                             newElement_0_lastRssiCtorSignature.c_str(),
                                                                             entry_0.lastRssi, newElement_0_lastRssi);
                jobject newElement_0_frameErrorRate;
                std::string newElement_0_frameErrorRateClassName     = "java/lang/Integer";
                std::string newElement_0_frameErrorRateCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_frameErrorRateClassName.c_str(),
                                                                              newElement_0_frameErrorRateCtorSignature.c_str(),
                                                                              entry_0.frameErrorRate, newElement_0_frameErrorRate);
                jobject newElement_0_messageErrorRate;
                std::string newElement_0_messageErrorRateClassName     = "java/lang/Integer";
                std::string newElement_0_messageErrorRateCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_messageErrorRateClassName.c_str(), newElement_0_messageErrorRateCtorSignature.c_str(),
                    entry_0.messageErrorRate, newElement_0_messageErrorRate);
                jobject newElement_0_rxOnWhenIdle;
                std::string newElement_0_rxOnWhenIdleClassName     = "java/lang/Boolean";
                std::string newElement_0_rxOnWhenIdleCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_rxOnWhenIdleClassName.c_str(),
                                                                           newElement_0_rxOnWhenIdleCtorSignature.c_str(),
                                                                           entry_0.rxOnWhenIdle, newElement_0_rxOnWhenIdle);
                jobject newElement_0_fullThreadDevice;
                std::string newElement_0_fullThreadDeviceClassName     = "java/lang/Boolean";
                std::string newElement_0_fullThreadDeviceCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_fullThreadDeviceClassName.c_str(),
                                                                           newElement_0_fullThreadDeviceCtorSignature.c_str(),
                                                                           entry_0.fullThreadDevice, newElement_0_fullThreadDevice);
                jobject newElement_0_fullNetworkData;
                std::string newElement_0_fullNetworkDataClassName     = "java/lang/Boolean";
                std::string newElement_0_fullNetworkDataCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_fullNetworkDataClassName.c_str(),
                                                                           newElement_0_fullNetworkDataCtorSignature.c_str(),
                                                                           entry_0.fullNetworkData, newElement_0_fullNetworkData);
                jobject newElement_0_isChild;
                std::string newElement_0_isChildClassName     = "java/lang/Boolean";
                std::string newElement_0_isChildCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_isChildClassName.c_str(),
                                                                           newElement_0_isChildCtorSignature.c_str(),
                                                                           entry_0.isChild, newElement_0_isChild);

                jclass neighborTableStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ThreadNetworkDiagnosticsClusterNeighborTable",
                    neighborTableStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ThreadNetworkDiagnosticsClusterNeighborTable");
                    return nullptr;
                }
                jmethodID neighborTableStructCtor =
                    env->GetMethodID(neighborTableStructClass, "<init>",
                                     "(Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;Ljava/"
                                     "lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                                     "Integer;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;)V");
                if (neighborTableStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ThreadNetworkDiagnosticsClusterNeighborTable constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(
                    neighborTableStructClass, neighborTableStructCtor, newElement_0_extAddress, newElement_0_age,
                    newElement_0_rloc16, newElement_0_linkFrameCounter, newElement_0_mleFrameCounter, newElement_0_lqi,
                    newElement_0_averageRssi, newElement_0_lastRssi, newElement_0_frameErrorRate, newElement_0_messageErrorRate,
                    newElement_0_rxOnWhenIdle, newElement_0_fullThreadDevice, newElement_0_fullNetworkData, newElement_0_isChild);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::RouteTableList::Id: {
            using TypeInfo = Attributes::RouteTableList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_extAddress;
                std::string newElement_0_extAddressClassName     = "java/lang/Long";
                std::string newElement_0_extAddressCtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(newElement_0_extAddressClassName.c_str(),
                                                                               newElement_0_extAddressCtorSignature.c_str(),
                                                                               entry_0.extAddress, newElement_0_extAddress);
                jobject newElement_0_rloc16;
                std::string newElement_0_rloc16ClassName     = "java/lang/Integer";
                std::string newElement_0_rloc16CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_rloc16ClassName.c_str(),
                                                                               newElement_0_rloc16CtorSignature.c_str(),
                                                                               entry_0.rloc16, newElement_0_rloc16);
                jobject newElement_0_routerId;
                std::string newElement_0_routerIdClassName     = "java/lang/Integer";
                std::string newElement_0_routerIdCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_routerIdClassName.c_str(),
                                                                              newElement_0_routerIdCtorSignature.c_str(),
                                                                              entry_0.routerId, newElement_0_routerId);
                jobject newElement_0_nextHop;
                std::string newElement_0_nextHopClassName     = "java/lang/Integer";
                std::string newElement_0_nextHopCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_nextHopClassName.c_str(),
                                                                              newElement_0_nextHopCtorSignature.c_str(),
                                                                              entry_0.nextHop, newElement_0_nextHop);
                jobject newElement_0_pathCost;
                std::string newElement_0_pathCostClassName     = "java/lang/Integer";
                std::string newElement_0_pathCostCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_pathCostClassName.c_str(),
                                                                              newElement_0_pathCostCtorSignature.c_str(),
                                                                              entry_0.pathCost, newElement_0_pathCost);
                jobject newElement_0_LQIIn;
                std::string newElement_0_LQIInClassName     = "java/lang/Integer";
                std::string newElement_0_LQIInCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_LQIInClassName.c_str(),
                                                                              newElement_0_LQIInCtorSignature.c_str(),
                                                                              entry_0.LQIIn, newElement_0_LQIIn);
                jobject newElement_0_LQIOut;
                std::string newElement_0_LQIOutClassName     = "java/lang/Integer";
                std::string newElement_0_LQIOutCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(newElement_0_LQIOutClassName.c_str(),
                                                                              newElement_0_LQIOutCtorSignature.c_str(),
                                                                              entry_0.LQIOut, newElement_0_LQIOut);
                jobject newElement_0_age;
                std::string newElement_0_ageClassName     = "java/lang/Integer";
                std::string newElement_0_ageCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0_ageClassName.c_str(), newElement_0_ageCtorSignature.c_str(), entry_0.age, newElement_0_age);
                jobject newElement_0_allocated;
                std::string newElement_0_allocatedClassName     = "java/lang/Boolean";
                std::string newElement_0_allocatedCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_allocatedClassName.c_str(),
                                                                           newElement_0_allocatedCtorSignature.c_str(),
                                                                           entry_0.allocated, newElement_0_allocated);
                jobject newElement_0_linkEstablished;
                std::string newElement_0_linkEstablishedClassName     = "java/lang/Boolean";
                std::string newElement_0_linkEstablishedCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_linkEstablishedClassName.c_str(),
                                                                           newElement_0_linkEstablishedCtorSignature.c_str(),
                                                                           entry_0.linkEstablished, newElement_0_linkEstablished);

                jclass routeTableStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ThreadNetworkDiagnosticsClusterRouteTable", routeTableStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ThreadNetworkDiagnosticsClusterRouteTable");
                    return nullptr;
                }
                jmethodID routeTableStructCtor = env->GetMethodID(
                    routeTableStructClass, "<init>",
                    "(Ljava/lang/Long;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/"
                    "Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Boolean;Ljava/lang/Boolean;)V");
                if (routeTableStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ThreadNetworkDiagnosticsClusterRouteTable constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(routeTableStructClass, routeTableStructCtor, newElement_0_extAddress, newElement_0_rloc16,
                                   newElement_0_routerId, newElement_0_nextHop, newElement_0_pathCost, newElement_0_LQIIn,
                                   newElement_0_LQIOut, newElement_0_age, newElement_0_allocated, newElement_0_linkEstablished);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::PartitionId::Id: {
            using TypeInfo = Attributes::PartitionId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Weighting::Id: {
            using TypeInfo = Attributes::Weighting::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::DataVersion::Id: {
            using TypeInfo = Attributes::DataVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::StableDataVersion::Id: {
            using TypeInfo = Attributes::StableDataVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::LeaderRouterId::Id: {
            using TypeInfo = Attributes::LeaderRouterId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::DetachedRoleCount::Id: {
            using TypeInfo = Attributes::DetachedRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ChildRoleCount::Id: {
            using TypeInfo = Attributes::ChildRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RouterRoleCount::Id: {
            using TypeInfo = Attributes::RouterRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::LeaderRoleCount::Id: {
            using TypeInfo = Attributes::LeaderRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::AttachAttemptCount::Id: {
            using TypeInfo = Attributes::AttachAttemptCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PartitionIdChangeCount::Id: {
            using TypeInfo = Attributes::PartitionIdChangeCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BetterPartitionAttachAttemptCount::Id: {
            using TypeInfo = Attributes::BetterPartitionAttachAttemptCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ParentChangeCount::Id: {
            using TypeInfo = Attributes::ParentChangeCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxTotalCount::Id: {
            using TypeInfo = Attributes::TxTotalCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxUnicastCount::Id: {
            using TypeInfo = Attributes::TxUnicastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxBroadcastCount::Id: {
            using TypeInfo = Attributes::TxBroadcastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxAckRequestedCount::Id: {
            using TypeInfo = Attributes::TxAckRequestedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxAckedCount::Id: {
            using TypeInfo = Attributes::TxAckedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxNoAckRequestedCount::Id: {
            using TypeInfo = Attributes::TxNoAckRequestedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxDataCount::Id: {
            using TypeInfo = Attributes::TxDataCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxDataPollCount::Id: {
            using TypeInfo = Attributes::TxDataPollCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxBeaconCount::Id: {
            using TypeInfo = Attributes::TxBeaconCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxBeaconRequestCount::Id: {
            using TypeInfo = Attributes::TxBeaconRequestCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxOtherCount::Id: {
            using TypeInfo = Attributes::TxOtherCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxRetryCount::Id: {
            using TypeInfo = Attributes::TxRetryCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxDirectMaxRetryExpiryCount::Id: {
            using TypeInfo = Attributes::TxDirectMaxRetryExpiryCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxIndirectMaxRetryExpiryCount::Id: {
            using TypeInfo = Attributes::TxIndirectMaxRetryExpiryCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxErrCcaCount::Id: {
            using TypeInfo = Attributes::TxErrCcaCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxErrAbortCount::Id: {
            using TypeInfo = Attributes::TxErrAbortCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::TxErrBusyChannelCount::Id: {
            using TypeInfo = Attributes::TxErrBusyChannelCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxTotalCount::Id: {
            using TypeInfo = Attributes::RxTotalCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxUnicastCount::Id: {
            using TypeInfo = Attributes::RxUnicastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxBroadcastCount::Id: {
            using TypeInfo = Attributes::RxBroadcastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxDataCount::Id: {
            using TypeInfo = Attributes::RxDataCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxDataPollCount::Id: {
            using TypeInfo = Attributes::RxDataPollCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxBeaconCount::Id: {
            using TypeInfo = Attributes::RxBeaconCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxBeaconRequestCount::Id: {
            using TypeInfo = Attributes::RxBeaconRequestCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxOtherCount::Id: {
            using TypeInfo = Attributes::RxOtherCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxAddressFilteredCount::Id: {
            using TypeInfo = Attributes::RxAddressFilteredCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxDestAddrFilteredCount::Id: {
            using TypeInfo = Attributes::RxDestAddrFilteredCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxDuplicatedCount::Id: {
            using TypeInfo = Attributes::RxDuplicatedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxErrNoFrameCount::Id: {
            using TypeInfo = Attributes::RxErrNoFrameCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxErrUnknownNeighborCount::Id: {
            using TypeInfo = Attributes::RxErrUnknownNeighborCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxErrInvalidSrcAddrCount::Id: {
            using TypeInfo = Attributes::RxErrInvalidSrcAddrCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxErrSecCount::Id: {
            using TypeInfo = Attributes::RxErrSecCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxErrFcsCount::Id: {
            using TypeInfo = Attributes::RxErrFcsCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::RxErrOtherCount::Id: {
            using TypeInfo = Attributes::RxErrOtherCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ActiveTimestamp::Id: {
            using TypeInfo = Attributes::ActiveTimestamp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PendingTimestamp::Id: {
            using TypeInfo = Attributes::PendingTimestamp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Delay::Id: {
            using TypeInfo = Attributes::Delay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::SecurityPolicy::Id: {
            using TypeInfo = Attributes::SecurityPolicy::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_rotationTime;
                std::string newElement_0_rotationTimeClassName     = "java/lang/Integer";
                std::string newElement_0_rotationTimeCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_rotationTimeClassName.c_str(),
                                                                               newElement_0_rotationTimeCtorSignature.c_str(),
                                                                               entry_0.rotationTime, newElement_0_rotationTime);
                jobject newElement_0_flags;
                std::string newElement_0_flagsClassName     = "java/lang/Integer";
                std::string newElement_0_flagsCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(newElement_0_flagsClassName.c_str(),
                                                                               newElement_0_flagsCtorSignature.c_str(),
                                                                               entry_0.flags, newElement_0_flags);

                jclass securityPolicyStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ThreadNetworkDiagnosticsClusterSecurityPolicy",
                    securityPolicyStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ThreadNetworkDiagnosticsClusterSecurityPolicy");
                    return nullptr;
                }
                jmethodID securityPolicyStructCtor =
                    env->GetMethodID(securityPolicyStructClass, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;)V");
                if (securityPolicyStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ThreadNetworkDiagnosticsClusterSecurityPolicy constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(securityPolicyStructClass, securityPolicyStructCtor, newElement_0_rotationTime,
                                              newElement_0_flags);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ChannelMask::Id: {
            using TypeInfo = Attributes::ChannelMask::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.size()));
            env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.size()),
                                    reinterpret_cast<const jbyte *>(cppValue.data()));
            value = valueByteArray;
            return value;
        }
        case Attributes::OperationalDatasetComponents::Id: {
            using TypeInfo = Attributes::OperationalDatasetComponents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_activeTimestampPresent;
                std::string newElement_0_activeTimestampPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_activeTimestampPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_activeTimestampPresentClassName.c_str(), newElement_0_activeTimestampPresentCtorSignature.c_str(),
                    entry_0.activeTimestampPresent, newElement_0_activeTimestampPresent);
                jobject newElement_0_pendingTimestampPresent;
                std::string newElement_0_pendingTimestampPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_pendingTimestampPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_pendingTimestampPresentClassName.c_str(),
                    newElement_0_pendingTimestampPresentCtorSignature.c_str(), entry_0.pendingTimestampPresent,
                    newElement_0_pendingTimestampPresent);
                jobject newElement_0_masterKeyPresent;
                std::string newElement_0_masterKeyPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_masterKeyPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_masterKeyPresentClassName.c_str(),
                                                                           newElement_0_masterKeyPresentCtorSignature.c_str(),
                                                                           entry_0.masterKeyPresent, newElement_0_masterKeyPresent);
                jobject newElement_0_networkNamePresent;
                std::string newElement_0_networkNamePresentClassName     = "java/lang/Boolean";
                std::string newElement_0_networkNamePresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_networkNamePresentClassName.c_str(), newElement_0_networkNamePresentCtorSignature.c_str(),
                    entry_0.networkNamePresent, newElement_0_networkNamePresent);
                jobject newElement_0_extendedPanIdPresent;
                std::string newElement_0_extendedPanIdPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_extendedPanIdPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_extendedPanIdPresentClassName.c_str(), newElement_0_extendedPanIdPresentCtorSignature.c_str(),
                    entry_0.extendedPanIdPresent, newElement_0_extendedPanIdPresent);
                jobject newElement_0_meshLocalPrefixPresent;
                std::string newElement_0_meshLocalPrefixPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_meshLocalPrefixPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_meshLocalPrefixPresentClassName.c_str(), newElement_0_meshLocalPrefixPresentCtorSignature.c_str(),
                    entry_0.meshLocalPrefixPresent, newElement_0_meshLocalPrefixPresent);
                jobject newElement_0_delayPresent;
                std::string newElement_0_delayPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_delayPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_delayPresentClassName.c_str(),
                                                                           newElement_0_delayPresentCtorSignature.c_str(),
                                                                           entry_0.delayPresent, newElement_0_delayPresent);
                jobject newElement_0_panIdPresent;
                std::string newElement_0_panIdPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_panIdPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_panIdPresentClassName.c_str(),
                                                                           newElement_0_panIdPresentCtorSignature.c_str(),
                                                                           entry_0.panIdPresent, newElement_0_panIdPresent);
                jobject newElement_0_channelPresent;
                std::string newElement_0_channelPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_channelPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_channelPresentClassName.c_str(),
                                                                           newElement_0_channelPresentCtorSignature.c_str(),
                                                                           entry_0.channelPresent, newElement_0_channelPresent);
                jobject newElement_0_pskcPresent;
                std::string newElement_0_pskcPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_pskcPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(newElement_0_pskcPresentClassName.c_str(),
                                                                           newElement_0_pskcPresentCtorSignature.c_str(),
                                                                           entry_0.pskcPresent, newElement_0_pskcPresent);
                jobject newElement_0_securityPolicyPresent;
                std::string newElement_0_securityPolicyPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_securityPolicyPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_securityPolicyPresentClassName.c_str(), newElement_0_securityPolicyPresentCtorSignature.c_str(),
                    entry_0.securityPolicyPresent, newElement_0_securityPolicyPresent);
                jobject newElement_0_channelMaskPresent;
                std::string newElement_0_channelMaskPresentClassName     = "java/lang/Boolean";
                std::string newElement_0_channelMaskPresentCtorSignature = "(Z)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<bool>(
                    newElement_0_channelMaskPresentClassName.c_str(), newElement_0_channelMaskPresentCtorSignature.c_str(),
                    entry_0.channelMaskPresent, newElement_0_channelMaskPresent);

                jclass operationalDatasetComponentsStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$ThreadNetworkDiagnosticsClusterOperationalDatasetComponents",
                    operationalDatasetComponentsStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl,
                                 "Could not find class ChipStructs$ThreadNetworkDiagnosticsClusterOperationalDatasetComponents");
                    return nullptr;
                }
                jmethodID operationalDatasetComponentsStructCtor =
                    env->GetMethodID(operationalDatasetComponentsStructClass, "<init>",
                                     "(Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/"
                                     "Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;Ljava/"
                                     "lang/Boolean;Ljava/lang/Boolean;Ljava/lang/Boolean;)V");
                if (operationalDatasetComponentsStructCtor == nullptr)
                {
                    ChipLogError(
                        Zcl, "Could not find ChipStructs$ThreadNetworkDiagnosticsClusterOperationalDatasetComponents constructor");
                    return nullptr;
                }

                newElement_0 = env->NewObject(
                    operationalDatasetComponentsStructClass, operationalDatasetComponentsStructCtor,
                    newElement_0_activeTimestampPresent, newElement_0_pendingTimestampPresent, newElement_0_masterKeyPresent,
                    newElement_0_networkNamePresent, newElement_0_extendedPanIdPresent, newElement_0_meshLocalPrefixPresent,
                    newElement_0_delayPresent, newElement_0_panIdPresent, newElement_0_channelPresent, newElement_0_pskcPresent,
                    newElement_0_securityPolicyPresent, newElement_0_channelMaskPresent);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ActiveNetworkFaultsList::Id: {
            using TypeInfo = Attributes::ActiveNetworkFaultsList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::TimeFormatLocalization::Id: {
        using namespace app::Clusters::TimeFormatLocalization;
        switch (aPath.mAttributeId)
        {
        case Attributes::HourFormat::Id: {
            using TypeInfo = Attributes::HourFormat::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::ActiveCalendarType::Id: {
            using TypeInfo = Attributes::ActiveCalendarType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::SupportedCalendarTypes::Id: {
            using TypeInfo = Attributes::SupportedCalendarTypes::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Integer";
                std::string newElement_0CtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), static_cast<uint8_t>(entry_0), newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::UnitLocalization::Id: {
        using namespace app::Clusters::UnitLocalization;
        switch (aPath.mAttributeId)
        {
        case Attributes::TemperatureUnit::Id: {
            using TypeInfo = Attributes::TemperatureUnit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          static_cast<uint8_t>(cppValue), value);
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::UserLabel::Id: {
        using namespace app::Clusters::UserLabel;
        switch (aPath.mAttributeId)
        {
        case Attributes::LabelList::Id: {
            using TypeInfo = Attributes::LabelList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                jobject newElement_0_label;
                newElement_0_label = env->NewStringUTF(std::string(entry_0.label.data(), entry_0.label.size()).c_str());
                jobject newElement_0_value;
                newElement_0_value = env->NewStringUTF(std::string(entry_0.value.data(), entry_0.value.size()).c_str());

                jclass labelStructStructClass;
                err = chip::JniReferences::GetInstance().GetClassRef(
                    env, "chip/devicecontroller/ChipStructs$UserLabelClusterLabelStruct", labelStructStructClass);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$UserLabelClusterLabelStruct");
                    return nullptr;
                }
                jmethodID labelStructStructCtor =
                    env->GetMethodID(labelStructStructClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
                if (labelStructStructCtor == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$UserLabelClusterLabelStruct constructor");
                    return nullptr;
                }

                newElement_0 =
                    env->NewObject(labelStructStructClass, labelStructStructCtor, newElement_0_label, newElement_0_value);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::WakeOnLan::Id: {
        using namespace app::Clusters::WakeOnLan;
        switch (aPath.mAttributeId)
        {
        case Attributes::WakeOnLanMacAddress::Id: {
            using TypeInfo = Attributes::WakeOnLanMacAddress::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            value = env->NewStringUTF(std::string(cppValue.data(), cppValue.size()).c_str());
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::WiFiNetworkDiagnostics::Id: {
        using namespace app::Clusters::WiFiNetworkDiagnostics;
        switch (aPath.mAttributeId)
        {
        case Attributes::Bssid::Id: {
            using TypeInfo = Attributes::Bssid::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(cppValue.Value().size()));
                env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(cppValue.Value().size()),
                                        reinterpret_cast<const jbyte *>(cppValue.Value().data()));
                value = valueByteArray;
            }
            return value;
        }
        case Attributes::SecurityType::Id: {
            using TypeInfo = Attributes::SecurityType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(cppValue.Value()), value);
            }
            return value;
        }
        case Attributes::WiFiVersion::Id: {
            using TypeInfo = Attributes::WiFiVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              static_cast<uint8_t>(cppValue.Value()), value);
            }
            return value;
        }
        case Attributes::ChannelNumber::Id: {
            using TypeInfo = Attributes::ChannelNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::Rssi::Id: {
            using TypeInfo = Attributes::Rssi::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<int8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                             cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::BeaconLostCount::Id: {
            using TypeInfo = Attributes::BeaconLostCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::BeaconRxCount::Id: {
            using TypeInfo = Attributes::BeaconRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PacketMulticastRxCount::Id: {
            using TypeInfo = Attributes::PacketMulticastRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PacketMulticastTxCount::Id: {
            using TypeInfo = Attributes::PacketMulticastTxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PacketUnicastRxCount::Id: {
            using TypeInfo = Attributes::PacketUnicastRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::PacketUnicastTxCount::Id: {
            using TypeInfo = Attributes::PacketUnicastTxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::CurrentMaxRate::Id: {
            using TypeInfo = Attributes::CurrentMaxRate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::OverrunCount::Id: {
            using TypeInfo = Attributes::OverrunCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint64_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    case app::Clusters::WindowCovering::Id: {
        using namespace app::Clusters::WindowCovering;
        switch (aPath.mAttributeId)
        {
        case Attributes::Type::Id: {
            using TypeInfo = Attributes::Type::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentPositionLift::Id: {
            using TypeInfo = Attributes::CurrentPositionLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::CurrentPositionTilt::Id: {
            using TypeInfo = Attributes::CurrentPositionTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::ConfigStatus::Id: {
            using TypeInfo = Attributes::ConfigStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentPositionLiftPercentage::Id: {
            using TypeInfo = Attributes::CurrentPositionLiftPercentage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::CurrentPositionTiltPercentage::Id: {
            using TypeInfo = Attributes::CurrentPositionTiltPercentage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                              cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::OperationalStatus::Id: {
            using TypeInfo = Attributes::OperationalStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::TargetPositionLiftPercent100ths::Id: {
            using TypeInfo = Attributes::TargetPositionLiftPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::TargetPositionTiltPercent100ths::Id: {
            using TypeInfo = Attributes::TargetPositionTiltPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::EndProductType::Id: {
            using TypeInfo = Attributes::EndProductType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::CurrentPositionLiftPercent100ths::Id: {
            using TypeInfo = Attributes::CurrentPositionLiftPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::CurrentPositionTiltPercent100ths::Id: {
            using TypeInfo = Attributes::CurrentPositionTiltPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                               cppValue.Value(), value);
            }
            return value;
        }
        case Attributes::InstalledOpenLimitLift::Id: {
            using TypeInfo = Attributes::InstalledOpenLimitLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::InstalledClosedLimitLift::Id: {
            using TypeInfo = Attributes::InstalledClosedLimitLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::InstalledOpenLimitTilt::Id: {
            using TypeInfo = Attributes::InstalledOpenLimitTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::InstalledClosedLimitTilt::Id: {
            using TypeInfo = Attributes::InstalledClosedLimitTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::Mode::Id: {
            using TypeInfo = Attributes::Mode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint8_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                          cppValue, value);
            return value;
        }
        case Attributes::SafetyStatus::Id: {
            using TypeInfo = Attributes::SafetyStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ServerGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ServerGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ClientGeneratedCommandList::Id: {
            using TypeInfo = Attributes::ClientGeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            chip::JniReferences::GetInstance().CreateArrayList(value);

            auto iter_value_0 = cppValue.begin();
            while (iter_value_0.Next())
            {
                auto & entry_0 = iter_value_0.GetValue();
                jobject newElement_0;
                std::string newElement_0ClassName     = "java/lang/Long";
                std::string newElement_0CtorSignature = "(J)V";
                chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), entry_0, newElement_0);
                chip::JniReferences::GetInstance().AddToArrayList(value, newElement_0);
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint32_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            chip::JniReferences::GetInstance().CreateBoxedObject<uint16_t>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           cppValue, value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
            break;
        }
        break;
    }
    default:
        *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
        break;
    }
    return nullptr;
}

} // namespace chip
