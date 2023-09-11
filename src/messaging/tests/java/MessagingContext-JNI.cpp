/*
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <jni.h>

#include "lib/support/CHIPMem.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/JniReferences.h"
#include "lib/support/logging/CHIPLogging.h"
#include "messaging/tests/MessagingContext.h"

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_testing_##CLASS_NAME##_##METHOD_NAME

using namespace chip;
using namespace chip::Test;

JNI_METHOD(jlong, MessagingContext, newMessagingContext)
(JNIEnv * env, jobject self, jboolean initializeNodes)
{
    LoopbackMessagingContext * messagingContext = chip::Platform::New<LoopbackMessagingContext>();
    messagingContext->ConfigInitializeNodes(initializeNodes != JNI_FALSE);
    CHIP_ERROR err = messagingContext->Init();
    if (err != CHIP_NO_ERROR)
    {
        jclass exceptionCls = env->FindClass("chip/platform/AndroidChipPlatformException");
        JniReferences::GetInstance().ThrowError(env, exceptionCls, err);
        return 0;
    }

    MessagingContext * messagingContextResult = messagingContext;
    return reinterpret_cast<jlong>(messagingContextResult);
}

JNI_METHOD(void, MessagingContext, deleteMessagingContext)
(JNIEnv * env, jobject self, jlong contextHandle)
{
    MessagingContext * messagingContext = reinterpret_cast<MessagingContext *>(contextHandle);
    VerifyOrReturn(messagingContext != nullptr, ChipLogError(Test, "MessagingContext handle is nullptr"));
    messagingContext->Shutdown();
    delete messagingContext;
}
