/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <commands/common/CredentialIssuerCommands.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <controller/CommissioningWindowOpener.h>
#include <controller/CurrentFabricRemover.h>
#include <crypto/CHIPCryptoPAL.h>

// Constants
constexpr uint16_t kMaxManualCodeLength = 22;

class CommissioningWindowDelegate
{
public:
    virtual void OnCommissioningWindowOpened(chip::NodeId deviceId, CHIP_ERROR err, chip::SetupPayload payload) = 0;
    virtual ~CommissioningWindowDelegate()                                                                      = default;
};

class CommissioningDelegate
{
public:
    virtual void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err) = 0;
    virtual ~CommissioningDelegate()                                            = default;
};

class PairingDelegate
{
public:
    virtual void OnDeviceRemoved(chip::NodeId deviceId, CHIP_ERROR err) = 0;
    virtual ~PairingDelegate()                                          = default;
};

/**
 * The PairingManager class is responsible for managing the commissioning and pairing process
 * of Matter devices. PairingManager is designed to be used as a singleton, meaning that there
 * should only be one instance of it running at any given time.
 *
 * Usage:
 *
 * 1. The class should be initialized when the system starts up, typically by invoking the static
 *    instance method to get the singleton.
 * 2. To open a commissioning window, the appropriate method should be called on the PairingManager instance.
 * 3. The PairingManager will handle the lifecycle of the CommissioningWindowOpener and ensure that
 *    resources are cleaned up appropriately when pairing is complete or the process is aborted.
 *
 * Example:
 *
 * @code
 * PairingManager& manager = PairingManager::Instance();
 * manager.OpenCommissioningWindow();
 * @endcode
 */
