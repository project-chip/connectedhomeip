/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#pragma once

#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxTransferProxy.h>
#include <protocols/bdx/BdxTransferServerDelegate.h>
#include <protocols/bdx/BdxUri.h>
#include <protocols/bdx/TransferFacilitator.h>

namespace chip {
namespace Controller {

typedef void (*OnBdxTransfer)(void * context, FabricIndex fabricIndex, NodeId remoteNodeId, const chip::ByteSpan & data,
                              CHIP_ERROR * errInfoOnFailure);
typedef void (*OnBdxTransferSuccess)(void * context, FabricIndex fabricIndex, NodeId remoteNodeId);
typedef void (*OnBdxTransferFailure)(void * context, FabricIndex fabricIndex, NodeId remoteNodeId, CHIP_ERROR status);

constexpr uint32_t kMaxBDXReceiverURILen = 256;

class BdxDiagnosticLogsReceiver : public chip::bdx::BDXTransferServerDelegate
{
public:
    BdxDiagnosticLogsReceiver(Callback::Callback<OnBdxTransfer> * onTransfer, Callback::Callback<OnBdxTransferSuccess> * onSuccess,
                              Callback::Callback<OnBdxTransferFailure> * onFailure, chip::FabricIndex fabricIndex,
                              chip::NodeId nodeId, chip::CharSpan fileDesignator);

    ~BdxDiagnosticLogsReceiver() {}

    /////////// BdxDiagnosticLogsReceiver Interface /////////
    CHIP_ERROR OnTransferBegin(chip::bdx::BDXTransferProxy * transfer) override;
    CHIP_ERROR OnTransferEnd(chip::bdx::BDXTransferProxy * transfer, CHIP_ERROR error) override;
    CHIP_ERROR OnTransferData(chip::bdx::BDXTransferProxy * transfer, const chip::ByteSpan & data) override;

    CHIP_ERROR StartBDXTransferTimeout(uint16_t timeoutInSeconds);
    void CancelBDXTransferTimeout();

private:
    static void OnTransferTimeout(chip::System::Layer * layer, void * context);

    chip::Callback::Callback<OnBdxTransfer> * mOnBdxTransferCallback;
    chip::Callback::Callback<OnBdxTransferSuccess> * mOnBdxTransferSuccessCallback;
    chip::Callback::Callback<OnBdxTransferFailure> * mOnBdxTransferFailureCallback;

    chip::FabricIndex mFabricIndex = kUndefinedFabricIndex;
    chip::NodeId mNodeId           = kUndefinedNodeId;
    chip::CharSpan mFileDesignator;
};
} // namespace Controller
} // namespace chip
