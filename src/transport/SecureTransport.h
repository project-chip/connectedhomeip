/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file
 *   This file defines a secure transport layer which adds encryption to data
 *   sent over a transport.
 *
 */

#ifndef __SECURETRANSPORT_H__
#define __SECURETRANSPORT_H__

#include <utility>

#include <core/CHIPCore.h>
#include <core/ReferenceCounted.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>
#include <transport/Base.h>
#include <transport/CHIPSecureChannel.h>

namespace chip {

using namespace System;

class DLL_EXPORT SecureTransport : public ReferenceCounted<SecureTransport>
{
public:
    /**
     *  @enum State
     *
     *  @brief
     *    The State of the CHIP connection object.
     *
     */
    enum class State
    {
        kNotReady,        /**< State before initialization. */
        kInitialized,     /**< State when the connection has been established. */
        kSecureConnected, /**< State when the security of the connection has been established. */
    };

    /**
     * @brief
     *   Initialize a Secure Transport
     *
     * @param transport  The transport to use.
     */
    CHIP_ERROR Init(Transport::Base * transport);

    /**
     * @brief
     *   Attempt to establish a connection to the given peer
     *
     * @param addrType      IPV4 or IPv6 address
     * @param intfId        Indicator for system network interfaces
     * @return CHIP_ERROR   The connection result
     */
    CHIP_ERROR Connect();

    /**
     * @brief
     *   The keypair for the secure channel. This is a utility function that will be used
     *   until we have automatic key exchange in place. The function is useful only for
     *   example applications for now. It will eventually be removed.
     *
     * @param remote_public_key  A pointer to peer's public key
     * @param public_key_length  Length of remote_public_key
     * @param local_private_key  A pointer to local private key
     * @param private_key_length Length of local_private_key
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR ManualKeyExchange(const unsigned char * remote_public_key, const size_t public_key_length,
                                 const unsigned char * local_private_key, const size_t private_key_length);

    /**
     * @brief
     *   Send a message to the currently connected peer
     *
     * @details
     *   This method calls <tt>chip::System::PacketBuffer::Free</tt> on
     *   behalf of the caller regardless of the return status.
     */
    CHIP_ERROR SendMessage(const MessageHeader & header, Inet::IPAddress address, System::PacketBuffer * msgBuf);

    SecureTransport();
    virtual ~SecureTransport() {}

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

private:
    Transport::Base * mTransport;

    IPAddress mPeerAddr;
    uint16_t mPeerPort;
    State mState;
    uint8_t mRefCount;
    ChipSecureChannel mSecureChannel;

    bool StateAllowsSend(void) const { return mState != State::kNotReady; }
    bool StateAllowsReceive(void) const { return mState == State::kSecureConnected; }

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

    static void HandleDataReceived(const MessageHeader & header, const Inet::IPPacketInfo & source, System::PacketBuffer * msgBuf,
                                   SecureTransport * transport);
};

} // namespace chip

#endif // __SECURETRANSPORT_H__
