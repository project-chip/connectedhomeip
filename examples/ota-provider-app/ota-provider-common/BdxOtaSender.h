/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#include <ios>
#include <string>
#include <unordered_map>

#pragma once

class BdxOtaSender : public chip::bdx::Responder
{
public:
    BdxOtaSender();

    // Initializes BDX transfer-related metadata. Should always be called first.
    CHIP_ERROR InitializeTransfer(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

    void AbortTransfer();

    // Same as Responder::PrepareForTransfer(), and remembers the arguments so that a transfer the requestor ends can be armed again
    // for the same requestor, see Reset().
    CHIP_ERROR PrepareForTransfer(chip::System::Layer * layer, chip::bdx::TransferRole role,
                                  chip::BitFlags<chip::bdx::TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                  chip::System::Clock::Timeout timeout, chip::System::Clock::Timeout pollFreq);

    void SetFileDesignatorMap(const std::unordered_map<std::string, std::string> & map) { mFileDesignatorMap = map; }

protected:
    // Inherited from bdx::TransferFacilitator
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                 chip::System::PacketBufferHandle && payload) override final;

    // Size in bytes of the image at path, or -1 when it cannot be opened or sized, e.g. when it names a directory.
    virtual std::streamoff ImageSize(const std::string & path);

private:
    // Inherited from bdx::TransferFacilitator
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    // Ends the transfer. When rearm is set, the sender stays armed for the same requestor, so that a requestor that restarts a
    // rejected or aborted download from offset 0 right away is served without another QueryImage.
    void Reset(bool rearm = false);

    // Null-terminated string representing file designator
    char mFileDesignator[chip::bdx::kMaxFileDesignatorLen];
    std::unordered_map<std::string, std::string> mFileDesignatorMap;
    uint32_t mNumBytesSent = 0;

    bool mInitialized = false;
    // Armed again by Reset() for a requestor that may restart its download, and not yet used by it. A QueryImage from any
    // requestor takes over a sender in this state.
    bool mRearmed = false;
    // Set when the StatusReport about to be sent rejects the requested start offset; the sender arms again once it is sent.
    bool mRearmAfterStatusReport = false;

    chip::Optional<chip::FabricIndex> mFabricIndex;

    chip::Optional<chip::NodeId> mNodeId;

    struct TransferParams
    {
        chip::System::Layer * layer  = nullptr;
        chip::bdx::TransferRole role = chip::bdx::TransferRole::kSender;
        chip::BitFlags<chip::bdx::TransferControlFlags> controlOpts;
        uint16_t maxBlockSize                 = 0;
        chip::System::Clock::Timeout timeout  = chip::System::Clock::kZero;
        chip::System::Clock::Timeout pollFreq = chip::System::Clock::kZero;
    };
    TransferParams mParams;
};
