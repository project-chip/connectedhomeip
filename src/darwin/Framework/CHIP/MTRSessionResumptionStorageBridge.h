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

#import <Foundation/Foundation.h>
#import <Matter/MTRDeviceControllerFactory.h>

#import "MTRDeviceControllerDataStore.h"

#include <protocols/secure_channel/SessionResumptionStorage.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Implements SessionResumptionStorage and dispatches the relevant lookup
 * requests to running controllers as needed.
 */
class MTRSessionResumptionStorageBridge : public chip::SessionResumptionStorage
{
public:
    MTRSessionResumptionStorageBridge(MTRDeviceControllerFactory * factory);

    ~MTRSessionResumptionStorageBridge() {}

    // SessionResumptionStorage API.
    CHIP_ERROR FindByScopedNodeId(const chip::ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                  chip::Crypto::P256ECDHDerivedSecret & sharedSecret, chip::CATValues & peerCATs) override;

    CHIP_ERROR FindByResumptionId(ConstResumptionIdView resumptionId, chip::ScopedNodeId & node,
                                  chip::Crypto::P256ECDHDerivedSecret & sharedSecret, chip::CATValues & peerCATs) override;
    CHIP_ERROR Save(const chip::ScopedNodeId & node, ConstResumptionIdView resumptionId,
                    const chip::Crypto::P256ECDHDerivedSecret & sharedSecret, const chip::CATValues & peerCATs) override;
    CHIP_ERROR DeleteAll(chip::FabricIndex fabricIndex) override;

private:
    /**
     * Helper method to convert a MTRCASESessionResumptionInfo into the pieces
     * we need to return from our Find* methods.
     */
    static CHIP_ERROR DeconstructResumptionInfo(MTRCASESessionResumptionInfo * resumptionInfo, chip::NodeId & nodeID,
                                                ResumptionIdStorage & resumptionId,
                                                chip::Crypto::P256ECDHDerivedSecret & sharedSecret, chip::CATValues & peerCATs);

    MTRDeviceControllerFactory * mFactory;
};

NS_ASSUME_NONNULL_END
