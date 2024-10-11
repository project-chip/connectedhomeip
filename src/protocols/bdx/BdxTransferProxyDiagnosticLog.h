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

#include <protocols/bdx/BdxTransferProxy.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/DiagnosticLogs.h>

namespace chip {
namespace bdx {

class BDXTransferProxyDiagnosticLog : public BDXTransferProxy
{
public:
    CHIP_ERROR Init(TransferSession * transferSession);
    void Reset();

    void SetFabricIndex(FabricIndex fabricIndex) { mFabricIndex = fabricIndex; }
    void SetPeerNodeId(NodeId nodeId) { mPeerNodeId = nodeId; }

    CHIP_ERROR Accept() override;
    CHIP_ERROR Reject(CHIP_ERROR error) override;
    CHIP_ERROR Continue() override;

    FabricIndex GetFabricIndex() const override { return mFabricIndex; }
    NodeId GetPeerNodeId() const override { return mPeerNodeId; }
    CharSpan GetFileDesignator() const override { return CharSpan(mFileDesignator, mFileDesignatorLen); };

private:
    CHIP_ERROR EnsureState() const;

    uint8_t mFileDesignatorLen                                  = 0;
    char mFileDesignator[DiagnosticLogs::kMaxFileDesignatorLen] = {};
    TransferSession * mTransfer                                 = nullptr;
    FabricIndex mFabricIndex                                    = kUndefinedFabricIndex;
    NodeId mPeerNodeId                                          = kUndefinedNodeId;
};

} // namespace bdx
} // namespace chip
