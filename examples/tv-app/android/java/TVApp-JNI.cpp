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

#include "ContentLauncherManager.h"
#include "KeypadInputManager.h"
#include "LowPowerManager.h"
#include "MediaInputManager.h"
#include "MediaPlaybackManager.h"
#include "TvChannelManager.h"
#include "WakeOnLanManager.h"
#include <app/server/java/AndroidAppServerWrapper.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_tcl_chip_tvapp_TvApp_##METHOD_NAME

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(void, setKeypadInputManager)(JNIEnv *, jobject, jobject manager)
{
    KeypadInputMgr().InitializeWithObjects(manager);
}

JNI_METHOD(void, setWakeOnLanManager)(JNIEnv *, jobject, jobject manager)
{
    WakeOnLanMgr().InitializeWithObjects(manager);
}

JNI_METHOD(void, setMediaInputManager)(JNIEnv *, jobject, jobject manager)
{
    MediaInputMgr().InitializeWithObjects(manager);
}

JNI_METHOD(void, setContentLaunchManager)(JNIEnv *, jobject, jobject manager)
{
    ContentLauncherMgr().InitializeWithObjects(manager);
}

JNI_METHOD(void, setLowPowerManager)(JNIEnv *, jobject, jobject manager)
{
    LowPowerMgr().InitializeWithObjects(manager);
}

JNI_METHOD(void, setMediaPlaybackManager)(JNIEnv *, jobject, jobject manager)
{
    MediaPlaybackMgr().InitializeWithObjects(manager);
}

JNI_METHOD(void, setTvChannelManager)(JNIEnv *, jobject, jobject manager)
{
    TvChannelMgr().InitializeWithObjects(manager);
}
