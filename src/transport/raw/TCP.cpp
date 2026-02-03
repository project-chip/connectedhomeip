/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements the CHIP Transport object that maintains TCP connections
 *      to peers. Handles both establishing new connections and accepting peer connection
 *      requests.
 */
#include <transport/raw/TCP.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <inttypes.h>
#include <limits>

namespace chip {
namespace Transport {
namespace {

using namespace chip::Encoding;

// Packets start with a 32-bit size field.
constexpr size_t kPacketSizeBytes = 4;

static_assert(System::PacketBuffer::kLargeBufMaxSizeWithoutReserve <= UINT32_MAX, "Cast below could truncate the value");
static_assert(System::PacketBuffer::kLargeBufMaxSizeWithoutReserve >= kPacketSizeBytes,
              "Large buffer allocation should be large enough to hold the length field");

constexpr uint32_t kMaxTCPMessageSize =
    static_cast<uint32_t>(System::PacketBuffer::kLargeBufMaxSizeWithoutReserve - kPacketSizeBytes);

constexpr int kListenBacklogSize = 2;

CHIP_ERROR GetPeerAddress(Inet::TCPEndPoint & endPoint, PeerAddress & outAddr)
{
    Inet::IPAddress ipAddress;
    uint16_t port;
    Inet::InterfaceId interfaceId;
    ReturnErrorOnFailure(endPoint.GetPeerInfo(&ipAddress, &port));
    ReturnErrorOnFailure(endPoint.GetInterfaceId(&interfaceId));
    outAddr = PeerAddress::TCP(ipAddress, port, interfaceId);

    return CHIP_NO_ERROR;
}

} // namespace

TCPBase::~TCPBase()
{
    // Call Close to free the listening socket and close all active connections.
    Close();
}

void TCPBase::CloseActiveConnections()
{
    // Nothing to do; we can't release as long as references are being held
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].InUse())
        {
            CloseConnectionInternal(mActiveConnections[i], CHIP_NO_ERROR, SuppressCallback::Yes);
        }
    }
}

CHIP_ERROR TCPBase::Init(TcpListenParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == TCPState::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

    mEndpointType = params.GetAddressType();

    // Primary socket endpoint created to help get EndPointManager handle for creating multiple
    // connection endpoints at runtime.
    err = params.GetEndPointManager()->NewEndPoint(mListenSocket);
    SuccessOrExit(err);

    if (params.IsServerListenEnabled())
    {
        err = mListenSocket->Bind(params.GetAddressType(), Inet::IPAddress::Any, params.GetListenPort(),
                                  params.GetInterfaceId().IsPresent());
        SuccessOrExit(err);

        mListenSocket->mAppState            = reinterpret_cast<void *>(this);
        mListenSocket->OnConnectionReceived = HandleIncomingConnection;
        mListenSocket->OnAcceptError        = HandleAcceptError;

        err = mListenSocket->Listen(kListenBacklogSize);
        SuccessOrExit(err);
        ChipLogProgress(Inet, "TCP server listening on port %d for incoming connections", params.GetListenPort());
    }

    mState = TCPState::kInitialized;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to initialize TCP transport: %" CHIP_ERROR_FORMAT, err.Format());
        mListenSocket.Release();
    }

    return err;
}

void TCPBase::Close()
{
    mListenSocket.Release();

    CloseActiveConnections();

    mState = TCPState::kNotReady;
}

ActiveTCPConnectionState * TCPBase::AllocateConnection(const Inet::TCPEndPointHandle & endpoint, const PeerAddress & address)
{
    // If a peer initiates a connection through HandleIncomingConnection but the connection is never claimed
    // in ProcessSingleMessage, we'll be left with a dangling ActiveTCPConnectionState which can be
    // reclaimed.  Don't try to reclaim these connections unless we're out of space
    for (int reclaim = 0; reclaim < 2; reclaim++)
    {
        for (size_t i = 0; i < mActiveConnectionsSize; i++)
        {
            ActiveTCPConnectionState * activeConnection = &mActiveConnections[i];
            if (!activeConnection->InUse() && (activeConnection->GetReferenceCount() == 0))
            {
                // Update state for the active connection
                activeConnection->Init(endpoint, address, [this](auto & conn) { TCPDisconnect(conn, true); });
                return activeConnection;
            }
        }

        // Out of space; reclaim connections that were never claimed by ProcessSingleMessage
        // (i.e. that have a ref count of 0)
        for (size_t i = 0; i < mActiveConnectionsSize; i++)
        {
            ActiveTCPConnectionState * activeConnection = &mActiveConnections[i];
            if (!activeConnection->InUse() && (activeConnection->GetReferenceCount() != 0))
            {
                char addrStr[Transport::PeerAddress::kMaxToStringSize];
                activeConnection->mPeerAddr.ToString(addrStr);
                ChipLogError(Inet, "Leaked TCP connection %p to %s.", activeConnection, addrStr);
                // Try to notify callbacks in the hope that they release; the connection is no good
                CloseConnectionInternal(*activeConnection, CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY, SuppressCallback::No);
            }
            ActiveTCPConnectionHandle releaseUnclaimed(activeConnection);
        }
    }
    return nullptr;
}

