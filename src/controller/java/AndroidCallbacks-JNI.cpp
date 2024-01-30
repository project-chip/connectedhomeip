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

#define JAVA_JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                           \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_##CLASS_NAME##_##METHOD_NAME

#define KOTLIN_JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                         \
    extern "C" JNIEXPORT RETURN JNICALL Java_matter_controller_##CLASS_NAME##_##METHOD_NAME

using namespace chip::Controller;

JAVA_JNI_METHOD(jlong, GetConnectedDeviceCallbackJni, newCallback)(JNIEnv * env, jobject self, jobject callback)
{
    chip::DeviceLayer::StackLock lock;
    GetConnectedDeviceCallback * connectedDeviceCallback = chip::Platform::New<GetConnectedDeviceCallback>(self, callback);
    return reinterpret_cast<jlong>(connectedDeviceCallback);
}

JAVA_JNI_METHOD(void, GetConnectedDeviceCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallback handle is nullptr"));
    chip::Platform::Delete(connectedDeviceCallback);
}

JAVA_JNI_METHOD(jlong, ReportCallbackJni, newCallback)
(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava, jobject resubscriptionAttemptCallbackJava)
{
    chip::DeviceLayer::StackLock lock;
    ReportCallback * reportCallback =
        chip::Platform::New<ReportCallback>(self, subscriptionEstablishedCallbackJava, resubscriptionAttemptCallbackJava);
    return reinterpret_cast<jlong>(reportCallback);
}

JAVA_JNI_METHOD(void, ReportCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    ReportCallback * reportCallback = reinterpret_cast<ReportCallback *>(callbackHandle);
    VerifyOrReturn(reportCallback != nullptr, ChipLogError(Controller, "ReportCallback handle is nullptr"));
    chip::Platform::Delete(reportCallback);
}

JAVA_JNI_METHOD(jlong, WriteAttributesCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    WriteAttributesCallback * writeAttributesCallback = chip::Platform::New<WriteAttributesCallback>(self);
    return reinterpret_cast<jlong>(writeAttributesCallback);
}

JAVA_JNI_METHOD(void, WriteAttributesCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    WriteAttributesCallback * writeAttributesCallback = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    VerifyOrReturn(writeAttributesCallback != nullptr, ChipLogError(Controller, "WriteAttributesCallback handle is nullptr"));
    chip::Platform::Delete(writeAttributesCallback);
}

JAVA_JNI_METHOD(jlong, InvokeCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    InvokeCallback * invokeCallback = chip::Platform::New<InvokeCallback>(self);
    return reinterpret_cast<jlong>(invokeCallback);
}

JAVA_JNI_METHOD(void, InvokeCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    InvokeCallback * invokeCallback = reinterpret_cast<InvokeCallback *>(callbackHandle);
    VerifyOrReturn(invokeCallback != nullptr, ChipLogError(Controller, "InvokeCallback handle is nullptr"));
    chip::Platform::Delete(invokeCallback);
}

KOTLIN_JNI_METHOD(jlong, GetConnectedDeviceCallbackJni, newCallback)(JNIEnv * env, jobject self, jobject callback)
{
    chip::DeviceLayer::StackLock lock;
    GetConnectedDeviceCallback * connectedDeviceCallback = chip::Platform::New<GetConnectedDeviceCallback>(self, callback);
    return reinterpret_cast<jlong>(connectedDeviceCallback);
}

KOTLIN_JNI_METHOD(void, GetConnectedDeviceCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallback handle is nullptr"));
    chip::Platform::Delete(connectedDeviceCallback);
}

KOTLIN_JNI_METHOD(jlong, ReportCallbackJni, newCallback)
(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava, jobject resubscriptionAttemptCallbackJava)
{
    chip::DeviceLayer::StackLock lock;
    ReportCallback * reportCallback =
        chip::Platform::New<ReportCallback>(self, subscriptionEstablishedCallbackJava, resubscriptionAttemptCallbackJava);
    return reinterpret_cast<jlong>(reportCallback);
}

KOTLIN_JNI_METHOD(void, ReportCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    ReportCallback * reportCallback = reinterpret_cast<ReportCallback *>(callbackHandle);
    VerifyOrReturn(reportCallback != nullptr, ChipLogError(Controller, "ReportCallback handle is nullptr"));
    chip::Platform::Delete(reportCallback);
}

KOTLIN_JNI_METHOD(jlong, WriteAttributesCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    WriteAttributesCallback * writeAttributesCallback = chip::Platform::New<WriteAttributesCallback>(self);
    return reinterpret_cast<jlong>(writeAttributesCallback);
}

KOTLIN_JNI_METHOD(void, WriteAttributesCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    WriteAttributesCallback * writeAttributesCallback = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    VerifyOrReturn(writeAttributesCallback != nullptr, ChipLogError(Controller, "WriteAttributesCallback handle is nullptr"));
    chip::Platform::Delete(writeAttributesCallback);
}

KOTLIN_JNI_METHOD(jlong, InvokeCallbackJni, newCallback)
(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    InvokeCallback * invokeCallback = chip::Platform::New<InvokeCallback>(self);
    return reinterpret_cast<jlong>(invokeCallback);
}

KOTLIN_JNI_METHOD(void, InvokeCallbackJni, deleteCallback)(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    InvokeCallback * invokeCallback = reinterpret_cast<InvokeCallback *>(callbackHandle);
    VerifyOrReturn(invokeCallback != nullptr, ChipLogError(Controller, "InvokeCallback handle is nullptr"));
    chip::Platform::Delete(invokeCallback);
}
