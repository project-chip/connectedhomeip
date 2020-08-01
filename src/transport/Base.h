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
#include <transport/PeerAddress.h>

namespace chip {
namespace Transport {

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
    void SetMessageReceiveHandler(void (*handler)(MessageHeader &, const PeerAddress &, System::PacketBuffer *, T *), T * param)
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
     */
    virtual CHIP_ERROR SendMessage(const MessageHeader & header, const PeerAddress & address, System::PacketBuffer * msgBuf) = 0;

    /**
     * Determine if this transport can SendMessage to the specified peer address.
     *
     * Generally it is expected that a transport can send to any peer from which it receives a message.
     */
    virtual bool CanSendToPeer(const Transport::PeerAddress & address) = 0;

protected:
    /**
     * Method used by subclasses to notify that a packet has been received after
     * any associated headers have been decoded.
     */
    void HandleMessageReceived(const MessageHeader & header, const PeerAddress & source, System::PacketBuffer * buffer)
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
    typedef void (*MessageReceiveHandler)(const MessageHeader & header, const PeerAddress & source, System::PacketBuffer * msgBuf,
                                          void * param);

    MessageReceiveHandler OnMessageReceived = nullptr; ///< Callback on message receiving
    void * mMessageReceivedArgument         = nullptr; ///< Argument for callback
};

} // namespace Transport
} // namespace chip

#endif // TRANSPORT_BASE_H_
