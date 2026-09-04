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

#include "MediaFileManagementBdxCoordinator.h"

#include <app/server/Server.h>
#include <credentials/FabricTable.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/bdx/BdxUri.h>

#include <string>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

CHIP_ERROR MediaFileManagementBdxCoordinator::GetSelfNodeId(FabricIndex fabricIndex, NodeId & nodeId) const
{
    const FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    nodeId = fabric->GetNodeId();
    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaFileManagementBdxCoordinator::StartIncomingFileTransfer(ScopedNodeId peer, uint64_t fileID, CharSpan fileName,
                                                                        uint64_t size, CharSpan thumbnailUri)
{
    // The file data is transferred under a designator equal to the file's Name
    // (per the spec usage examples); pull it into the file's on-disk blob.
    //
    // NOTE: The thumbnail (thumbnailUri) would be fetched with a second BDX
    // transfer keyed by the parsed bdx:// designator. Omitted here because the
    // example manager keeps a single blob per file.
    const std::string designator(fileName.data(), fileName.size());
    const std::string targetPath = mManager.DataFilePathForFile(fileID);
    return mRequestor.StartDownload(peer, designator, targetPath, size);
}

CHIP_ERROR MediaFileManagementBdxCoordinator::ShareFileWithClient(ScopedNodeId peer, uint16_t requestID, uint64_t fileID,
                                                                  CharSpan fileName)
{
    // Arm the sender to serve this file's blob under a designator equal to its
    // Name to the requesting node.
    const std::string designator(fileName.data(), fileName.size());
    mProvider.Allow(peer, designator, mManager.DataFilePathForFile(fileID));

    // Allocate a ResponseID the client will use in GetSharedFile.
    const uint16_t responseID = mNextResponseID++;
    mShared[responseID]       = SharedEntry{ peer, fileID };

    mCluster.GenerateSharedFilesAddedEvent(requestID, responseID);
    return CHIP_NO_ERROR;
}

bool MediaFileManagementBdxCoordinator::LookupSharedFile(ScopedNodeId peer, uint16_t responseID, uint64_t & fileID)
{
    const auto entry = mShared.find(responseID);
    VerifyOrReturnValue(entry != mShared.cend() && entry->second.peer == peer, false);
    fileID = entry->second.fileID;
    return true;
}

CHIP_ERROR MediaFileManagementBdxCoordinator::MakeSelfBdxUri(uint64_t fileID, CharSpan designator, MutableCharSpan & out)
{
    // Build bdx://<own-node-id-on-peer-fabric>/<designator>. A shared file is
    // bound to a single peer, so resolve our node-id on that peer's fabric.
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    for (const auto & kv : mShared)
    {
        if (kv.second.fileID == fileID)
        {
            fabricIndex = kv.second.peer.GetFabricIndex();
            break;
        }
    }
    VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_NOT_FOUND);

    NodeId selfNodeId = kUndefinedNodeId;
    ReturnErrorOnFailure(GetSelfNodeId(fabricIndex, selfNodeId));
    ReturnErrorOnFailure(bdx::MakeURI(selfNodeId, designator, out));

    // Arm the sender so it is waiting when the client opens its BDX pull.
    ReturnErrorOnFailure(mProvider.Arm());
    return CHIP_NO_ERROR;
}

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
