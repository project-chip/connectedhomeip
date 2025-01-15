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
    messagingContext->SetUp();

    MessagingContext * messagingContextResult = messagingContext;
    return reinterpret_cast<jlong>(messagingContextResult);
}

JNI_METHOD(void, MessagingContext, deleteMessagingContext)
(JNIEnv * env, jobject self, jlong contextHandle)
{
    LoopbackMessagingContext * messagingContext = reinterpret_cast<LoopbackMessagingContext *>(contextHandle);
    VerifyOrReturn(messagingContext != nullptr, ChipLogError(Test, "MessagingContext handle is nullptr"));
    messagingContext->TearDown();
    delete messagingContext;
}
