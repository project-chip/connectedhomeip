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
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_tv_server_tvapp_UserPrompterResolver_##METHOD_NAME

JNI_METHOD(void, OnPinCodeEntered)(JNIEnv *, jobject, jint jPinCode)
{
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    chip::DeviceLayer::StackLock lock;
    uint32_t pinCode = (uint32_t) jPinCode;
    ChipLogProgress(Zcl, "OnPinCodeEntered %d", pinCode);
    GetCommissionerDiscoveryController()->CommissionWithPincode(pinCode);
#endif
}

JNI_METHOD(void, OnPinCodeDeclined)(JNIEnv *, jobject)
{
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "OnPinCodeDeclined");
    GetCommissionerDiscoveryController()->Cancel();
#endif
}

JNI_METHOD(void, OnPromptAccepted)(JNIEnv *, jobject)
{
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "OnPromptAccepted");
    GetCommissionerDiscoveryController()->Ok();
#endif
}

JNI_METHOD(void, OnPromptDeclined)(JNIEnv *, jobject)
{
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "OnPromptDeclined");
    GetCommissionerDiscoveryController()->Cancel();
#endif
}
