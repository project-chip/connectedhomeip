/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Declaration of DevicePairingDelegate of CHIP Device Controller for Python
 *
 */

#pragma once

#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningWindowOpener.h>
#include <controller/python/chip/native/PyChipError.h>

namespace chip {
namespace Controller {

extern "C" {
typedef void (*DevicePairingDelegate_OnPairingCompleteFunct)(PyChipError err);
typedef void (*DevicePairingDelegate_OnCommissioningCompleteFunct)(NodeId nodeId, PyChipError err);
typedef void (*DevicePairingDelegate_OnWindowOpenCompleteFunct)(NodeId nodeId, uint32_t setupPinCode, const char * setupCode,
                                                                PyChipError err);

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
    ScriptDevicePairingDelegate();
    ~ScriptDevicePairingDelegate() = default;
    void SetKeyExchangeCallback(DevicePairingDelegate_OnPairingCompleteFunct callback);
    void SetCommissioningCompleteCallback(DevicePairingDelegate_OnCommissioningCompleteFunct callback);
    void SetCommissioningStatusUpdateCallback(DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback);
    void SetCommissioningSuccessCallback(DevicePairingDelegate_OnCommissioningSuccessFunct callback);
    void SetCommissioningFailureCallback(DevicePairingDelegate_OnCommissioningFailureFunct callback);
    void SetCommissioningWindowOpenCallback(DevicePairingDelegate_OnWindowOpenCompleteFunct callback);
    void OnStatusUpdate(Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err) override;
    void OnCommissioningSuccess(PeerId peerId) override;
    void OnCommissioningFailure(PeerId peerId, CHIP_ERROR error, CommissioningStage stageFailed,
                                Optional<Credentials::AttestationVerificationResult> additionalErrorInfo) override;
    void OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error) override;
    Callback::Callback<Controller::OnOpenCommissioningWindow> *
    GetOpenWindowCallback(Controller::CommissioningWindowOpener * context);
    void OnOpenCommissioningWindow(NodeId deviceId, CHIP_ERROR status, SetupPayload payload);
    void SetExpectingPairingComplete(bool value) { expectingPairingComplete = value; }

private:
    DevicePairingDelegate_OnPairingCompleteFunct mOnPairingCompleteCallback                     = nullptr;
    DevicePairingDelegate_OnCommissioningCompleteFunct mOnCommissioningCompleteCallback         = nullptr;
    DevicePairingDelegate_OnWindowOpenCompleteFunct mOnWindowOpenCompleteCallback               = nullptr;
    DevicePairingDelegate_OnCommissioningSuccessFunct mOnCommissioningSuccessCallback           = nullptr;
    DevicePairingDelegate_OnCommissioningFailureFunct mOnCommissioningFailureCallback           = nullptr;
    DevicePairingDelegate_OnCommissioningStatusUpdateFunct mOnCommissioningStatusUpdateCallback = nullptr;
    Callback::Callback<Controller::OnOpenCommissioningWindow> mOpenWindowCallback;
    Controller::CommissioningWindowOpener * mWindowOpener = nullptr;
    bool expectingPairingComplete                         = false;
};

} // namespace Controller
} // namespace chip
