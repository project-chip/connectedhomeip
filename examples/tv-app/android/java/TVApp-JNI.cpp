/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "TvApp-JNI.h"
#include "ChannelManager.h"
#include "ContentLauncherManager.h"
#include "KeypadInputManager.h"
#include "LowPowerManager.h"
#include "MediaInputManager.h"
#include "MediaPlaybackManager.h"
#include "WakeOnLanManager.h"
#include <app/server/java/AndroidAppServerWrapper.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_tcl_chip_tvapp_TvApp_##METHOD_NAME

TvAppJNI TvAppJNI::sInstance;

void TvAppJNI::InitializeWithObjects(jobject app)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for TvAppJNI"));

    mTvAppObject = env->NewGlobalRef(app);
    VerifyOrReturn(mTvAppObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef TvAppJNI"));

    jclass managerClass = env->GetObjectClass(mTvAppObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get TvAppJNI Java class"));

    mPostClusterInitMethod = env->GetMethodID(managerClass, "postClusterInit", "(II)V");
    if (mPostClusterInitMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'postClusterInit' method");
        env->ExceptionClear();
    }
}

void TvAppJNI::PostClusterInit(int clusterId, int endpoint)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for TvAppJNI::PostClusterInit"));
    VerifyOrReturn(mTvAppObject != nullptr, ChipLogError(Zcl, "TvAppJNI::mTvAppObject null"));
    VerifyOrReturn(mPostClusterInitMethod != nullptr, ChipLogError(Zcl, "TvAppJNI::mPostClusterInitMethod null"));

    env->CallVoidMethod(mTvAppObject, mPostClusterInitMethod, static_cast<jint>(clusterId), static_cast<jint>(endpoint));
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Failed to call TvAppJNI 'postClusterInit' method");
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
    TvAppJNIMgr().InitializeWithObjects(app);
}

JNI_METHOD(void, setKeypadInputManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    KeypadInputManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setWakeOnLanManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    WakeOnLanManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setMediaInputManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    MediaInputManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setContentLaunchManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    ContentLauncherManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setLowPowerManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    LowPowerManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setMediaPlaybackManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    MediaPlaybackManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setChannelManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    ChannelManager::NewManager(endpoint, manager);
}
