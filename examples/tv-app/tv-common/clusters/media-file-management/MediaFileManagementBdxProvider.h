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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#include <string>
#include <unordered_map>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

/**
 * BDX sender for the tv-app's GetSharedFile flow.
 *
 * When a client pulls a shared file (or its thumbnail), it opens a
 * receiver-drive BDX transfer to this device using a `bdx://` file designator
 * that we handed out in the GetSharedFileResponse ImageUri. This Responder waits
 * for that incoming ReceiveInit, validates the designator was shared with the
 * requesting node, and streams the on-disk blob back block by block.
 *
 * Modeled on the OTA provider's BdxOtaSender. A single instance serves one
 * transfer at a time; it registers as the unsolicited BDX handler.
 */
class MediaFileManagementBdxProvider : public chip::bdx::Responder
{
public:
    MediaFileManagementBdxProvider() = default;

    /**
     * Authorize `designator` to be served to `peer` from the file at `path`.
     * Called by the coordinator when a file is shared (RequestSharedFiles) so a
     * later GetSharedFile pull for the same designator can be satisfied.
     */
    void Allow(ScopedNodeId peer, std::string designator, std::string path);

    /// Forget every designator authorized for `peer` (e.g. on fabric removal).
    void Revoke(ScopedNodeId peer);

    /**
     * Arm the TransferSession to wait for an incoming receiver-drive transfer
     * request and start the poll timer. Must be called (once idle) before the
     * client opens its BDX pull; safe to call repeatedly.
     */
    CHIP_ERROR Arm();

    void AbortTransfer();

private:
    struct Grant
    {
        ScopedNodeId peer;
        std::string path;
    };

    // Inherited from bdx::TransferFacilitator.
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void Reset();

    // Designators currently authorized for pull, keyed by the designator string.
    // NOTE: a production multi-peer server should key on the (ScopedNodeId,
    // designator) pair so two peers sharing an identically-named file do not
    // overwrite each other's grant. This example serves one transfer at a time
    // and arms each grant immediately before its pull, so the collision window
    // is not exercised; the designator-only key is kept for brevity.
    std::unordered_map<std::string, Grant> mGrants;

    // Null-terminated designator for the transfer in progress.
    char mFileDesignator[chip::bdx::kMaxFileDesignatorLen] = {};
    // Path of the blob being served for the active transfer.
    std::string mActivePath;
    uint64_t mNumBytesSent = 0;
};

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
