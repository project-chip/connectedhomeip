/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

} // namespace

TCPBase::~TCPBase()
{
    // Call Close to free the listening socket and close all active connections.
    Close();
}

void TCPBase::CloseActiveConnections()
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].InUse())
        {
            CloseConnectionInternal(&mActiveConnections[i], CHIP_NO_ERROR, SuppressCallback::Yes);
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
    err = params.GetEndPointManager()->NewEndPoint(&mListenSocket);
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
        if (mListenSocket)
        {
            mListenSocket->Free();
            mListenSocket = nullptr;
        }
    }

    return err;
}

void TCPBase::Close()
{
    if (mListenSocket)
    {
        mListenSocket->Free();
        mListenSocket = nullptr;
    }

    CloseActiveConnections();

    mState = TCPState::kNotReady;
}

ActiveTCPConnectionState * TCPBase::AllocateConnection()
{
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (!mActiveConnections[i].InUse())
        {
            return &mActiveConnections[i];
        }
    }

    return nullptr;
}

// Find an ActiveTCPConnectionState corresponding to a peer address
ActiveTCPConnectionState * TCPBase::FindActiveConnection(const PeerAddress & address)
{
    if (address.GetTransportType() != Type::kTcp)
    {
        return nullptr;
    }

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (!mActiveConnections[i].IsConnected())
        {
            continue;
        }
        Inet::IPAddress addr;
        uint16_t port;
        mActiveConnections[i].mEndPoint->GetPeerInfo(&addr, &port);

        if ((addr == address.GetIPAddress()) && (port == address.GetPort()))
        {
            return &mActiveConnections[i];
        }
    }

    return nullptr;
}

// Find the ActiveTCPConnectionState for a given TCPEndPoint
ActiveTCPConnectionState * TCPBase::FindActiveConnection(const Inet::TCPEndPoint * endPoint)
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

ActiveTCPConnectionState * TCPBase::FindInUseConnection(const Inet::TCPEndPoint * endPoint)
{
    if (endPoint == nullptr)
    {
        return nullptr;
    }

    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].mEndPoint == endPoint)
        {
            return &mActiveConnections[i];
        }
    }
    return nullptr;
}

CHIP_ERROR TCPBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    // Sent buffer data format is:
    //    - packet size as a uint32_t
    //    - actual data

    VerifyOrReturnError(address.GetTransportType() == Type::kTcp, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState == TCPState::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(kPacketSizeBytes + msgBuf->DataLength() <= System::PacketBuffer::kLargeBufMaxSizeWithoutReserve,
                        CHIP_ERROR_INVALID_ARGUMENT);

    static_assert(kPacketSizeBytes <= UINT16_MAX);
    VerifyOrReturnError(msgBuf->EnsureReservedSize(static_cast<uint16_t>(kPacketSizeBytes)), CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - kPacketSizeBytes);

    uint8_t * output = msgBuf->Start();
    LittleEndian::Write32(output, static_cast<uint32_t>(msgBuf->DataLength() - kPacketSizeBytes));

    // Reuse existing connection if one exists, otherwise a new one
    // will be established
    ActiveTCPConnectionState * connection = FindActiveConnection(address);

    if (connection != nullptr)
    {
        return connection->mEndPoint->Send(std::move(msgBuf));
    }

    return SendAfterConnect(address, std::move(msgBuf));
}

