/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

#include <memory>

#include <jni.h>

#include <controller/CHIPDeviceController.h>

#include "AndroidDevicePairingDelegate.h"

class AndroidDeviceControllerWrapper
{
public:
    ~AndroidDeviceControllerWrapper();

    chip::DeviceController::ChipDeviceController * Controller() { return mController.get(); }

    jlong toJNIHandle()
    {
        static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
        return reinterpret_cast<jlong>(this);
    }

    static AndroidDeviceControllerWrapper * fromJNIHandle(jlong handle)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(handle);
    }

    static AndroidDeviceControllerWrapper * allocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                        chip::Inet::InetLayer * inetLayer, CHIP_ERROR * errInfoOnFailure);

private:
    using ChipDeviceControllerPtr         = std::unique_ptr<chip::DeviceController::ChipDeviceController>;
    using AndroidDevicePairingDelegatePtr = std::unique_ptr<AndroidDevicePairingDelegate>;

    ChipDeviceControllerPtr mController;
    AndroidDevicePairingDelegatePtr mDelegate;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr && controller, AndroidDevicePairingDelegatePtr && delegate) :
        mController(std::move(controller)), mDelegate(std::move(delegate))
    {}
};
