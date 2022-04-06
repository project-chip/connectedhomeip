/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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

#include <lib/support/JniReferences.h>

#include <memory>

#include <jni.h>

#include <controller/CHIPDeviceController.h>
#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/TimeUtils.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include "AndroidOperationalCredentialsIssuer.h"

/**
 * This class contains all relevant information for the JNI view of CHIPDeviceController
 * to handle all controller-related processing.
 *
 * Generally it contains the DeviceController class itself, plus any related delegates/callbacks.
 */
class AndroidDeviceControllerWrapper : public chip::Controller::DevicePairingDelegate, public chip::PersistentStorageDelegate
{
public:
    ~AndroidDeviceControllerWrapper();

    chip::Controller::DeviceCommissioner * Controller() { return mController.get(); }
    void SetJavaObjectRef(JavaVM * vm, jobject obj);
    jobject JavaObjectRef() { return mJavaObjectRef; }
    jlong ToJNIHandle();

    void CallJavaMethod(const char * methodName, jint argument);
    CHIP_ERROR InitializeOperationalCredentialsIssuer();

    /**
     * Convert network credentials from Java, and apply them to the commissioning parameters object.
     */
    CHIP_ERROR ApplyNetworkCredentials(chip::Controller::CommissioningParameters & params, jobject networkCredentials);

    // DevicePairingDelegate implementation
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;

    // PersistentStorageDelegate implementation
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    static AndroidDeviceControllerWrapper * FromJNIHandle(jlong handle)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(handle);
    }

    using AndroidOperationalCredentialsIssuerPtr = std::unique_ptr<chip::Controller::AndroidOperationalCredentialsIssuer>;

    static AndroidDeviceControllerWrapper * AllocateNew(JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId,
                                                        const chip::CATValues & cats, chip::System::Layer * systemLayer,
                                                        chip::Inet::EndPointManager<chip::Inet::TCPEndPoint> * tcpEndPointManager,
                                                        chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager,
                                                        AndroidOperationalCredentialsIssuerPtr opCredsIssuer,
                                                        CHIP_ERROR * errInfoOnFailure);

private:
    using ChipDeviceControllerPtr = std::unique_ptr<chip::Controller::DeviceCommissioner>;

    ChipDeviceControllerPtr mController;
    AndroidOperationalCredentialsIssuerPtr mOpCredsIssuer;
    // TODO: This may need to be injected as a GroupDataProvider*
    chip::Credentials::GroupDataProviderImpl mGroupDataProvider;

    JavaVM * mJavaVM       = nullptr;
    jobject mJavaObjectRef = nullptr;

    // These fields allow us to release the string/byte array memory later.
    jstring ssidStr                    = nullptr;
    jstring passwordStr                = nullptr;
    const char * ssid                  = nullptr;
    const char * password              = nullptr;
    jbyteArray operationalDatasetBytes = nullptr;
    jbyte * operationalDataset         = nullptr;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr controller, AndroidOperationalCredentialsIssuerPtr opCredsIssuer) :
        mController(std::move(controller)), mOpCredsIssuer(std::move(opCredsIssuer))
    {}
};

inline jlong AndroidDeviceControllerWrapper::ToJNIHandle()
{
    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
    return reinterpret_cast<jlong>(this);
}
