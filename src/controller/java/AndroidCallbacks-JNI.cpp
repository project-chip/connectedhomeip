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
    return newConnectedDeviceCallback(env, self, callback);
}

JNI_METHOD(void, GetConnectedDeviceCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    deleteConnectedDeviceCallback(env, self, callbackHandle);
}

JNI_METHOD(jlong, ReportCallbackJni, newCallback)
(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava, jobject resubscriptionAttemptCallbackJava)
{
    return newReportCallback(env, self, subscriptionEstablishedCallbackJava, resubscriptionAttemptCallbackJava,
                             "()Lchip/devicecontroller/model/NodeState;");
}

JNI_METHOD(void, ReportCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    deleteReportCallback(env, self, callbackHandle);
}

JNI_METHOD(jlong, WriteAttributesCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    return newWriteAttributesCallback(env, self);
}

JNI_METHOD(void, WriteAttributesCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    deleteWriteAttributesCallback(env, self, callbackHandle);
}

JNI_METHOD(jlong, InvokeCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    return newInvokeCallback(env, self);
}

JNI_METHOD(void, InvokeCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    deleteInvokeCallback(env, self, callbackHandle);
}

JNI_METHOD(jlong, ExtendableInvokeCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    return newExtendableInvokeCallback(env, self);
}

JNI_METHOD(void, ExtendableInvokeCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    deleteExtendableInvokeCallback(env, self, callbackHandle);
}
