/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MyUserPrompterResolver-JNI.h"
#include "TvApp-JNI.h"
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_tcl_chip_tvapp_UserPrompterResolver_##METHOD_NAME

JNI_METHOD(void, OnPinCodeEntered)(JNIEnv *, jobject, jint jPinCode)
{
    uint32_t pinCode = (uint32_t) jPinCode;
    ChipLogProgress(Zcl, "OnPinCodeEntered %d", pinCode);
    GetCommissionerDiscoveryController()->CommissionWithPincode(pinCode);
}

JNI_METHOD(void, OnPinCodeDeclined)(JNIEnv *, jobject)
{
    ChipLogProgress(Zcl, "OnPinCodeDeclined");
    GetCommissionerDiscoveryController()->Cancel();
}

JNI_METHOD(void, OnPromptAccepted)(JNIEnv *, jobject)
{
    ChipLogProgress(Zcl, "OnPromptAccepted");
    GetCommissionerDiscoveryController()->Ok();
}

JNI_METHOD(void, OnPromptDeclined)(JNIEnv *, jobject)
{
    ChipLogProgress(Zcl, "OnPromptDeclined");
    GetCommissionerDiscoveryController()->Cancel();
}
