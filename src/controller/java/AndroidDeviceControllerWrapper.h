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

#include "JniReferences.h"

#include <memory>

#include <jni.h>

#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <platform/internal/DeviceNetworkInfo.h>

/**
 * This class contains all relevant information for the JNI view of CHIPDeviceController
 * to handle all controller-related processing.
 *
 * Generally it contains the DeviceController class itself, plus any related delegates/callbacks.
 */
class AndroidDeviceControllerWrapper : public chip::Controller::DevicePairingDelegate,
                                       public chip::Controller::DeviceStatusDelegate,
                                       public chip::PersistentStorageDelegate
{
public:
    ~AndroidDeviceControllerWrapper();

    // Use StackUnlockGuard to temporarily unlock the CHIP BLE stack, e.g. when calling application
    // or Android BLE code as a result of a BLE event.
    struct StackUnlockGuard
    {
    public:
        StackUnlockGuard(pthread_mutex_t * mutex) : mMutex(mutex) { pthread_mutex_unlock(mMutex); }
        ~StackUnlockGuard() { pthread_mutex_lock(mMutex); }

    private:
        pthread_mutex_t * mMutex;
    };

    chip::Controller::DeviceCommissioner * Controller() { return mController.get(); }
    chip::Controller::ExampleOperationalCredentialsIssuer & OpCredsIssuer() { return mOpCredsIssuer; }
    void SetJavaObjectRef(JavaVM * vm, jobject obj);
    jobject JavaObjectRef() { return mJavaObjectRef; }
    jlong ToJNIHandle();

    void CallJavaMethod(const char * methodName, jint argument);

    // DevicePairingDelegate implementation
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;

    // DeviceStatusDelegate implementation
    void OnMessage(chip::System::PacketBufferHandle && msg) override;
    void OnStatusChange(void) override;

    // PersistentStorageDelegate implementation
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    static AndroidDeviceControllerWrapper * FromJNIHandle(jlong handle)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(handle);
    }

    static AndroidDeviceControllerWrapper * AllocateNew(JavaVM * vm, jobject deviceControllerObj, pthread_mutex_t * stackLock,
                                                        chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                        chip::Inet::InetLayer * inetLayer, CHIP_ERROR * errInfoOnFailure);

private:
    using ChipDeviceControllerPtr = std::unique_ptr<chip::Controller::DeviceCommissioner>;

    ChipDeviceControllerPtr mController;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;

    pthread_mutex_t * mStackLock;

    JavaVM * mJavaVM       = nullptr;
    jobject mJavaObjectRef = nullptr;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr controller, pthread_mutex_t * stackLock) :
        mController(std::move(controller)), mStackLock(stackLock)
    {}
};

inline jlong AndroidDeviceControllerWrapper::ToJNIHandle()
{
    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
    return reinterpret_cast<jlong>(this);
}
