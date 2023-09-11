/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "MyUserPrompter-JNI.h"
#include <jni.h>

class TvAppJNI
{
public:
    void InitializeWithObjects(jobject app);
    void PostClusterInit(int clusterId, int endpoint);
    void InitializeCommissioner(JNIMyUserPrompter * userPrompter);

private:
    friend TvAppJNI & TvAppJNIMgr();

    static TvAppJNI sInstance;
    jobject mTvAppObject             = nullptr;
    jmethodID mPostClusterInitMethod = nullptr;
};

inline class TvAppJNI & TvAppJNIMgr()
{
    return TvAppJNI::sInstance;
}
