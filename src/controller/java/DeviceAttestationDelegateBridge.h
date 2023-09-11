/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <jni.h>
#include <lib/core/NodeId.h>
#include <platform/CHIPDeviceBuildConfig.h>

#include <credentials/attestation_verifier/DeviceAttestationDelegate.h>

class DeviceAttestationDelegateBridge : public chip::Credentials::DeviceAttestationDelegate
{
public:
    DeviceAttestationDelegateBridge(jobject deviceAttestationDelegate, chip::Optional<uint16_t> expiryTimeoutSecs,
                                    bool shouldWaitAfterDeviceAttestation) :
        mResult(chip::Credentials::AttestationVerificationResult::kSuccess),
        mDeviceAttestationDelegate(deviceAttestationDelegate), mExpiryTimeoutSecs(expiryTimeoutSecs),
        mShouldWaitAfterDeviceAttestation(shouldWaitAfterDeviceAttestation)
    {}

    ~DeviceAttestationDelegateBridge();

    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override { return mExpiryTimeoutSecs; }

    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
                                      const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                      chip::Credentials::AttestationVerificationResult attestationResult) override;

    bool ShouldWaitAfterDeviceAttestation() override { return mShouldWaitAfterDeviceAttestation; }

    chip::Credentials::AttestationVerificationResult attestationVerificationResult() const { return mResult; }

private:
    chip::Credentials::AttestationVerificationResult mResult;
    jobject mDeviceAttestationDelegate = nullptr;
    chip::Optional<uint16_t> mExpiryTimeoutSecs;
    const bool mShouldWaitAfterDeviceAttestation;
};
