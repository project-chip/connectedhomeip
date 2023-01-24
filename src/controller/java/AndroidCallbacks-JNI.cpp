/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include "AndroidCallbacks.h"

#include <jni.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_##CLASS_NAME##_##METHOD_NAME

using namespace chip::Controller;

JNI_METHOD(jlong, GetConnectedDeviceCallbackJni, newCallback)(JNIEnv * env, jobject self, jobject callback)
{
    GetConnectedDeviceCallback * connectedDeviceCallback = chip::Platform::New<GetConnectedDeviceCallback>(self, callback);
    return reinterpret_cast<jlong>(connectedDeviceCallback);
}

JNI_METHOD(void, GetConnectedDeviceCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallback handle is nullptr"));
    delete connectedDeviceCallback;
}

JNI_METHOD(jlong, ReportCallbackJni, newCallback)
(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava, jobject reportCallbackJava,
 jobject resubscriptionAttemptCallbackJava)
{
    ReportCallback * reportCallback = chip::Platform::New<ReportCallback>(self, subscriptionEstablishedCallbackJava,
                                                                          reportCallbackJava, resubscriptionAttemptCallbackJava);
    return reinterpret_cast<jlong>(reportCallback);
}

JNI_METHOD(void, ReportCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    ReportCallback * reportCallback = reinterpret_cast<ReportCallback *>(callbackHandle);
    VerifyOrReturn(reportCallback != nullptr, ChipLogError(Controller, "ReportCallback handle is nullptr"));
    delete reportCallback;
}

JNI_METHOD(jlong, ReportEventCallbackJni, newCallback)
(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava, jobject reportCallbackJava,
 jobject resubscriptionAttemptCallbackJava)
{
    ReportEventCallback * reportCallback = chip::Platform::New<ReportEventCallback>(
        self, subscriptionEstablishedCallbackJava, reportCallbackJava, resubscriptionAttemptCallbackJava);
    return reinterpret_cast<jlong>(reportCallback);
}

JNI_METHOD(void, ReportEventCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    ReportEventCallback * reportCallback = reinterpret_cast<ReportEventCallback *>(callbackHandle);
    VerifyOrReturn(reportCallback != nullptr, ChipLogError(Controller, "ReportCallback handle is nullptr"));
    delete reportCallback;
}
