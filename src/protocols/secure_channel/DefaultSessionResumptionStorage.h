/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <protocols/secure_channel/SessionResumptionStorage.h>

namespace chip {

/**
 * @brief Stores assets for session resumption. The resumption data are indexed by 2 indexes: ScopedNodeId and ResumptionId. The
 *   index of ScopedNodeId is used when initiating a CASE session, it will look up the storage and check whether it is able to
 *   resume a previous session. The index of ResumptionId is used when receiving a Sigma1 with ResumptionId.
 *
 *   The implementation saves 2 maps:
 *     * <FabricIndex, PeerNodeId>   => <ResumptionId, ShareSecret, PeerCATs>
 *     * <ResumptionId>              => <FabricIndex, PeerNodeId>
 */
class DefaultSessionResumptionStorage : public SessionResumptionStorage
{
public:
    using ResumptionIdView = FixedSpan<uint8_t, kResumptionIdSize>;

    struct SessionIndex
    {
        size_t mSize;
        ScopedNodeId mNodes[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE];
    };

    virtual ~DefaultSessionResumptionStorage() {}

    CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override;
    CHIP_ERROR FindByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node,
                                  Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override;
    CHIP_ERROR FindNodeByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node);
    CHIP_ERROR Save(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                    const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs) override;
    /**
     * Removes the resumption record of a node.
     *
     * Returns CHIP_NO_ERROR when no state record for the node remains and the index no longer
     * lists the node; a node with no record counts as removed. Returns the storage error when
     * the index could not be read, the state record could not be deleted, or the index could
     * not be rewritten. On error the index entry is kept so a later Delete or DeleteAll finds
     * the node again. A state record that cannot be read, or a link that cannot be deleted, is
     * logged only; the link may then remain in storage without a record.
     */
    CHIP_ERROR Delete(const ScopedNodeId & node);
    /**
     * Removes every record listed in the index for the fabric. Returns the first storage error
     * when a state record could not be deleted or the index could not be rewritten; that record
     * stays in the index. A record that cannot be read, or a link that cannot be deleted, is
     * logged only; the record is still deleted and the link may remain in storage.
     */
    CHIP_ERROR DeleteAll(FabricIndex fabricIndex) override;

protected:
    CHIP_ERROR virtual SaveIndex(const SessionIndex & index) = 0;
    CHIP_ERROR virtual LoadIndex(SessionIndex & index)       = 0;

    CHIP_ERROR virtual SaveLink(ConstResumptionIdView resumptionId, const ScopedNodeId & node) = 0;
    CHIP_ERROR virtual LoadLink(ConstResumptionIdView resumptionId, ScopedNodeId & node)       = 0;
    CHIP_ERROR virtual DeleteLink(ConstResumptionIdView resumptionId)                          = 0;

    CHIP_ERROR virtual SaveState(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                                 const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs) = 0;
    CHIP_ERROR virtual LoadState(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                 Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs)             = 0;
    CHIP_ERROR virtual DeleteState(const ScopedNodeId & node)                                                    = 0;
};

} // namespace chip
