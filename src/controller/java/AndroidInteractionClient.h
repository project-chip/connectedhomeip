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

#pragma once

#include <lib/support/JniReferences.h>

CHIP_ERROR subscribe(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList,
                     jobject eventPathList, jobject dataVersionFilterList, jint minInterval, jint maxInterval,
                     jboolean keepSubscriptions, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin, jboolean isPeerLIT);
CHIP_ERROR read(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
                jobject dataVersionFilterList, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin);
CHIP_ERROR write(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributeList,
                 jint timedRequestTimeoutMs, jint imTimeoutMs);
CHIP_ERROR invoke(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElement,
                  jint timedRequestTimeoutMs, jint imTimeoutMs);
CHIP_ERROR extendableInvoke(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElementList,
                            jint timedRequestTimeoutMs, jint imTimeoutMs);
CHIP_ERROR shutdownSubscriptions(JNIEnv * env, jlong handle, jobject fabricIndex, jobject peerNodeId, jobject subscriptionId);

jlong getRemoteDeviceId(jlong devicePtr);
jint getFabricIndex(jlong devicePtr);
