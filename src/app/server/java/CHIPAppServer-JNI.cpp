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

/**
 *    @file
 *      Implementation of JNI bridge for CHIP App Server for Android TV apps
 *
 */
#include "AndroidAppServerWrapper.h"
#include "ChipAppServerDelegate.h"
#include "ChipFabricProvider-JNI.h"
#include "ChipThreadWork.h"
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/android/AndroidChipPlatform-JNI.h>
#include <platform/internal/BLEManager.h>

using namespace chip;
using namespace chip::DeviceLayer;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_appserver_ChipAppServer_##METHOD_NAME

#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL

static void * IOThreadAppMain(void * arg);

namespace {
JavaVM * sJVM       = nullptr;
pthread_t sIOThread = PTHREAD_NULL;
JniGlobalReference sChipAppServerExceptionCls;
ChipAppServerDelegate sChipAppServerDelegate;
} // namespace

jint AndroidAppServerJNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;

    ChipLogProgress(AppServer, "JNI_OnLoad() called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    JniReferences::GetInstance().SetJavaVm(jvm, "chip/appserver/ChipAppServer");
    sJVM = jvm;

    // Get a JNI environment object.
    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(AppServer, "Loading Java class references.");

    // Get various class references need by the API.
    jclass appServerExceptionCls;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/appserver/ChipAppServerException", appServerExceptionCls);
    SuccessOrExit(err);
    err = sChipAppServerExceptionCls.Init(static_cast<jobject>(appServerExceptionCls));
    SuccessOrExit(err);
    ChipLogProgress(AppServer, "Java class references loaded.");

    err = AndroidChipPlatformJNI_OnLoad(jvm, reserved);
    SuccessOrExit(err);
    err = AndroidChipFabricProviderJNI_OnLoad(jvm, reserved);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        JniReferences::GetInstance().ThrowError(env, sChipAppServerExceptionCls, err);
        chip::DeviceLayer::StackUnlock unlock;
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void AndroidAppServerJNI_OnUnload(JavaVM * jvm, void * reserved)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "JNI_OnUnload() called");

    // If the IO thread has been started, shut it down and wait for it to exit.
    if (sIOThread != PTHREAD_NULL)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();

        chip::DeviceLayer::StackUnlock unlock;
        pthread_join(sIOThread, NULL);
    }

    sJVM = NULL;

    chip::Platform::MemoryShutdown();
}

JNI_METHOD(jboolean, startApp)(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;

    CHIP_ERROR err = ChipAndroidAppInit();
    SuccessOrExit(err);

    if (sIOThread == PTHREAD_NULL)
    {
        pthread_create(&sIOThread, NULL, IOThreadAppMain, NULL);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(jboolean, startAppWithDelegate)(JNIEnv * env, jobject self, jobject appDelegate)
{
    chip::DeviceLayer::StackLock lock;

    CHIP_ERROR err = sChipAppServerDelegate.InitializeWithObjects(appDelegate);
    SuccessOrExit(err);

    err = ChipAndroidAppInit(&sChipAppServerDelegate);
    SuccessOrExit(err);

    if (sIOThread == PTHREAD_NULL)
    {
        pthread_create(&sIOThread, NULL, IOThreadAppMain, NULL);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(jboolean, stopApp)(JNIEnv * env, jobject self)
{
    chip::ThreadWork::ChipMainThreadScheduleAndWait([] { ChipAndroidAppShutdown(); });
    return JNI_TRUE;
}

JNI_METHOD(jboolean, resetApp)(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    ChipAndroidAppReset();

    return JNI_TRUE;
}

void * IOThreadAppMain(void * arg)
{
    JNIEnv * env;
    JavaVMAttachArgs attachArgs;

    // Attach the IO thread to the JVM as a daemon thread.
    // This allows the JVM to shutdown without waiting for this thread to exit.
    attachArgs.version = JNI_VERSION_1_6;
    attachArgs.name    = (char *) "CHIP AppServer IO Thread";
    attachArgs.group   = NULL;
#ifdef __ANDROID__
    sJVM->AttachCurrentThreadAsDaemon(&env, (void *) &attachArgs);
#else
    sJVM->AttachCurrentThreadAsDaemon((void **) &env, (void *) &attachArgs);
#endif

    ChipLogProgress(AppServer, "IO thread starting");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(AppServer, "IO thread ending");

    // Detach the thread from the JVM.
    sJVM->DetachCurrentThread();

    return NULL;
}
