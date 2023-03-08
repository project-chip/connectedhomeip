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
#include <vector>

#include <jni.h>

#include <controller/CHIPDeviceController.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
#include <crypto/RawKeySessionKeystore.h>
#include <lib/support/TimeUtils.h>
#include <platform/internal/DeviceNetworkInfo.h>

#ifdef JAVA_MATTER_CONTROLLER_TEST
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <controller/ExamplePersistentStorage.h>
#else
#include <platform/android/AndroidChipPlatform-JNI.h>
#include <platform/android/CHIPP256KeypairBridge.h>
#endif // JAVA_MATTER_CONTROLLER_TEST

#include "AndroidOperationalCredentialsIssuer.h"
#include "AttestationTrustStoreBridge.h"
#include "DeviceAttestationDelegateBridge.h"

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

#ifndef JAVA_MATTER_CONTROLLER_TEST
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
#endif // JAVA_MATTER_CONTROLLER_TEST

    void CallJavaMethod(const char * methodName, jint argument);
    CHIP_ERROR InitializeOperationalCredentialsIssuer();

    /**
     * Convert network credentials from Java, and apply them to the commissioning parameters object.
     */
    CHIP_ERROR ApplyNetworkCredentials(chip::Controller::CommissioningParameters & params, jobject networkCredentials);

    /**
     * Update the CommissioningParameters used by the active device commissioner
     */
    CHIP_ERROR UpdateCommissioningParameters(const chip::Controller::CommissioningParameters & params);

    // DevicePairingDelegate implementation
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;
    void OnCommissioningStatusUpdate(chip::PeerId peerId, chip::Controller::CommissioningStage stageCompleted,
                                     CHIP_ERROR error) override;
    void OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) override;
    void OnScanNetworksSuccess(
        const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & dataResponse) override;
    void OnScanNetworksFailure(CHIP_ERROR error) override;

    // PersistentStorageDelegate implementation
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    chip::Controller::AutoCommissioner * GetAutoCommissioner() { return &mAutoCommissioner; }

    chip::Credentials::PartialDACVerifier * GetPartialDACVerifier() { return &mPartialDACVerifier; }

    const chip::Controller::CommissioningParameters & GetCommissioningParameters() const
    {
        return mAutoCommissioner.GetCommissioningParameters();
    }

    static AndroidDeviceControllerWrapper * FromJNIHandle(jlong handle)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(handle);
    }

#ifdef JAVA_MATTER_CONTROLLER_TEST
    using ExampleOperationalCredentialsIssuerPtr = std::unique_ptr<chip::Controller::ExampleOperationalCredentialsIssuer>;
#else
    using AndroidOperationalCredentialsIssuerPtr = std::unique_ptr<chip::Controller::AndroidOperationalCredentialsIssuer>;
#endif

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
     * @param[in] controllerVendorId the vendor ID identifying the controller
     * @param[in] failsafeTimerSeconds the failsafe timer in seconds
     * @param[in] attemptNetworkScanWiFi whether to attempt a network scan when configuring the network for a WiFi device
     * @param[in] attemptNetworkScanThread whether to attempt a network scan when configuring the network for a Thread device
     * @param[in] skipCommissioningComplete whether to skip the CASE commissioningComplete command during commissioning
     * @param[out] errInfoOnFailure a pointer to a CHIP_ERROR that will be populated if this method returns nullptr
     */
    static AndroidDeviceControllerWrapper *
    AllocateNew(JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId, chip::FabricId fabricId,
                const chip::CATValues & cats, chip::System::Layer * systemLayer,
                chip::Inet::EndPointManager<chip::Inet::TCPEndPoint> * tcpEndPointManager,
                chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager,
#ifdef JAVA_MATTER_CONTROLLER_TEST
                ExampleOperationalCredentialsIssuerPtr opCredsIssuer,
#else
                AndroidOperationalCredentialsIssuerPtr opCredsIssuer,
#endif
                jobject keypairDelegate, jbyteArray rootCertificate, jbyteArray intermediateCertificate,
                jbyteArray nodeOperationalCertificate, jbyteArray ipkEpochKey, uint16_t listenPort, uint16_t controllerVendorId,
                uint16_t failsafeTimerSeconds, bool attemptNetworkScanWiFi, bool attemptNetworkScanThread,
                bool skipCommissioningComplete, CHIP_ERROR * errInfoOnFailure);

