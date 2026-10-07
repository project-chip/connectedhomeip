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

namespace matter {
namespace casting {
namespace core {

/**
 * BDX sender for the casting client's AddFile / OfferFile flows.
 *
 * When the client adds or offers a file to a Media Device, the tv-app pulls the
 * bytes from this client: it opens a receiver-drive BDX transfer using the file
 * designator the client referenced in the command. This Responder waits for that
 * incoming ReceiveInit, validates the designator was armed for the requesting
 * node, and streams the on-disk blob back block by block.
 *
 * Mirror of the tv-app's MediaFileManagementBdxProvider (roles reversed). A
 * single instance serves one transfer at a time; it registers as the unsolicited
 * BDX handler.
 */
class MediaFileManagementBdxServer : public chip::bdx::Responder
{
public:
    MediaFileManagementBdxServer() = default;

    /**
     * Authorize `designator` to be served to `peer` from the file at `path`.
     * Called before invoking AddFile / OfferFile so the tv-app's subsequent BDX
     * pull for the same designator can be satisfied.
     */
    void Allow(chip::ScopedNodeId peer, std::string designator, std::string path);

    /// Forget every designator armed for `peer` (e.g. on fabric removal).
    void Revoke(chip::ScopedNodeId peer);

    /**
     * Arm the TransferSession to wait for an incoming receiver-drive transfer
     * request and start the poll timer. Must be called (once idle) before the
     * tv-app opens its BDX pull; safe to call repeatedly.
     */
    CHIP_ERROR Arm();

    void AbortTransfer();

private:
    struct Grant
    {
        chip::ScopedNodeId peer;
        std::string path;
    };

    // Inherited from bdx::TransferFacilitator.
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void Reset();

    // Designators currently armed for pull, keyed by the designator string.
    std::unordered_map<std::string, Grant> mGrants;

    // Null-terminated designator for the transfer in progress.
    char mFileDesignator[chip::bdx::kMaxFileDesignatorLen] = {};
    // Path of the blob being served for the active transfer.
    std::string mActivePath;
    uint64_t mNumBytesSent = 0;
};

} // namespace core
} // namespace casting
} // namespace matter
