/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "lib/support/logging/CHIPLogging.h"
#include <jni.h>

class DeviceCallbacks
{

public:
    static void NewManager(jobject manager);
    void OnPlatformEvent(const chip::DeviceLayer::ChipDeviceEvent * event);
    void InitializeWithObjects(jobject manager);

private:
    jobject mProvider                      = nullptr;
    jmethodID mCommissioningCompleteMethod = nullptr;
    void OnCommissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event);
};