CHIP_ERROR TCPBase::StartConnect(const PeerAddress & addr, Transport::AppTCPConnectionCallbackCtxt * appState,
                                 Transport::ActiveTCPConnectionState ** outPeerConnState)
{
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    ActiveTCPConnectionState * activeConnection = nullptr;
    Inet::TCPEndPoint * endPoint                = nullptr;
    *outPeerConnState                           = nullptr;
    ReturnErrorOnFailure(mListenSocket->GetEndPointManager().NewEndPoint(&endPoint));

    auto EndPointDeletor = [](Inet::TCPEndPoint * e) { e->Free(); };
    std::unique_ptr<Inet::TCPEndPoint, decltype(EndPointDeletor)> endPointHolder(endPoint, EndPointDeletor);

    endPoint->mAppState         = reinterpret_cast<void *>(this);
    endPoint->OnConnectComplete = HandleTCPEndPointConnectComplete;
    endPoint->SetConnectTimeout(mConnectTimeout);

    activeConnection = AllocateConnection();
    VerifyOrReturnError(activeConnection != nullptr, CHIP_ERROR_NO_MEMORY);
    activeConnection->Init(endPoint, addr);
    activeConnection->mAppState        = appState;
    activeConnection->mConnectionState = TCPState::kConnecting;
    // Set the return value of the peer connection state to the allocated
    // connection.
    *outPeerConnState = activeConnection;

    ReturnErrorOnFailure(endPoint->Connect(addr.GetIPAddress(), addr.GetPort(), addr.GetInterface()));

    mUsedEndPointCount++;

    endPointHolder.release();

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR TCPBase::SendAfterConnect(const PeerAddress & addr, System::PacketBufferHandle && msg)
{
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
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

    // Ensures sufficient active connections size exist
    VerifyOrReturnError(mUsedEndPointCount < mActiveConnectionsSize, CHIP_ERROR_NO_MEMORY);

    Transport::ActiveTCPConnectionState * peerConnState = nullptr;
    ReturnErrorOnFailure(StartConnect(addr, nullptr, &peerConnState));

    // enqueue the packet once the connection succeeds
    VerifyOrReturnError(mPendingPackets.CreateObject(addr, std::move(msg)) != nullptr, CHIP_ERROR_NO_MEMORY);
    mUsedEndPointCount++;

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR TCPBase::ProcessReceivedBuffer(Inet::TCPEndPoint * endPoint, const PeerAddress & peerAddress,
                                          System::PacketBufferHandle && buffer)
{
    ActiveTCPConnectionState * state = FindActiveConnection(endPoint);
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
            CloseConnectionInternal(state, CHIP_ERROR_MESSAGE_TOO_LONG, SuppressCallback::No);

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

        ReturnErrorOnFailure(ProcessSingleMessage(peerAddress, state, messageSize));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPBase::ProcessSingleMessage(const PeerAddress & peerAddress, ActiveTCPConnectionState * state, size_t messageSize)
{
    // We enter with `state->mReceived` containing at least one full message, perhaps in a chain.
    // `state->mReceived->Start()` currently points to the message data.
    // On exit, `state->mReceived` will have had `messageSize` bytes consumed, no matter what.
    System::PacketBufferHandle message;
    MessageTransportContext msgContext;
    msgContext.conn = state;

    if (state->mReceived->DataLength() == messageSize)
    {
        // In this case, the head packet buffer contains exactly the message.
        // This is common because typical messages fit in a network packet, and are delivered as such.
        // Peel off the head to pass upstream, which effectively consumes it from `state->mReceived`.
        message = state->mReceived.PopHead();
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
        CHIP_ERROR err = state->mReceived->Read(message->Start(), messageSize);
        state->mReceived.Consume(messageSize);
        ReturnErrorOnFailure(err);
        message->SetDataLength(messageSize);
    }

    HandleMessageReceived(peerAddress, std::move(message), &msgContext);
    return CHIP_NO_ERROR;
}

void TCPBase::CloseConnectionInternal(ActiveTCPConnectionState * connection, CHIP_ERROR err, SuppressCallback suppressCallback)
{
    TCPState prevState;

    if (connection == nullptr)
    {
        return;
    }

    if (connection->mConnectionState != TCPState::kClosed && connection->mEndPoint)
    {
        char addrStr[Transport::PeerAddress::kMaxToStringSize];
        connection->mPeerAddr.ToString(addrStr);
        ChipLogProgress(Inet, "Closing connection with peer %s.", addrStr);

        if (err == CHIP_NO_ERROR)
        {
            connection->mEndPoint->Close();
        }
        else
        {
            connection->mEndPoint->Abort();
        }

        prevState                    = connection->mConnectionState;
        connection->mConnectionState = TCPState::kClosed;

        if (suppressCallback == SuppressCallback::No)
        {
            if (prevState == TCPState::kConnecting)
            {
                // Call upper layer connection attempt complete handler
                HandleConnectionAttemptComplete(connection, err);
            }
            else
            {
                // Call upper layer connection closed handler
                HandleConnectionClosed(connection, err);
            }
        }

        connection->Free();
        mUsedEndPointCount--;
    }
}

CHIP_ERROR TCPBase::HandleTCPEndPointDataReceived(Inet::TCPEndPoint * endPoint, System::PacketBufferHandle && buffer)
{
    Inet::IPAddress ipAddress;
    uint16_t port;
    Inet::InterfaceId interfaceId;

    endPoint->GetPeerInfo(&ipAddress, &port);
    endPoint->GetInterfaceId(&interfaceId);
    PeerAddress peerAddress = PeerAddress::TCP(ipAddress, port, interfaceId);

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

void TCPBase::HandleTCPEndPointConnectComplete(Inet::TCPEndPoint * endPoint, CHIP_ERROR conErr)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool foundPendingPacket = false;
    TCPBase * tcp           = reinterpret_cast<TCPBase *>(endPoint->mAppState);
    Inet::IPAddress ipAddress;
    uint16_t port;
    Inet::InterfaceId interfaceId;
    ActiveTCPConnectionState * activeConnection = nullptr;

    endPoint->GetPeerInfo(&ipAddress, &port);
    endPoint->GetInterfaceId(&interfaceId);
    char addrStr[Transport::PeerAddress::kMaxToStringSize];
    PeerAddress addr = PeerAddress::TCP(ipAddress, port, interfaceId);
    addr.ToString(addrStr);

    if (conErr == CHIP_NO_ERROR)
    {
        // Set the Data received handler when connection completes
        endPoint->OnDataReceived     = HandleTCPEndPointDataReceived;
        endPoint->OnDataSent         = nullptr;
        endPoint->OnConnectionClosed = HandleTCPEndPointConnectionClosed;

        activeConnection = tcp->FindInUseConnection(endPoint);
        VerifyOrDie(activeConnection != nullptr);

        // Set to Connected state
        activeConnection->mConnectionState = TCPState::kConnected;

        // Disable TCP Nagle buffering by setting TCP_NODELAY socket option to true.
        // This is to expedite transmission of payload data and not rely on the
        // network stack's configuration of collating enough data in the TCP
        // window to begin transmission.
        err = endPoint->EnableNoDelay();
        if (err != CHIP_NO_ERROR)
        {
            tcp->CloseConnectionInternal(activeConnection, err, SuppressCallback::No);
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
                    err = endPoint->Send(std::move(buffer));
                }
            }
            return Loop::Continue;
        });

        // Set the TCPKeepalive configurations on the established connection
        endPoint->EnableKeepAlive(activeConnection->mTCPKeepAliveIntervalSecs, activeConnection->mTCPMaxNumKeepAliveProbes);

        ChipLogProgress(Inet, "Connection established successfully with %s.", addrStr);

        // Let higher layer/delegate know that connection is successfully
        // established
        tcp->HandleConnectionAttemptComplete(activeConnection, CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(Inet, "Connection establishment with %s encountered an error: %" CHIP_ERROR_FORMAT, addrStr, err.Format());
        endPoint->Free();
        tcp->mUsedEndPointCount--;
    }
}

void TCPBase::HandleTCPEndPointConnectionClosed(Inet::TCPEndPoint * endPoint, CHIP_ERROR err)
{
    TCPBase * tcp                               = reinterpret_cast<TCPBase *>(endPoint->mAppState);
    ActiveTCPConnectionState * activeConnection = tcp->FindInUseConnection(endPoint);

    if (activeConnection == nullptr)
    {
        endPoint->Free();
        return;
    }

    if (err == CHIP_NO_ERROR && activeConnection->IsConnected())
    {
        err = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;
    }

    tcp->CloseConnectionInternal(activeConnection, err, SuppressCallback::No);
}

// Handler for incoming connection requests from peer nodes
void TCPBase::HandleIncomingConnection(Inet::TCPEndPoint * listenEndPoint, Inet::TCPEndPoint * endPoint,
                                       const Inet::IPAddress & peerAddress, uint16_t peerPort)
{
    TCPBase * tcp                               = reinterpret_cast<TCPBase *>(listenEndPoint->mAppState);
    ActiveTCPConnectionState * activeConnection = nullptr;
    Inet::InterfaceId interfaceId;
    Inet::IPAddress ipAddress;
    uint16_t port;

    endPoint->GetPeerInfo(&ipAddress, &port);
    endPoint->GetInterfaceId(&interfaceId);
    PeerAddress addr = PeerAddress::TCP(ipAddress, port, interfaceId);

    if (tcp->mUsedEndPointCount < tcp->mActiveConnectionsSize)
    {
        activeConnection = tcp->AllocateConnection();

        endPoint->mAppState          = listenEndPoint->mAppState;
        endPoint->OnDataReceived     = HandleTCPEndPointDataReceived;
        endPoint->OnDataSent         = nullptr;
        endPoint->OnConnectionClosed = HandleTCPEndPointConnectionClosed;

        // By default, disable TCP Nagle buffering by setting TCP_NODELAY socket option to true
        endPoint->EnableNoDelay();

        // Update state for the active connection
        activeConnection->Init(endPoint, addr);
        tcp->mUsedEndPointCount++;
        activeConnection->mConnectionState = TCPState::kConnected;

        // Set the TCPKeepalive configurations on the received connection
        endPoint->EnableKeepAlive(activeConnection->mTCPKeepAliveIntervalSecs, activeConnection->mTCPMaxNumKeepAliveProbes);

        char addrStr[Transport::PeerAddress::kMaxToStringSize];
        peerAddress.ToString(addrStr);
        ChipLogProgress(Inet, "Incoming connection established with peer at %s.", addrStr);

        // Call the upper layer handler for incoming connection received.
        tcp->HandleConnectionReceived(activeConnection);
    }
    else
    {
        ChipLogError(Inet, "Insufficient connection space to accept new connections.");
        endPoint->Free();
        listenEndPoint->OnAcceptError(endPoint, CHIP_ERROR_TOO_MANY_CONNECTIONS);
    }
}

void TCPBase::HandleAcceptError(Inet::TCPEndPoint * endPoint, CHIP_ERROR err)
{
    endPoint->Free();
    ChipLogError(Inet, "Accept error: %" CHIP_ERROR_FORMAT, err.Format());
}

CHIP_ERROR TCPBase::TCPConnect(const PeerAddress & address, Transport::AppTCPConnectionCallbackCtxt * appState,
                               Transport::ActiveTCPConnectionState ** outPeerConnState)
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

void TCPBase::TCPDisconnect(const PeerAddress & address)
{
    // Closes an existing connection
    for (size_t i = 0; i < mActiveConnectionsSize; i++)
    {
        if (mActiveConnections[i].IsConnected())
        {
            const Inet::IPAddress & ipAddress = mActiveConnections[i].mPeerAddr.GetIPAddress();
            uint16_t port                     = mActiveConnections[i].mPeerAddr.GetPort();

            // Ignoring the InterfaceID in the check as it may not have been provided in
            // the PeerAddress during connection establishment. The IPAddress and Port
            // are the necessary and sufficient set of parameters for searching
            // through the connections.
            if (ipAddress == address.GetIPAddress() && port == address.GetPort() && address.GetTransportType() == Type::kTcp)
            {
                // NOTE: this leaves the socket in TIME_WAIT.
                // Calling Abort() would clean it since SO_LINGER would be set to 0,
                // however this seems not to be useful.
                CloseConnectionInternal(&mActiveConnections[i], CHIP_NO_ERROR, SuppressCallback::Yes);
            }
        }
    }
}

void TCPBase::TCPDisconnect(Transport::ActiveTCPConnectionState * conn, bool shouldAbort)
{

    if (conn == nullptr)
    {
        ChipLogError(Inet, "Failed to Disconnect. Passed in Connection is null.");
        return;
    }

    // This call should be able to disconnect the connection either when it is
    // already established, or when it is being set up.
    if ((conn->IsConnected() && shouldAbort) || conn->IsConnecting())
    {
        CloseConnectionInternal(conn, CHIP_ERROR_CONNECTION_ABORTED, SuppressCallback::Yes);
    }

    if (conn->IsConnected() && !shouldAbort)
    {
        CloseConnectionInternal(conn, CHIP_NO_ERROR, SuppressCallback::Yes);
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

} // namespace Transport
} // namespace chip
