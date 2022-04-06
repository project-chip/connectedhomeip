/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

CHIP_ERROR AndroidClusterExceptions::CreateChipClusterException(JNIEnv * env, jint errorCode, jthrowable & outEx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID exceptionConstructor;
    jclass clusterExceptionCls;

    err = chip::JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/ChipClusterException", clusterExceptionCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    chip::JniClass clusterExceptionJniCls(clusterExceptionCls);

    exceptionConstructor = env->GetMethodID(clusterExceptionCls, "<init>", "(I)V");
    VerifyOrReturnError(exceptionConstructor != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    outEx = (jthrowable) env->NewObject(clusterExceptionCls, exceptionConstructor, errorCode);
    VerifyOrReturnError(outEx != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    return err;
}

CHIP_ERROR AndroidClusterExceptions::CreateIllegalStateException(JNIEnv * env, const char message[], ChipError errorCode,
                                                                 jthrowable & outEx)
{
    return CreateIllegalStateException(env, message, errorCode.AsInteger(), outEx);
}

CHIP_ERROR AndroidClusterExceptions::CreateIllegalStateException(JNIEnv * env, const char message[],
                                                                 ChipError::StorageType errorCode, jthrowable & outEx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID exceptionConstructor;
    jclass exceptionClass;
    jstring errStr;

    err = JniReferences::GetInstance().GetClassRef(env, "java/lang/IllegalStateException", exceptionClass);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    JniClass exceptionJniClass(exceptionClass);

    exceptionConstructor = env->GetMethodID(exceptionClass, "<init>", "(Ljava/lang/String;)V");
    VerifyOrReturnError(exceptionConstructor != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    char buf[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    snprintf(buf, sizeof(buf), "%s: %d", message, errorCode);
    errStr = env->NewStringUTF(buf);

    outEx = static_cast<jthrowable>(env->NewObject(exceptionClass, exceptionConstructor, errStr));
    VerifyOrReturnError(outEx != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    return err;
}

void AndroidClusterExceptions::ReturnIllegalStateException(JNIEnv * env, jobject callback, const char message[], ChipError error)
{
    ReturnIllegalStateException(env, callback, message, error.AsInteger());
}

void AndroidClusterExceptions::ReturnIllegalStateException(JNIEnv * env, jobject callback, const char message[],
                                                           ChipError::StorageType errorCode)
{
    VerifyOrReturn(callback != nullptr, ChipLogDetail(Zcl, "Callback is null in ReturnIllegalStateException(), exiting early"));
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID method;
    err = JniReferences::GetInstance().FindMethod(env, callback, "onError", "(Ljava/lang/Exception;)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Error throwing IllegalStateException %d", err.AsInteger());
        return;
    }

    jthrowable exception;
    err = CreateIllegalStateException(env, message, errorCode, exception);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Error throwing IllegalStateException %d", err.AsInteger());
        return;
    }
    env->CallVoidMethod(callback, method, exception);
}

} // namespace chip
