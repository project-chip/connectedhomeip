/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include "KeypadInputManager.h"
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <cstddef>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>

using namespace chip;

KeypadInputManager KeypadInputManager::sInstance;

EmberAfKeypadInputStatus keypadInputClusterSendKey(EmberAfKeypadInputCecKeyCode keyCode)
{
    return KeypadInputMgr().SendKey(keyCode);
}

EmberAfKeypadInputStatus KeypadInputManager::SendKey(EmberAfKeypadInputCecKeyCode keyCode)
{
    jint ret       = -1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogProgress(Zcl, "Received keypadInputClusterSendKey: %d", keyCode);
    VerifyOrExit(mKeypadInputManagerObject != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mSendKeyMethod != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(env != NULL, err = CHIP_JNI_ERROR_NO_ENV);

    env->ExceptionClear();
    ret = env->CallIntMethod(mKeypadInputManagerObject, mSendKeyMethod, static_cast<jint>(keyCode));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    if (err != CHIP_NO_ERROR)
    {
        return EMBER_ZCL_KEYPAD_INPUT_STATUS_SUCCESS;
    }

    return static_cast<EmberAfKeypadInputStatus>(ret);
}

void KeypadInputManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for KeypadInputManager"));

    mKeypadInputManagerObject = env->NewGlobalRef(managerObject);
    VerifyOrReturn(mKeypadInputManagerObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef KeypadInputManager"));

    jclass KeypadInputManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturn(KeypadInputManagerClass != nullptr, ChipLogError(Zcl, "Failed to get KeypadInputManager Java class"));

    mSendKeyMethod = env->GetMethodID(KeypadInputManagerClass, "sendKey", "(I)I");
    if (mSendKeyMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access KeypadInputManager 'sendKey' method");
        env->ExceptionClear();
    }
}
