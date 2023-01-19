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

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "lib/support/TypeTraits.h"
#include <controller/python/chip/native/PyChipError.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

namespace chip {
namespace Controller {

namespace {
void OnWindowCompleteStatic(void * context, NodeId deviceId, CHIP_ERROR status, SetupPayload payload)
{
    auto self = reinterpret_cast<ScriptDevicePairingDelegate *>(context);
    self->OnOpenCommissioningWindow(deviceId, status, payload);
}
} // namespace

ScriptDevicePairingDelegate::ScriptDevicePairingDelegate() : mOpenWindowCallback(OnWindowCompleteStatic, this) {}

void ScriptDevicePairingDelegate::SetKeyExchangeCallback(DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    mOnPairingCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningCompleteCallback(DevicePairingDelegate_OnCommissioningCompleteFunct callback)
{
    mOnCommissioningCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningWindowOpenCallback(DevicePairingDelegate_OnWindowOpenCompleteFunct callback)
{
    mOnWindowOpenCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningSuccessCallback(DevicePairingDelegate_OnCommissioningSuccessFunct callback)
{
    mOnCommissioningSuccessCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningFailureCallback(DevicePairingDelegate_OnCommissioningFailureFunct callback)
{
    mOnCommissioningFailureCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningStatusUpdateCallback(
    DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback)
{
    mOnCommissioningStatusUpdateCallback = callback;
}

void ScriptDevicePairingDelegate::OnPairingComplete(CHIP_ERROR error)
{
    if (mOnPairingCompleteCallback != nullptr)
    {
        mOnPairingCompleteCallback(ToPyChipError(error));
    }
}

void ScriptDevicePairingDelegate::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR error)
{
    if (mOnCommissioningCompleteCallback != nullptr)
    {
        mOnCommissioningCompleteCallback(nodeId, ToPyChipError(error));
    }
}

void ScriptDevicePairingDelegate::OnCommissioningSuccess(PeerId peerId)
{
    if (mOnCommissioningSuccessCallback != nullptr)
    {
        mOnCommissioningSuccessCallback(peerId);
    }
}

void ScriptDevicePairingDelegate::OnCommissioningFailure(PeerId peerId, CHIP_ERROR error, CommissioningStage stageFailed,
                                                         Optional<Credentials::AttestationVerificationResult> additionalErrorInfo)
{
    if (mOnCommissioningFailureCallback != nullptr)
    {
        mOnCommissioningFailureCallback(peerId, error, stageFailed, additionalErrorInfo);
    }
}

void ScriptDevicePairingDelegate::OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error)
{
    if (mOnCommissioningStatusUpdateCallback != nullptr)
    {
        mOnCommissioningStatusUpdateCallback(peerId, stageCompleted, error);
    }
}

void ScriptDevicePairingDelegate::OnOpenCommissioningWindow(NodeId deviceId, CHIP_ERROR status, SetupPayload payload)
{
    if (mOnWindowOpenCompleteCallback != nullptr)
    {
        QRCodeSetupPayloadGenerator generator(payload);
        std::string code;
        generator.payloadBase38Representation(code);
        ChipLogProgress(Zcl, "code = %s", code.c_str());
        mOnWindowOpenCompleteCallback(deviceId, payload.setUpPINCode, code.c_str(), ToPyChipError(status));
    }
    if (mWindowOpener != nullptr)
    {
        Platform::Delete(mWindowOpener);
        mWindowOpener = nullptr;
    }
}
Callback::Callback<Controller::OnOpenCommissioningWindow> *
ScriptDevicePairingDelegate::GetOpenWindowCallback(Controller::CommissioningWindowOpener * context)
{
    mWindowOpener = context;
    return &mOpenWindowCallback;
}

} // namespace Controller
} // namespace chip
