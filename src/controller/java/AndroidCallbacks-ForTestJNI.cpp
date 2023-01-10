/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#include <jni.h>

#include "AndroidCallbacks.h"
#include "lib/support/CHIPMem.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/logging/CHIPLogging.h"
#include "messaging/tests/MessagingContext.h"

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_##CLASS_NAME##_##METHOD_NAME

using namespace chip;
using namespace chip::Controller;
using namespace chip::Test;

JNI_METHOD(void, GetConnectedDeviceCallbackForTestJni, onDeviceConnected)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong messagingContextHandle)
{
    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallbackJni handle is nullptr"));

    MessagingContext * messagingContext = reinterpret_cast<MessagingContext *>(messagingContextHandle);
    VerifyOrReturn(messagingContext != nullptr, ChipLogError(Controller, "MessagingContext handle is nullptr"));

    GetConnectedDeviceCallback::OnDeviceConnectedFn(connectedDeviceCallback, messagingContext->GetExchangeManager(),
                                                    messagingContext->GetSessionBobToAlice());
}

JNI_METHOD(void, GetConnectedDeviceCallbackForTestJni, onDeviceConnectionFailure)
(JNIEnv * env, jobject self, jlong callbackHandle, jint errorCode)
{
    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallbackJni handle is nullptr"));

    GetConnectedDeviceCallback::OnDeviceConnectionFailureFn(connectedDeviceCallback, ScopedNodeId(), chip::ChipError(errorCode));
}
