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
#include <support/TimeUtils.h>

/**
 * This class contains all relevant information for the JNI view of CHIPDeviceController
 * to handle all controller-related processing.
 *
 * Generally it contains the DeviceController class itself, plus any related delegates/callbacks.
 */
class AndroidDeviceControllerWrapper : public chip::Controller::DevicePairingDelegate,
                                       public chip::Controller::DeviceStatusDelegate,
                                       public chip::Controller::OperationalCredentialsDelegate,
                                       public chip::PersistentStorageDelegate
{
public:
    ~AndroidDeviceControllerWrapper();

    chip::Controller::DeviceCommissioner * Controller() { return mController.get(); }
    chip::Controller::ExampleOperationalCredentialsIssuer & OpCredsIssuer() { return mOpCredsIssuer; }
    void SetJavaObjectRef(JavaVM * vm, jobject obj);
    jobject JavaObjectRef() { return mJavaObjectRef; }
    jlong ToJNIHandle();

    void CallJavaMethod(const char * methodName, jint argument);
    CHIP_ERROR Initialize();

    // DevicePairingDelegate implementation
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;

    // OperationalCredentialsDelegate implementation
    CHIP_ERROR
    GenerateNodeOperationalCertificate(const chip::Optional<chip::NodeId> & nodeId, chip::FabricId fabricId,
                                       const chip::ByteSpan & csr, const chip::ByteSpan & DAC,
                                       chip::Callback::Callback<chip::Controller::NOCGenerated> * onNOCGenerated) override;

    CHIP_ERROR GetRootCACertificate(chip::FabricId fabricId, chip::MutableByteSpan & outCert) override;

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
    chip::Crypto::P256Keypair mIssuer;
    bool mInitialized  = false;
    uint32_t mIssuerId = 0;
    uint32_t mNow      = chip::CalendarToChipEpochTime(2021, 06, 10, 0, 0, 0, mNow);
    uint32_t mValidity = 10 * chip::kSecondsPerStandardYear;

    ChipDeviceControllerPtr mController;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;

    pthread_mutex_t * mStackLock;

    JavaVM * mJavaVM       = nullptr;
    jobject mJavaObjectRef = nullptr;

    chip::NodeId mNextAvailableNodeId = 1;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr controller, pthread_mutex_t * stackLock) :
        mController(std::move(controller)), mStackLock(stackLock)
    {}
};

inline jlong AndroidDeviceControllerWrapper::ToJNIHandle()
{
    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
    return reinterpret_cast<jlong>(this);
}
