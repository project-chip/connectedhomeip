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
#include "CallbackHelper.h"
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

void TvCastingAppJNI::InitializeWithObjects(jobject app)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "Failed to GetEnvForCurrentThread for TvCastingAppJNI"));

    mTvCastingAppObject = env->NewGlobalRef(app);
    VerifyOrReturn(mTvCastingAppObject != nullptr, ChipLogError(AppServer, "Failed to NewGlobalRef TvCastingAppJNI"));

    jclass managerClass = env->GetObjectClass(mTvCastingAppObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(AppServer, "Failed to get TvCastingAppJNI Java class"));

    mPostClusterInitMethod = env->GetMethodID(managerClass, "postClusterInit", "(II)V");
    if (mPostClusterInitMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access ChannelManager 'postClusterInit' method");
        env->ExceptionClear();
    }
}

void TvCastingAppJNI::PostClusterInit(chip::ClusterId clusterId, chip::EndpointId endpoint)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr,
                   ChipLogError(AppServer, "Failed to GetEnvForCurrentThread for TvCastingAppJNI::PostClusterInit"));
    VerifyOrReturn(mTvCastingAppObject != nullptr, ChipLogError(AppServer, "TvCastingAppJNI::mTvCastingAppObject null"));
    VerifyOrReturn(mPostClusterInitMethod != nullptr, ChipLogError(AppServer, "TvCastingAppJNI::mPostClusterInitMethod null"));

    env->CallVoidMethod(mTvCastingAppObject, mPostClusterInitMethod, static_cast<jint>(clusterId), static_cast<jint>(endpoint));
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Failed to call TvCastingAppJNI 'postClusterInit' method");
        env->ExceptionClear();
    }
}

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(void, nativeInit)(JNIEnv *, jobject app)
{
    TvCastingAppJNIMgr().InitializeWithObjects(app);
}

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        JNIDACProvider * p = new JNIDACProvider(provider);
        chip::Credentials::SetDeviceAttestationCredentialsProvider(p);
    }
}

JNI_METHOD(jboolean, openBasicCommissioningWindow)(JNIEnv *, jobject, jint duration)
{
    ChipLogProgress(AppServer, "JNI_METHOD openBasicCommissioningWindow called with duration %d", duration);

    CHIP_ERROR err = CastingServer::GetInstance()->OpenBasicCommissioningWindow();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingServer::OpenBasicCommissioningWindow failed: %" CHIP_ERROR_FORMAT, err.Format());
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
        ChipLogError(AppServer, "CastingServer::SendUserDirectedCommissioningRequest failed: %" CHIP_ERROR_FORMAT, err.Format());
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
        ChipLogError(AppServer, "CastingServer::DiscoverCommissioners failed: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }

    return true;
}

JNI_METHOD(jboolean, initServer)(JNIEnv * env, jobject, jobject jCommissioningCompleteHandler)
{
    ChipLogProgress(AppServer, "JNI_METHOD initServer called");
    CHIP_ERROR err = SetUpMatterCallbackHandler(env, jCommissioningCompleteHandler, gCommissioningCompleteHandler);
    if (err == CHIP_NO_ERROR)
    {
        CastingServer::GetInstance()->InitServer(CommissioningCompleteHandler);
        return true;
    }
    else
    {
        ChipLogError(AppServer, "initServer error: %s", err.AsString());
        return false;
    }
}

JNI_METHOD(void, contentLauncherLaunchURL)(JNIEnv * env, jobject, jstring contentUrl, jstring contentDisplayStr)
{
    ChipLogProgress(AppServer, "JNI_METHOD contentLauncherLaunchURL called");
    const char * nativeContentUrl        = env->GetStringUTFChars(contentUrl, 0);
    const char * nativeContentDisplayStr = env->GetStringUTFChars(contentDisplayStr, 0);

    CastingServer::GetInstance()->ContentLauncherLaunchURL(nativeContentUrl, nativeContentDisplayStr);

    env->ReleaseStringUTFChars(contentUrl, nativeContentUrl);
    env->ReleaseStringUTFChars(contentDisplayStr, nativeContentDisplayStr);
}
