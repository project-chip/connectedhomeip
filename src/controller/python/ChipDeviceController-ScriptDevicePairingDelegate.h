/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Declaration of DevicePairingDelegate of CHIP Device Controller for Python
 *
 */

#pragma once

#include <controller/CHIPDeviceController.h>
#include <controller/python/chip/native/PyChipError.h>

namespace chip {
namespace Controller {

extern "C" {
typedef void (*DevicePairingDelegate_OnPairingCompleteFunct)(PyChipError err);
typedef void (*DevicePairingDelegate_OnCommissioningCompleteFunct)(NodeId nodeId, PyChipError err);

// Used for testing by OpCredsBinding
typedef void (*DevicePairingDelegate_OnCommissioningSuccessFunct)(PeerId peerId);
typedef void (*DevicePairingDelegate_OnCommissioningFailureFunct)(
    PeerId peerId, CHIP_ERROR err, chip::Controller::CommissioningStage stageFailed,
    chip::Optional<chip::Credentials::AttestationVerificationResult> attestationResult);
typedef void (*DevicePairingDelegate_OnCommissioningStatusUpdateFunct)(PeerId peerId,
                                                                       chip::Controller::CommissioningStage stageCompleted,
                                                                       CHIP_ERROR err);
}

class ScriptDevicePairingDelegate final : public Controller::DevicePairingDelegate
{
public:
    ~ScriptDevicePairingDelegate() = default;
    void SetKeyExchangeCallback(DevicePairingDelegate_OnPairingCompleteFunct callback);
    void SetCommissioningCompleteCallback(DevicePairingDelegate_OnCommissioningCompleteFunct callback);
    void SetCommissioningStatusUpdateCallback(DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback);
    void SetCommissioningSuccessCallback(DevicePairingDelegate_OnCommissioningSuccessFunct callback);
    void SetCommissioningFailureCallback(DevicePairingDelegate_OnCommissioningFailureFunct callback);
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err) override;
    void OnCommissioningSuccess(PeerId peerId) override;
    void OnCommissioningFailure(PeerId peerId, CHIP_ERROR error, CommissioningStage stageFailed,
                                Optional<Credentials::AttestationVerificationResult> additionalErrorInfo) override;
    void OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error) override;

private:
    DevicePairingDelegate_OnPairingCompleteFunct mOnPairingCompleteCallback                     = nullptr;
    DevicePairingDelegate_OnCommissioningCompleteFunct mOnCommissioningCompleteCallback         = nullptr;
    DevicePairingDelegate_OnCommissioningSuccessFunct mOnCommissioningSuccessCallback           = nullptr;
    DevicePairingDelegate_OnCommissioningFailureFunct mOnCommissioningFailureCallback           = nullptr;
    DevicePairingDelegate_OnCommissioningStatusUpdateFunct mOnCommissioningStatusUpdateCallback = nullptr;
};

} // namespace Controller
} // namespace chip
