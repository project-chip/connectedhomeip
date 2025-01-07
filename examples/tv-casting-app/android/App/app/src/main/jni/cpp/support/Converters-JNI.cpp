/*
 *   Copyright (c) 2020-24 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "Converters-JNI.h"
#include <lib/support/JniReferences.h>

namespace matter {
namespace casting {
namespace support {

using namespace chip;

extern "C" {

JNIEXPORT jshort JNICALL
Java_com_matter_casting_support_IdentificationDeclarationOptions_getChipDeviceConfigUdcMaxTargetApps(JNIEnv *, jclass clazz)
{
    ChipLogProgress(AppServer, "Converters-JNI::getChipDeviceConfigUdcMaxTargetApps(), CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS: %d",
                    CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS);
    return CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS;
}
}

jobject convertLongFromCppToJava(jlong value)
{
    ChipLogProgress(AppServer, "convertLongFromCppToJava called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass responseTypeClass = env->FindClass("java/lang/Long");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "ConvertToJObject: Class for Response Type not found!");
        env->ExceptionClear();
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(J)V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "Failed to access Long constructor");
        env->ExceptionClear();
        return nullptr;
    }
    return env->NewObject(responseTypeClass, constructor, value);
}

jobject convertMatterErrorFromCppToJava(CHIP_ERROR inErr)
{
    ChipLogProgress(AppServer, "convertMatterErrorFromCppToJava() called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass jMatterErrorClass;
    CHIP_ERROR err =
        chip::JniReferences::GetInstance().GetLocalClassRef(env, "com/matter/casting/support/MatterError", jMatterErrorClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr);

    jmethodID jMatterErrorConstructor = env->GetMethodID(jMatterErrorClass, "<init>", "(JLjava/lang/String;)V");
    if (jMatterErrorConstructor == nullptr)
    {
        ChipLogError(AppServer, "Failed to access MatterError constructor");
        env->ExceptionClear();
        return nullptr;
    }

    return env->NewObject(jMatterErrorClass, jMatterErrorConstructor, inErr.AsInteger(), nullptr);
}

jobject convertEndpointFromCppToJava(matter::casting::memory::Strong<core::Endpoint> endpoint)
{
    ChipLogProgress(AppServer, "convertEndpointFromCppToJava() called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    // Get a reference to the MatterEndpoint Java class
    jclass matterEndpointJavaClass = env->FindClass("com/matter/casting/core/MatterEndpoint");
    if (matterEndpointJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertEndpointFromCppToJava() could not locate MatterEndpoint Java class");
        env->ExceptionClear();
        return nullptr;
    }

    // Get the constructor for the com/matter/casting/core/MatterEndpoint Java class
    jmethodID constructor = env->GetMethodID(matterEndpointJavaClass, "<init>", "()V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertEndpointFromCppToJava() could not locate MatterEndpoint Java class constructor");
        env->ExceptionClear();
        return nullptr;
    }

    // Create a new instance of the MatterEndpoint Java class
    jobject jMatterEndpoint = nullptr;
    jMatterEndpoint         = env->NewObject(matterEndpointJavaClass, constructor);
    if (jMatterEndpoint == nullptr)
    {
        ChipLogError(AppServer, "convertEndpointFromCppToJava(): Could not create MatterEndpoint Java object");
        return jMatterEndpoint;
    }
    // Set the value of the _cppEndpoint field in the Java object to the C++ Endpoint pointer.
    jfieldID longFieldId = env->GetFieldID(matterEndpointJavaClass, "_cppEndpoint", "J");
    env->SetLongField(jMatterEndpoint, longFieldId, reinterpret_cast<jlong>(endpoint.get()));
    return jMatterEndpoint;
}

/**
 * @brief Get the matter::casting::core::Endpoint object from the jobject jEndpointObject
 */
