/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BdxDiagnosticLogsReceiver.h"

#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Controller {

using namespace ::chip::DeviceLayer;

BdxDiagnosticLogsReceiver::BdxDiagnosticLogsReceiver(Callback::Callback<OnBdxTransfer> * onTransfer,
                                                     Callback::Callback<OnBdxTransferSuccess> * onSuccess,
                                                     Callback::Callback<OnBdxTransferFailure> * onFailure,
                                                     chip::FabricIndex fabricIndex, chip::NodeId nodeId,
                                                     chip::CharSpan fileDesignator)
{
    mOnBdxTransferCallback        = onTransfer;
    mOnBdxTransferSuccessCallback = onSuccess;
    mOnBdxTransferFailureCallback = onFailure;

    mFabricIndex    = fabricIndex;
    mNodeId         = nodeId;
    mFileDesignator = fileDesignator;
}

CHIP_ERROR BdxDiagnosticLogsReceiver::OnTransferBegin(chip::bdx::BDXTransferProxy * transfer)
{
    VerifyOrReturnError(transfer != nullptr, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(Controller, "transfer is nullptr."));

    chip::CharSpan fileDesignator = transfer->GetFileDesignator();
    chip::FabricIndex fabricIndex = transfer->GetFabricIndex();
    chip::NodeId nodeId           = transfer->GetPeerNodeId();

    if (mFileDesignator.data_equal(fileDesignator) && mFabricIndex == fabricIndex && mNodeId == nodeId)
    {
        transfer->Accept();
    }
    else
    {
        transfer->Reject(CHIP_ERROR_INVALID_DESTINATION_NODE_ID);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxDiagnosticLogsReceiver::OnTransferEnd(chip::bdx::BDXTransferProxy * transfer, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "OnTransferEnd: %" CHIP_ERROR_FORMAT, error.Format());
    chip::FabricIndex fabricIndex = transfer->GetFabricIndex();
    chip::NodeId nodeId           = transfer->GetPeerNodeId();
    if (error == CHIP_NO_ERROR)
    {
        mOnBdxTransferSuccessCallback->mCall(mOnBdxTransferCallback->mContext, fabricIndex, nodeId);
    }
    else
    {
        mOnBdxTransferFailureCallback->mCall(mOnBdxTransferCallback->mContext, fabricIndex, nodeId, error);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxDiagnosticLogsReceiver::OnTransferData(chip::bdx::BDXTransferProxy * transfer, const chip::ByteSpan & data)
{
    ChipLogProgress(Controller, "OnTransferData");
    chip::FabricIndex fabricIndex = transfer->GetFabricIndex();
    chip::NodeId nodeId           = transfer->GetPeerNodeId();

    CHIP_ERROR err = CHIP_NO_ERROR;

    mOnBdxTransferCallback->mCall(mOnBdxTransferCallback->mContext, fabricIndex, nodeId, data, &err);

    if (err == CHIP_NO_ERROR)
    {
        transfer->Continue();
    }
    else
    {
        transfer->Reject(err);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxDiagnosticLogsReceiver::StartBDXTransferTimeout(uint16_t timeoutInSeconds)
{
    ChipLogProgress(Controller, "StartBDXTransferTimeout %u", timeoutInSeconds);
    return chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeoutInSeconds), OnTransferTimeout,
                                                       static_cast<void *>(this));
}

void BdxDiagnosticLogsReceiver::CancelBDXTransferTimeout()
{
    ChipLogProgress(Controller, "CancelBDXTransferTimeout");
    chip::DeviceLayer::SystemLayer().CancelTimer(OnTransferTimeout, static_cast<void *>(this));
}

void BdxDiagnosticLogsReceiver::OnTransferTimeout(chip::System::Layer * layer, void * context)
{
    ChipLogProgress(Controller, "OnTransferTimeout");
    auto * self = static_cast<BdxDiagnosticLogsReceiver *>(context);
    self->mOnBdxTransferFailureCallback->mCall(self->mOnBdxTransferFailureCallback->mContext, self->mFabricIndex, self->mNodeId,
                                               CHIP_ERROR_TIMEOUT);
}
} // namespace Controller
} // namespace chip
