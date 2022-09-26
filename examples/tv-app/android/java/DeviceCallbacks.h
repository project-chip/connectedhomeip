/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
