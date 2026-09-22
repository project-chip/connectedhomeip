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

#include "MediaFileManagementBdxProvider.h"
#include "MediaFileManagementBdxRequestor.h"
#include "MediaFileManagementManager.h"

#include <app/clusters/media-file-management-server/MediaFileManagementCluster.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>

#include <cstdint>
#include <unordered_map>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

/**
 * Concrete BdxCoordinator for the Linux tv-app.
 *
 * Ties the metadata-only MediaFileManagementManager to the two BDX endpoints:
 *   - MediaFileManagementBdxRequestor (receiver) for AddFile / OfferFile, and
 *   - MediaFileManagementBdxProvider (sender) for GetSharedFile.
 *
 * It owns the RequestID/ResponseID bookkeeping for the sharing flow and emits
 * SharedFilesAdded events through the cluster. By spec convention the BDX file
 * designator for the file data itself is the file's Name; the thumbnail uses
 * the explicit `bdx://` ImageUri.
 */
class MediaFileManagementBdxCoordinator : public BdxCoordinator
{
public:
    MediaFileManagementBdxCoordinator(MediaFileManagementManager & manager, MediaFileManagementBdxProvider & provider,
                                      MediaFileManagementBdxRequestor & requestor, MediaFileManagementCluster & cluster) :
        mManager(manager),
        mProvider(provider), mRequestor(requestor), mCluster(cluster)
    {}

    CHIP_ERROR StartIncomingFileTransfer(ScopedNodeId peer, uint64_t fileID, CharSpan fileName, uint64_t size,
                                         CharSpan thumbnailUri) override;
    CHIP_ERROR ShareFileWithClient(ScopedNodeId peer, uint16_t requestID, uint64_t fileID, CharSpan fileName) override;
    bool LookupSharedFile(ScopedNodeId peer, uint16_t responseID, uint64_t & fileID) override;
    CHIP_ERROR MakeSelfBdxUri(uint64_t fileID, CharSpan designator, MutableCharSpan & out) override;

private:
    struct SharedEntry
    {
        ScopedNodeId peer;
        uint64_t fileID;
    };

    // Resolve this device's own operational node-id on the peer's fabric.
    CHIP_ERROR GetSelfNodeId(FabricIndex fabricIndex, NodeId & nodeId) const;

    MediaFileManagementManager & mManager;
    MediaFileManagementBdxProvider & mProvider;
    MediaFileManagementBdxRequestor & mRequestor;
    MediaFileManagementCluster & mCluster;

    // ResponseID -> shared file, handed out by ShareFileWithClient and resolved
    // by LookupSharedFile.
    std::unordered_map<uint16_t, SharedEntry> mShared;
    uint16_t mNextResponseID = 1;
};

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
