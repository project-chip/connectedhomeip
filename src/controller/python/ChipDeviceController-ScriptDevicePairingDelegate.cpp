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

namespace chip {
namespace Controller {

void ScriptDevicePairingDelegate::SetKeyExchangeCallback(DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    mOnPairingCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::SetCommissioningCompleteCallback(DevicePairingDelegate_OnCommissioningCompleteFunct callback)
{
    mOnCommissioningCompleteCallback = callback;
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
        mOnPairingCompleteCallback(error.AsInteger());
    }
}

void ScriptDevicePairingDelegate::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR error)
{
    if (mOnCommissioningCompleteCallback != nullptr)
    {
        mOnCommissioningCompleteCallback(nodeId, error.AsInteger());
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

} // namespace Controller
} // namespace chip
