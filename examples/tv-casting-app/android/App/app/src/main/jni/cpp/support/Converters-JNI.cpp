/*
 *   Copyright (c) 2023 Project CHIP Authors
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

jobject convertMatterErrorFromCppToJava(CHIP_ERROR inErr)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jclass jMatterErrorClass;
    CHIP_ERROR err =
        chip::JniReferences::GetInstance().GetClassRef(env, "com/matter/casting/support/MatterError", jMatterErrorClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr);

    jmethodID jMatterErrorConstructor = env->GetMethodID(jMatterErrorClass, "<init>", "(JLjava/lang/String;)V");

    return env->NewObject(jMatterErrorClass, jMatterErrorConstructor, inErr.AsInteger(), nullptr);
}

jobject convertEndpointFromCppToJava(matter::casting::memory::Strong<core::Endpoint> endpoint)
{
    ChipLogProgress(AppServer, "EndpointConverter-JNI.convertEndpointFromCppToJava() called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    // Get a reference to the MatterEndpoint Java class
    jclass matterEndpointJavaClass = env->FindClass("com/matter/casting/core/MatterEndpoint");
    if (matterEndpointJavaClass == nullptr)
    {
        ChipLogError(AppServer, "EndpointConverter-JNI.convertEndpointFromCppToJava() could not locate MatterEndpoint Java class");
        return nullptr;
    }

    // Get the constructor for the com/matter/casting/core/MatterEndpoint Java class
    jmethodID constructor = env->GetMethodID(matterEndpointJavaClass, "<init>", "()V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer,
                     "EndpointConverter-JNI.convertEndpointFromCppToJava() could not locate MatterEndpoint Java class constructor");
        return nullptr;
    }

    // Create a new instance of the MatterEndpoint Java class
    jobject jMatterEndpoint = nullptr;
    jMatterEndpoint         = env->NewObject(matterEndpointJavaClass, constructor);
    if (jMatterEndpoint == nullptr)
    {
        ChipLogError(AppServer,
                     "EndpointConverter-JNI.convertEndpointFromCppToJava(): Could not create MatterEndpoint Java object");
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
    JNIEnv * env                 = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jclass endpointClass         = env->GetObjectClass(jEndpointObject);
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

    // Get a reference to the MatterCastingPlayer Java class
    jclass matterCastingPlayerJavaClass = env->FindClass("com/matter/casting/core/MatterCastingPlayer");
    if (matterCastingPlayerJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromCppToJava() could not locate MatterCastingPlayer Java class");
        return nullptr;
    }

    // Get the constructor for the com/matter/casting/core/MatterCastingPlayer Java class
    jmethodID constructor =
        env->GetMethodID(matterCastingPlayerJavaClass, "<init>",
                         "(ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/List;IIIJ)V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromCppToJava() could not locate MatterCastingPlayer Java class constructor");
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
                                          (jint) (player->GetVendorId()), (jlong) (player->GetDeviceType()));
    if (jMatterCastingPlayer == nullptr)
    {
        ChipLogError(AppServer, "convertCastingPlayerFromCppToJava(): Could not create MatterCastingPlayer Java object");
        return jMatterCastingPlayer;
    }
    // Set the value of the _cppCastingPlayer field in the Java object to the C++ CastingPlayer pointer.
    jfieldID longFieldId = env->GetFieldID(matterCastingPlayerJavaClass, "_cppCastingPlayer", "J");
    env->SetLongField(jMatterCastingPlayer, longFieldId, reinterpret_cast<jlong>(player.get()));
    return jMatterCastingPlayer;
}

core::CastingPlayer * convertCastingPlayerFromJavaToCpp(jobject jCastingPlayerObject)
{
    JNIEnv * env                      = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jclass castingPlayerClass         = env->GetObjectClass(jCastingPlayerObject);
    jfieldID _cppCastingPlayerFieldId = env->GetFieldID(castingPlayerClass, "_cppCastingPlayer", "J");
    VerifyOrReturnValue(_cppCastingPlayerFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertCastingPlayerFromJavaToCpp _cppCastingPlayerFieldId == nullptr"));
    jlong _cppCastingPlayerValue = env->GetLongField(jCastingPlayerObject, _cppCastingPlayerFieldId);
    return reinterpret_cast<core::CastingPlayer *>(_cppCastingPlayerValue);
}

jobject convertClusterFromCppToJava(matter::casting::memory::Strong<core::BaseCluster> cluster, const char * className)
{
    ChipLogProgress(AppServer, "convertClusterFromCppToJava called");
    VerifyOrReturnValue(cluster.get() != nullptr, nullptr,
                        ChipLogError(AppServer, "ClusterConverter-JNI::convertClusterFromCppToJava() cluster.get() == nullptr"));
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    // Get a reference to the cluster's Java class
    jclass clusterJavaClass = env->FindClass(className);
    if (clusterJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertClusterFromCppToJava could not locate cluster's Java class");
        return nullptr;
    }

    // Get the constructor for the cluster's Java class
    jmethodID constructor = env->GetMethodID(clusterJavaClass, "<init>", "()V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertClusterFromCppToJava could not locate cluster's Java class constructor");
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
    JNIEnv * env                = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jclass clusterClass         = env->GetObjectClass(jClusterObject);
    jfieldID _cppClusterFieldId = env->GetFieldID(clusterClass, "_cppCluster", "J");
    VerifyOrReturnValue(_cppClusterFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertClusterFromJavaToCpp() _cppCluster == nullptr"));
    jlong _cppClusterValue = env->GetLongField(jClusterObject, _cppClusterFieldId);
    return reinterpret_cast<core::BaseCluster *>(_cppClusterValue);
}

jobject convertCommandFromCppToJava(void * command, const char * className)
{
    ChipLogProgress(AppServer, "convertCommandFromCppToJava() called");
    VerifyOrReturnValue(command != nullptr, nullptr,
                        ChipLogError(AppServer, "CommandConverter-JNI::convertCommandFromCppToJava() command == nullptr"));
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    // Get a reference to the command's Java class
    jclass commandJavaClass = env->FindClass(className);
    if (commandJavaClass == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromCppToJava() could not locate command's Java class");
        return nullptr;
    }

    // Get the constructor for the command's Java class
    jmethodID constructor = env->GetMethodID(commandJavaClass, "<init>", "()V");
    if (constructor == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromCppToJava() could not locate command's Java class constructor");
        return nullptr;
    }

    // Create a new instance of the command's Java class
    jobject jMatterCommand = env->NewObject(commandJavaClass, constructor);
    if (jMatterCommand == nullptr)
    {
        ChipLogError(AppServer, "convertCommandFromCppToJava(): Could not create command's Java object");
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
    JNIEnv * env                = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jclass commandClass         = env->GetObjectClass(jCommandObject);
    jfieldID _cppCommandFieldId = env->GetFieldID(commandClass, "_cppCommand", "J");
    VerifyOrReturnValue(_cppCommandFieldId != nullptr, nullptr,
                        ChipLogError(AppServer, "convertCommandFromJavaToCpp() _cppCommand == nullptr"));
    jlong _cppCommandValue = env->GetLongField(jCommandObject, _cppCommandFieldId);
    return reinterpret_cast<void *>(_cppCommandValue);
}

}; // namespace support
}; // namespace casting
}; // namespace matter
