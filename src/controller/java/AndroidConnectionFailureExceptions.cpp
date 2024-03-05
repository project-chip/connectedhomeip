/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "AndroidConnectionFailureExceptions.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {

CHIP_ERROR AndroidConnectionFailureExceptions::CreateAndroidConnectionFailureException(JNIEnv * env, const char * message,
                                                                                       uint32_t errorCode,
                                                                                       SessionEstablishmentStage state,
                                                                                       jthrowable & outEx)
{
    jclass controllerExceptionCls;
    CHIP_ERROR err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ConnectionFailureException",
                                                                   controllerExceptionCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    jmethodID exceptionConstructor = env->GetMethodID(controllerExceptionCls, "<init>", "(JILjava/lang/String;)V");
    outEx = static_cast<jthrowable>(env->NewObject(controllerExceptionCls, exceptionConstructor, static_cast<jlong>(errorCode),
                                                   static_cast<jint>(state), env->NewStringUTF(message)));
    VerifyOrReturnError(outEx != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    return CHIP_NO_ERROR;
}

} // namespace chip
