/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
