/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/TCPEndPoint.h>
#include <lib/core/CHIPCore.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/TCPConfig.h>

namespace chip {
namespace Transport {

/**
 *  The State of the TCP connection
 */
enum class TCPState
{
    kNotReady    = 0, /**< State before initialization. */
    kInitialized = 1, /**< State after class is listening and ready. */
    kConnecting  = 3, /**< Connection with peer has been initiated. */
    kConnected   = 4, /**< Connected with peer and ready for Send/Receive. */
    kClosed      = 5, /**< Connection is closed. */
};

struct AppTCPConnectionCallbackCtxt;
/**
 *  State for each active TCP connection
 */
struct ActiveTCPConnectionState
{

    void Init(Inet::TCPEndPoint * endPoint, const PeerAddress & peerAddr)
    {
        mEndPoint = endPoint;
        mPeerAddr = peerAddr;
        mReceived = nullptr;
        mAppState = nullptr;
    }

    void Free()
    {
        if (mEndPoint)
        {
            mEndPoint->Free();
        }
        mPeerAddr = PeerAddress::Uninitialized();
        mEndPoint = nullptr;
        mReceived = nullptr;
        mAppState = nullptr;
    }

    bool InUse() const { return mEndPoint != nullptr; }

    bool IsConnected() const { return (mEndPoint != nullptr && mConnectionState == TCPState::kConnected); }

    bool IsConnecting() const { return (mEndPoint != nullptr && mConnectionState == TCPState::kConnecting); }

    // Associated endpoint.
    Inet::TCPEndPoint * mEndPoint;

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
};

// Functors for callbacks into higher layers
using OnTCPConnectionReceivedCallback = void (*)(ActiveTCPConnectionState * conn);

using OnTCPConnectionCompleteCallback = void (*)(ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

using OnTCPConnectionClosedCallback = void (*)(ActiveTCPConnectionState * conn, CHIP_ERROR conErr);

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
