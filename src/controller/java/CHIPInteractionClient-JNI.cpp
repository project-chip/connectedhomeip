/*
 *   Copyright (c) 2024 Project CHIP Authors
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
#include "CHIPInteractionClient-JNI.h"
#include "AndroidInteractionClient.h"

#include <lib/support/CHIPJNIError.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipInteractionClient_##METHOD_NAME

jint AndroidChipInteractionJNI_OnLoad(JavaVM * jvm, void * reserved)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env;

    ChipLogProgress(Controller, "AndroidChipInteractionJNI_OnLoad called");

    chip::Platform::MemoryInit();

    // Save a reference to the JVM.  Will need this to call back into Java.
    chip::JniReferences::GetInstance().SetJavaVm(jvm, "chip/devicecontroller/ChipInteractionClient");

    // Get a JNI environment object.
    env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    ChipLogProgress(Controller, "Loading Java class references.");

    // Get various class references need by the API.
    jclass controllerExceptionCls;
    err = chip::JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ChipDeviceControllerException",
                                                              controllerExceptionCls);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, JNI_ERR);

    ChipLogProgress(Controller, "Java class references loaded.");

exit:
    if (err != CHIP_NO_ERROR)
    {
        chip::JniReferences::GetInstance().ThrowError(env, controllerExceptionCls, err);
        chip::DeviceLayer::StackUnlock unlock;
        JNI_OnUnload(jvm, reserved);
    }

    return (err == CHIP_NO_ERROR) ? JNI_VERSION_1_6 : JNI_ERR;
}

void AndroidChipInteractionJNI_OnUnload(JavaVM * jvm, void * reserved)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "AndroidChipInteractionJNI_OnUnload() called");
    chip::Platform::MemoryShutdown();
}

JNI_METHOD(void, subscribe)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jobject dataVersionFilterList, jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered,
 jint imTimeoutMs, jobject eventMin, jboolean isPeerLIT)
{
    CHIP_ERROR err = subscribe(env, handle, callbackHandle, devicePtr, attributePathList, eventPathList, dataVersionFilterList,
                               minInterval, maxInterval, keepSubscriptions, isFabricFiltered, imTimeoutMs, eventMin, isPeerLIT);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Subscribe Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, read)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jobject dataVersionFilterList, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    CHIP_ERROR err = read(env, handle, callbackHandle, devicePtr, attributePathList, eventPathList, dataVersionFilterList,
                          isFabricFiltered, imTimeoutMs, eventMin);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Read Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, write)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributeList, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    CHIP_ERROR err = write(env, handle, callbackHandle, devicePtr, attributeList, timedRequestTimeoutMs, imTimeoutMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Write Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, invoke)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElement, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    CHIP_ERROR err = invoke(env, handle, callbackHandle, devicePtr, invokeElement, timedRequestTimeoutMs, imTimeoutMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Invoke Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, extendableInvoke)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElementList,
 jint timedRequestTimeoutMs, jint imTimeoutMs)
{
    CHIP_ERROR err =
        extendableInvoke(env, handle, callbackHandle, devicePtr, invokeElementList, timedRequestTimeoutMs, imTimeoutMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Batch Invoke Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, shutdownSubscriptions)
(JNIEnv * env, jobject self, jlong handle, jobject fabricIndex, jobject peerNodeId, jobject subscriptionId)
{
    CHIP_ERROR err = shutdownSubscriptions(env, handle, fabricIndex, peerNodeId, subscriptionId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to shutdown subscriptions with Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(jlong, getRemoteDeviceId)
(JNIEnv * env, jobject self, jlong devicePtr)
{
    return getRemoteDeviceId(devicePtr);
}

JNI_METHOD(jint, getFabricIndex)
(JNIEnv * env, jobject self, jlong devicePtr)
{
    return getFabricIndex(devicePtr);
}