// Find an ActiveTCPConnectionState corresponding to a peer address
ActiveTCPConnectionHandle TCPBase::FindInUseConnection(const PeerAddress & address)
{
    if (address.GetTransportType() != Type::kTcp)
    {
        return nullptr;
    }

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        auto & conn = mActiveConnections[i];
        if (!conn.InUse())
        {
            continue;
        }

        if (conn.mPeerAddr == address)
        {
            Inet::IPAddress addr;
            uint16_t port;
            if (conn.IsConnected())
            {
                // Failure to get peer information means the connection is bad; close it
                CHIP_ERROR err = conn.mEndPoint->GetPeerInfo(&addr, &port);
                if (err != CHIP_NO_ERROR)
                {
                    CloseConnectionInternal(conn, err, SuppressCallback::No);
                    return nullptr;
                }
            }

            return ActiveTCPConnectionHandle(&conn);
        }
    }

    return nullptr;
}

// Find the ActiveTCPConnectionState for a given TCPEndPoint
ActiveTCPConnectionState * TCPBase::FindActiveConnection(const Inet::TCPEndPointHandle & endPoint)
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].mEndPoint == endPoint && mActiveConnections[i].IsConnected())
        {
            return &mActiveConnections[i];
        }
    }
    return nullptr;
}

ActiveTCPConnectionHandle TCPBase::FindInUseConnection(const Inet::TCPEndPoint & endPoint)
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].mEndPoint == endPoint)
        {
            return ActiveTCPConnectionHandle(&mActiveConnections[i]);
        }
    }
    return nullptr;
}