#ifdef JAVA_MATTER_CONTROLLER_TEST
    chip::Controller::ExampleOperationalCredentialsIssuer * GetAndroidOperationalCredentialsIssuer()
#else
    chip::Controller::AndroidOperationalCredentialsIssuer * GetAndroidOperationalCredentialsIssuer()
#endif
    {
        return mOpCredsIssuer.get();
    }

    DeviceAttestationDelegateBridge * GetDeviceAttestationDelegateBridge() { return mDeviceAttestationDelegateBridge; }

    CHIP_ERROR UpdateDeviceAttestationDelegateBridge(jobject deviceAttestationDelegate, chip::Optional<uint16_t> expiryTimeoutSecs,
                                                     bool shouldWaitAfterDeviceAttestation);

    CHIP_ERROR UpdateAttestationTrustStoreBridge(jobject attestationTrustStoreDelegate);

private:
    using ChipDeviceControllerPtr = std::unique_ptr<chip::Controller::DeviceCommissioner>;

    ChipDeviceControllerPtr mController;

    // TODO: This may need to be injected as a GroupDataProvider*
    chip::Credentials::GroupDataProviderImpl mGroupDataProvider;
    // TODO: This may need to be injected as an OperationalCertificateStore *
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
    // TODO: This may need to be injected as a SessionKeystore*
    chip::Crypto::RawKeySessionKeystore mSessionKeystore;

    JavaVM * mJavaVM       = nullptr;
    jobject mJavaObjectRef = nullptr;
#ifdef JAVA_MATTER_CONTROLLER_TEST
    ExampleOperationalCredentialsIssuerPtr mOpCredsIssuer;
    PersistentStorage mExampleStorage;
#else
    AndroidOperationalCredentialsIssuerPtr mOpCredsIssuer;
    CHIPP256KeypairBridge * mKeypairBridge = nullptr;
#endif // JAVA_MATTER_CONTROLLER_TEST

    // These fields allow us to release the string/byte array memory later.
    jstring ssidStr                    = nullptr;
    jstring passwordStr                = nullptr;
    const char * ssid                  = nullptr;
    const char * password              = nullptr;
    jbyteArray operationalDatasetBytes = nullptr;
    jbyte * operationalDataset         = nullptr;

    std::vector<uint8_t> mNocCertificate;
    std::vector<uint8_t> mIcacCertificate;
    std::vector<uint8_t> mRcacCertificate;

    chip::Controller::AutoCommissioner mAutoCommissioner;

    chip::Credentials::PartialDACVerifier mPartialDACVerifier;

    DeviceAttestationDelegateBridge * mDeviceAttestationDelegateBridge        = nullptr;
    AttestationTrustStoreBridge * mAttestationTrustStoreBridge                = nullptr;
    chip::Credentials::DeviceAttestationVerifier * mDeviceAttestationVerifier = nullptr;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr controller,
#ifdef JAVA_MATTER_CONTROLLER_TEST
                                   ExampleOperationalCredentialsIssuerPtr opCredsIssuer
#else
                                   AndroidOperationalCredentialsIssuerPtr opCredsIssuer
#endif
                                   ) :
        mController(std::move(controller)),
        mOpCredsIssuer(std::move(opCredsIssuer))
    {}
};

inline jlong AndroidDeviceControllerWrapper::ToJNIHandle()
{
    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
    return reinterpret_cast<jlong>(this);
}
