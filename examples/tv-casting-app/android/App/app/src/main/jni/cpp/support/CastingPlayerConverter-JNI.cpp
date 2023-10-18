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
                         "(ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/util/List;IIII)V");
    if (constructor == nullptr)
    {
        ChipLogError(
            AppServer,
            "CastingPlayerConverter-JNI.createJCastingPlayer() could not locate MatterCastingPlayer Java class constructor");
        return nullptr;
    }

    // Convert the CastingPlayer fields to MatterCastingPlayer Java types
    bool connected = false;
    if (player->IsConnected())
    {
        connected = true;
    }
    jboolean connectedJavaBoolean  = static_cast<jboolean>(connected);
    jstring idJavaString           = env->NewStringUTF(player->GetId());
    jstring deviceNameJavaString   = env->NewStringUTF(player->GetDeviceName());
    jstring hostNameJavaString     = env->NewStringUTF(player->GetHostName());
    jstring instanceNameJavaString = env->NewStringUTF(player->GetInstanceName());
    jint numIPsJavaInt             = (jint) (player->GetNumIPs());

    jobject ipAddressListJavaObject           = nullptr;
    const chip::Inet::IPAddress * ipAddresses = player->GetIPAddresses();
    if (ipAddresses != nullptr)
    {
        chip::JniReferences::GetInstance().CreateArrayList(ipAddressListJavaObject);
        for (size_t i = 0; i < player->GetNumIPs() && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
        {
            char addrCString[chip::Inet::IPAddress::kMaxStringLength];
            ipAddresses[i].ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
            jstring jIPAddressStr = env->NewStringUTF(addrCString);

            jclass jIPAddressClass = env->FindClass("java/net/InetAddress");
            jmethodID jGetByNameMid =
                env->GetStaticMethodID(jIPAddressClass, "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;");
            jobject jIPAddress = env->CallStaticObjectMethod(jIPAddressClass, jGetByNameMid, jIPAddressStr);

            chip::JniReferences::GetInstance().AddToList(ipAddressListJavaObject, jIPAddress);
        }
    }

    jint portJavaInt       = (jint) (player->GetPort());
    jint productIdJavaInt  = (jint) (player->GetProductId());
    jint vendorIdJavaInt   = (jint) (player->GetVendorId());
    jint deviceTypeJavaInt = (jint) (player->GetDeviceType());

    // Create a new instance of the MatterCastingPlayer Java class
    jobject matterCastingPlayerJavaObject = nullptr;
    matterCastingPlayerJavaObject =
        env->NewObject(matterCastingPlayerJavaClass, constructor, connectedJavaBoolean, idJavaString, hostNameJavaString,
                       deviceNameJavaString, instanceNameJavaString, numIPsJavaInt, ipAddressListJavaObject, portJavaInt,
                       productIdJavaInt, vendorIdJavaInt, deviceTypeJavaInt);
    if (matterCastingPlayerJavaObject == nullptr)
    {
        ChipLogError(AppServer,
                     "CastingPlayerConverter-JNI.createJCastingPlayer() Warning: Could not create MatterCastingPlayer Java object");
    }
    return matterCastingPlayerJavaObject;
}

}; // namespace support
}; // namespace casting
}; // namespace matter
