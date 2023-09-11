/*
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
