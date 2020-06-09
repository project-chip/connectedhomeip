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
 * @file  Defines base properties and constants valid across all transport
 *        classes (UDP, TCP, BLE, ....)
 */

#ifndef TRANSPORT_BASE_H_
#define TRANSPORT_BASE_H_

#include <core/CHIPError.h>
#include <core/ReferenceCounted.h>
#include <inet/IPAddress.h>
#include <inet/UDPEndPoint.h>
#include <system/SystemPacketBuffer.h>
#include <transport/MessageHeader.h>

namespace chip {
namespace Transport {

/**
 * Communication path defines how two peers communicate.
 *
 * When a peer contacts another peer, it defines how the peers communicate.
 *
 * Once communication between two peers is established, the same transport
 * path should be used: a client contacting a server over UDP will receive
 * messages back over UDP. A communication channel established over TCP
 * will keep the same TCP channel.
 *
 */
enum class Type
{
    kUndefined,
    kUdp,
    // More constants to be added later, such as TCP and BLE
};

/**
 * Transport class base, defining common methods among transports (message
 * packing by encoding and decoding headers) and generic message transport
 * methods.
 */
class Base : public ReferenceCounted<Base>
{
public:
    virtual ~Base() {}

    /**
     * Sets the message receive handler and associated argument
     *
     * @param[in] handler The callback to call when a message is received
     * @param[in] param   The argument to pass in to the handler function
     *
     */
    template <class T>
    void SetMessageReceiveHandler(void (*handler)(const MessageHeader &, const Inet::IPPacketInfo &, System::PacketBuffer *, T *),
                                  T * param)
    {
        mMessageReceivedArgument = param;
        OnMessageReceived        = reinterpret_cast<MessageReceiveHandler>(handler);
    }

    /**
     * @brief Send a message to the specified target.
     *
     * @details
     *   This method calls <tt>chip::System::PacketBuffer::Free</tt> on
     *   behalf of the caller regardless of the return status.
     *
     * @details
     *   Point to Point transports (e.g. TCP) MUST have a connection already
     *   established and the send will occur over an existing connection.
     *   Connectionless transports (e.g. UDP) are able to attempt a message send
     *   at any time.
     *
     * TODO: if CHIP decides to use ULA, we may be able to remove redundant bits
     *       as node and address could be interchangeable (although some fabric logic may be needed).
     */
    virtual CHIP_ERROR SendMessage(const MessageHeader & header, Inet::IPAddress address, System::PacketBuffer * msgBuf) = 0;

    /**
     * Get the path that this transport is associated with.
     *
     * Within a system, only one transport should be associated with a path.
     */
    virtual Type GetType() = 0;

protected:
    /**
     * Method used by subclasses to notify that a packet has been received after
     * any associated headers have been decoded.
     */
    void HandleMessageReceived(const MessageHeader & header, const Inet::IPPacketInfo & source, System::PacketBuffer * buffer)
    {
        if (OnMessageReceived)
        {
            OnMessageReceived(header, source, buffer, mMessageReceivedArgument);
        }
    }

    /**
     * This function is the application callback that is invoked when a message is received over a
     * Chip connection.
     *
     * @param[in]    msgBuf        A pointer to the PacketBuffer object holding the message.
     */
    typedef void (*MessageReceiveHandler)(const MessageHeader & header, const Inet::IPPacketInfo & source,
                                          System::PacketBuffer * msgBuf, void * param);

    MessageReceiveHandler OnMessageReceived = nullptr; ///< Callback on message receiving
    void * mMessageReceivedArgument         = nullptr; ///< Argument for callback

    uint8_t mRefCount = 0;
};

} // namespace Transport
} // namespace chip

#endif // TRANSPORT_BASE_H_
