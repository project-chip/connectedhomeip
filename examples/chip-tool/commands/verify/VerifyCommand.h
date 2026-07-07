/*
 *   Copyright (c) 2026 Project CHIP Authors
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

#include "../common/CHIPCommand.h"
#include <controller/CommissioningDelegate.h>
#include <controller/CurrentFabricRemover.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

#include <optional>
#include <thread>

enum class VerifyMode
{
    None,
    Code,
    CodePaseOnly,
    Ble,
    SoftAP,
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    WiFiPAF,
#endif
    AlreadyDiscovered,
    AlreadyDiscoveredByIndex,
    AlreadyDiscoveredByIndexWithCode,
    OnNetwork,
    Nfc,
#if CHIP_SUPPORT_THREAD_MESHCOP
    ThreadMeshcop,
#endif
};

enum class VerifyNetworkType
{
    None,
    WiFi,
    Thread,
    WiFiOrThread,
};

class VerifyCommand : public CHIPCommand,
                      public chip::Controller::DevicePairingDelegate,
                      public chip::Controller::DeviceDiscoveryDelegate,
                      public chip::Credentials::DeviceAttestationDelegate
{
public:
    VerifyCommand(const char * commandName, CredentialIssuerCommands * credIssuerCmds) : CHIPCommand(commandName, credIssuerCmds)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("payload", &mOnboardingPayload);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR Run() override;
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(120); };

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;
    void OnICDRegistrationComplete(chip::ScopedNodeId deviceId, uint32_t icdCounter) override;
    void OnICDStayActiveComplete(chip::ScopedNodeId deviceId, uint32_t promisedActiveDuration) override;
    void OnCommissioningStageStart(chip::PeerId peerId, chip::Controller::CommissioningStage stageStarting) override;
    CHIP_ERROR WiFiCredentialsNeeded(chip::EndpointId endpoint) override;
    CHIP_ERROR ThreadCredentialsNeeded(chip::EndpointId endpoint) override;

    /////////// DeviceDiscoveryDelegate Interface /////////
    void OnDiscoveredDevice(const chip::Dnssd::CommissionNodeData & nodeData) override;

    /////////// DeviceAttestationDelegate /////////
    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override;
    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
                                      const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                      chip::Credentials::AttestationVerificationResult attestationResult) override;
    bool ShouldWaitAfterDeviceAttestation() override;

private:
    void PrintDeviceInformation();
    void PrintCert(const char * name, chip::ByteSpan & buffer);

    // Device attestation information
    chip::ByteSpan mDacDerBuffer;
    chip::ByteSpan mPaiDerBuffer;
    chip::ByteSpan mCdBuffer;
    uint16_t mVendorId;
    uint16_t mProductId;
    chip::Credentials::AttestationVerificationResult mAttestationResult;

    NodeId mNodeId            = chip::kUndefinedNodeId;
    char * mOnboardingPayload = nullptr;
};
