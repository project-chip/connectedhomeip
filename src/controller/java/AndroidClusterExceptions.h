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
    CHIP_ERROR CreateChipClusterException(JNIEnv * env, jint errorCode, jthrowable & outEx);

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
