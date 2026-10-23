/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

/**
 *    @file
 *      This file defines the CHIP Active Connection object that maintains TCP connections.
 */

#pragma once

#include <functional>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/TCPEndPoint.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/ReferenceCountedPtr.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/TCPConfig.h>

namespace chip {
namespace Transport {

// Forward declaration of friend class for test access.
template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
class TCPBaseTestAccess;

/**
 *  The State of the TCP connection
 */
enum class TCPState : uint8_t
{
    kNotReady    = 0, /**< State before initialization. */
    kInitialized = 1, /**< State after class is listening and ready. */
    kConnecting  = 3, /**< Connection with peer has been initiated. */
    kConnected   = 4, /**< Connected with peer and ready for Send/Receive. */
    kClosed      = 5, /**< Connection is closed. */
};

struct AppTCPConnectionCallbackCtxt;

// Templatized to force inlining
template <typename State>
class ActiveTCPConnectionStateDeleter
{
public:
    inline static void Release(State * entry) { entry->mReleaseConnection(*entry); }
};

/**
 *  State for each active TCP connection
 */
class ActiveTCPConnectionHandle;
struct ActiveTCPConnectionState
    : public ReferenceCountedProtected<ActiveTCPConnectionState, ActiveTCPConnectionStateDeleter<ActiveTCPConnectionState>>
{
    using ReleaseFnType = std::function<void(ActiveTCPConnectionState & connection)>;

    bool InUse() const { return !mEndPoint.IsNull(); }

    bool IsConnected() const { return (!mEndPoint.IsNull() && mConnectionState == TCPState::kConnected); }

    bool IsConnecting() const { return (!mEndPoint.IsNull() && mConnectionState == TCPState::kConnecting); }

    inline bool operator==(const ActiveTCPConnectionHandle & other) const;
    inline bool operator!=(const ActiveTCPConnectionHandle & other) const;

    // Peer Node Address
    PeerAddress mPeerAddr;

    // Buffers received but not yet consumed.
    System::PacketBufferHandle mReceived;

    // Current state of the connection
    TCPState mConnectionState;

    // A pointer to an application-specific state object. It should
    // represent an object that is at a layer above the SessionManager. The
    // SessionManager would accept this object at the time of connecting to
    // the peer, and percolate it down to the TransportManager that then,
    // should store this state in the corresponding connection object that
    // is created.
    // At various connection events, this state is passed back to the
    // corresponding application.
    AppTCPConnectionCallbackCtxt * mAppState = nullptr;

    // KeepAlive interval in seconds
    uint16_t mTCPKeepAliveIntervalSecs = CHIP_CONFIG_TCP_KEEPALIVE_INTERVAL_SECS;
    uint16_t mTCPMaxNumKeepAliveProbes = CHIP_CONFIG_MAX_TCP_KEEPALIVE_PROBES;

    // This is bad and should not normally be done; we are explicitly closing the TCP connection
    // instead of gracefully releasing our reference, which will theoretically cause anyone
    // holding a reference (who should have a listener for connection closing) to release their reference
    void ForceDisconnect() { mReleaseConnection(*this); }

private:
    template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
    friend class TCP;
    friend class TCPBase;
    friend class ActiveTCPConnectionStateDeleter<ActiveTCPConnectionState>;
    friend class ActiveTCPConnectionHandle;
    // Allow tests to access private members.
    template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
    friend class TCPBaseTestAccess;

    // Associated endpoint.
    Inet::TCPEndPointHandle mEndPoint;
    ReleaseFnType mReleaseConnection;

    void Init(Inet::TCPEndPointHandle endPoint, const PeerAddress & peerAddr, ReleaseFnType releaseConnection)
    {
        mEndPoint          = endPoint;
        mPeerAddr          = peerAddr;
        mReceived          = nullptr;
        mAppState          = nullptr;
        mReleaseConnection = releaseConnection;
    }

    void Free()
    {
        mPeerAddr          = PeerAddress::Uninitialized();
        mEndPoint          = nullptr;
        mReceived          = nullptr;
        mAppState          = nullptr;
        mReleaseConnection = [](auto &) {};
    }
};

/**
 * A holder for ActiveTCPConnectionState which properly ref-counts on ctor/copy/dtor.
 */
class ActiveTCPConnectionHandle : public ReferenceCountedPtr<ActiveTCPConnectionState>
{
    friend class TCPBase;
    friend struct ActiveTCPConnectionState;

public:
    using ReferenceCountedPtr<ActiveTCPConnectionState>::ReferenceCountedPtr;

    // For printing
    inline operator const void *() const { return mRefCounted; }
};

inline bool ActiveTCPConnectionState::operator==(const ActiveTCPConnectionHandle & other) const
{
    return this == other.mRefCounted;
}
inline bool ActiveTCPConnectionState::operator!=(const ActiveTCPConnectionHandle & other) const
{
    return this != other.mRefCounted;
}

// Functors for callbacks into higher layers
using OnTCPConnectionReceivedCallback = void (*)(ActiveTCPConnectionState & conn);

using OnTCPConnectionCompleteCallback = void (*)(ActiveTCPConnectionHandle & conn, CHIP_ERROR conErr);

using OnTCPConnectionClosedCallback = void (*)(ActiveTCPConnectionState & conn, CHIP_ERROR conErr);

/*
 *  Application callback state that is passed down at connection establishment
 *  stage.
 * */
struct AppTCPConnectionCallbackCtxt
{
    void * appContext                              = nullptr; // A pointer to an application context object.
    OnTCPConnectionReceivedCallback connReceivedCb = nullptr;
    OnTCPConnectionCompleteCallback connCompleteCb = nullptr;
    OnTCPConnectionClosedCallback connClosedCb     = nullptr;
};

} // namespace Transport
} // namespace chip