CHIP_ERROR TCPBase::PrepareBuffer(System::PacketBufferHandle & msgBuf)
{
    // Sent buffer data format is:
    //    - packet size as a uint32_t
    //    - actual data

    VerifyOrReturnError(mState == TCPState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(kPacketSizeBytes + msgBuf->DataLength() <= System::PacketBuffer::kLargeBufMaxSizeWithoutReserve,
                        CHIP_ERROR_INVALID_ARGUMENT);

    static_assert(kPacketSizeBytes <= UINT16_MAX);
    VerifyOrReturnError(msgBuf->EnsureReservedSize(static_cast<uint16_t>(kPacketSizeBytes)), CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - kPacketSizeBytes);

    uint8_t * output = msgBuf->Start();
    LittleEndian::Write32(output, static_cast<uint32_t>(msgBuf->DataLength() - kPacketSizeBytes));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kTcp, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(PrepareBuffer(msgBuf));

    // Must find a previously-established connection with an owning reference
    auto connection = FindInUseConnection(address);
    VerifyOrReturnError(!connection.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    if (connection->IsConnected())
    {
        return connection->mEndPoint->Send(std::move(msgBuf));
    }

    return SendAfterConnect(connection, std::move(msgBuf));
}

CHIP_ERROR TCPBase::SendMessage(const ActiveTCPConnectionHandle & connection, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(!connection.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(PrepareBuffer(msgBuf));

    if (connection->IsConnected())
    {
        return connection->mEndPoint->Send(std::move(msgBuf));
    }

    return SendAfterConnect(connection, std::move(msgBuf));
}

CHIP_ERROR TCPBase::StartConnect(const PeerAddress & addr, Transport::AppTCPConnectionCallbackCtxt * appState,
                                 ActiveTCPConnectionHandle & outPeerConnState)
{
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    Inet::TCPEndPointHandle endPoint;
    outPeerConnState.Release();
    ReturnErrorOnFailure(mListenSocket->GetEndPointManager().NewEndPoint(endPoint));

    InitEndpoint(endPoint);

    ActiveTCPConnectionHandle activeConnection = FindInUseConnection(addr);
    // Re-use existing connection to peer if already connected
    if (!activeConnection.IsNull())
    {
        if (appState != nullptr)
        {
            // We do not support parallel attempts to connect to peer when setting appState
            VerifyOrReturnError(activeConnection->mConnectionState == TCPState::kConnected &&
                                    activeConnection->mAppState == nullptr,
                                CHIP_ERROR_INCORRECT_STATE);
            activeConnection->mAppState = appState;
        }
        outPeerConnState = activeConnection;

        if (activeConnection->mConnectionState == TCPState::kConnected)
        {
            HandleConnectionAttemptComplete(activeConnection, CHIP_NO_ERROR);
        }

        return CHIP_NO_ERROR;
    }

    activeConnection = AllocateConnection(endPoint, addr);
    VerifyOrReturnError(!activeConnection.IsNull(), CHIP_ERROR_NO_MEMORY);
    activeConnection->mAppState        = appState;
    activeConnection->mConnectionState = TCPState::kConnecting;

    ReturnErrorOnFailure(endPoint->Connect(addr.GetIPAddress(), addr.GetPort(), addr.GetInterface()));

    mUsedEndPointCount++;

    // Set the return value of the peer connection state to the allocated
    // connection.
    outPeerConnState = activeConnection;

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR TCPBase::SendAfterConnect(const ActiveTCPConnectionHandle & existing, System::PacketBufferHandle && msg)
{
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    VerifyOrReturnError(!existing.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    const PeerAddress & addr = existing->mPeerAddr;

    // This will initiate a connection to the specified peer
    bool alreadyConnecting = false;

    // Iterate through the ENTIRE array. If a pending packet for
    // the address already exists, this means a connection is pending and
    // does NOT need to be re-established.
    mPendingPackets.ForEachActiveObject([&](PendingPacket * pending) {
        if (pending->mPeerAddress == addr)
        {
            // same destination exists.
            alreadyConnecting = true;
            pending->mPacketBuffer->AddToEnd(std::move(msg));
            return Loop::Break;
        }
        return Loop::Continue;
    });

    // If already connecting, buffer was just enqueued for more sending
    if (alreadyConnecting)
    {
        return CHIP_NO_ERROR;
    }

    // enqueue the packet once the connection succeeds
    VerifyOrReturnError(mPendingPackets.CreateObject(addr, std::move(msg)) != nullptr, CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR TCPBase::ProcessReceivedBuffer(const Inet::TCPEndPointHandle & endPoint, const PeerAddress & peerAddress,
                                          System::PacketBufferHandle && buffer)
{
    ActiveTCPConnectionState * state = FindActiveConnection(endPoint);
    // There must be a preceding TCPConnect to hold a reference to connection
    VerifyOrReturnError(state != nullptr, CHIP_ERROR_INTERNAL);
    state->mReceived.AddToEnd(std::move(buffer));

    while (!state->mReceived.IsNull())
    {
        uint8_t messageSizeBuf[kPacketSizeBytes];
        CHIP_ERROR err = state->mReceived->Read(messageSizeBuf);
        if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // We don't have enough data to read the message size. Wait until there's more.
            return CHIP_NO_ERROR;
        }
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        uint32_t messageSize = LittleEndian::Get32(messageSizeBuf);
        if (messageSize >= kMaxTCPMessageSize)
        {
            // Message is too big for this node to process. Disconnect from peer.
            ChipLogError(Inet, "Received TCP message of length %" PRIu32 " exceeds limit.", messageSize);
            CloseConnectionInternal(*state, CHIP_ERROR_MESSAGE_TOO_LONG, SuppressCallback::No);

            return CHIP_ERROR_MESSAGE_TOO_LONG;
        }
        // The subtraction will not underflow because we successfully read kPacketSizeBytes.
        if (messageSize > (state->mReceived->TotalLength() - kPacketSizeBytes))
        {
            // We have not yet received the complete message.
            return CHIP_NO_ERROR;
        }

        state->mReceived.Consume(kPacketSizeBytes);

        if (messageSize == 0)
        {
            // No payload but considered a valid message. Return success to keep the connection alive.
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(ProcessSingleMessage(peerAddress, *state, messageSize));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPBase::ProcessSingleMessage(const PeerAddress & peerAddress, ActiveTCPConnectionState & state, size_t messageSize)
{
    // We enter with `state->mReceived` containing at least one full message, perhaps in a chain.
    // `state->mReceived->Start()` currently points to the message data.
    // On exit, `state->mReceived` will have had `messageSize` bytes consumed, no matter what.
    System::PacketBufferHandle message;

    if (state.mReceived->DataLength() == messageSize)
    {
        // In this case, the head packet buffer contains exactly the message.
        // This is common because typical messages fit in a network packet, and are delivered as such.
        // Peel off the head to pass upstream, which effectively consumes it from `state->mReceived`.
        message = state.mReceived.PopHead();
    }
    else
    {
        // The message is either longer or shorter than the head buffer.
        // In either case, copy the message to a fresh linear buffer to pass upstream. We always copy, rather than provide
        // a shared reference to the current buffer, in case upper layers manipulate the buffer in ways that would affect
        // our use, e.g. chaining it elsewhere or reusing space beyond the current message.
        message = System::PacketBufferHandle::New(messageSize, 0);
        if (message.IsNull())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        CHIP_ERROR err = state.mReceived->Read(message->Start(), messageSize);
        state.mReceived.Consume(messageSize);
        ReturnErrorOnFailure(err);
        message->SetDataLength(messageSize);
    }

    MessageTransportContext msgContext;
    msgContext.conn = &state; // Take ownership
    HandleMessageReceived(peerAddress, std::move(message), &msgContext);
    return CHIP_NO_ERROR;
}

void TCPBase::CloseConnectionInternal(ActiveTCPConnectionState & connection, CHIP_ERROR err, SuppressCallback suppressCallback)
{
    if (connection.mConnectionState == TCPState::kClosed || !connection.mEndPoint)
    {
        return;
    }
    TCPState prevState;
    char addrStr[Transport::PeerAddress::kMaxToStringSize];
    connection.mPeerAddr.ToString(addrStr);
    ChipLogProgress(Inet, "Closing connection with peer %s.", addrStr);

    Inet::TCPEndPointHandle endpoint = connection.mEndPoint;
    connection.mEndPoint.Release();
    if (err == CHIP_NO_ERROR)
    {
        endpoint->Close();
    }
    else
    {
        endpoint->Abort();
    }

    prevState                   = connection.mConnectionState;
    connection.mConnectionState = TCPState::kClosed;

    if (suppressCallback == SuppressCallback::No)
    {
        if (prevState == TCPState::kConnecting)
        {
            ActiveTCPConnectionHandle holder(&connection);
            // Call upper layer connection attempt complete handler
            HandleConnectionAttemptComplete(holder, err);
        }
        else
        {
            // Call upper layer connection closed handler
            HandleConnectionClosed(connection, err);
        }
    }

    connection.Free();
    mUsedEndPointCount--;
}

CHIP_ERROR TCPBase::HandleTCPEndPointDataReceived(const Inet::TCPEndPointHandle & endPoint, System::PacketBufferHandle && buffer)
{
    PeerAddress peerAddress;
    ReturnErrorOnFailure(GetPeerAddress(*endPoint, peerAddress));

    TCPBase * tcp  = reinterpret_cast<TCPBase *>(endPoint->mAppState);
    CHIP_ERROR err = tcp->ProcessReceivedBuffer(endPoint, peerAddress, std::move(buffer));

    if (err != CHIP_NO_ERROR)
    {
        // Connection could need to be closed at this point
        ChipLogError(Inet, "Failed to accept received TCP message: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_UNEXPECTED_EVENT;
    }
    return CHIP_NO_ERROR;
}

void TCPBase::HandleTCPEndPointConnectComplete(const Inet::TCPEndPointHandle & endPoint, CHIP_ERROR conErr)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool foundPendingPacket = false;
    TCPBase * tcp           = reinterpret_cast<TCPBase *>(endPoint->mAppState);
    ActiveTCPConnectionHandle activeConnection;

    PeerAddress addr;
    char addrStr[Transport::PeerAddress::kMaxToStringSize];
    activeConnection = tcp->FindInUseConnection(*endPoint);
    if (activeConnection.IsNull())
    {
        err = GetPeerAddress(*endPoint, addr);
    }
    else
    {
        addr = activeConnection->mPeerAddr;
    }
    if (err == CHIP_NO_ERROR)
    {
        addr.ToString(addrStr);
    }

    if (conErr != CHIP_NO_ERROR || err != CHIP_NO_ERROR)
    {
        auto failure = (conErr != CHIP_NO_ERROR) ? conErr : err;
        if (!activeConnection.IsNull())
        {
            tcp->CloseConnectionInternal(*activeConnection, failure, SuppressCallback::No);
        }
        ChipLogError(Inet, "Connection establishment with %s encountered an error: %" CHIP_ERROR_FORMAT, addrStr, failure.Format());
        return;
    }
    // Set the Data received handler when connection completes
    endPoint->OnDataReceived     = HandleTCPEndPointDataReceived;
    endPoint->OnDataSent         = nullptr;
    endPoint->OnConnectionClosed = HandleTCPEndPointConnectionClosed;

    VerifyOrDie(!activeConnection.IsNull());

    // Set to Connected state
    activeConnection->mConnectionState = TCPState::kConnected;

    // Disable TCP Nagle buffering by setting TCP_NODELAY socket option to true.
    // This is to expedite transmission of payload data and not rely on the
    // network stack's configuration of collating enough data in the TCP
    // window to begin transmission.
    err = endPoint->EnableNoDelay();
    if (err != CHIP_NO_ERROR)
    {
        tcp->CloseConnectionInternal(*activeConnection, err, SuppressCallback::No);
        return;
    }

    // Send any pending packets that are queued for this connection
    tcp->mPendingPackets.ForEachActiveObject([&](PendingPacket * pending) {
        if (pending->mPeerAddress == addr)
        {
            foundPendingPacket                = true;
            System::PacketBufferHandle buffer = std::move(pending->mPacketBuffer);
            tcp->mPendingPackets.ReleaseObject(pending);

            if ((conErr == CHIP_NO_ERROR) && (err == CHIP_NO_ERROR))
            {
                // TODO(gmarcosb): These errors are just swallowed; caller unaware their message is just dropped?
                // Likely just falls through to a timeout instead of fail-fast
                err = endPoint->Send(std::move(buffer));
            }
        }
        return Loop::Continue;
    });

    // Set the TCPKeepalive configurations on the established connection
    TEMPORARY_RETURN_IGNORED endPoint->EnableKeepAlive(activeConnection->mTCPKeepAliveIntervalSecs,
                                                       activeConnection->mTCPMaxNumKeepAliveProbes);

    ChipLogProgress(Inet, "Connection established successfully with %s.", addrStr);

    // Let higher layer/delegate know that connection is successfully
    // established
    tcp->HandleConnectionAttemptComplete(activeConnection, CHIP_NO_ERROR);
}

void TCPBase::HandleTCPEndPointConnectionClosed(const Inet::TCPEndPointHandle & endPoint, CHIP_ERROR err)
{
    TCPBase * tcp                              = reinterpret_cast<TCPBase *>(endPoint->mAppState);
    ActiveTCPConnectionHandle activeConnection = tcp->FindInUseConnection(*endPoint);

    if (activeConnection.IsNull())
    {
        return;
    }

    if (err == CHIP_NO_ERROR && activeConnection->IsConnected())
    {
        err = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;
    }

    tcp->CloseConnectionInternal(*activeConnection, err, SuppressCallback::No);
}

// Handler for incoming connection requests from peer nodes
void TCPBase::HandleIncomingConnection(const Inet::TCPEndPointHandle & listenEndPoint, const Inet::TCPEndPointHandle & endPoint,
                                       const Inet::IPAddress & peerAddress, uint16_t peerPort)
{
    TCPBase * tcp = reinterpret_cast<TCPBase *>(listenEndPoint->mAppState);
    ReturnAndLogOnFailure(tcp->DoHandleIncomingConnection(listenEndPoint, endPoint, peerAddress, peerPort), Inet,
                          "Failure accepting incoming connection");
}

CHIP_ERROR TCPBase::DoHandleIncomingConnection(const Inet::TCPEndPointHandle & listenEndPoint,
                                               const Inet::TCPEndPointHandle & endPoint, const Inet::IPAddress & peerAddress,
                                               uint16_t peerPort)
{
#if INET_CONFIG_TEST
    if (sForceFailureInDoHandleIncomingConnection)
    {
        return CHIP_ERROR_INTERNAL;
    }
#endif

    // GetPeerAddress may fail if the client has already closed the connection, just drop it.
    PeerAddress addr;
    ReturnErrorOnFailure(GetPeerAddress(*endPoint, addr));

    ActiveTCPConnectionState * activeConnection = AllocateConnection(endPoint, addr);
    VerifyOrReturnError(activeConnection != nullptr, CHIP_ERROR_TOO_MANY_CONNECTIONS);

    auto connectionCleanup = ScopeExit([&]() { activeConnection->Free(); });

    endPoint->mAppState          = this;
    endPoint->OnDataReceived     = HandleTCPEndPointDataReceived;
    endPoint->OnDataSent         = nullptr;
    endPoint->OnConnectionClosed = HandleTCPEndPointConnectionClosed;

    // By default, disable TCP Nagle buffering by setting TCP_NODELAY socket option to true
    // If it fails, we can still use the connection
    RETURN_SAFELY_IGNORED endPoint->EnableNoDelay();

    mUsedEndPointCount++;
    activeConnection->mConnectionState = TCPState::kConnected;

    // Set the TCPKeepalive configurations on the received connection
    // If it fails, we can still use the connection until it dies
    RETURN_SAFELY_IGNORED endPoint->EnableKeepAlive(activeConnection->mTCPKeepAliveIntervalSecs,
                                                    activeConnection->mTCPMaxNumKeepAliveProbes);

    char addrStr[Transport::PeerAddress::kMaxToStringSize];
    peerAddress.ToString(addrStr);
    ChipLogProgress(Inet, "Incoming connection established with peer at %s.", addrStr);

    // Call the upper layer handler for incoming connection received.
    HandleConnectionReceived(*activeConnection);

    connectionCleanup.release();

    return CHIP_NO_ERROR;
}

void TCPBase::HandleAcceptError(const Inet::TCPEndPointHandle & listeningEndpoint, CHIP_ERROR err)
{
    ChipLogError(Inet, "Accept error: %" CHIP_ERROR_FORMAT, err.Format());
}

CHIP_ERROR TCPBase::TCPConnect(const PeerAddress & address, Transport::AppTCPConnectionCallbackCtxt * appState,
                               ActiveTCPConnectionHandle & outPeerConnState)
{
    VerifyOrReturnError(mState == TCPState::kInitialized, CHIP_ERROR_INCORRECT_STATE);

    // Verify that PeerAddress AddressType is TCP
    VerifyOrReturnError(address.GetTransportType() == Transport::Type::kTcp, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(mUsedEndPointCount < mActiveConnectionsSize, CHIP_ERROR_NO_MEMORY);

    char addrStr[Transport::PeerAddress::kMaxToStringSize];
    address.ToString(addrStr);
    ChipLogProgress(Inet, "Connecting to peer %s.", addrStr);

    ReturnErrorOnFailure(StartConnect(address, appState, outPeerConnState));

    return CHIP_NO_ERROR;
}

void TCPBase::TCPDisconnect(ActiveTCPConnectionState & conn, bool shouldAbort)
{
    // If there are still active references, we need to notify them of connection closure
    SuppressCallback suppressCallback = (conn.GetReferenceCount() > 0) ? SuppressCallback::No : SuppressCallback::Yes;

    // This call should be able to disconnect the connection either when it is
    // already established, or when it is being set up.
    if ((conn.IsConnected() && shouldAbort) || conn.IsConnecting())
    {
        CloseConnectionInternal(conn, CHIP_ERROR_CONNECTION_ABORTED, suppressCallback);
    }

    if (conn.IsConnected() && !shouldAbort)
    {
        CloseConnectionInternal(conn, CHIP_NO_ERROR, suppressCallback);
    }
}

bool TCPBase::HasActiveConnections() const
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].IsConnected())
        {
            return true;
        }
    }

    return false;
}

void TCPBase::InitEndpoint(const Inet::TCPEndPointHandle & endpoint)
{
    endpoint->mAppState         = reinterpret_cast<void *>(this);
    endpoint->OnConnectComplete = HandleTCPEndPointConnectComplete;
    endpoint->SetConnectTimeout(mConnectTimeout);
}

#if INET_CONFIG_TEST
bool TCPBase::sForceFailureInDoHandleIncomingConnection = false;
#endif

} // namespace Transport
} // namespace chip
