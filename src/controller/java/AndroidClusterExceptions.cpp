/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    err = chip::JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/ChipClusterException", clusterExceptionCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    chip::JniClass clusterExceptionJniCls(clusterExceptionCls);

    exceptionConstructor = env->GetMethodID(clusterExceptionCls, "<init>", "(J)V");
    VerifyOrReturnError(exceptionConstructor != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    outEx = (jthrowable) env->NewObject(clusterExceptionCls, exceptionConstructor, static_cast<jlong>(errorCode));
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
