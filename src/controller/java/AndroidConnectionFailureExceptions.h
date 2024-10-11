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

#pragma once

#include <app/OperationalSessionSetup.h>
#include <jni.h>
#include <lib/core/CHIPError.h>

namespace chip {
class AndroidConnectionFailureExceptions
{
public:
    AndroidConnectionFailureExceptions(const AndroidConnectionFailureExceptions &)             = delete;
    AndroidConnectionFailureExceptions(const AndroidConnectionFailureExceptions &&)            = delete;
    AndroidConnectionFailureExceptions & operator=(const AndroidConnectionFailureExceptions &) = delete;

    static AndroidConnectionFailureExceptions & GetInstance()
    {
        static AndroidConnectionFailureExceptions androidConnectionFailureExceptions;
        return androidConnectionFailureExceptions;
    }

    /**
     * Creates a Java ConnectionFailureException object in outEx.
     */
    CHIP_ERROR CreateAndroidConnectionFailureException(JNIEnv * env, const char * message, uint32_t errorCode,
                                                       SessionEstablishmentStage state, jthrowable & outEx);

private:
    AndroidConnectionFailureExceptions() {}
};
} // namespace chip
