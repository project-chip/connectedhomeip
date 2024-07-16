/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "MyUserPrompter-JNI.h"
#include <jni.h>
#include <lib/support/JniReferences.h>

class TvAppJNI
{
public:
    void InitializeWithObjects(jobject app);
    void PostClusterInit(int clusterId, int endpoint);
    void InitializeCommissioner(JNIMyUserPrompter * userPrompter);

private:
    friend TvAppJNI & TvAppJNIMgr();

    static TvAppJNI sInstance;
    chip::JniGlobalReference mTvAppObject;
    jmethodID mPostClusterInitMethod = nullptr;
};

inline class TvAppJNI & TvAppJNIMgr()
{
    return TvAppJNI::sInstance;
}
