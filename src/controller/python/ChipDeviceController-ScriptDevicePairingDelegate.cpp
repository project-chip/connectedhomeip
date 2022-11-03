/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "lib/support/TypeTraits.h"
#include <controller/python/chip/native/PyChipError.h>

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

} // namespace Controller
} // namespace chip
