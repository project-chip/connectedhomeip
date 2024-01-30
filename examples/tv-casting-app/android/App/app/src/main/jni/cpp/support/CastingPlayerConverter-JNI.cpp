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

#include "CastingPlayerConverter-JNI.h"
#include <lib/support/JniReferences.h>

namespace matter {
namespace casting {
namespace support {

using namespace chip;

jobject createJCastingPlayer(matter::casting::memory::Strong<core::CastingPlayer> player)
{
    ChipLogProgress(AppServer, "CastingPlayerConverter-JNI.createJCastingPlayer() called");
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    // Get a reference to the MatterCastingPlayer Java class
    jclass matterCastingPlayerJavaClass = env->FindClass("com/matter/casting/core/MatterCastingPlayer");
    if (matterCastingPlayerJavaClass == nullptr)
    {
        ChipLogError(AppServer,
                     "CastingPlayerConverter-JNI.createJCastingPlayer() could not locate MatterCastingPlayer Java class");
        return nullptr;
    }

    // Get the constructor for the com/matter/casting/core/MatterCastingPlayer Java class
    jmethodID constructor =
        env->GetMethodID(matterCastingPlayerJavaClass, "<init>",
                         "(ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/List;IIIJ)V");
    if (constructor == nullptr)
    {
        ChipLogError(
            AppServer,
            "CastingPlayerConverter-JNI.createJCastingPlayer() could not locate MatterCastingPlayer Java class constructor");
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
        ChipLogError(AppServer,
                     "CastingPlayerConverter-JNI.createJCastingPlayer() Warning: Could not create MatterCastingPlayer Java object");
        return jMatterCastingPlayer;
    }
    // Set the value of the _cppCastingPlayer field in the Java object to the C++ CastingPlayer pointer.
    jfieldID longFieldId = env->GetFieldID(matterCastingPlayerJavaClass, "_cppCastingPlayer", "J");
    env->SetLongField(jMatterCastingPlayer, longFieldId, reinterpret_cast<jlong>(player.get()));
    return jMatterCastingPlayer;
}

}; // namespace support
}; // namespace casting
}; // namespace matter
