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
#ifndef PEER_CONNECTIONS_H_
#define PEER_CONNECTIONS_H_

#include <array>

#include <core/CHIPConfig.h>
#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <system/TimeSource.h>
#include <transport/PeerConnectionState.h>

namespace chip {
namespace Transport {

/**
 * Handles a set of peer connection states.
 *
 * Intended for:
 *   - handle connection active time and expiration
 *   - allocate and free space for connection states.
 */
class PeerConnectionsBase
{
public:
    /**
     * Construct a PeerConnectionsBase object using a preallocated array used for connection state  storage.
     */
    PeerConnectionsBase(PeerConnectionState * storageArray, size_t arraySize) :
        mConnectionStateArray(storageArray), mConnectionStateArraySize(arraySize)
    {}
    virtual ~PeerConnectionsBase() {}

    /**
     * Allocates a new peer connection state state object out of the internal resource pool.
     *
     * @param address represents the connection state address
     * @param state [out] will contain the connection state if one was available. May be null if no return value is desired.
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum connection count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR CreateNewPeerConnectionState(const PeerAddress & address, PeerConnectionState ** state);

    /**
     * Get a peer connection state given a peer address.
     *
     * @param address is the connection to find (based on address)
     * @param state [out] the connection if found, null otherwise. MUST not be null.
     *
     * @return true if a corresponding state was found.
     */
    CHECK_RETURN_VALUE
    bool FindPeerConnectionState(const PeerAddress & address, PeerConnectionState ** state);

    /**
     * Get a peer connection state given a Node Id.
     *
     * @param nodeId is the connection to find (based on nodeId). Note that initial connections
     *        do not have a node id set. Use this if you know the node id should be set.
     * @param state [out] the connection if found, null otherwise. MUST not be null.
     *
     * @return true if a corresponding state was found.
     */
    CHECK_RETURN_VALUE
    bool FindPeerConnectionState(NodeId nodeId, PeerConnectionState ** state);

    /// Convenience method to mark a peer connection state as active
    void MarkConnectionActive(PeerConnectionState * state) { state->SetLastActivityTimeMs(GetCurrentMonotonicTimeMs()); }

    /**
     * Iterates through all active connections and expires any connection with an idle time
     * larger than the given amount.
     *
     * Expiring a connection involves callback execution and then clearing the internal state.
     */
    void ExpireInactiveConnections(uint64_t maxIdleTimeMs);

    /**
     * Sets the handler for expired connections
     *
     * @param[in] handler The callback to call when a connection is marked as expired
     * @param[in] param   The argument to pass in to the handler function
     *
     */
    template <class T>
    void SetConnectionExpiredHandler(void (*handler)(const PeerConnectionState &, T *), T * param)
    {
        mConnectionExpiredArgument = param;
        OnConnectionExpired        = reinterpret_cast<ConnectionExpiredHandler>(handler);
    }

protected:
    /// Get the current time from a Time::TimeSource or equivalent
    virtual uint64_t GetCurrentMonotonicTimeMs() = 0;

private:
    PeerConnectionState * mConnectionStateArray;
    const size_t mConnectionStateArraySize;

    typedef void (*ConnectionExpiredHandler)(const PeerConnectionState & state, void * param);

    ConnectionExpiredHandler OnConnectionExpired = nullptr; ///< Callback for when a connection expires
    void * mConnectionExpiredArgument            = nullptr; ///< Argument for callback
};

/**
 * Concrete peer connections implementation based on system sizes and timers.
 */
class PeerConnections : public PeerConnectionsBase
{
public:
    PeerConnections() : PeerConnectionsBase(mState, ArraySize(mState)) {}

protected:
    uint64_t GetCurrentMonotonicTimeMs() override { return mTimeSource.GetCurrentMonotonicTimeMs(); }

private:
    PeerConnectionState mState[CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE];
    Time::TimeSource<Time::Source::kSystem> mTimeSource;
};

} // namespace Transport
} // namespace chip

#endif // PEER_CONNECTIONS_H_
