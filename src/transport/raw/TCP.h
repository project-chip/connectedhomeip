/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines the CHIP Connection object that maintains TCP connections.
 *      It binds to any available local addr and port and begins listening.
 */

#pragma once

#include <algorithm>
#include <new>
#include <utility>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/TCPEndPoint.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PoolWrapper.h>
#include <transport/raw/ActiveTCPConnectionState.h>
#include <transport/raw/Base.h>
#include <transport/raw/TCPConfig.h>

namespace chip {
namespace Transport {

// Forward declaration of friend class for test access.
template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
class TCPBaseTestAccess;

/** Defines listening parameters for setting up a TCP transport */
class TcpListenParameters
{
public:
    explicit TcpListenParameters(Inet::EndPointManager<Inet::TCPEndPoint> * endPointManager) : mEndPointManager(endPointManager) {}
    TcpListenParameters(const TcpListenParameters &) = default;
    TcpListenParameters(TcpListenParameters &&)      = default;

    Inet::EndPointManager<Inet::TCPEndPoint> * GetEndPointManager() { return mEndPointManager; }

    Inet::IPAddressType GetAddressType() const { return mAddressType; }
    TcpListenParameters & SetAddressType(Inet::IPAddressType type)
    {
        mAddressType = type;

        return *this;
    }

    uint16_t GetListenPort() const { return mListenPort; }
    TcpListenParameters & SetListenPort(uint16_t port)
    {
        mListenPort = port;

        return *this;
    }

    Inet::InterfaceId GetInterfaceId() const { return mInterfaceId; }
    TcpListenParameters & SetInterfaceId(Inet::InterfaceId id)
    {
        mInterfaceId = id;

        return *this;
    }

private:
    Inet::EndPointManager<Inet::TCPEndPoint> * mEndPointManager;   ///< Associated endpoint factory
    Inet::IPAddressType mAddressType = Inet::IPAddressType::kIPv6; ///< type of listening socket
    uint16_t mListenPort             = CHIP_PORT;                  ///< TCP listen port
    Inet::InterfaceId mInterfaceId   = Inet::InterfaceId::Null();  ///< Interface to listen on
};

/**
 * Packets scheduled for sending once a connection has been established.
 */
struct PendingPacket
{
    PendingPacket(const PeerAddress & peerAddress, System::PacketBufferHandle && packetBuffer) :
        mPeerAddress(peerAddress), mPacketBuffer(std::move(packetBuffer))
    {}

    PeerAddress mPeerAddress;                 // where the packet is being sent to
    System::PacketBufferHandle mPacketBuffer; // what data needs to be sent
};

/** Implements a transport using TCP. */
class DLL_EXPORT TCPBase : public Base
{

protected:
    enum class ShouldAbort : uint8_t
    {
        Yes,
        No
    };

    enum class SuppressCallback : uint8_t
    {
        Yes,
        No
    };

public:
    using PendingPacketPoolType = PoolInterface<PendingPacket, const PeerAddress &, System::PacketBufferHandle &&>;
    TCPBase(ActiveTCPConnectionState * activeConnectionsBuffer, size_t bufferSize, PendingPacketPoolType & packetBuffers) :
        mActiveConnections(activeConnectionsBuffer), mActiveConnectionsSize(bufferSize), mPendingPackets(packetBuffers)
    {
        // activeConnectionsBuffer must be initialized by the caller.
    }
    ~TCPBase() override;

    /**
     * Initialize a TCP transport on a given port.
     *
     * @param params        TCP configuration parameters for this transport
     *
     * @details
     *   Generally send and receive ports should be the same and equal to CHIP_PORT.
     *   The class allows separate definitions to allow local execution of several
     *   Nodes.
     */
    CHIP_ERROR Init(TcpListenParameters & params);

    /**
     * Set the timeout (in milliseconds) for the node to wait for the TCP
     * connection attempt to complete.
     *
     */
    void SetConnectTimeout(const uint32_t connTimeoutMsecs) { mConnectTimeout = connTimeoutMsecs; }

