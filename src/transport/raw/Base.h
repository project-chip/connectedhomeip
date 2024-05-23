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

/**
 * @file
 *    Defines base properties and constants valid across all transport
 *    classes (UDP, TCP, BLE, ....)
 */

#pragma once

#include <inet/IPAddress.h>
#include <inet/TCPEndPoint.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPError.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/raw/ActiveTCPConnectionState.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

namespace chip {
namespace Transport {

struct MessageTransportContext
{
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    ActiveTCPConnectionState * conn = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
};

class RawTransportDelegate
{
public:
    virtual ~RawTransportDelegate() {}
    virtual void HandleMessageReceived(const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg,
                                       MessageTransportContext * ctxt = nullptr) = 0;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    virtual void HandleConnectionReceived(ActiveTCPConnectionState * conn){};
    virtual void HandleConnectionAttemptComplete(ActiveTCPConnectionState * conn, CHIP_ERROR conErr){};
    virtual void HandleConnectionClosed(ActiveTCPConnectionState * conn, CHIP_ERROR conErr){};
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
};

/**
 * Transport class base, defining common methods among transports (message
 * packing by encoding and decoding headers) and generic message transport
 * methods.
 */
class Base
{
public:
    virtual ~Base() {}

    /**
     * Sets the delegate of the transport
     *
     * @param[in] delegate  The argument to pass in to the handler function
     *
     */
    void SetDelegate(RawTransportDelegate * delegate) { mDelegate = delegate; }

    /**
     * @brief Send a message to the specified target.
     *
     * On connection-oriented transports, sending a message implies connecting to the target first.
     */
    virtual CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) = 0;

    /**
     * Determine if this transport can SendMessage to the specified peer address.
     *
     * Generally it is expected that a transport can send to any peer from which it receives a message.
     */
    virtual bool CanSendToPeer(const PeerAddress & address) = 0;

    /**
     * Determine if this transport can Listen to IPV6 Multicast.
     */
    virtual bool CanListenMulticast() { return false; }

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    /**
     * Connect to the specified peer.
     */
    virtual CHIP_ERROR TCPConnect(const PeerAddress & address, Transport::AppTCPConnectionCallbackCtxt * appState,
                                  Transport::ActiveTCPConnectionState ** peerConnState)
    {
        return CHIP_NO_ERROR;
    }

    /**
     * Handle disconnection from the specified peer if currently connected to it.
     */
    virtual void TCPDisconnect(const PeerAddress & address) {}

    /**
     * Disconnect on the active connection that is passed in.
     */
    virtual void TCPDisconnect(Transport::ActiveTCPConnectionState * conn, bool shouldAbort = 0) {}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    /**
     * Enable Listening for multicast messages ( IPV6 UDP only)
     */
    virtual CHIP_ERROR MulticastGroupJoinLeave(const Transport::PeerAddress & address, bool join) { return CHIP_ERROR_INTERNAL; }

    /**
     * Close the open endpoint without destroying the object
     */
    virtual void Close() {}

protected:
    /**
     * Method used by subclasses to notify that a packet has been received after
     * any associated headers have been decoded.
     */
    void HandleMessageReceived(const PeerAddress & source, System::PacketBufferHandle && buffer,
                               MessageTransportContext * ctxt = nullptr)
    {
        mDelegate->HandleMessageReceived(source, std::move(buffer), ctxt);
    }

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Handle an incoming connection request from a peer.
    void HandleConnectionReceived(ActiveTCPConnectionState * conn) { mDelegate->HandleConnectionReceived(conn); }

    // Callback during connection establishment to notify of success or any
    // error.
    void HandleConnectionAttemptComplete(ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
    {
        mDelegate->HandleConnectionAttemptComplete(conn, conErr);
    }

    // Callback to notify the higher layer of an unexpected connection closure.
    void HandleConnectionClosed(ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
    {
        mDelegate->HandleConnectionClosed(conn, conErr);
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    RawTransportDelegate * mDelegate = nullptr;
};

} // namespace Transport
} // namespace chip
