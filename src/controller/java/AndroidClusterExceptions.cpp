/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include "AndroidClusterExceptions.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {

CHIP_ERROR AndroidClusterExceptions::CreateChipClusterException(JNIEnv * env, uint32_t errorCode, jthrowable & outEx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID exceptionConstructor;
    jclass clusterExceptionCls;

    err =
        chip::JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/ChipClusterException", clusterExceptionCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    exceptionConstructor = env->GetMethodID(clusterExceptionCls, "<init>", "(J)V");
    VerifyOrReturnError(exceptionConstructor != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    outEx = static_cast<jthrowable>(env->NewObject(clusterExceptionCls, exceptionConstructor, static_cast<jlong>(errorCode)));
    VerifyOrReturnError(outEx != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    return err;
}

} // namespace chip