    /**
     * Close the open endpoint without destroying the object
     */
    void Close() override;

    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

    /*
     * Connect to the given peerAddress over TCP.
     *
     * @param address           The address of the peer.
     *
     * @param appState          Context passed in by the application to be sent back
     *                          via the connection attempt complete callback when
     *                          connection attempt with peer completes.
     *
     * @param outPeerConnState  Pointer to pointer to the active TCP connection state. This is
     *                          an output parameter that is allocated by the
     *                          transport layer and held by the caller object.
     *                          This allows the caller object to abort the
     *                          connection attempt if the caller object dies
     *                          before the attempt completes.
     *
     */
    CHIP_ERROR TCPConnect(const PeerAddress & address, Transport::AppTCPConnectionCallbackCtxt * appState,
                          Transport::ActiveTCPConnectionState ** outPeerConnState) override;

    void TCPDisconnect(const PeerAddress & address) override;

    // Close an active connection (corresponding to the passed
    // ActiveTCPConnectionState object)
    // and release from the pool.
    void TCPDisconnect(Transport::ActiveTCPConnectionState * conn, bool shouldAbort = false) override;

    bool CanSendToPeer(const PeerAddress & address) override
    {
        return (mState == TCPState::kInitialized) && (address.GetTransportType() == Type::kTcp) &&
            (address.GetIPAddress().Type() == mEndpointType);
    }

    const Optional<PeerAddress> GetConnectionPeerAddress(const Inet::TCPEndPoint * con)
    {
        ActiveTCPConnectionState * activeConState = FindActiveConnection(con);

        return activeConState != nullptr ? MakeOptional<PeerAddress>(activeConState->mPeerAddr) : Optional<PeerAddress>::Missing();
    }

    /**
     * Helper method to determine if IO processing is still required for a TCP transport
     * before everything is cleaned up (socket closing is async, so after calling 'Close' on
     * the transport, some time may be needed to actually be able to close.)
     */
    bool HasActiveConnections() const;

    /**
     * Close all active connections.
     */
    void CloseActiveConnections();

private:
    // Allow tests to access private members.
    template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
    friend class TCPBaseTestAccess;

    /**
     * Allocate an unused connection from the pool
     *
     */
    ActiveTCPConnectionState * AllocateConnection();
    /**
     * Find an active connection to the given peer or return nullptr if
     * no active connection exists.
     */
    ActiveTCPConnectionState * FindActiveConnection(const PeerAddress & addr);
    ActiveTCPConnectionState * FindActiveConnection(const Inet::TCPEndPoint * endPoint);

    /**
     * Find an allocated connection that matches the corresponding TCPEndPoint.
     */
    ActiveTCPConnectionState * FindInUseConnection(const Inet::TCPEndPoint * endPoint);

    /**
     * Sends the specified message once a connection has been established.
     *
     * @param addr - what peer to connect to
     * @param msg - what buffer to send once a connection has been established.
     *
     * Ownership of msg is taken over and will be freed at some unspecified time
     * in the future (once connection succeeds/fails).
     */
    CHIP_ERROR SendAfterConnect(const PeerAddress & addr, System::PacketBufferHandle && msg);

    /**
     * Process a single received buffer from the specified peer address.
     *
     * @param endPoint the source end point from which the data comes from
     * @param peerAddress the peer the data is coming from
     * @param buffer the actual data
     *
     * Ownership of buffer is taken over and will be freed (or re-enqueued to the endPoint receive queue)
     * as needed during processing.
     */
    CHIP_ERROR ProcessReceivedBuffer(Inet::TCPEndPoint * endPoint, const PeerAddress & peerAddress,
                                     System::PacketBufferHandle && buffer);

