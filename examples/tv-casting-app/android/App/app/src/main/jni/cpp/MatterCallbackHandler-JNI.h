/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <jni.h>
#include <lib/core/CHIPError.h>

class MatterCallbackHandlerJNI
{
public:
    CHIP_ERROR SetUp(JNIEnv * env, jobject inHandler);

    void Handle(CHIP_ERROR err);

private:
    jobject mObject   = nullptr;
    jclass mClazz     = nullptr;
    jmethodID mMethod = nullptr;
};
