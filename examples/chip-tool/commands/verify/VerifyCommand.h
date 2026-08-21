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
#include <credentials/CertificationDeclaration.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

#include <optional>
#include <thread>

class VerifyCommand : public CHIPCommand,
                      public chip::Controller::DevicePairingDelegate,
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

    /////////// DeviceAttestationDelegate /////////
    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override;
    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
                                      const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                      chip::Credentials::AttestationVerificationResult attestationResult) override;
    bool ShouldWaitAfterDeviceAttestation() override;

private:
    void PrintDeviceInformation();
    void PrintCert(const char * name, chip::ByteSpan buffer);

    CHIP_ERROR SetBuffer(chip::Platform::ScopedMemoryBufferWithSize<uint8_t> & buf, const chip::ByteSpan & span)
    {
        VerifyOrReturnError(buf.Alloc(span.size()), CHIP_ERROR_NO_MEMORY);
        memcpy(buf.Get(), span.data(), span.size());
        return CHIP_NO_ERROR;
    }

    chip::Optional<chip::ByteSpan> GetBuffer(const chip::Platform::ScopedMemoryBufferWithSize<uint8_t> & buf) const
    {
        if (!buf.Get())
        {
            return chip::Optional<chip::ByteSpan>();
        }
        return chip::MakeOptional(chip::ByteSpan(buf.Get(), buf.AllocatedSize()));
    }

    chip::Optional<chip::ByteSpan> DacDerBuffer() const { return GetBuffer(mDacCertBuf); }
    chip::Optional<chip::ByteSpan> PaiDerBuffer() const { return GetBuffer(mPaiCertBuf); }
    chip::Optional<chip::ByteSpan> CdBuffer() const { return GetBuffer(mCdBuf); }
    chip::Optional<chip::ByteSpan> PaaDerBuffer() const { return GetBuffer(mPaaCertBuf); }

    // Device attestation information
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> mDacCertBuf;
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> mPaiCertBuf;
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> mCdBuf;
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> mPaaCertBuf;
    uint16_t mVendorId  = 0;
    uint16_t mProductId = 0;
    chip::Credentials::AttestationVerificationResult mAttestationResult =
        chip::Credentials::AttestationVerificationResult::kInternalError;

    NodeId mNodeId            = chip::kUndefinedNodeId;
    char * mOnboardingPayload = nullptr;
};
