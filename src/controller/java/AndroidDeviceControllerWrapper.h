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

#include <controller/CHIPDeviceController_deprecated.h>
#include <platform/internal/DeviceNetworkInfo.h>

/**
 * This class contains all relevant information for the JNI view of CHIPDeviceController
 * to handle all controller-related processing.
 *
 * Generally it contains the DeviceController class itself, plus any related delegates/callbacks.
 */
class AndroidDeviceControllerWrapper : public chip::Controller::DevicePairingDelegate
{
public:
    ~AndroidDeviceControllerWrapper();

    chip::DeviceController::ChipDeviceController * Controller() { return mController.get(); }
    void SetJavaObjectRef(JavaVM * vm, jobject obj);

    void SendNetworkCredentials(const char * ssid, const char * password);
    void SendThreadCredentials(const chip::DeviceLayer::Internal::DeviceNetworkInfo & threadData);

    // DevicePairingDelegate implementation
    void OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback) override;
    void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                           chip::RendezvousDeviceCredentialsDelegate * callback) override;
    void OnStatusUpdate(chip::RendezvousSessionDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;

    jlong ToJNIHandle()
    {
        static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
        return reinterpret_cast<jlong>(this);
    }

    jobject JavaObjectRef() { return mJavaObjectRef; }

    static AndroidDeviceControllerWrapper * FromJNIHandle(jlong handle)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(handle);
    }

    static AndroidDeviceControllerWrapper * FromController(chip::DeviceController::ChipDeviceController * controller)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(controller->AppState);
    }

    static AndroidDeviceControllerWrapper * AllocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                        chip::Inet::InetLayer * inetLayer, CHIP_ERROR * errInfoOnFailure);

private:
    using ChipDeviceControllerPtr = std::unique_ptr<chip::DeviceController::ChipDeviceController>;

    ChipDeviceControllerPtr mController;
    chip::RendezvousDeviceCredentialsDelegate * mCredentialsDelegate = nullptr;

    JavaVM * mJavaVM       = nullptr;
    jobject mJavaObjectRef = nullptr;

    JNIEnv * GetJavaEnv();

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr controller) : mController(std::move(controller))
    {
        mController->AppState = reinterpret_cast<void *>(this);
    }
};
