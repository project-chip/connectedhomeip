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
#include <app/icd/client/DefaultICDClientStorage.h>
#include <controller/python/chip/native/PyChipError.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include <string>

extern chip::app::DefaultICDClientStorage sICDClientStorage;
extern chip::Controller::CommissioningParameters sCommissioningParameters;
extern uint8_t sICDSymmetricKey[chip::Crypto::kAES_CCM128_Key_Length];

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
void ScriptDevicePairingDelegate::SetFabricCheckCallback(DevicePairingDelegate_OnFabricCheckFunct callback)
{
    mOnFabricCheckCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningStatusUpdateCallback(
    DevicePairingDelegate_OnCommissioningStatusUpdateFunct callback)
{
    mOnCommissioningStatusUpdateCallback = callback;
}

void ScriptDevicePairingDelegate::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(Zcl, "Secure Pairing Success");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(Zcl, "Secure Pairing Failed");
        if (mOnPairingCompleteCallback != nullptr && expectingPairingComplete)
        {
            // Incorrect state is the same error that chip-tool sends. We are also
            // leveraging the on pairing complete callback to indicate that pairing
            // has failed.
            expectingPairingComplete = false;
            mOnPairingCompleteCallback(ToPyChipError(CHIP_ERROR_INCORRECT_STATE));
        }
        break;
    }
}

void ScriptDevicePairingDelegate::OnPairingComplete(CHIP_ERROR error)
{
    if (mOnPairingCompleteCallback != nullptr && expectingPairingComplete)
    {
        expectingPairingComplete = false;
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
        std::string setupManualCode;
        std::string setupQRCode;

        ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(setupManualCode);
        QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(setupQRCode);
        ChipLogProgress(Zcl, "SetupManualCode = %s", setupManualCode.c_str());
        ChipLogProgress(Zcl, "SetupQRCode = %s", setupQRCode.c_str());
        mOnWindowOpenCompleteCallback(deviceId, payload.setUpPINCode, setupManualCode.c_str(), setupQRCode.c_str(),
                                      ToPyChipError(status));
    }
    if (mWindowOpener != nullptr)
    {
        Platform::Delete(mWindowOpener);
        mWindowOpener = nullptr;
    }
}

void ScriptDevicePairingDelegate::OnFabricCheck(NodeId matchingNodeId)
{
    if (matchingNodeId == kUndefinedNodeId)
    {
        ChipLogProgress(Zcl, "No matching fabric found");
    }
    else
    {
        ChipLogProgress(Zcl, "Matching fabric found");
    }
    if (mOnFabricCheckCallback != nullptr)
    {
        mOnFabricCheckCallback(matchingNodeId);
    }
}

Callback::Callback<Controller::OnOpenCommissioningWindow> *
ScriptDevicePairingDelegate::GetOpenWindowCallback(Controller::CommissioningWindowOpener * context)
{
    mWindowOpener = context;
    return &mOpenWindowCallback;
}

void ScriptDevicePairingDelegate::OnICDRegistrationComplete(ScopedNodeId nodeId, uint32_t icdCounter)
{
    app::ICDClientInfo clientInfo;
    clientInfo.peer_node     = nodeId;
    clientInfo.check_in_node = chip::ScopedNodeId(sCommissioningParameters.GetICDCheckInNodeId().Value(), nodeId.GetFabricIndex());
    clientInfo.monitored_subject = sCommissioningParameters.GetICDMonitoredSubject().Value();
    clientInfo.start_icd_counter = icdCounter;
    clientInfo.client_type       = sCommissioningParameters.GetICDClientType().Value();

    CHIP_ERROR err = sICDClientStorage.SetKey(clientInfo, ByteSpan(sICDSymmetricKey));
    if (err == CHIP_NO_ERROR)
    {
        err = sICDClientStorage.StoreEntry(clientInfo);
    }

    if (err != CHIP_NO_ERROR)
    {
        sICDClientStorage.RemoveKey(clientInfo);
        ChipLogError(Controller, "Failed to persist symmetric key for " ChipLogFormatX64 ": %s",
                     ChipLogValueX64(nodeId.GetNodeId()), err.AsString());
        return;
    }

    ChipLogProgress(Controller, "Saved ICD Symmetric key for " ChipLogFormatX64, ChipLogValueX64(nodeId.GetNodeId()));
    ChipLogProgress(Controller,
                    "ICD Registration Complete for device " ChipLogFormatX64 " / Check-In NodeID: " ChipLogFormatX64
                    " / Monitored Subject: " ChipLogFormatX64 " / ICDCounter %u",
                    ChipLogValueX64(nodeId.GetNodeId()), ChipLogValueX64(sCommissioningParameters.GetICDCheckInNodeId().Value()),
                    ChipLogValueX64(clientInfo.monitored_subject), icdCounter);
}

void ScriptDevicePairingDelegate::OnICDStayActiveComplete(ScopedNodeId deviceId, uint32_t promisedActiveDuration)
{
    ChipLogProgress(Controller, "ICD Stay Active Complete for device " ChipLogFormatX64 " / promisedActiveDuration: %u",
                    ChipLogValueX64(deviceId.GetNodeId()), promisedActiveDuration);
}

} // namespace Controller
} // namespace chip
