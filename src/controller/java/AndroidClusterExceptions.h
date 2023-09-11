/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>
#include <lib/core/CHIPError.h>

namespace chip {
class AndroidClusterExceptions
{
public:
    AndroidClusterExceptions(const AndroidClusterExceptions &)  = delete;
    AndroidClusterExceptions(const AndroidClusterExceptions &&) = delete;
    AndroidClusterExceptions & operator=(const AndroidClusterExceptions &) = delete;

    static AndroidClusterExceptions & GetInstance()
    {
        static AndroidClusterExceptions androidClusterExceptions;
        return androidClusterExceptions;
    }

    /**
     * Creates a Java ChipClusterException object in outEx.
     */
    CHIP_ERROR CreateChipClusterException(JNIEnv * env, uint32_t errorCode, jthrowable & outEx);

    /**
     * Creates a Java IllegalStateException in outEx.
     */
    CHIP_ERROR CreateIllegalStateException(JNIEnv * env, const char message[], ChipError errorCode, jthrowable & outEx);

    CHIP_ERROR CreateIllegalStateException(JNIEnv * env, const char message[], ChipError::StorageType errorCode,
                                           jthrowable & outEx);

    /**
     * Creates an IllegalStateException and passes it to the Java onError() function of the provided callback object.
     */
    void ReturnIllegalStateException(JNIEnv * env, jobject callback, const char message[], ChipError errorCode);

    void ReturnIllegalStateException(JNIEnv * env, jobject callback, const char message[], ChipError::StorageType errorCode);

private:
    AndroidClusterExceptions() {}
};
} // namespace chip
