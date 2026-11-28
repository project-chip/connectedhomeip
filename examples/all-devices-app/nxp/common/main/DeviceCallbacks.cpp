/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright 2026 NXP
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

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack.
 * In a code-driven application the cluster attribute changes are handled
 * inside the cluster objects themselves, so this file only provides the
 * common device manager callbacks required by the NXP AppTaskBase.
 *
 **/
#include "DeviceCallbacks.h"

using namespace ::chip;
using namespace ::chip::DeviceLayer;

// This returns an instance of this class.
AllDevicesApp::DeviceCallbacks & AllDevicesApp::DeviceCallbacks::GetDefaultInstance()
{
    static AllDevicesApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::NXP::App::GetDeviceCallbacks()
{
    return AllDevicesApp::DeviceCallbacks::GetDefaultInstance();
}
