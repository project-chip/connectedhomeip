/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