class PairingManager : public chip::Controller::DevicePairingDelegate,
                       public chip::Controller::DeviceDiscoveryDelegate,
                       public chip::Credentials::DeviceAttestationDelegate
{
public:
    static PairingManager & Instance()
    {
        static PairingManager instance;
        return instance;
    }

    CHIP_ERROR Init(chip::Controller::DeviceCommissioner * commissioner, CredentialIssuerCommands * credIssuerCmds);

    void SetOpenCommissioningWindowDelegate(CommissioningWindowDelegate * delegate) { mCommissioningWindowDelegate = delegate; }
    void SetCommissioningDelegate(CommissioningDelegate * delegate) { mCommissioningDelegate = delegate; }
    void SetPairingDelegate(PairingDelegate * delegate) { mPairingDelegate = delegate; }
    PairingDelegate * GetPairingDelegate() { return mPairingDelegate; }

    chip::Controller::DeviceCommissioner & CurrentCommissioner() { return *mCommissioner; };

    /**
     * Opens a commissioning window on the specified node and endpoint.
     * Only one commissioning window can be active at a time. If a commissioning
     * window is already open, this function will return an error.
     *
     * @param nodeId The target node ID for commissioning.
     * @param endpointId The target endpoint ID for commissioning.
     * @param commissioningTimeoutSec Timeout for the commissioning window in seconds.
     * @param iterations Iterations for PBKDF calculations.
     * @param discriminator Discriminator for commissioning.
     * @param salt Optional salt for verifier-based commissioning.
     * @param verifier Optional verifier for enhanced commissioning security.
     *
     * @return CHIP_ERROR_INCORRECT_STATE if a commissioning window is already open.
     */
    CHIP_ERROR OpenCommissioningWindow(chip::NodeId nodeId, chip::EndpointId endpointId, uint16_t commissioningTimeoutSec,
                                       uint32_t iterations, uint16_t discriminator, const chip::ByteSpan & salt,
                                       const chip::ByteSpan & verifier);

    /**
     * Pairs a device using a setup code payload.
     *
     * @param nodeId The target node ID for pairing.
     * @param payload The setup code payload, which typically contains device-specific pairing information.
     *
     * @return CHIP_NO_ERROR on successful initiation of the pairing process, or an appropriate CHIP_ERROR if pairing fails.
     */
    CHIP_ERROR PairDeviceWithCode(chip::NodeId nodeId, const char * payload);

    /**
     * Pairs a device using its setup PIN code and remote IP address.
     *
     * @param nodeId The target node ID for pairing.
     * @param setupPINCode The setup PIN code for the device, used for establishing a secure connection.
     * @param deviceRemoteIp The IP address of the remote device.
     * @param deviceRemotePort The port number on which the device is listening for pairing requests.
     *
     * @return CHIP_NO_ERROR if the pairing process is initiated successfully, or an appropriate CHIP_ERROR if pairing fails.
     */
    CHIP_ERROR PairDevice(chip::NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp, uint16_t deviceRemotePort);

    /**
     * Unpairs a device with the specified node ID.
     *
     * @param nodeId The node ID of the device to be unpaired.
     *
     * @return CHIP_NO_ERROR if the device is successfully unpaired, or an appropriate CHIP_ERROR if the process fails.
     */
    CHIP_ERROR UnpairDevice(chip::NodeId nodeId);

private:
    struct CommissioningWindowParams
    {
        chip::NodeId nodeId;
        chip::EndpointId endpointId;
        uint16_t commissioningWindowTimeout;
        uint32_t iteration;
        uint16_t discriminator;
        chip::Optional<uint32_t> setupPIN;
        uint8_t verifierBuffer[chip::Crypto::kSpake2p_VerifierSerialized_Length];
        chip::ByteSpan verifier;
        uint8_t saltBuffer[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length];
        chip::ByteSpan salt;
    };

    struct PairDeviceWithCodeParams
    {
        chip::NodeId nodeId;
        char payloadBuffer[kMaxManualCodeLength + 1];
    };

    struct PairDeviceParams
    {
        chip::NodeId nodeId;
        uint32_t setupPINCode;
        uint16_t deviceRemotePort;
        char ipAddrBuffer[chip::Inet::IPAddress::kMaxStringLength];
    };

    struct UnpairDeviceParams
    {
        chip::NodeId nodeId;
    };

    // Constructors
    PairingManager();
    PairingManager(const PairingManager &)             = delete;
    PairingManager & operator=(const PairingManager &) = delete;

    // Private member functions (static and non-static)
    chip::Controller::CommissioningParameters GetCommissioningParameters();
    void InitPairingCommand();
    CHIP_ERROR Pair(chip::NodeId remoteId, chip::Transport::PeerAddress address);

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) override;
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;
    void OnICDRegistrationComplete(chip::ScopedNodeId deviceId, uint32_t icdCounter) override;
    void OnICDStayActiveComplete(chip::ScopedNodeId deviceId, uint32_t promisedActiveDuration) override;

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::CommissionNodeData & nodeData) override;

    /////////// DeviceAttestationDelegate Interface /////////
    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override;
    bool ShouldWaitAfterDeviceAttestation() override;
    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
                                      const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                      chip::Credentials::AttestationVerificationResult attestationResult) override;

    static void OnOpenCommissioningWindow(intptr_t context);
    static void OnOpenCommissioningWindowResponse(void * context, chip::NodeId deviceId, CHIP_ERROR status,
                                                  chip::SetupPayload payload);
    static void OnOpenCommissioningWindowVerifierResponse(void * context, chip::NodeId deviceId, CHIP_ERROR status);
    static void OnCurrentFabricRemove(void * context, chip::NodeId remoteNodeId, CHIP_ERROR status);
    static void OnPairDeviceWithCode(intptr_t context);
    static void OnPairDevice(intptr_t context);
    static void OnUnpairDevice(intptr_t context);

    // Private data members
    chip::Controller::DeviceCommissioner * mCommissioner = nullptr;
    CredentialIssuerCommands * mCredIssuerCmds           = nullptr;

    CommissioningWindowDelegate * mCommissioningWindowDelegate = nullptr;
    CommissioningDelegate * mCommissioningDelegate             = nullptr;
    PairingDelegate * mPairingDelegate                         = nullptr;

    chip::NodeId mNodeId            = chip::kUndefinedNodeId;
    uint16_t mDiscriminator         = 0;
    uint32_t mSetupPINCode          = 0;
    const char * mOnboardingPayload = nullptr;
    bool mDeviceIsICD               = false;
    uint8_t mRandomGeneratedICDSymmetricKey[chip::Crypto::kAES_CCM128_Key_Length];

    chip::Optional<bool> mICDRegistration;
    chip::Optional<chip::NodeId> mICDCheckInNodeId;
    chip::Optional<chip::app::Clusters::IcdManagement::ClientTypeEnum> mICDClientType;
    chip::Optional<chip::ByteSpan> mICDSymmetricKey;
    chip::Optional<uint64_t> mICDMonitoredSubject;
    chip::Optional<uint32_t> mICDStayActiveDurationMsec;

    /**
     * Holds the unique_ptr to the current CommissioningWindowOpener.
     * Only one commissioning window opener can be active at a time.
     * The pointer is reset when the commissioning window is closed or when an error occurs.
     */
    chip::Platform::UniquePtr<chip::Controller::CommissioningWindowOpener> mWindowOpener;
    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindow> mOnOpenCommissioningWindowCallback;
    chip::Callback::Callback<chip::Controller::OnOpenCommissioningWindowWithVerifier> mOnOpenCommissioningWindowVerifierCallback;

    // For Unpair
    chip::Platform::UniquePtr<chip::Controller::CurrentFabricRemover> mCurrentFabricRemover;
    chip::Callback::Callback<chip::Controller::OnCurrentFabricRemove> mCurrentFabricRemoveCallback;
};
