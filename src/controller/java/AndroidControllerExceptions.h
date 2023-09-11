/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>
#include <lib/core/CHIPError.h>

namespace chip {
class AndroidControllerExceptions
{
public:
    AndroidControllerExceptions(const AndroidControllerExceptions &)  = delete;
    AndroidControllerExceptions(const AndroidControllerExceptions &&) = delete;
    AndroidControllerExceptions & operator=(const AndroidControllerExceptions &) = delete;

    static AndroidControllerExceptions & GetInstance()
    {
        static AndroidControllerExceptions androidControllerExceptions;
        return androidControllerExceptions;
    }

    /**
     * Creates a Java AndroidControllerException object in outEx.
     */
    CHIP_ERROR CreateAndroidControllerException(JNIEnv * env, const char * message, uint32_t errorCode, jthrowable & outEx);

private:
    AndroidControllerExceptions() {}
};
} // namespace chip
