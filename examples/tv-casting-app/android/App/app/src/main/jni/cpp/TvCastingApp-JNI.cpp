/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include "TvCastingApp-JNI.h"
#include "CastingServer.h"
#include "JNIDACProvider.h"

#include <app/server/Server.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/InetInterface.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_chip_casting_TvCastingApp_##METHOD_NAME

TvCastingAppJNI TvCastingAppJNI::sInstance;

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        JNIDACProvider * p = new JNIDACProvider(provider);
        chip::Credentials::SetDeviceAttestationCredentialsProvider(p);
    }
}

JNI_METHOD(jboolean, openBasicCommissioningWindow)(JNIEnv * env, jobject, jint duration, jobject jCommissioningCompleteHandler)
{
    chip::DeviceLayer::StackLock lock;

    ChipLogProgress(AppServer, "JNI_METHOD openBasicCommissioningWindow called with duration %d", duration);
    CHIP_ERROR err = TvCastingAppJNIMgr().getCommissioningCompleteHandler().SetUp(env, jCommissioningCompleteHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->OpenBasicCommissioningWindow(
        [](CHIP_ERROR err) { TvCastingAppJNIMgr().getCommissioningCompleteHandler().Handle(err); });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::OpenBasicCommissioningWindow failed: %" CHIP_ERROR_FORMAT, err.Format()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, sendUserDirectedCommissioningRequest)(JNIEnv * env, jobject, jstring addressJStr, jint port)
{
    ChipLogProgress(AppServer, "JNI_METHOD sendUserDirectedCommissioningRequest called with port %d", port);
    Inet::IPAddress addressInet;
    JniUtfString addressJniString(env, addressJStr);
    if (Inet::IPAddress::FromString(addressJniString.c_str(), addressInet) == false)
    {
        ChipLogError(AppServer, "Failed to parse IP address");
        return false;
    }

    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::FromIPAddress(addressInet);
    chip::Transport::PeerAddress peerAddress =
        chip::Transport::PeerAddress::UDP(addressInet, static_cast<uint16_t>(port), interfaceId);
    CHIP_ERROR err = CastingServer::GetInstance()->SendUserDirectedCommissioningRequest(peerAddress);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TVCastingApp-JNI::sendUserDirectedCommissioningRequest failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }
    return true;
}

JNI_METHOD(jboolean, discoverCommissioners)(JNIEnv *, jobject)
{
    ChipLogProgress(AppServer, "JNI_METHOD discoverCommissioners called");
    CHIP_ERROR err = CastingServer::GetInstance()->DiscoverCommissioners();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "TVCastingApp-JNI::discoverCommissioners failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(void, init)(JNIEnv *, jobject)
{
    ChipLogProgress(AppServer, "JNI_METHOD init called");
    CastingServer::GetInstance()->Init();
}

JNI_METHOD(jboolean, contentLauncherLaunchURL)
(JNIEnv * env, jobject, jstring contentUrl, jstring contentDisplayStr, jobject jLaunchURLResponseHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD contentLauncherLaunchURL called");
    const char * nativeContentUrl        = env->GetStringUTFChars(contentUrl, 0);
    const char * nativeContentDisplayStr = env->GetStringUTFChars(contentDisplayStr, 0);

    CHIP_ERROR err = TvCastingAppJNIMgr().getLaunchURLResponseHandler().SetUp(env, jLaunchURLResponseHandler);
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "MatterCallbackHandlerJNI::SetUp failed %" CHIP_ERROR_FORMAT, err.Format()));

    err = CastingServer::GetInstance()->ContentLauncherLaunchURL(nativeContentUrl, nativeContentDisplayStr, [](CHIP_ERROR err) {
        TvCastingAppJNIMgr().getLaunchURLResponseHandler().Handle(err);
    });
    VerifyOrExit(CHIP_NO_ERROR == err,
                 ChipLogError(AppServer, "CastingServer::ContentLauncherLaunchURL failed %" CHIP_ERROR_FORMAT, err.Format()));

    env->ReleaseStringUTFChars(contentUrl, nativeContentUrl);
    env->ReleaseStringUTFChars(contentDisplayStr, nativeContentDisplayStr);

exit:
    if (err != CHIP_NO_ERROR)
    {
        return false;
    }

    return true;
}
