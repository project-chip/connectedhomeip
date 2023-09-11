/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AndroidControllerExceptions.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {

CHIP_ERROR AndroidControllerExceptions::CreateAndroidControllerException(JNIEnv * env, const char * message, uint32_t errorCode,
                                                                         jthrowable & outEx)
{
    jclass controllerExceptionCls;
    CHIP_ERROR err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/ChipDeviceControllerException",
                                                              controllerExceptionCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    JniClass controllerExceptionJniCls(controllerExceptionCls);

    jmethodID exceptionConstructor = env->GetMethodID(controllerExceptionCls, "<init>", "(JLjava/lang/String;)V");
    outEx = (jthrowable) env->NewObject(controllerExceptionCls, exceptionConstructor, static_cast<jlong>(errorCode),
                                        env->NewStringUTF(message));
    VerifyOrReturnError(outEx != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    return CHIP_NO_ERROR;
}

} // namespace chip
