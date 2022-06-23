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
#include <credentials/PersistentStorageOpCertStore.h>
#include <lib/support/TimeUtils.h>
#include <platform/android/CHIPP256KeypairBridge.h>
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

    /**
     * Returns a CHIPP256KeypairBridge which can be used to delegate signing operations
     * to a KeypairDelegate in the Java layer. Note that this will always return a pointer
     * to the same instance, once initialized.
     */
    CHIPP256KeypairBridge * GetP256KeypairBridge()
    {
        if (mKeypairBridge == nullptr)
        {
            mKeypairBridge = chip::Platform::New<CHIPP256KeypairBridge>();
        }
        return mKeypairBridge;
    }

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

    /**
     * Initializes a new CHIPDeviceController using the given parameters, and returns a pointer to the
     * AndroidDeviceControllerWrapper that holds the underlying controller.
     *
     * If the keypairDelegate is provided, then the rootCertificate, nodeOperationalCertificate, and
     * ipkEpochKey must also be specified. If no operational credentials are specified here, then an
     * ephemeral signing configuration will be generated for you.
     *
     * If there are any errors during the initialization of this controller, then a nullptr will be
     * returned.
     *
     * @param[in] vm the JavaVM
     * @param[in] deviceControllerObj a reference to the Java ChipDeviceController
     * @param[in] nodeId the local node ID to use for this controller instance
     * @param[in] cats the set of CASE authenticated tags
     * @param[in] systemLayer a pointer to the System::Layer instance
     * @param[in] tcpEndpointManager a pointer to a Inet::EndPointManager for TCP connections
     * @param[in] udpEndpointManager a pointer to a Inet::EndPointManager for UDP connections
     * @param[in] opCredsIssuer a pointer to an issuer for Android operational credentials
     * @param[in] keypairDelegate a pointer to a Java KeypairDelegate implementation.
     * @param[in] rootCertificate an X.509 DER-encoded trusted root certificate for this node
     * @param[in] intermediateCertificate an X.509 DER-encoded intermediate certificate for this node
     * @param[in] nodeOperationalCertificate an X.509 DER-encoded operational certificate for this node
     * @param[in] ipkEpochKey the IPK epoch key to use for this node
     * @param[in] listenPort the UDP port to listen on
     * @param[out] errInfoOnFailure a pointer to a CHIP_ERROR that will be populated if this method returns nullptr
     */
    static AndroidDeviceControllerWrapper * AllocateNew(JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId,
                                                        const chip::CATValues & cats, chip::System::Layer * systemLayer,
                                                        chip::Inet::EndPointManager<chip::Inet::TCPEndPoint> * tcpEndPointManager,
                                                        chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager,
                                                        AndroidOperationalCredentialsIssuerPtr opCredsIssuer,
                                                        jobject keypairDelegate, jbyteArray rootCertificate,
                                                        jbyteArray intermediateCertificate, jbyteArray nodeOperationalCertificate,
                                                        jbyteArray ipkEpochKey, uint16_t listenPort, CHIP_ERROR * errInfoOnFailure);

private:
    using ChipDeviceControllerPtr = std::unique_ptr<chip::Controller::DeviceCommissioner>;

    ChipDeviceControllerPtr mController;
    AndroidOperationalCredentialsIssuerPtr mOpCredsIssuer;
    // TODO: This may need to be injected as a GroupDataProvider*
    chip::Credentials::GroupDataProviderImpl mGroupDataProvider;
    // TODO: This may need to be injected as an OperationalCertificateStore *
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;

    JavaVM * mJavaVM                       = nullptr;
    jobject mJavaObjectRef                 = nullptr;
    CHIPP256KeypairBridge * mKeypairBridge = nullptr;

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