    /**
     * Process a single message of the specified size from a buffer.
     *
     * @param[in]     peerAddress   The peer the data is coming from.
     * @param[in,out] state         The connection state, which contains the message. On entry, the payload points to the message
     *                              body (after the length). On exit, it points after the message (or the queue is null, if there
     *                              is no other data).
     * @param[in]     messageSize   Size of the single message.
     */
    CHIP_ERROR ProcessSingleMessage(const PeerAddress & peerAddress, ActiveTCPConnectionState * state, size_t messageSize);

    /**
     * Initiate a connection to the given peer. On connection completion,
     * HandleTCPConnectComplete callback would be called.
     *
     */
    CHIP_ERROR StartConnect(const PeerAddress & addr, AppTCPConnectionCallbackCtxt * appState,
                            Transport::ActiveTCPConnectionState ** outPeerConnState);

    /**
     * Gracefully Close or Abort a given connection.
     *
     */
    void CloseConnectionInternal(ActiveTCPConnectionState * connection, CHIP_ERROR err, SuppressCallback suppressCallback);

    // Close the listening socket endpoint
    void CloseListeningSocket();

    // Callback handler for TCPEndPoint. TCP message receive handler.
    // @see TCPEndpoint::OnDataReceivedFunct
    static CHIP_ERROR HandleTCPEndPointDataReceived(Inet::TCPEndPoint * endPoint, System::PacketBufferHandle && buffer);

    // Callback handler for TCPEndPoint. Called when a connection has been completed.
    // @see TCPEndpoint::OnConnectCompleteFunct
    static void HandleTCPEndPointConnectComplete(Inet::TCPEndPoint * endPoint, CHIP_ERROR err);

    // Callback handler for TCPEndPoint. Called when a connection has been closed.
    // @see TCPEndpoint::OnConnectionClosedFunct
    static void HandleTCPEndPointConnectionClosed(Inet::TCPEndPoint * endPoint, CHIP_ERROR err);

    // Callback handler for TCPEndPoint. Called when a connection is received on the listening port.
    // @see TCPEndpoint::OnConnectionReceivedFunct
    static void HandleIncomingConnection(Inet::TCPEndPoint * listenEndPoint, Inet::TCPEndPoint * endPoint,
                                         const Inet::IPAddress & peerAddress, uint16_t peerPort);

    // Callback handler for handling accept error
    // @see TCPEndpoint::OnAcceptErrorFunct
    static void HandleAcceptError(Inet::TCPEndPoint * endPoint, CHIP_ERROR err);

    Inet::TCPEndPoint * mListenSocket = nullptr;                       ///< TCP socket used by the transport
    Inet::IPAddressType mEndpointType = Inet::IPAddressType::kUnknown; ///< Socket listening type
    TCPState mState                   = TCPState::kNotReady;           ///< State of the TCP transport

    // The configured timeout for the connection attempt to the peer, before
    // giving up.
    uint32_t mConnectTimeout = CHIP_CONFIG_TCP_CONNECT_TIMEOUT_MSECS;

    // Number of active and 'pending connection' endpoints
    size_t mUsedEndPointCount = 0;

    // Currently active connections
    ActiveTCPConnectionState * mActiveConnections;
    const size_t mActiveConnectionsSize;

    // Data to be sent when connections succeed
    PendingPacketPoolType & mPendingPackets;
};

template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
class TCP : public TCPBase
{
public:
    TCP() : TCPBase(mConnectionsBuffer, kActiveConnectionsSize, mPendingPackets)
    {
        for (size_t i = 0; i < kActiveConnectionsSize; ++i)
        {
            mConnectionsBuffer[i].Init(nullptr, PeerAddress::Uninitialized());
        }
    }

    ~TCP() override { mPendingPackets.ReleaseAll(); }

private:
    ActiveTCPConnectionState mConnectionsBuffer[kActiveConnectionsSize];
    PoolImpl<PendingPacket, kPendingPacketSize, ObjectPoolMem::kInline, PendingPacketPoolType::Interface> mPendingPackets;
};

} // namespace Transport
} // namespace chip
