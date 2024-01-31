/*
 *   Copyright (c) 2023 Project CHIP Authors
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

#include <jni.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxUri.h>
#include <protocols/bdx/TransferFacilitator.h>

constexpr uint32_t kMaxBDXURILen = 256;

class BdxOTASender : public chip::bdx::Responder
{
public:
    BdxOTASender(jobject otaDelegate) : mOtaDelegate(otaDelegate) {}

    ~BdxOTASender() {}

    CHIP_ERROR PrepareForTransfer(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

    CHIP_ERROR Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeMgr);

    CHIP_ERROR Shutdown();

    void ResetState();

private:
    /**
     * Timer callback called when we don't receive a BDX init within a reasonable time after a successful QueryImage response.
     */
    static void HandleBdxInitReceivedTimeoutExpired(chip::System::Layer * systemLayer, void * state)
    {
        VerifyOrReturn(state != nullptr);
        static_cast<BdxOTASender *>(state)->ResetState();
    }

    CHIP_ERROR OnMessageToSend(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnTransferSessionBegin(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnTransferSessionEnd(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnBlockQuery(chip::bdx::TransferSession::OutputEvent & event);

    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    CHIP_ERROR ConfigureState(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

    bool mInitialized                               = false;
    chip::FabricIndex mFabricIndex                  = chip::kUndefinedFabricIndex;
    chip::NodeId mNodeId                            = chip::kUndefinedNodeId;
    chip::Messaging::ExchangeManager * mExchangeMgr = nullptr;

    jobject mOtaDelegate = nullptr;

    // Since we are a singleton, we get reused across transfers, but also have
    // async calls that can happen.  The transfer generation keeps track of
    // which transfer we are currently doing, so we can ignore async calls
    // attached to no-longer-running transfers.
    uint64_t mTransferGeneration = 0;
};
