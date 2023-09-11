/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *    Defines base properties and constants valid across all transport
 *    classes (UDP, TCP, BLE, ....)
 */

#pragma once

#include <inet/IPAddress.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPError.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

class RawTransportDelegate
{
public:
    virtual ~RawTransportDelegate() {}
    virtual void HandleMessageReceived(const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg) = 0;
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

    /**
     * Handle disconnection from the specified peer if currently connected to it.
     */
    virtual void Disconnect(const PeerAddress & address) {}

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
    void HandleMessageReceived(const PeerAddress & source, System::PacketBufferHandle && buffer)
    {
        mDelegate->HandleMessageReceived(source, std::move(buffer));
    }

    RawTransportDelegate * mDelegate;
};

} // namespace Transport
} // namespace chip
