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

#pragma once

class BdxOtaSender : public chip::bdx::Responder
{
public:
    BdxOtaSender();

    // Initializes BDX transfer-related metadata. Should always be called first.
    CHIP_ERROR InitializeTransfer(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

private:
    // Inherited from bdx::TransferFacilitator
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void Reset();

    // Null-terminated string representing file designator
    char mFileDesignator[chip::bdx::kMaxFileDesignatorLen];

    uint32_t mNumBytesSent = 0;

    bool mInitialized = false;

    chip::Optional<chip::FabricIndex> mFabricIndex;

    chip::Optional<chip::NodeId> mNodeId;
};
