/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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
#include "JniReferences.h"
#include "MdnsImpl.h"
#include "StackLock.h"

#include <jni.h>

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_mdns_ChipMdnsCallbackImpl_##METHOD_NAME

JNI_METHOD(void, handleServiceResolve)
(JNIEnv * env, jclass self, jstring instanceName, jstring serviceType, jstring address, jint port, jlong callbackHandle,
 jlong contextHandle)
{
    StackLockGuard lock(chip::Controller::JniReferences::GetInstance().GetStackLock());
    using ::chip::Mdns::HandleResolve;
    HandleResolve(instanceName, serviceType, address, port, callbackHandle, contextHandle);
}
