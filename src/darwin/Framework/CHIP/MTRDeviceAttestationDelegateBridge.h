/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/core/NodeId.h>
#include <platform/CHIPDeviceBuildConfig.h>

#include <Matter/MTRDeviceAttestationDelegate.h>
#include <credentials/attestation_verifier/DeviceAttestationDelegate.h>

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

class MTRDeviceAttestationDelegateBridge : public chip::Credentials::DeviceAttestationDelegate {
public:
    MTRDeviceAttestationDelegateBridge(MTRDeviceController * deviceController,
        id<MTRDeviceAttestationDelegate> deviceAttestationDelegate, chip::Optional<uint16_t> expiryTimeoutSecs,
        bool shouldWaitAfterDeviceAttestation = false)
        : mResult(chip::Credentials::AttestationVerificationResult::kSuccess)
        , mDeviceController(deviceController)
        , mDeviceAttestationDelegate(deviceAttestationDelegate)
        , mQueue(dispatch_queue_create(
              "org.csa-iot.matter.framework.device_attestation.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL))
        , mExpiryTimeoutSecs(expiryTimeoutSecs)
        , mShouldWaitAfterDeviceAttestation(shouldWaitAfterDeviceAttestation)
    {
    }

    ~MTRDeviceAttestationDelegateBridge() {}

    chip::Optional<uint16_t> FailSafeExpiryTimeoutSecs() const override { return mExpiryTimeoutSecs; }

    void OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
        const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
        chip::Credentials::AttestationVerificationResult attestationResult) override;

    bool ShouldWaitAfterDeviceAttestation() override { return mShouldWaitAfterDeviceAttestation; }

    chip::Credentials::AttestationVerificationResult attestationVerificationResult() const { return mResult; }

private:
    chip::Credentials::AttestationVerificationResult mResult;
    MTRDeviceController * __weak mDeviceController;
    id<MTRDeviceAttestationDelegate> mDeviceAttestationDelegate;
    dispatch_queue_t mQueue;
    chip::Optional<uint16_t> mExpiryTimeoutSecs;
    const bool mShouldWaitAfterDeviceAttestation;
};

NS_ASSUME_NONNULL_END
