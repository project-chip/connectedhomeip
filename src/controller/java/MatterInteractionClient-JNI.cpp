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
#include "AndroidInteractionClient.h"

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_matter_controller_MatterControllerImpl_##METHOD_NAME

JNI_METHOD(void, subscribe)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered, jint imTimeoutMs)
{
    CHIP_ERROR err = subscribe(env, handle, callbackHandle, devicePtr, attributePathList, eventPathList, nullptr, minInterval,
                               maxInterval, keepSubscriptions, isFabricFiltered, imTimeoutMs, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Subscribe Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

JNI_METHOD(void, read)
(JNIEnv * env, jobject self, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jboolean isFabricFiltered, jint imTimeoutMs)
{
    CHIP_ERROR err = read(env, handle, callbackHandle, devicePtr, attributePathList, eventPathList, nullptr, isFabricFiltered,
                          imTimeoutMs, nullptr);
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
