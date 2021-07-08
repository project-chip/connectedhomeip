/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <system/TimeSource.h>
#include <transport/FabricTable.h>
#include <transport/PeerConnectionState.h>

namespace chip {
namespace Transport {

// TODO; use 0xffff to match any key id, this is a temporary solution for
// InteractionModel, where key id is not obtainable. This will be removed when
// InteractionModel is migrated to messaging layer
constexpr const uint16_t kAnyKeyId = 0xffff;

/**
 * Handles a set of peer connection states.
 *
 * Intended for:
 *   - handle connection active time and expiration
 *   - allocate and free space for connection states.
 */
template <size_t kMaxConnectionCount, Time::Source kTimeSource = Time::Source::kSystem>
class PeerConnections
{
public:
    /**
     * Allocates a new peer connection state state object out of the internal resource pool.
     *
     * @param peerNode represents peer Node's ID
     * @param peerKeyId represents the encryption key ID assigned by peer node
     * @param localKeyId represents the encryption key ID assigned by local node
     * @param state [out] will contain the connection state if one was available.
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum connection count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR CreateNewPeerConnectionState(PeerCacheEntry & peer, FabricIndex fabric, uint16_t peerKeyId, uint16_t localKeyId,
                                            PeerConnectionState *& state)
    {
        state = mStates.CreateObject(peer, fabric, peerKeyId, localKeyId);
        if (state == nullptr)
            return CHIP_ERROR_NO_MEMORY;

        state->SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs());

        return CHIP_NO_ERROR;
    }

    /**
     * Get a peer connection state given a Node Id and Peer's Encryption Key Id.
     *
     * @param nodeId is the connection to find (based on nodeId).
     * @param peerKeyId Encryption key ID used by the peer node.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerConnectionState * FindPeerConnectionState(const PeerId & peer, uint16_t peerKeyId)
    {
        PeerConnectionState * result = nullptr;
        mStates.ForEachActiveObject([&](PeerConnectionState * state) {
            auto & entry = state->GetPeerInfo();
            // TODO: relax the check, omit fabric id check until fabric is not used in operational-credentials-server.cpp
            if (entry.GetPeer().GetNodeId() == peer.GetNodeId() && state->GetPeerKeyID() == peerKeyId)
            {
                result = state;
                return false;
            }
            return true;
        });
        return result;
    }

    /**
     * Get a peer connection state given the local Encryption Key Id.
     *
     * @param keyId Encryption key ID assigned by the local node.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    PeerConnectionState * FindPeerConnectionStateByLocalKeyId(uint16_t keyId)
    {
        PeerConnectionState * result = nullptr;
        mStates.ForEachActiveObject([&](PeerConnectionState * state) {
            if (state->GetLocalKeyID() == keyId)
            {
                result = state;
                return false;
            }
            return true;
        });
        return result;
    }

    template <typename Function>
    void ForEachActiveConnection(Function && function)
    {
        mStates.ForEachActiveObject(std::move(function));
    }

    /// Convenience method to mark a peer connection state as active
    void MarkConnectionActive(PeerConnectionState * state)
    {
        state->SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs());
    }

    /// Convenience method to expired a peer connection state and fired the related callback
    template <typename Callback>
    void MarkConnectionExpired(PeerConnectionState * state, Callback callback)
    {
        callback(*state);
        mStates.ReleaseObject(state);
    }

    /**
     * Iterates through all active connections and expires any connection with an idle time
     * larger than the given amount.
     *
     * Expiring a connection involves callback execution and then clearing the internal state.
     */
    template <typename Callback>
    void ExpireInactiveConnections(uint64_t maxIdleTimeMs, Callback callback)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        mStates.ForEachActiveObject([&](PeerConnectionState * state) {
            uint64_t connectionActiveTime = state->GetLastActivityTimeMs();
            if (connectionActiveTime + maxIdleTimeMs < currentTime)
            {
                MarkConnectionExpired(state, callback);
            }
            return true;
        });
    }

    /// Allows access to the underlying time source used for keeping track of connection active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    BitMapObjectPool<PeerConnectionState, kMaxConnectionCount> mStates;
};

} // namespace Transport
} // namespace chip
