/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRSessionResumptionStorageBridge.h"

#import "MTRConversion.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"

#include <platform/LockTracker.h>

using namespace chip;

MTRSessionResumptionStorageBridge::MTRSessionResumptionStorageBridge(MTRDeviceControllerFactory * factory)
    : mFactory(factory)
{
}

CHIP_ERROR MTRSessionResumptionStorageBridge::FindByScopedNodeId(const chip::ScopedNodeId & node,
    ResumptionIdStorage & resumptionId, chip::Crypto::P256ECDHDerivedSecret & sharedSecret, chip::CATValues & peerCATs)
{
    assertChipStackLockedByCurrentThread();

    auto * controller = [mFactory runningControllerForFabricIndex:node.GetFabricIndex()];
    if (controller == nil) {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    auto * resumptionInfo = [controller.controllerDataStore findResumptionInfoByNodeID:@(node.GetNodeId())];
    if (resumptionInfo == nil) {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    NodeId ignored;
    return DeconstructResumptionInfo(resumptionInfo, ignored, resumptionId, sharedSecret, peerCATs);
}

CHIP_ERROR MTRSessionResumptionStorageBridge::FindByResumptionId(ConstResumptionIdView resumptionId, chip::ScopedNodeId & node,
    chip::Crypto::P256ECDHDerivedSecret & sharedSecret, chip::CATValues & peerCATs)
{
    assertChipStackLockedByCurrentThread();

    auto * resumptionIDData = AsData(resumptionId);

    auto * controllerList = [mFactory getRunningControllers];
    for (MTRDeviceController * controller in controllerList) {
        FabricIndex fabricIndex = controller.fabricIndex;
        if (!IsValidFabricIndex(fabricIndex)) {
            // This controller is not sufficiently "running"; it does not have a
            // fabric index yet.  Just skip it.
            continue;
        }

        auto * resumptionInfo = [controller.controllerDataStore findResumptionInfoByResumptionID:resumptionIDData];
        if (resumptionInfo != nil) {
            NodeId nodeID;
            ResumptionIdStorage ignored;
            ReturnErrorOnFailure(DeconstructResumptionInfo(resumptionInfo, nodeID, ignored, sharedSecret, peerCATs));
            node = ScopedNodeId(nodeID, fabricIndex);
            return CHIP_NO_ERROR;
        }
    }

    // None of the controllers matched.
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MTRSessionResumptionStorageBridge::Save(const chip::ScopedNodeId & node, ConstResumptionIdView resumptionId,
    const chip::Crypto::P256ECDHDerivedSecret & sharedSecret, const chip::CATValues & peerCATs)
{
    assertChipStackLockedByCurrentThread();

    auto * controller = [mFactory runningControllerForFabricIndex:node.GetFabricIndex()];
    if (controller == nil) {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    auto * resumptionInfo = [[MTRCASESessionResumptionInfo alloc] init];
    resumptionInfo.nodeID = @(node.GetNodeId());
    resumptionInfo.resumptionID = AsData(resumptionId);
    resumptionInfo.sharedSecret = AsData(ByteSpan(sharedSecret.ConstBytes(), sharedSecret.Length()));
    resumptionInfo.caseAuthenticatedTags = CATValuesToSet(peerCATs);

    [controller.controllerDataStore storeResumptionInfo:resumptionInfo];
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRSessionResumptionStorageBridge::DeleteAll(chip::FabricIndex fabricIndex)
{
    assertChipStackLockedByCurrentThread();

    // NOTE: During controller startup and shutdown, the SDK DeviceControllerFactory
    // calls this method from ClearCASEResumptionStateOnFabricChange() due to fabric
    // update or removal.  For us that does not matter, since we don't key resumption
    // data on fabric index anyway.  But we do want to avoid executing the DeleteAll
    // so we can actually store session resumption data persistently.

    // And that is the only use of DeleteAll for controllers in practice, in the
    // situations where we are using MTRSessionResumptionStorageBridge at all.
    // So just no-op this function, but verify that our assumptions hold.
    auto * controller = [mFactory runningControllerForFabricIndex:fabricIndex
                                      includeControllerStartingUp:NO
                                    includeControllerShuttingDown:NO];
    VerifyOrDieWithMsg(controller == nil, Controller, "ResumptionStorage::DeleteAll called for running controller");
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRSessionResumptionStorageBridge::DeconstructResumptionInfo(MTRCASESessionResumptionInfo * resumptionInfo,
    chip::NodeId & nodeID, ResumptionIdStorage & resumptionId, chip::Crypto::P256ECDHDerivedSecret & sharedSecret,
    chip::CATValues & peerCATs)
{
    if (resumptionInfo.resumptionID.length != resumptionId.size()) {
        MTR_LOG_ERROR("Unable to return resumption ID: Stored size %llu does not match required size %llu",
            static_cast<unsigned long long>(resumptionInfo.resumptionID.length),
            static_cast<unsigned long long>(resumptionId.size()));
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    if (resumptionInfo.sharedSecret.length > sharedSecret.Capacity()) {
        MTR_LOG_ERROR("Unable to return resumption shared secret: Stored size %llu is larger than allowed size %llu",
            static_cast<unsigned long long>(resumptionInfo.sharedSecret.length),
            static_cast<unsigned long long>(sharedSecret.Capacity()));
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    nodeID = resumptionInfo.nodeID.unsignedLongLongValue;
    memcpy(resumptionId.data(), resumptionInfo.resumptionID.bytes, resumptionInfo.resumptionID.length);
    sharedSecret.SetLength(resumptionInfo.sharedSecret.length);
    memcpy(sharedSecret.Bytes(), resumptionInfo.sharedSecret.bytes, resumptionInfo.sharedSecret.length);
    ReturnErrorOnFailure(SetToCATValues(resumptionInfo.caseAuthenticatedTags, peerCATs));

    return CHIP_NO_ERROR;
}
