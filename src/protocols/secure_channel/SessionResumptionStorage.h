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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/ScopedNodeId.h>

namespace chip {

/**
 * @brief Interface to store and recover assets for session resumption. The
 *   resumption data are indexed by 2 parameters: ScopedNodeId and
 *   ResumptionId. The index on ScopedNodeId is used when initiating a CASE
 *   session.  It allows the caller to query storage to check whether there is a
 *   previous session with the given peer for which session resumption may be
 *   attempted.  The index on ResumptionId is used when receiving a Sigma1 with
 *   ResumptionId.
 *
 */
class SessionResumptionStorage
{
public:
    static constexpr size_t kResumptionIdSize = 16;
    using ResumptionIdStorage                 = std::array<uint8_t, kResumptionIdSize>;
    using ConstResumptionIdView               = FixedSpan<const uint8_t, kResumptionIdSize>;

    virtual ~SessionResumptionStorage(){};

    /**
     * Recover session resumption ID, shared secret and CAT values for a given
     * fabric-scoped node identity.
     *
     * @param node the node for which to recover session resumption information
     * @param resumptionId (out) recovered session resumption ID
     * @param sharedSecret (out) recovered session shared secret
     * @param peerCATs (out) recovered CATs for the session peer
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_KEY_NOT_FOUND if no session resumption information can be found, else an
     * appropriate CHIP error on failure
     */
    virtual CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                          Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) = 0;
    /**
     * Recover session shared secret, fabric-scoped node identity and CAT values
     * for a given session resumption ID.
     *
     * @param resumptionId the session resumption ID for which to recover session resumption information
     * @param node (out) the peer node associated with the session resumption ID
     * @param sharedSecret (out) recovered session shared secret
     * @param peerCATs (out) recovered CATs for the session peer
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_KEY_NOT_FOUND if no session resumption information can be found, else an
     * appropriate CHIP error on failure
     */
    virtual CHIP_ERROR FindByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node,
                                          Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) = 0;
    /**
     * Save session resumption information to storage.
     *
     * @param resumptionId the session resumption ID for the current session
     * @param node the peer node for the session
     * @param sharedSecret the session shared secret
     * @param peerCATs the CATs of the session peer
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR Save(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                            const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs) = 0;

    /**
     * Remove all session resumption information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     *
     * @param fabricIndex the index of the fabric for which to remove session resumption information
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR DeleteAll(FabricIndex fabricIndex) = 0;
};

} // namespace chip
