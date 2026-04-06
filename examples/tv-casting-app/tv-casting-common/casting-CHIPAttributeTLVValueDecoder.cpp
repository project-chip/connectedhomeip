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
 * @file casting-CHIPAttributeTLVValueDecoder.cpp
 *
 * Slim attribute TLV decoder for the TV Casting App.
 * Contains DecodeAttributeValue() cases for ONLY the casting clusters.
 *
 * === Included Clusters (19) ===
 *   AccountLogin, ApplicationBasic, ApplicationLauncher, AudioOutput,
 *   Binding, Channel, ContentAppObserver, ContentControl,
 *   ContentLauncher, Descriptor, KeypadInput, LevelControl,
 *   LowPower, MediaInput, MediaPlayback, Messages, OnOff,
 *   TargetNavigator, WakeOnLAN
 *
 * To add a cluster:
 *   1. Copy its case block from zap-generated/CHIPAttributeTLVValueDecoder.cpp
 *   2. Add it to the switch statement below (keep alphabetical order)
 *   3. Update this cluster list
 *   4. Ensure the cluster's .ipp files are included in casting-cluster-objects.cpp
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
    // --- AccountLogin (0x050E) ---
    case app::Clusters::AccountLogin::Id: {
        using namespace app::Clusters::AccountLogin;
        switch (aPath.mAttributeId)
        {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- ApplicationBasic (0x050D) ---
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
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        case Attributes::Application::Id: {
            using TypeInfo = Attributes::Application::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            jobject value_catalogVendorID;
            std::string value_catalogVendorIDClassName     = "java/lang/Integer";
            std::string value_catalogVendorIDCtorSignature = "(I)V";
            jint jnivalue_catalogVendorID                  = static_cast<jint>(cppValue.catalogVendorID);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_catalogVendorIDClassName.c_str(),
                                                                       value_catalogVendorIDCtorSignature.c_str(),
                                                                       jnivalue_catalogVendorID, value_catalogVendorID);
            jobject value_applicationID;
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.applicationID, value_applicationID));

            {
                jclass applicationStructStructClass_0;
                err = chip::JniReferences::GetInstance().GetLocalClassRef(
                    env, "chip/devicecontroller/ChipStructs$ApplicationBasicClusterApplicationStruct",
                    applicationStructStructClass_0);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "Could not find class ChipStructs$ApplicationBasicClusterApplicationStruct");
                    return nullptr;
                }

                jmethodID applicationStructStructCtor_0;
                err = chip::JniReferences::GetInstance().FindMethod(env, applicationStructStructClass_0, "<init>",
                                                                    "(Ljava/lang/Integer;Ljava/lang/String;)V",
                                                                    &applicationStructStructCtor_0);
                if (err != CHIP_NO_ERROR || applicationStructStructCtor_0 == nullptr)
                {
                    ChipLogError(Zcl, "Could not find ChipStructs$ApplicationBasicClusterApplicationStruct constructor");
                    return nullptr;
                }

                value = env->NewObject(applicationStructStructClass_0, applicationStructStructCtor_0, value_catalogVendorID,
                                       value_applicationID);
            }
            return value;
        }
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
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
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- ApplicationLauncher (0x050C) ---
    case app::Clusters::ApplicationLauncher::Id: {
        using namespace app::Clusters::ApplicationLauncher;
        switch (aPath.mAttributeId)
        {
        case Attributes::CatalogList::Id: {
            using TypeInfo = Attributes::CatalogList::TypeInfo;
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
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentApp::Id: {
            using TypeInfo = Attributes::CurrentApp::TypeInfo;
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
                jobject value_application_catalogVendorID;
                std::string value_application_catalogVendorIDClassName     = "java/lang/Integer";
                std::string value_application_catalogVendorIDCtorSignature = "(I)V";
                jint jnivalue_application_catalogVendorID = static_cast<jint>(cppValue.Value().application.catalogVendorID);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    value_application_catalogVendorIDClassName.c_str(), value_application_catalogVendorIDCtorSignature.c_str(),
                    jnivalue_application_catalogVendorID, value_application_catalogVendorID);
                jobject value_application_applicationID;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().application.applicationID,
                                                                                     value_application_applicationID));

                {
                    jclass applicationStructStructClass_2;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ApplicationLauncherClusterApplicationStruct",
                        applicationStructStructClass_2);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ApplicationLauncherClusterApplicationStruct");
                        return nullptr;
                    }

                    jmethodID applicationStructStructCtor_2;
                    err = chip::JniReferences::GetInstance().FindMethod(env, applicationStructStructClass_2, "<init>",
                                                                        "(Ljava/lang/Integer;Ljava/lang/String;)V",
                                                                        &applicationStructStructCtor_2);
                    if (err != CHIP_NO_ERROR || applicationStructStructCtor_2 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ApplicationLauncherClusterApplicationStruct constructor");
                        return nullptr;
                    }

                    value_application = env->NewObject(applicationStructStructClass_2, applicationStructStructCtor_2,
                                                       value_application_catalogVendorID, value_application_applicationID);
                }
                jobject value_endpoint;
                if (!cppValue.Value().endpoint.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_endpoint);
                }
                else
                {
                    jobject value_endpointInsideOptional;
                    std::string value_endpointInsideOptionalClassName     = "java/lang/Integer";
                    std::string value_endpointInsideOptionalCtorSignature = "(I)V";
                    jint jnivalue_endpointInsideOptional                  = static_cast<jint>(cppValue.Value().endpoint.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        value_endpointInsideOptionalClassName.c_str(), value_endpointInsideOptionalCtorSignature.c_str(),
                        jnivalue_endpointInsideOptional, value_endpointInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_endpointInsideOptional, value_endpoint);
                }

                {
                    jclass applicationEPStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ApplicationLauncherClusterApplicationEPStruct",
                        applicationEPStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ApplicationLauncherClusterApplicationEPStruct");
                        return nullptr;
                    }

                    jmethodID applicationEPStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, applicationEPStructStructClass_1, "<init>",
                        "(Lchip/devicecontroller/ChipStructs$ApplicationLauncherClusterApplicationStruct;Ljava/util/Optional;)V",
                        &applicationEPStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || applicationEPStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ApplicationLauncherClusterApplicationEPStruct constructor");
                        return nullptr;
                    }

                    value = env->NewObject(applicationEPStructStructClass_1, applicationEPStructStructCtor_1, value_application,
                                           value_endpoint);
                }
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- AudioOutput (0x050B) ---
    case app::Clusters::AudioOutput::Id: {
        using namespace app::Clusters::AudioOutput;
        switch (aPath.mAttributeId)
        {
        case Attributes::OutputList::Id: {
            using TypeInfo = Attributes::OutputList::TypeInfo;
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
                jint jninewElement_0_index                  = static_cast<jint>(entry_0.index);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_indexClassName.c_str(),
                                                                           newElement_0_indexCtorSignature.c_str(),
                                                                           jninewElement_0_index, newElement_0_index);
                jobject newElement_0_outputType;
                std::string newElement_0_outputTypeClassName     = "java/lang/Integer";
                std::string newElement_0_outputTypeCtorSignature = "(I)V";
                jint jninewElement_0_outputType                  = static_cast<jint>(entry_0.outputType);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_outputTypeClassName.c_str(),
                                                                           newElement_0_outputTypeCtorSignature.c_str(),
                                                                           jninewElement_0_outputType, newElement_0_outputType);
                jobject newElement_0_name;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.name, newElement_0_name));

                {
                    jclass outputInfoStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$AudioOutputClusterOutputInfoStruct", outputInfoStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$AudioOutputClusterOutputInfoStruct");
                        return nullptr;
                    }

                    jmethodID outputInfoStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, outputInfoStructStructClass_1, "<init>", "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/String;)V",
                        &outputInfoStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || outputInfoStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$AudioOutputClusterOutputInfoStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(outputInfoStructStructClass_1, outputInfoStructStructCtor_1, newElement_0_index,
                                                  newElement_0_outputType, newElement_0_name);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentOutput::Id: {
            using TypeInfo = Attributes::CurrentOutput::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- Binding (0x001E) ---
    case app::Clusters::Binding::Id: {
        using namespace app::Clusters::Binding;
        switch (aPath.mAttributeId)
        {
        case Attributes::Binding::Id: {
            using TypeInfo = Attributes::Binding::TypeInfo;
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
                jobject newElement_0_node;
                if (!entry_0.node.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_node);
                }
                else
                {
                    jobject newElement_0_nodeInsideOptional;
                    std::string newElement_0_nodeInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_nodeInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_nodeInsideOptional                 = static_cast<jlong>(entry_0.node.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_nodeInsideOptionalClassName.c_str(), newElement_0_nodeInsideOptionalCtorSignature.c_str(),
                        jninewElement_0_nodeInsideOptional, newElement_0_nodeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_nodeInsideOptional, newElement_0_node);
                }
                jobject newElement_0_group;
                if (!entry_0.group.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_group);
                }
                else
                {
                    jobject newElement_0_groupInsideOptional;
                    std::string newElement_0_groupInsideOptionalClassName     = "java/lang/Integer";
                    std::string newElement_0_groupInsideOptionalCtorSignature = "(I)V";
                    jint jninewElement_0_groupInsideOptional                  = static_cast<jint>(entry_0.group.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        newElement_0_groupInsideOptionalClassName.c_str(), newElement_0_groupInsideOptionalCtorSignature.c_str(),
                        jninewElement_0_groupInsideOptional, newElement_0_groupInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_groupInsideOptional, newElement_0_group);
                }
                jobject newElement_0_endpoint;
                if (!entry_0.endpoint.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_endpoint);
                }
                else
                {
                    jobject newElement_0_endpointInsideOptional;
                    std::string newElement_0_endpointInsideOptionalClassName     = "java/lang/Integer";
                    std::string newElement_0_endpointInsideOptionalCtorSignature = "(I)V";
                    jint jninewElement_0_endpointInsideOptional                  = static_cast<jint>(entry_0.endpoint.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        newElement_0_endpointInsideOptionalClassName.c_str(),
                        newElement_0_endpointInsideOptionalCtorSignature.c_str(), jninewElement_0_endpointInsideOptional,
                        newElement_0_endpointInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_endpointInsideOptional, newElement_0_endpoint);
                }
                jobject newElement_0_cluster;
                if (!entry_0.cluster.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_cluster);
                }
                else
                {
                    jobject newElement_0_clusterInsideOptional;
                    std::string newElement_0_clusterInsideOptionalClassName     = "java/lang/Long";
                    std::string newElement_0_clusterInsideOptionalCtorSignature = "(J)V";
                    jlong jninewElement_0_clusterInsideOptional                 = static_cast<jlong>(entry_0.cluster.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        newElement_0_clusterInsideOptionalClassName.c_str(),
                        newElement_0_clusterInsideOptionalCtorSignature.c_str(), jninewElement_0_clusterInsideOptional,
                        newElement_0_clusterInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_clusterInsideOptional, newElement_0_cluster);
                }
                jobject newElement_0_fabricIndex;
                std::string newElement_0_fabricIndexClassName     = "java/lang/Integer";
                std::string newElement_0_fabricIndexCtorSignature = "(I)V";
                jint jninewElement_0_fabricIndex                  = static_cast<jint>(entry_0.fabricIndex);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_fabricIndexClassName.c_str(),
                                                                           newElement_0_fabricIndexCtorSignature.c_str(),
                                                                           jninewElement_0_fabricIndex, newElement_0_fabricIndex);

                {
                    jclass targetStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$BindingClusterTargetStruct", targetStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$BindingClusterTargetStruct");
                        return nullptr;
                    }

                    jmethodID targetStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, targetStructStructClass_1, "<init>",
                        "(Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/lang/Integer;)V",
                        &targetStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || targetStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$BindingClusterTargetStruct constructor");
                        return nullptr;
                    }

                    newElement_0 =
                        env->NewObject(targetStructStructClass_1, targetStructStructCtor_1, newElement_0_node, newElement_0_group,
                                       newElement_0_endpoint, newElement_0_cluster, newElement_0_fabricIndex);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- Channel (0x0504) ---
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
                jint jninewElement_0_majorNumber                  = static_cast<jint>(entry_0.majorNumber);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_majorNumberClassName.c_str(),
                                                                           newElement_0_majorNumberCtorSignature.c_str(),
                                                                           jninewElement_0_majorNumber, newElement_0_majorNumber);
                jobject newElement_0_minorNumber;
                std::string newElement_0_minorNumberClassName     = "java/lang/Integer";
                std::string newElement_0_minorNumberCtorSignature = "(I)V";
                jint jninewElement_0_minorNumber                  = static_cast<jint>(entry_0.minorNumber);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_minorNumberClassName.c_str(),
                                                                           newElement_0_minorNumberCtorSignature.c_str(),
                                                                           jninewElement_0_minorNumber, newElement_0_minorNumber);
                jobject newElement_0_name;
                if (!entry_0.name.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_name);
                }
                else
                {
                    jobject newElement_0_nameInsideOptional;
                    LogErrorOnFailure(
                        chip::JniReferences::GetInstance().CharToStringUTF(entry_0.name.Value(), newElement_0_nameInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_nameInsideOptional, newElement_0_name);
                }
                jobject newElement_0_callSign;
                if (!entry_0.callSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_callSign);
                }
                else
                {
                    jobject newElement_0_callSignInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.callSign.Value(),
                                                                                         newElement_0_callSignInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_callSignInsideOptional, newElement_0_callSign);
                }
                jobject newElement_0_affiliateCallSign;
                if (!entry_0.affiliateCallSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_affiliateCallSign);
                }
                else
                {
                    jobject newElement_0_affiliateCallSignInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                        entry_0.affiliateCallSign.Value(), newElement_0_affiliateCallSignInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_affiliateCallSignInsideOptional,
                                                                      newElement_0_affiliateCallSign);
                }
                jobject newElement_0_identifier;
                if (!entry_0.identifier.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_identifier);
                }
                else
                {
                    jobject newElement_0_identifierInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.identifier.Value(),
                                                                                         newElement_0_identifierInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_identifierInsideOptional,
                                                                      newElement_0_identifier);
                }
                jobject newElement_0_type;
                if (!entry_0.type.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_type);
                }
                else
                {
                    jobject newElement_0_typeInsideOptional;
                    std::string newElement_0_typeInsideOptionalClassName     = "java/lang/Integer";
                    std::string newElement_0_typeInsideOptionalCtorSignature = "(I)V";
                    jint jninewElement_0_typeInsideOptional                  = static_cast<jint>(entry_0.type.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        newElement_0_typeInsideOptionalClassName.c_str(), newElement_0_typeInsideOptionalCtorSignature.c_str(),
                        jninewElement_0_typeInsideOptional, newElement_0_typeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_typeInsideOptional, newElement_0_type);
                }

                {
                    jclass channelInfoStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ChannelClusterChannelInfoStruct", channelInfoStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ChannelClusterChannelInfoStruct");
                        return nullptr;
                    }

                    jmethodID channelInfoStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, channelInfoStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/"
                        "util/Optional;Ljava/util/Optional;)V",
                        &channelInfoStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || channelInfoStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ChannelClusterChannelInfoStruct constructor");
                        return nullptr;
                    }

                    newElement_0 =
                        env->NewObject(channelInfoStructStructClass_1, channelInfoStructStructCtor_1, newElement_0_majorNumber,
                                       newElement_0_minorNumber, newElement_0_name, newElement_0_callSign,
                                       newElement_0_affiliateCallSign, newElement_0_identifier, newElement_0_type);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::Lineup::Id: {
            using TypeInfo = Attributes::Lineup::TypeInfo;
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
                LogErrorOnFailure(
                    chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().operatorName, value_operatorName));
                jobject value_lineupName;
                if (!cppValue.Value().lineupName.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_lineupName);
                }
                else
                {
                    jobject value_lineupNameInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().lineupName.Value(),
                                                                                         value_lineupNameInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(value_lineupNameInsideOptional, value_lineupName);
                }
                jobject value_postalCode;
                if (!cppValue.Value().postalCode.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_postalCode);
                }
                else
                {
                    jobject value_postalCodeInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().postalCode.Value(),
                                                                                         value_postalCodeInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(value_postalCodeInsideOptional, value_postalCode);
                }
                jobject value_lineupInfoType;
                std::string value_lineupInfoTypeClassName     = "java/lang/Integer";
                std::string value_lineupInfoTypeCtorSignature = "(I)V";
                jint jnivalue_lineupInfoType                  = static_cast<jint>(cppValue.Value().lineupInfoType);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_lineupInfoTypeClassName.c_str(),
                                                                           value_lineupInfoTypeCtorSignature.c_str(),
                                                                           jnivalue_lineupInfoType, value_lineupInfoType);

                {
                    jclass lineupInfoStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ChannelClusterLineupInfoStruct", lineupInfoStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ChannelClusterLineupInfoStruct");
                        return nullptr;
                    }

                    jmethodID lineupInfoStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, lineupInfoStructStructClass_1, "<init>",
                        "(Ljava/lang/String;Ljava/util/Optional;Ljava/util/Optional;Ljava/lang/Integer;)V",
                        &lineupInfoStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || lineupInfoStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ChannelClusterLineupInfoStruct constructor");
                        return nullptr;
                    }

                    value = env->NewObject(lineupInfoStructStructClass_1, lineupInfoStructStructCtor_1, value_operatorName,
                                           value_lineupName, value_postalCode, value_lineupInfoType);
                }
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
                jint jnivalue_majorNumber                  = static_cast<jint>(cppValue.Value().majorNumber);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_majorNumberClassName.c_str(),
                                                                           value_majorNumberCtorSignature.c_str(),
                                                                           jnivalue_majorNumber, value_majorNumber);
                jobject value_minorNumber;
                std::string value_minorNumberClassName     = "java/lang/Integer";
                std::string value_minorNumberCtorSignature = "(I)V";
                jint jnivalue_minorNumber                  = static_cast<jint>(cppValue.Value().minorNumber);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(value_minorNumberClassName.c_str(),
                                                                           value_minorNumberCtorSignature.c_str(),
                                                                           jnivalue_minorNumber, value_minorNumber);
                jobject value_name;
                if (!cppValue.Value().name.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_name);
                }
                else
                {
                    jobject value_nameInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().name.Value(),
                                                                                         value_nameInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(value_nameInsideOptional, value_name);
                }
                jobject value_callSign;
                if (!cppValue.Value().callSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_callSign);
                }
                else
                {
                    jobject value_callSignInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().callSign.Value(),
                                                                                         value_callSignInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(value_callSignInsideOptional, value_callSign);
                }
                jobject value_affiliateCallSign;
                if (!cppValue.Value().affiliateCallSign.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_affiliateCallSign);
                }
                else
                {
                    jobject value_affiliateCallSignInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().affiliateCallSign.Value(),
                                                                                         value_affiliateCallSignInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(value_affiliateCallSignInsideOptional,
                                                                      value_affiliateCallSign);
                }
                jobject value_identifier;
                if (!cppValue.Value().identifier.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_identifier);
                }
                else
                {
                    jobject value_identifierInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().identifier.Value(),
                                                                                         value_identifierInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(value_identifierInsideOptional, value_identifier);
                }
                jobject value_type;
                if (!cppValue.Value().type.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, value_type);
                }
                else
                {
                    jobject value_typeInsideOptional;
                    std::string value_typeInsideOptionalClassName     = "java/lang/Integer";
                    std::string value_typeInsideOptionalCtorSignature = "(I)V";
                    jint jnivalue_typeInsideOptional                  = static_cast<jint>(cppValue.Value().type.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                        value_typeInsideOptionalClassName.c_str(), value_typeInsideOptionalCtorSignature.c_str(),
                        jnivalue_typeInsideOptional, value_typeInsideOptional);
                    chip::JniReferences::GetInstance().CreateOptional(value_typeInsideOptional, value_type);
                }

                {
                    jclass channelInfoStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ChannelClusterChannelInfoStruct", channelInfoStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ChannelClusterChannelInfoStruct");
                        return nullptr;
                    }

                    jmethodID channelInfoStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, channelInfoStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;Ljava/util/Optional;Ljava/util/Optional;Ljava/"
                        "util/Optional;Ljava/util/Optional;)V",
                        &channelInfoStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || channelInfoStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ChannelClusterChannelInfoStruct constructor");
                        return nullptr;
                    }

                    value = env->NewObject(channelInfoStructStructClass_1, channelInfoStructStructCtor_1, value_majorNumber,
                                           value_minorNumber, value_name, value_callSign, value_affiliateCallSign, value_identifier,
                                           value_type);
                }
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- ContentAppObserver (0x0510) ---
    case app::Clusters::ContentAppObserver::Id: {
        using namespace app::Clusters::ContentAppObserver;
        switch (aPath.mAttributeId)
        {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- ContentControl (0x050F) ---
    case app::Clusters::ContentControl::Id: {
        using namespace app::Clusters::ContentControl;
        switch (aPath.mAttributeId)
        {
        case Attributes::Enabled::Id: {
            using TypeInfo = Attributes::Enabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            jboolean jnivalue              = static_cast<jboolean>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
            return value;
        }
        case Attributes::OnDemandRatings::Id: {
            using TypeInfo = Attributes::OnDemandRatings::TypeInfo;
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
                jobject newElement_0_ratingName;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.ratingName, newElement_0_ratingName));
                jobject newElement_0_ratingNameDesc;
                if (!entry_0.ratingNameDesc.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_ratingNameDesc);
                }
                else
                {
                    jobject newElement_0_ratingNameDescInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                        entry_0.ratingNameDesc.Value(), newElement_0_ratingNameDescInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_ratingNameDescInsideOptional,
                                                                      newElement_0_ratingNameDesc);
                }

                {
                    jclass ratingNameStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ContentControlClusterRatingNameStruct",
                        ratingNameStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ContentControlClusterRatingNameStruct");
                        return nullptr;
                    }

                    jmethodID ratingNameStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(env, ratingNameStructStructClass_1, "<init>",
                                                                        "(Ljava/lang/String;Ljava/util/Optional;)V",
                                                                        &ratingNameStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || ratingNameStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ContentControlClusterRatingNameStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(ratingNameStructStructClass_1, ratingNameStructStructCtor_1,
                                                  newElement_0_ratingName, newElement_0_ratingNameDesc);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::OnDemandRatingThreshold::Id: {
            using TypeInfo = Attributes::OnDemandRatingThreshold::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
            return value;
        }
        case Attributes::ScheduledContentRatings::Id: {
            using TypeInfo = Attributes::ScheduledContentRatings::TypeInfo;
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
                jobject newElement_0_ratingName;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.ratingName, newElement_0_ratingName));
                jobject newElement_0_ratingNameDesc;
                if (!entry_0.ratingNameDesc.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_ratingNameDesc);
                }
                else
                {
                    jobject newElement_0_ratingNameDescInsideOptional;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                        entry_0.ratingNameDesc.Value(), newElement_0_ratingNameDescInsideOptional));
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_ratingNameDescInsideOptional,
                                                                      newElement_0_ratingNameDesc);
                }

                {
                    jclass ratingNameStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$ContentControlClusterRatingNameStruct",
                        ratingNameStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$ContentControlClusterRatingNameStruct");
                        return nullptr;
                    }

                    jmethodID ratingNameStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(env, ratingNameStructStructClass_1, "<init>",
                                                                        "(Ljava/lang/String;Ljava/util/Optional;)V",
                                                                        &ratingNameStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || ratingNameStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$ContentControlClusterRatingNameStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(ratingNameStructStructClass_1, ratingNameStructStructCtor_1,
                                                  newElement_0_ratingName, newElement_0_ratingNameDesc);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ScheduledContentRatingThreshold::Id: {
            using TypeInfo = Attributes::ScheduledContentRatingThreshold::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
            return value;
        }
        case Attributes::ScreenDailyTime::Id: {
            using TypeInfo = Attributes::ScreenDailyTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
            return value;
        }
        case Attributes::RemainingScreenTime::Id: {
            using TypeInfo = Attributes::RemainingScreenTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Long";
            std::string valueCtorSignature = "(J)V";
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
            return value;
        }
        case Attributes::BlockUnrated::Id: {
            using TypeInfo = Attributes::BlockUnrated::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Boolean";
            std::string valueCtorSignature = "(Z)V";
            jboolean jnivalue              = static_cast<jboolean>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- ContentLauncher (0x050A) ---
    case app::Clusters::ContentLauncher::Id: {
        using namespace app::Clusters::ContentLauncher;
        switch (aPath.mAttributeId)
        {
        case Attributes::AcceptHeader::Id: {
            using TypeInfo = Attributes::AcceptHeader::TypeInfo;
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
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0, newElement_0));
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- Descriptor (0x001D) ---
    case app::Clusters::Descriptor::Id: {
        using namespace app::Clusters::Descriptor;
        switch (aPath.mAttributeId)
        {
        case Attributes::DeviceTypeList::Id: {
            using TypeInfo = Attributes::DeviceTypeList::TypeInfo;
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
                jobject newElement_0_deviceType;
                std::string newElement_0_deviceTypeClassName     = "java/lang/Long";
                std::string newElement_0_deviceTypeCtorSignature = "(J)V";
                jlong jninewElement_0_deviceType                 = static_cast<jlong>(entry_0.deviceType);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(newElement_0_deviceTypeClassName.c_str(),
                                                                            newElement_0_deviceTypeCtorSignature.c_str(),
                                                                            jninewElement_0_deviceType, newElement_0_deviceType);
                jobject newElement_0_revision;
                std::string newElement_0_revisionClassName     = "java/lang/Integer";
                std::string newElement_0_revisionCtorSignature = "(I)V";
                jint jninewElement_0_revision                  = static_cast<jint>(entry_0.revision);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_revisionClassName.c_str(),
                                                                           newElement_0_revisionCtorSignature.c_str(),
                                                                           jninewElement_0_revision, newElement_0_revision);

                {
                    jclass deviceTypeStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$DescriptorClusterDeviceTypeStruct", deviceTypeStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$DescriptorClusterDeviceTypeStruct");
                        return nullptr;
                    }

                    jmethodID deviceTypeStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(env, deviceTypeStructStructClass_1, "<init>",
                                                                        "(Ljava/lang/Long;Ljava/lang/Integer;)V",
                                                                        &deviceTypeStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || deviceTypeStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$DescriptorClusterDeviceTypeStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(deviceTypeStructStructClass_1, deviceTypeStructStructCtor_1,
                                                  newElement_0_deviceType, newElement_0_revision);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jint jninewElement_0                  = static_cast<jint>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::TagList::Id: {
            using TypeInfo = Attributes::TagList::TypeInfo;
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
                jobject newElement_0_mfgCode;
                if (entry_0.mfgCode.IsNull())
                {
                    newElement_0_mfgCode = nullptr;
                }
                else
                {
                    std::string newElement_0_mfgCodeClassName     = "java/lang/Integer";
                    std::string newElement_0_mfgCodeCtorSignature = "(I)V";
                    jint jninewElement_0_mfgCode                  = static_cast<jint>(entry_0.mfgCode.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_mfgCodeClassName.c_str(),
                                                                               newElement_0_mfgCodeCtorSignature.c_str(),
                                                                               jninewElement_0_mfgCode, newElement_0_mfgCode);
                }
                jobject newElement_0_namespaceID;
                std::string newElement_0_namespaceIDClassName     = "java/lang/Integer";
                std::string newElement_0_namespaceIDCtorSignature = "(I)V";
                jint jninewElement_0_namespaceID                  = static_cast<jint>(entry_0.namespaceID);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_namespaceIDClassName.c_str(),
                                                                           newElement_0_namespaceIDCtorSignature.c_str(),
                                                                           jninewElement_0_namespaceID, newElement_0_namespaceID);
                jobject newElement_0_tag;
                std::string newElement_0_tagClassName     = "java/lang/Integer";
                std::string newElement_0_tagCtorSignature = "(I)V";
                jint jninewElement_0_tag                  = static_cast<jint>(entry_0.tag);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_tagClassName.c_str(),
                                                                           newElement_0_tagCtorSignature.c_str(),
                                                                           jninewElement_0_tag, newElement_0_tag);
                jobject newElement_0_label;
                if (!entry_0.label.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_label);
                }
                else
                {
                    jobject newElement_0_labelInsideOptional;
                    if (entry_0.label.Value().IsNull())
                    {
                        newElement_0_labelInsideOptional = nullptr;
                    }
                    else
                    {
                        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.label.Value().Value(),
                                                                                             newElement_0_labelInsideOptional));
                    }
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_labelInsideOptional, newElement_0_label);
                }

                {
                    jclass semanticTagStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$DescriptorClusterSemanticTagStruct",
                        semanticTagStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$DescriptorClusterSemanticTagStruct");
                        return nullptr;
                    }

                    jmethodID semanticTagStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, semanticTagStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Integer;Ljava/util/Optional;)V",
                        &semanticTagStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || semanticTagStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$DescriptorClusterSemanticTagStruct constructor");
                        return nullptr;
                    }

                    newElement_0 =
                        env->NewObject(semanticTagStructStructClass_1, semanticTagStructStructCtor_1, newElement_0_mfgCode,
                                       newElement_0_namespaceID, newElement_0_tag, newElement_0_label);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::EndpointUniqueID::Id: {
            using TypeInfo = Attributes::EndpointUniqueID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- KeypadInput (0x0509) ---
    case app::Clusters::KeypadInput::Id: {
        using namespace app::Clusters::KeypadInput;
        switch (aPath.mAttributeId)
        {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- LevelControl (0x0008) ---
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
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
            }
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue.Raw());
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
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
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
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
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
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
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
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
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- LowPower (0x0508) ---
    case app::Clusters::LowPower::Id: {
        using namespace app::Clusters::LowPower;
        switch (aPath.mAttributeId)
        {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- MediaInput (0x0507) ---
    case app::Clusters::MediaInput::Id: {
        using namespace app::Clusters::MediaInput;
        switch (aPath.mAttributeId)
        {
        case Attributes::InputList::Id: {
            using TypeInfo = Attributes::InputList::TypeInfo;
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
                jint jninewElement_0_index                  = static_cast<jint>(entry_0.index);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_indexClassName.c_str(),
                                                                           newElement_0_indexCtorSignature.c_str(),
                                                                           jninewElement_0_index, newElement_0_index);
                jobject newElement_0_inputType;
                std::string newElement_0_inputTypeClassName     = "java/lang/Integer";
                std::string newElement_0_inputTypeCtorSignature = "(I)V";
                jint jninewElement_0_inputType                  = static_cast<jint>(entry_0.inputType);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_inputTypeClassName.c_str(),
                                                                           newElement_0_inputTypeCtorSignature.c_str(),
                                                                           jninewElement_0_inputType, newElement_0_inputType);
                jobject newElement_0_name;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_0.name, newElement_0_name));
                jobject newElement_0_description;
                LogErrorOnFailure(
                    chip::JniReferences::GetInstance().CharToStringUTF(entry_0.description, newElement_0_description));

                {
                    jclass inputInfoStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$MediaInputClusterInputInfoStruct", inputInfoStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$MediaInputClusterInputInfoStruct");
                        return nullptr;
                    }

                    jmethodID inputInfoStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, inputInfoStructStructClass_1, "<init>",
                        "(Ljava/lang/Integer;Ljava/lang/Integer;Ljava/lang/String;Ljava/lang/String;)V",
                        &inputInfoStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || inputInfoStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$MediaInputClusterInputInfoStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(inputInfoStructStructClass_1, inputInfoStructStructCtor_1, newElement_0_index,
                                                  newElement_0_inputType, newElement_0_name, newElement_0_description);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentInput::Id: {
            using TypeInfo = Attributes::CurrentInput::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- MediaPlayback (0x0506) ---
    case app::Clusters::MediaPlayback::Id: {
        using namespace app::Clusters::MediaPlayback;
        switch (aPath.mAttributeId)
        {
        case Attributes::CurrentState::Id: {
            using TypeInfo = Attributes::CurrentState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
                jlong jnivalue                 = static_cast<jlong>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                            jnivalue, value);
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
                jlong jnivalue                 = static_cast<jlong>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                            jnivalue, value);
            }
            return value;
        }
        case Attributes::SampledPosition::Id: {
            using TypeInfo = Attributes::SampledPosition::TypeInfo;
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
                jobject value_updatedAt;
                std::string value_updatedAtClassName     = "java/lang/Long";
                std::string value_updatedAtCtorSignature = "(J)V";
                jlong jnivalue_updatedAt                 = static_cast<jlong>(cppValue.Value().updatedAt);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    value_updatedAtClassName.c_str(), value_updatedAtCtorSignature.c_str(), jnivalue_updatedAt, value_updatedAt);
                jobject value_position;
                if (cppValue.Value().position.IsNull())
                {
                    value_position = nullptr;
                }
                else
                {
                    std::string value_positionClassName     = "java/lang/Long";
                    std::string value_positionCtorSignature = "(J)V";
                    jlong jnivalue_position                 = static_cast<jlong>(cppValue.Value().position.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                        value_positionClassName.c_str(), value_positionCtorSignature.c_str(), jnivalue_position, value_position);
                }

                {
                    jclass playbackPositionStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterPlaybackPositionStruct",
                        playbackPositionStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterPlaybackPositionStruct");
                        return nullptr;
                    }

                    jmethodID playbackPositionStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(env, playbackPositionStructStructClass_1, "<init>",
                                                                        "(Ljava/lang/Long;Ljava/lang/Long;)V",
                                                                        &playbackPositionStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || playbackPositionStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterPlaybackPositionStruct constructor");
                        return nullptr;
                    }

                    value = env->NewObject(playbackPositionStructStructClass_1, playbackPositionStructStructCtor_1, value_updatedAt,
                                           value_position);
                }
            }
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
            jfloat jnivalue                = static_cast<jfloat>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jfloat>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                         jnivalue, value);
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
                jlong jnivalue                 = static_cast<jlong>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                            jnivalue, value);
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
                jlong jnivalue                 = static_cast<jlong>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                            jnivalue, value);
            }
            return value;
        }
        case Attributes::ActiveAudioTrack::Id: {
            using TypeInfo = Attributes::ActiveAudioTrack::TypeInfo;
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
                jobject value_id;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().id, value_id));
                jobject value_trackAttributes;
                if (cppValue.Value().trackAttributes.IsNull())
                {
                    value_trackAttributes = nullptr;
                }
                else
                {
                    jobject value_trackAttributes_languageCode;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                        cppValue.Value().trackAttributes.Value().languageCode, value_trackAttributes_languageCode));
                    jobject value_trackAttributes_displayName;
                    if (!cppValue.Value().trackAttributes.Value().displayName.HasValue())
                    {
                        chip::JniReferences::GetInstance().CreateOptional(nullptr, value_trackAttributes_displayName);
                    }
                    else
                    {
                        jobject value_trackAttributes_displayNameInsideOptional;
                        if (cppValue.Value().trackAttributes.Value().displayName.Value().IsNull())
                        {
                            value_trackAttributes_displayNameInsideOptional = nullptr;
                        }
                        else
                        {
                            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                                cppValue.Value().trackAttributes.Value().displayName.Value().Value(),
                                value_trackAttributes_displayNameInsideOptional));
                        }
                        chip::JniReferences::GetInstance().CreateOptional(value_trackAttributes_displayNameInsideOptional,
                                                                          value_trackAttributes_displayName);
                    }

                    {
                        jclass trackAttributesStructStructClass_3;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct",
                            trackAttributesStructStructClass_3);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackAttributesStruct");
                            return nullptr;
                        }

                        jmethodID trackAttributesStructStructCtor_3;
                        err = chip::JniReferences::GetInstance().FindMethod(env, trackAttributesStructStructClass_3, "<init>",
                                                                            "(Ljava/lang/String;Ljava/util/Optional;)V",
                                                                            &trackAttributesStructStructCtor_3);
                        if (err != CHIP_NO_ERROR || trackAttributesStructStructCtor_3 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterTrackAttributesStruct constructor");
                            return nullptr;
                        }

                        value_trackAttributes =
                            env->NewObject(trackAttributesStructStructClass_3, trackAttributesStructStructCtor_3,
                                           value_trackAttributes_languageCode, value_trackAttributes_displayName);
                    }
                }

                {
                    jclass trackStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackStruct", trackStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackStruct");
                        return nullptr;
                    }

                    jmethodID trackStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, trackStructStructClass_1, "<init>",
                        "(Ljava/lang/String;Lchip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct;)V",
                        &trackStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || trackStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterTrackStruct constructor");
                        return nullptr;
                    }

                    value = env->NewObject(trackStructStructClass_1, trackStructStructCtor_1, value_id, value_trackAttributes);
                }
            }
            return value;
        }
        case Attributes::AvailableAudioTracks::Id: {
            using TypeInfo = Attributes::AvailableAudioTracks::TypeInfo;
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
                chip::JniReferences::GetInstance().CreateArrayList(value);

                auto iter_value_1 = cppValue.Value().begin();
                while (iter_value_1.Next())
                {
                    auto & entry_1 = iter_value_1.GetValue();
                    jobject newElement_1;
                    jobject newElement_1_id;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_1.id, newElement_1_id));
                    jobject newElement_1_trackAttributes;
                    if (entry_1.trackAttributes.IsNull())
                    {
                        newElement_1_trackAttributes = nullptr;
                    }
                    else
                    {
                        jobject newElement_1_trackAttributes_languageCode;
                        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                            entry_1.trackAttributes.Value().languageCode, newElement_1_trackAttributes_languageCode));
                        jobject newElement_1_trackAttributes_displayName;
                        if (!entry_1.trackAttributes.Value().displayName.HasValue())
                        {
                            chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_1_trackAttributes_displayName);
                        }
                        else
                        {
                            jobject newElement_1_trackAttributes_displayNameInsideOptional;
                            if (entry_1.trackAttributes.Value().displayName.Value().IsNull())
                            {
                                newElement_1_trackAttributes_displayNameInsideOptional = nullptr;
                            }
                            else
                            {
                                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                                    entry_1.trackAttributes.Value().displayName.Value().Value(),
                                    newElement_1_trackAttributes_displayNameInsideOptional));
                            }
                            chip::JniReferences::GetInstance().CreateOptional(
                                newElement_1_trackAttributes_displayNameInsideOptional, newElement_1_trackAttributes_displayName);
                        }

                        {
                            jclass trackAttributesStructStructClass_4;
                            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                                env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct",
                                trackAttributesStructStructClass_4);
                            if (err != CHIP_NO_ERROR)
                            {
                                ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackAttributesStruct");
                                return nullptr;
                            }

                            jmethodID trackAttributesStructStructCtor_4;
                            err = chip::JniReferences::GetInstance().FindMethod(env, trackAttributesStructStructClass_4, "<init>",
                                                                                "(Ljava/lang/String;Ljava/util/Optional;)V",
                                                                                &trackAttributesStructStructCtor_4);
                            if (err != CHIP_NO_ERROR || trackAttributesStructStructCtor_4 == nullptr)
                            {
                                ChipLogError(Zcl,
                                             "Could not find ChipStructs$MediaPlaybackClusterTrackAttributesStruct constructor");
                                return nullptr;
                            }

                            newElement_1_trackAttributes =
                                env->NewObject(trackAttributesStructStructClass_4, trackAttributesStructStructCtor_4,
                                               newElement_1_trackAttributes_languageCode, newElement_1_trackAttributes_displayName);
                        }
                    }

                    {
                        jclass trackStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackStruct", trackStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackStruct");
                            return nullptr;
                        }

                        jmethodID trackStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(
                            env, trackStructStructClass_2, "<init>",
                            "(Ljava/lang/String;Lchip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct;)V",
                            &trackStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || trackStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterTrackStruct constructor");
                            return nullptr;
                        }

                        newElement_1 = env->NewObject(trackStructStructClass_2, trackStructStructCtor_2, newElement_1_id,
                                                      newElement_1_trackAttributes);
                    }
                    chip::JniReferences::GetInstance().AddToList(value, newElement_1);
                }
            }
            return value;
        }
        case Attributes::ActiveTextTrack::Id: {
            using TypeInfo = Attributes::ActiveTextTrack::TypeInfo;
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
                jobject value_id;
                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue.Value().id, value_id));
                jobject value_trackAttributes;
                if (cppValue.Value().trackAttributes.IsNull())
                {
                    value_trackAttributes = nullptr;
                }
                else
                {
                    jobject value_trackAttributes_languageCode;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                        cppValue.Value().trackAttributes.Value().languageCode, value_trackAttributes_languageCode));
                    jobject value_trackAttributes_displayName;
                    if (!cppValue.Value().trackAttributes.Value().displayName.HasValue())
                    {
                        chip::JniReferences::GetInstance().CreateOptional(nullptr, value_trackAttributes_displayName);
                    }
                    else
                    {
                        jobject value_trackAttributes_displayNameInsideOptional;
                        if (cppValue.Value().trackAttributes.Value().displayName.Value().IsNull())
                        {
                            value_trackAttributes_displayNameInsideOptional = nullptr;
                        }
                        else
                        {
                            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                                cppValue.Value().trackAttributes.Value().displayName.Value().Value(),
                                value_trackAttributes_displayNameInsideOptional));
                        }
                        chip::JniReferences::GetInstance().CreateOptional(value_trackAttributes_displayNameInsideOptional,
                                                                          value_trackAttributes_displayName);
                    }

                    {
                        jclass trackAttributesStructStructClass_3;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct",
                            trackAttributesStructStructClass_3);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackAttributesStruct");
                            return nullptr;
                        }

                        jmethodID trackAttributesStructStructCtor_3;
                        err = chip::JniReferences::GetInstance().FindMethod(env, trackAttributesStructStructClass_3, "<init>",
                                                                            "(Ljava/lang/String;Ljava/util/Optional;)V",
                                                                            &trackAttributesStructStructCtor_3);
                        if (err != CHIP_NO_ERROR || trackAttributesStructStructCtor_3 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterTrackAttributesStruct constructor");
                            return nullptr;
                        }

                        value_trackAttributes =
                            env->NewObject(trackAttributesStructStructClass_3, trackAttributesStructStructCtor_3,
                                           value_trackAttributes_languageCode, value_trackAttributes_displayName);
                    }
                }

                {
                    jclass trackStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackStruct", trackStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackStruct");
                        return nullptr;
                    }

                    jmethodID trackStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, trackStructStructClass_1, "<init>",
                        "(Ljava/lang/String;Lchip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct;)V",
                        &trackStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || trackStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterTrackStruct constructor");
                        return nullptr;
                    }

                    value = env->NewObject(trackStructStructClass_1, trackStructStructCtor_1, value_id, value_trackAttributes);
                }
            }
            return value;
        }
        case Attributes::AvailableTextTracks::Id: {
            using TypeInfo = Attributes::AvailableTextTracks::TypeInfo;
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
                chip::JniReferences::GetInstance().CreateArrayList(value);

                auto iter_value_1 = cppValue.Value().begin();
                while (iter_value_1.Next())
                {
                    auto & entry_1 = iter_value_1.GetValue();
                    jobject newElement_1;
                    jobject newElement_1_id;
                    LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_1.id, newElement_1_id));
                    jobject newElement_1_trackAttributes;
                    if (entry_1.trackAttributes.IsNull())
                    {
                        newElement_1_trackAttributes = nullptr;
                    }
                    else
                    {
                        jobject newElement_1_trackAttributes_languageCode;
                        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                            entry_1.trackAttributes.Value().languageCode, newElement_1_trackAttributes_languageCode));
                        jobject newElement_1_trackAttributes_displayName;
                        if (!entry_1.trackAttributes.Value().displayName.HasValue())
                        {
                            chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_1_trackAttributes_displayName);
                        }
                        else
                        {
                            jobject newElement_1_trackAttributes_displayNameInsideOptional;
                            if (entry_1.trackAttributes.Value().displayName.Value().IsNull())
                            {
                                newElement_1_trackAttributes_displayNameInsideOptional = nullptr;
                            }
                            else
                            {
                                LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(
                                    entry_1.trackAttributes.Value().displayName.Value().Value(),
                                    newElement_1_trackAttributes_displayNameInsideOptional));
                            }
                            chip::JniReferences::GetInstance().CreateOptional(
                                newElement_1_trackAttributes_displayNameInsideOptional, newElement_1_trackAttributes_displayName);
                        }

                        {
                            jclass trackAttributesStructStructClass_4;
                            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                                env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct",
                                trackAttributesStructStructClass_4);
                            if (err != CHIP_NO_ERROR)
                            {
                                ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackAttributesStruct");
                                return nullptr;
                            }

                            jmethodID trackAttributesStructStructCtor_4;
                            err = chip::JniReferences::GetInstance().FindMethod(env, trackAttributesStructStructClass_4, "<init>",
                                                                                "(Ljava/lang/String;Ljava/util/Optional;)V",
                                                                                &trackAttributesStructStructCtor_4);
                            if (err != CHIP_NO_ERROR || trackAttributesStructStructCtor_4 == nullptr)
                            {
                                ChipLogError(Zcl,
                                             "Could not find ChipStructs$MediaPlaybackClusterTrackAttributesStruct constructor");
                                return nullptr;
                            }

                            newElement_1_trackAttributes =
                                env->NewObject(trackAttributesStructStructClass_4, trackAttributesStructStructCtor_4,
                                               newElement_1_trackAttributes_languageCode, newElement_1_trackAttributes_displayName);
                        }
                    }

                    {
                        jclass trackStructStructClass_2;
                        err = chip::JniReferences::GetInstance().GetLocalClassRef(
                            env, "chip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackStruct", trackStructStructClass_2);
                        if (err != CHIP_NO_ERROR)
                        {
                            ChipLogError(Zcl, "Could not find class ChipStructs$MediaPlaybackClusterTrackStruct");
                            return nullptr;
                        }

                        jmethodID trackStructStructCtor_2;
                        err = chip::JniReferences::GetInstance().FindMethod(
                            env, trackStructStructClass_2, "<init>",
                            "(Ljava/lang/String;Lchip/devicecontroller/ChipStructs$MediaPlaybackClusterTrackAttributesStruct;)V",
                            &trackStructStructCtor_2);
                        if (err != CHIP_NO_ERROR || trackStructStructCtor_2 == nullptr)
                        {
                            ChipLogError(Zcl, "Could not find ChipStructs$MediaPlaybackClusterTrackStruct constructor");
                            return nullptr;
                        }

                        newElement_1 = env->NewObject(trackStructStructClass_2, trackStructStructCtor_2, newElement_1_id,
                                                      newElement_1_trackAttributes);
                    }
                    chip::JniReferences::GetInstance().AddToList(value, newElement_1);
                }
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- Messages (0x0097) ---
    case app::Clusters::Messages::Id: {
        using namespace app::Clusters::Messages;
        switch (aPath.mAttributeId)
        {
        case Attributes::Messages::Id: {
            using TypeInfo = Attributes::Messages::TypeInfo;
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
                jobject newElement_0_messageID;
                jbyteArray newElement_0_messageIDByteArray = env->NewByteArray(static_cast<jsize>(entry_0.messageID.size()));
                env->SetByteArrayRegion(newElement_0_messageIDByteArray, 0, static_cast<jsize>(entry_0.messageID.size()),
                                        reinterpret_cast<const jbyte *>(entry_0.messageID.data()));
                newElement_0_messageID = newElement_0_messageIDByteArray;
                jobject newElement_0_priority;
                std::string newElement_0_priorityClassName     = "java/lang/Integer";
                std::string newElement_0_priorityCtorSignature = "(I)V";
                jint jninewElement_0_priority                  = static_cast<jint>(entry_0.priority);
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(newElement_0_priorityClassName.c_str(),
                                                                           newElement_0_priorityCtorSignature.c_str(),
                                                                           jninewElement_0_priority, newElement_0_priority);
                jobject newElement_0_messageControl;
                std::string newElement_0_messageControlClassName     = "java/lang/Integer";
                std::string newElement_0_messageControlCtorSignature = "(I)V";
                jint jninewElement_0_messageControl                  = static_cast<jint>(entry_0.messageControl.Raw());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
                    newElement_0_messageControlClassName.c_str(), newElement_0_messageControlCtorSignature.c_str(),
                    jninewElement_0_messageControl, newElement_0_messageControl);
                jobject newElement_0_startTime;
                if (entry_0.startTime.IsNull())
                {
                    newElement_0_startTime = nullptr;
                }
                else
                {
                    std::string newElement_0_startTimeClassName     = "java/lang/Long";
                    std::string newElement_0_startTimeCtorSignature = "(J)V";
                    jlong jninewElement_0_startTime                 = static_cast<jlong>(entry_0.startTime.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(newElement_0_startTimeClassName.c_str(),
                                                                                newElement_0_startTimeCtorSignature.c_str(),
                                                                                jninewElement_0_startTime, newElement_0_startTime);
                }
                jobject newElement_0_duration;
                if (entry_0.duration.IsNull())
                {
                    newElement_0_duration = nullptr;
                }
                else
                {
                    std::string newElement_0_durationClassName     = "java/lang/Long";
                    std::string newElement_0_durationCtorSignature = "(J)V";
                    jlong jninewElement_0_duration                 = static_cast<jlong>(entry_0.duration.Value());
                    chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(newElement_0_durationClassName.c_str(),
                                                                                newElement_0_durationCtorSignature.c_str(),
                                                                                jninewElement_0_duration, newElement_0_duration);
                }
                jobject newElement_0_messageText;
                LogErrorOnFailure(
                    chip::JniReferences::GetInstance().CharToStringUTF(entry_0.messageText, newElement_0_messageText));
                jobject newElement_0_responses;
                if (!entry_0.responses.HasValue())
                {
                    chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_0_responses);
                }
                else
                {
                    jobject newElement_0_responsesInsideOptional;
                    chip::JniReferences::GetInstance().CreateArrayList(newElement_0_responsesInsideOptional);

                    auto iter_newElement_0_responsesInsideOptional_3 = entry_0.responses.Value().begin();
                    while (iter_newElement_0_responsesInsideOptional_3.Next())
                    {
                        auto & entry_3 = iter_newElement_0_responsesInsideOptional_3.GetValue();
                        jobject newElement_3;
                        jobject newElement_3_messageResponseID;
                        if (!entry_3.messageResponseID.HasValue())
                        {
                            chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_3_messageResponseID);
                        }
                        else
                        {
                            jobject newElement_3_messageResponseIDInsideOptional;
                            std::string newElement_3_messageResponseIDInsideOptionalClassName     = "java/lang/Long";
                            std::string newElement_3_messageResponseIDInsideOptionalCtorSignature = "(J)V";
                            jlong jninewElement_3_messageResponseIDInsideOptional =
                                static_cast<jlong>(entry_3.messageResponseID.Value());
                            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                                newElement_3_messageResponseIDInsideOptionalClassName.c_str(),
                                newElement_3_messageResponseIDInsideOptionalCtorSignature.c_str(),
                                jninewElement_3_messageResponseIDInsideOptional, newElement_3_messageResponseIDInsideOptional);
                            chip::JniReferences::GetInstance().CreateOptional(newElement_3_messageResponseIDInsideOptional,
                                                                              newElement_3_messageResponseID);
                        }
                        jobject newElement_3_label;
                        if (!entry_3.label.HasValue())
                        {
                            chip::JniReferences::GetInstance().CreateOptional(nullptr, newElement_3_label);
                        }
                        else
                        {
                            jobject newElement_3_labelInsideOptional;
                            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(entry_3.label.Value(),
                                                                                                 newElement_3_labelInsideOptional));
                            chip::JniReferences::GetInstance().CreateOptional(newElement_3_labelInsideOptional, newElement_3_label);
                        }

                        {
                            jclass messageResponseOptionStructStructClass_4;
                            err = chip::JniReferences::GetInstance().GetLocalClassRef(
                                env, "chip/devicecontroller/ChipStructs$MessagesClusterMessageResponseOptionStruct",
                                messageResponseOptionStructStructClass_4);
                            if (err != CHIP_NO_ERROR)
                            {
                                ChipLogError(Zcl, "Could not find class ChipStructs$MessagesClusterMessageResponseOptionStruct");
                                return nullptr;
                            }

                            jmethodID messageResponseOptionStructStructCtor_4;
                            err = chip::JniReferences::GetInstance().FindMethod(
                                env, messageResponseOptionStructStructClass_4, "<init>",
                                "(Ljava/util/Optional;Ljava/util/Optional;)V", &messageResponseOptionStructStructCtor_4);
                            if (err != CHIP_NO_ERROR || messageResponseOptionStructStructCtor_4 == nullptr)
                            {
                                ChipLogError(Zcl,
                                             "Could not find ChipStructs$MessagesClusterMessageResponseOptionStruct constructor");
                                return nullptr;
                            }

                            newElement_3 =
                                env->NewObject(messageResponseOptionStructStructClass_4, messageResponseOptionStructStructCtor_4,
                                               newElement_3_messageResponseID, newElement_3_label);
                        }
                        chip::JniReferences::GetInstance().AddToList(newElement_0_responsesInsideOptional, newElement_3);
                    }
                    chip::JniReferences::GetInstance().CreateOptional(newElement_0_responsesInsideOptional, newElement_0_responses);
                }

                {
                    jclass messageStructStructClass_1;
                    err = chip::JniReferences::GetInstance().GetLocalClassRef(
                        env, "chip/devicecontroller/ChipStructs$MessagesClusterMessageStruct", messageStructStructClass_1);
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(Zcl, "Could not find class ChipStructs$MessagesClusterMessageStruct");
                        return nullptr;
                    }

                    jmethodID messageStructStructCtor_1;
                    err = chip::JniReferences::GetInstance().FindMethod(
                        env, messageStructStructClass_1, "<init>",
                        "([BLjava/lang/Integer;Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Long;Ljava/lang/String;Ljava/util/"
                        "Optional;)V",
                        &messageStructStructCtor_1);
                    if (err != CHIP_NO_ERROR || messageStructStructCtor_1 == nullptr)
                    {
                        ChipLogError(Zcl, "Could not find ChipStructs$MessagesClusterMessageStruct constructor");
                        return nullptr;
                    }

                    newElement_0 = env->NewObject(messageStructStructClass_1, messageStructStructCtor_1, newElement_0_messageID,
                                                  newElement_0_priority, newElement_0_messageControl, newElement_0_startTime,
                                                  newElement_0_duration, newElement_0_messageText, newElement_0_responses);
                }
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::ActiveMessageIDs::Id: {
            using TypeInfo = Attributes::ActiveMessageIDs::TypeInfo;
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
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- OnOff (0x0006) ---
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
            jboolean jnivalue              = static_cast<jboolean>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
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
            jboolean jnivalue              = static_cast<jboolean>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
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
            if (cppValue.IsNull())
            {
                value = nullptr;
            }
            else
            {
                std::string valueClassName     = "java/lang/Integer";
                std::string valueCtorSignature = "(I)V";
                jint jnivalue                  = static_cast<jint>(cppValue.Value());
                chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                           jnivalue, value);
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- TargetNavigator (0x0505) ---
    case app::Clusters::TargetNavigator::Id: {
        using namespace app::Clusters::TargetNavigator;
        switch (aPath.mAttributeId)
        {
        case Attributes::TargetList::Id: {
            using TypeInfo = Attributes::TargetList::TypeInfo;
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
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::CurrentTarget::Id: {
            using TypeInfo = Attributes::CurrentTarget::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            std::string valueClassName     = "java/lang/Integer";
            std::string valueCtorSignature = "(I)V";
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    // --- WakeOnLAN (0x0503) ---
    case app::Clusters::WakeOnLan::Id: {
        using namespace app::Clusters::WakeOnLan;
        switch (aPath.mAttributeId)
        {
        case Attributes::MACAddress::Id: {
            using TypeInfo = Attributes::MACAddress::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = app::DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR)
            {
                return nullptr;
            }
            jobject value;
            LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(cppValue, value));
            return value;
        }
        case Attributes::LinkLocalAddress::Id: {
            using TypeInfo = Attributes::LinkLocalAddress::TypeInfo;
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
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
                jlong jninewElement_0                 = static_cast<jlong>(entry_0);
                chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(
                    newElement_0ClassName.c_str(), newElement_0CtorSignature.c_str(), jninewElement_0, newElement_0);
                chip::JniReferences::GetInstance().AddToList(value, newElement_0);
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
            jlong jnivalue                 = static_cast<jlong>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jlong>(valueClassName.c_str(), valueCtorSignature.c_str(),
                                                                        jnivalue, value);
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
            jint jnivalue                  = static_cast<jint>(cppValue);
            chip::JniReferences::GetInstance().CreateBoxedObject<jint>(valueClassName.c_str(), valueCtorSignature.c_str(), jnivalue,
                                                                       value);
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }

    default:
        *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
        break;
    }
    return nullptr;
}

} // namespace chip
