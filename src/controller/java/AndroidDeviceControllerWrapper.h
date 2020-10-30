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

#include <AndroidDevicePairingDelegate.h>
#include <controller/CHIPDeviceController.h>

class AndroidDeviceControllerWrapper
{
public:
    ~AndroidDeviceControllerWrapper();

    chip::DeviceController::ChipDeviceController * Controller() { return mController.get(); }

    jlong toJNIHandle()
    {
        static_assert(sizeog(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
        return static_cast<jlong>(this);
    }

    static AndroidDeviceControllerWrapper * fromJNIHandle(jlong handle)
    {
        return static_cast<AndroidDeviceControllerWrapper *>(handle);
    }

    static AndroidDeviceControllerWrapper * allocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                        chip::Inet::InetLayer * inetLayer, CHIP_ERROR * errInfoOnFailure);

private:
    using ChipDeviceControllerPtr          = std::unique_ptr<chip::DeviceController::ChipDeviceController>;
    using AndroidBleApplicationDelegatePtr = std::unique_ptr<AndroidBleApplicationDelegate>;

    ChipDeviceControllerPtr mController;
    AndroidBleApplicationDelegatePtr mDelegate;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr && controller, AndroidBleApplicationDelegatePtr && delegate) :
        mController(controller), mDelegate(delegate)
    {}
};
