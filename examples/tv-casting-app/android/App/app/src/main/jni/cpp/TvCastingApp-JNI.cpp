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
#include <app/server/java/AndroidAppServerWrapper.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>

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

// TBD: Temp dummy function for testing
JNI_METHOD(void, doSomethingInCpp)(JNIEnv *, jobject, jint endpoint)
{
    ChipLogProgress(AppServer, "JNI_METHOD doSomethingInCpp called with endpoint %d", endpoint);
}