core::Endpoint * convertEndpointFromJavaToCpp(jobject jEndpointObject)
{
    ChipLogProgress(AppServer, "convertEndpointFromJavaToCpp() called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass endpointClass = env->GetObjectClass(jEndpointObject);
    if (endpointClass == nullptr)
    {
        ChipLogError(AppServer, "convertEndpointFromJavaToCpp() could not locate Endpoint Java class");
        env->ExceptionClear();
        return nullptr;
    }

    jfieldID _cppEndpointFieldId = env->GetFieldID(endpointClass, "_cppEndpoint", "J");
    VerifyOrReturnValue(_cppEndpointFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertEndpointFromJavaToCpp _cppEndpointFieldId == nullptr"));
    jlong _cppEndpointValue = env->GetLongField(jEndpointObject, _cppEndpointFieldId);
    return reinterpret_cast<core::Endpoint *>(_cppEndpointValue);
}

jobject convertCastingPlayerFromCppToJava(matter::casting::memory::Strong<core::CastingPlayer> player)
{
    ChipLogProgress(AppServer, "convertCastingPlayerFromCppToJava() called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    // Get a reference to the MatterCastingPlayer Java class
    jclass matterCastingPlayerJavaClass = env->FindClass("com/matter/casting/core/MatterCastingPlayer");
    if (matterCastingPlayerJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromCppToJava() could not locate MatterCastingPlayer Java class");
        env->ExceptionClear();
        return nullptr;
    }

    // Get the constructor for the com/matter/casting/core/MatterCastingPlayer Java class
    jmethodID constructor =
        env->GetMethodID(matterCastingPlayerJavaClass, "<init>",
                         "(ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/List;IIIJZ)V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromCppToJava() could not locate MatterCastingPlayer Java class constructor");
        env->ExceptionClear();
        return nullptr;
    }

    // Convert the CastingPlayer fields to MatterCastingPlayer Java types
    jobject jIpAddressList                    = nullptr;
    const chip::Inet::IPAddress * ipAddresses = player->GetIPAddresses();
    if (ipAddresses != nullptr)
    {
        chip::JniReferences::GetInstance().CreateArrayList(jIpAddressList);
        for (size_t i = 0; i < player->GetNumIPs() && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
        {
            char addrCString[chip::Inet::IPAddress::kMaxStringLength];
            ipAddresses[i].ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
            jstring jIPAddressStr = env->NewStringUTF(addrCString);

            jclass jIPAddressClass = env->FindClass("java/net/InetAddress");
            jmethodID jGetByNameMid =
                env->GetStaticMethodID(jIPAddressClass, "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;");
            jobject jIPAddress = env->CallStaticObjectMethod(jIPAddressClass, jGetByNameMid, jIPAddressStr);

            chip::JniReferences::GetInstance().AddToList(jIpAddressList, jIPAddress);
        }
    }

    // Create a new instance of the MatterCastingPlayer Java class
    jobject jMatterCastingPlayer = nullptr;
    jMatterCastingPlayer = env->NewObject(matterCastingPlayerJavaClass, constructor, static_cast<jboolean>(player->IsConnected()),
                                          env->NewStringUTF(player->GetId()), env->NewStringUTF(player->GetHostName()),
                                          env->NewStringUTF(player->GetDeviceName()), env->NewStringUTF(player->GetInstanceName()),
                                          jIpAddressList, (jint) (player->GetPort()), (jint) (player->GetProductId()),
                                          (jint) (player->GetVendorId()), (jlong) (player->GetDeviceType()),
                                          static_cast<jboolean>(player->GetSupportsCommissionerGeneratedPasscode()));
    if (jMatterCastingPlayer == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromCppToJava(): Could not create MatterCastingPlayer Java object");
        env->ExceptionClear();
        return jMatterCastingPlayer;
    }
    // Set the value of the _cppCastingPlayer field in the Java object to the C++ CastingPlayer pointer.
    jfieldID longFieldId = env->GetFieldID(matterCastingPlayerJavaClass, "_cppCastingPlayer", "J");
    env->SetLongField(jMatterCastingPlayer, longFieldId, reinterpret_cast<jlong>(player.get()));
    return jMatterCastingPlayer;
}

core::CastingPlayer * convertCastingPlayerFromJavaToCpp(jobject jCastingPlayerObject)
{
    ChipLogProgress(AppServer, "convertCastingPlayerFromJavaToCpp() called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass castingPlayerClass = env->GetObjectClass(jCastingPlayerObject);
    if (castingPlayerClass == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromJavaToCpp() could not locate CastingPlayer Java class");
        env->ExceptionClear();
        return nullptr;
    }

    jfieldID _cppCastingPlayerFieldId = env->GetFieldID(castingPlayerClass, "_cppCastingPlayer", "J");
    VerifyOrReturnValue(_cppCastingPlayerFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertCastingPlayerFromJavaToCpp _cppCastingPlayerFieldId == nullptr"));
    jlong _cppCastingPlayerValue = env->GetLongField(jCastingPlayerObject, _cppCastingPlayerFieldId);
    return reinterpret_cast<core::CastingPlayer *>(_cppCastingPlayerValue);
}

jobject convertClusterFromCppToJava(matter::casting::memory::Strong<core::BaseCluster> cluster, const char * className)
{
    ChipLogProgress(AppServer, "convertClusterFromCppToJava() called");
    VerifyOrReturnValue(cluster.get() != nullptr, nullptr,
                        ChipLogError(AppServer, "convertClusterFromCppToJava() cluster.get() == nullptr"));
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    // Get a reference to the cluster's Java class
    jclass clusterJavaClass = env->FindClass(className);
    if (clusterJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertClusterFromCppToJava could not locate cluster's Java class");
        env->ExceptionClear();
        return nullptr;
    }

    // Get the constructor for the cluster's Java class
    jmethodID constructor = env->GetMethodID(clusterJavaClass, "<init>", "()V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertClusterFromCppToJava could not locate cluster's Java class constructor");
        env->ExceptionClear();
        return nullptr;
    }

    // Create a new instance of the cluster's Java class
    jobject jMatterCluster = nullptr;
    jMatterCluster         = env->NewObject(clusterJavaClass, constructor);
    if (jMatterCluster == nullptr)
    {
        ChipLogError(AppServer, "convertClusterFromCppToJava: Could not create cluster's Java object");
        return jMatterCluster;
    }
    // Set the value of the _cppEndpoint field in the Java object to the C++ Endpoint pointer.
    jfieldID longFieldId = env->GetFieldID(clusterJavaClass, "_cppCluster", "J");
    env->SetLongField(jMatterCluster, longFieldId, reinterpret_cast<jlong>(cluster.get()));
    return jMatterCluster;
}

/**
 * @brief Get the matter::casting::core::Cluster object from the jobject jClusterObject
 */
core::BaseCluster * convertClusterFromJavaToCpp(jobject jClusterObject)
{
    ChipLogProgress(AppServer, "convertClusterFromJavaToCpp() called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass clusterClass = env->GetObjectClass(jClusterObject);
    if (clusterClass == nullptr)
    {
        ChipLogError(AppServer, "convertClusterFromJavaToCpp could not locate cluster's Java class");
        env->ExceptionClear();
        return nullptr;
    }

    jfieldID _cppClusterFieldId = env->GetFieldID(clusterClass, "_cppCluster", "J");
    VerifyOrReturnValue(_cppClusterFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertClusterFromJavaToCpp() _cppCluster == nullptr"));
    jlong _cppClusterValue = env->GetLongField(jClusterObject, _cppClusterFieldId);
    return reinterpret_cast<core::BaseCluster *>(_cppClusterValue);
}

jobject convertCommandFromCppToJava(void * command, const char * className)
{
    ChipLogProgress(AppServer, "convertCommandFromCppToJava() called");
    VerifyOrReturnValue(command != nullptr, nullptr, ChipLogError(AppServer, "convertCommandFromCppToJava() command == nullptr"));
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    // Get a reference to the command's Java class
    jclass commandJavaClass = env->FindClass(className);
    if (commandJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromCppToJava() could not locate command's Java class");
        env->ExceptionClear();
        return nullptr;
    }

    // Get the constructor for the command's Java class
    jmethodID constructor = env->GetMethodID(commandJavaClass, "<init>", "()V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromCppToJava() could not locate command's Java class constructor");
        env->ExceptionClear();
        return nullptr;
    }

    // Create a new instance of the command's Java class
    jobject jMatterCommand = env->NewObject(commandJavaClass, constructor);
    if (jMatterCommand == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromCppToJava(): Could not create command's Java object");
        env->ExceptionClear();
        return jMatterCommand;
    }
    // Set the value of the _cppEndpoint field in the Java object to the C++ Endpoint pointer.
    jfieldID longFieldId = env->GetFieldID(commandJavaClass, "_cppCommand", "J");
    env->SetLongField(jMatterCommand, longFieldId, reinterpret_cast<jlong>(command));
    return jMatterCommand;
}

/**
 * @brief Get the matter::casting::core::Command object from the jobject jCommandObject
 */
void * convertCommandFromJavaToCpp(jobject jCommandObject)
{
    ChipLogProgress(AppServer, "convertCommandFromJavaToCpp() called");
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass commandClass = env->GetObjectClass(jCommandObject);
    if (commandClass == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromJavaToCpp() could not locate command's Java class");
        env->ExceptionClear();
        return nullptr;
    }

    jfieldID _cppCommandFieldId = env->GetFieldID(commandClass, "_cppCommand", "J");
    VerifyOrReturnValue(_cppCommandFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertCommandFromJavaToCpp() _cppCommand == nullptr"));
    jlong _cppCommandValue = env->GetLongField(jCommandObject, _cppCommandFieldId);
    return reinterpret_cast<void *>(_cppCommandValue);
}

jobject convertLongFromCppToJava(uint64_t responseData)
{
    ChipLogProgress(AppServer, "convertLongFromCppToJava() called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr, ChipLogError(AppServer, "Could not get JNIEnv for current thread"));

    jclass responseTypeClass = env->FindClass("java/lang/Long");
    if (responseTypeClass == nullptr)
    {
        ChipLogError(AppServer, "convertLongFromCppToJava: Class for Response Type not found!");
        env->ExceptionClear();
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseTypeClass, "<init>", "(J)V");
    return env->NewObject(responseTypeClass, constructor, responseData);
}

chip::Protocols::UserDirectedCommissioning::TargetAppInfo * convertTargetAppInfoFromJavaToCpp(jobject jTargetAppInfo)
{
    ChipLogProgress(AppServer, "convertTargetAppInfoFromJavaToCpp() called");

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, nullptr,
                        ChipLogError(AppServer, "convertTargetAppInfoFromJavaToCpp() Could not get JNIEnv for current thread"));
    jclass targetAppInfoClass = env->GetObjectClass(jTargetAppInfo);
    VerifyOrReturnValue(targetAppInfoClass != nullptr, nullptr,
                        ChipLogError(AppServer, "convertTargetAppInfoFromJavaToCpp() TargetAppInfo class not found!"));

    jfieldID vendorIdField  = env->GetFieldID(targetAppInfoClass, "vendorId", "Ljava/lang/Integer;");
    jfieldID productIdField = env->GetFieldID(targetAppInfoClass, "productId", "Ljava/lang/Integer;");
    VerifyOrReturnValue(vendorIdField != nullptr, nullptr,
                        ChipLogError(AppServer, "convertTargetAppInfoFromJavaToCpp() vendorIdField not found!"));
    VerifyOrReturnValue(productIdField != nullptr, nullptr,
                        ChipLogError(AppServer, "convertTargetAppInfoFromJavaToCpp() productIdField not found!"));

    jobject jVendorIdObject  = env->GetObjectField(jTargetAppInfo, vendorIdField);
    jobject jProductIdObject = env->GetObjectField(jTargetAppInfo, productIdField);

    jclass integerClass      = env->FindClass("java/lang/Integer");
    jmethodID intValueMethod = env->GetMethodID(integerClass, "intValue", "()I");

    jint vendorId  = 0;
    jint productId = 0;
    if (jVendorIdObject != nullptr)
    {
        vendorId = env->CallIntMethod(jVendorIdObject, intValueMethod);
    }
    if (jProductIdObject != nullptr)
    {
        productId = env->CallIntMethod(jProductIdObject, intValueMethod);
    }

    chip::Protocols::UserDirectedCommissioning::TargetAppInfo * cppTargetAppInfo =
        new chip::Protocols::UserDirectedCommissioning::TargetAppInfo();

    cppTargetAppInfo->vendorId  = static_cast<uint16_t>(vendorId);
    cppTargetAppInfo->productId = static_cast<uint16_t>(productId);

    env->DeleteLocalRef(targetAppInfoClass);
    env->DeleteLocalRef(jVendorIdObject);
    env->DeleteLocalRef(jProductIdObject);
    env->DeleteLocalRef(integerClass);

    return reinterpret_cast<chip::Protocols::UserDirectedCommissioning::TargetAppInfo *>(cppTargetAppInfo);
}

matter::casting::core::IdentificationDeclarationOptions * convertIdentificationDeclarationOptionsFromJavaToCpp(jobject jIdOptions)
{
    ChipLogProgress(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() called");

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(
        env != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() Could not get JNIEnv for current thread"));

    jclass idOptionsClass = env->GetObjectClass(jIdOptions);
    VerifyOrReturnValue(
        idOptionsClass != nullptr, nullptr,
        ChipLogError(AppServer,
                     "convertIdentificationDeclarationOptionsFromJavaToCpp() IdentificationDeclarationOptions class not found!"));

    jfieldID noPasscodeField                = env->GetFieldID(idOptionsClass, "noPasscode", "Z");
    jfieldID cdUponPasscodeDialogField      = env->GetFieldID(idOptionsClass, "cdUponPasscodeDialog", "Z");
    jfieldID commissionerPasscodeField      = env->GetFieldID(idOptionsClass, "commissionerPasscode", "Z");
    jfieldID commissionerPasscodeReadyField = env->GetFieldID(idOptionsClass, "commissionerPasscodeReady", "Z");
    jfieldID cancelPasscodeField            = env->GetFieldID(idOptionsClass, "cancelPasscode", "Z");
    jfieldID targetAppInfosField            = env->GetFieldID(idOptionsClass, "targetAppInfos", "Ljava/util/List;");
    VerifyOrReturnValue(
        noPasscodeField != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() noPasscodeField not found!"));
    VerifyOrReturnValue(
        cdUponPasscodeDialogField != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() cdUponPasscodeDialogField not found!"));
    VerifyOrReturnValue(
        commissionerPasscodeField != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() commissionerPasscodeField not found!"));
    VerifyOrReturnValue(
        commissionerPasscodeReadyField != nullptr, nullptr,
        ChipLogError(AppServer,
                     "convertIdentificationDeclarationOptionsFromJavaToCpp() commissionerPasscodeReadyField not found!"));
    VerifyOrReturnValue(
        cancelPasscodeField != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() cancelPasscodeField not found!"));
    VerifyOrReturnValue(
        targetAppInfosField != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() targetAppInfosField not found!"));

    matter::casting::core::IdentificationDeclarationOptions * cppIdOptions =
        new matter::casting::core::IdentificationDeclarationOptions();

    cppIdOptions->mNoPasscode                = env->GetBooleanField(jIdOptions, noPasscodeField);
    cppIdOptions->mCdUponPasscodeDialog      = env->GetBooleanField(jIdOptions, cdUponPasscodeDialogField);
    cppIdOptions->mCommissionerPasscode      = env->GetBooleanField(jIdOptions, commissionerPasscodeField);
    cppIdOptions->mCommissionerPasscodeReady = env->GetBooleanField(jIdOptions, commissionerPasscodeReadyField);
    cppIdOptions->mCancelPasscode            = env->GetBooleanField(jIdOptions, cancelPasscodeField);

    jobject targetAppInfosList = env->GetObjectField(jIdOptions, targetAppInfosField);
    VerifyOrReturnValue(
        targetAppInfosList != nullptr, nullptr,
        ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() targetAppInfosList not found!"));
    jclass listClass     = env->FindClass("java/util/List");
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod  = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");

    jint size = env->CallIntMethod(targetAppInfosList, sizeMethod);

    for (jint i = 0; i < size; i++)
    {
        jobject jTargetAppInfo = env->CallObjectMethod(targetAppInfosList, getMethod, i);

        chip::Protocols::UserDirectedCommissioning::TargetAppInfo * cppTargetAppInfo =
            convertTargetAppInfoFromJavaToCpp(jTargetAppInfo);
        VerifyOrReturnValue(
            cppTargetAppInfo != nullptr, nullptr,
            ChipLogError(AppServer, "convertIdentificationDeclarationOptionsFromJavaToCpp() Could not convert jTargetAppInfo"));

        CHIP_ERROR err = cppIdOptions->addTargetAppInfo(*cppTargetAppInfo);

        env->DeleteLocalRef(jTargetAppInfo);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(AppServer,
                                         "convertIdentificationDeclarationOptionsFromJavaToCpp() failed to addTargetAppInfo, due "
                                         "to err: %" CHIP_ERROR_FORMAT,
                                         err.Format()));
    }

    env->DeleteLocalRef(targetAppInfosList);
    env->DeleteLocalRef(listClass);
    env->DeleteLocalRef(idOptionsClass);

    return reinterpret_cast<matter::casting::core::IdentificationDeclarationOptions *>(cppIdOptions);
}

jobject
convertCommissionerDeclarationFromCppToJava(const chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration & cppCd)
{
    ChipLogProgress(AppServer, "convertCommissionerDeclarationFromCppToJava() called");

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(
        env != nullptr, nullptr,
        ChipLogError(AppServer, "convertCommissionerDeclarationFromCppToJava() Could not get JNIEnv for current thread"));

    jclass jCommissionerDeclarationClass;
    CHIP_ERROR err = chip::JniReferences::GetInstance().GetLocalClassRef(env, "com/matter/casting/support/CommissionerDeclaration",
                                                                         jCommissionerDeclarationClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr);

    jmethodID jCommissionerDeclarationConstructor = env->GetMethodID(jCommissionerDeclarationClass, "<init>", "(IZZZZZZ)V");
    if (jCommissionerDeclarationConstructor == nullptr)
    {
        ChipLogError(AppServer,
                     "convertCommissionerDeclarationFromCppToJava() Failed to access Java CommissionerDeclaration constructor");
        env->ExceptionClear();
        return nullptr;
    }

    return env->NewObject(jCommissionerDeclarationClass, jCommissionerDeclarationConstructor,
                          static_cast<jint>(cppCd.GetErrorCode()), cppCd.GetNeedsPasscode(), cppCd.GetNoAppsFound(),
                          cppCd.GetPasscodeDialogDisplayed(), cppCd.GetCommissionerPasscode(), cppCd.GetQRCodeDisplayed(),
                          cppCd.GetCancelPasscode());
}

}; // namespace support
}; // namespace casting
}; // namespace matter
