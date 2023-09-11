/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

class DeviceAppJNI
{
public:
    void InitializeWithObjects(jobject app);
    void PostClusterInit(int clusterId, int endpoint);
    void PostEvent(int event);

private:
    friend DeviceAppJNI & DeviceAppJNIMgr();

    static DeviceAppJNI sInstance;
    jobject mDeviceAppObject         = nullptr;
    jmethodID mPostClusterInitMethod = nullptr;
    jmethodID mPostEventMethod       = nullptr;
};

inline class DeviceAppJNI & DeviceAppJNIMgr()
{
    return DeviceAppJNI::sInstance;
}
