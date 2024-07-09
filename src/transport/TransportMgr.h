/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 */

/**
 * @file
 *   This file implements a stateless TransportMgr, it will took a raw message
 * buffer from transports, and then extract the message header without decode it.
 * For secure messages, it will pass it to the SessionManager, and for unsecure
 * messages (rendezvous messages), it will pass it to RendezvousSession.
 *   When sending messages, it will encode the packet header, and pass it to the
 * transports.
 *   The whole process is fully stateless.
 */

#pragma once

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/TransportMgrBase.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/Tuple.h>
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <transport/raw/TCP.h>
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

namespace chip {

class TransportMgrBase;

class TransportMgrDelegate
{
public:
    virtual ~TransportMgrDelegate() = default;
    /**
     * @brief
     *   Handle received secure message.
     *
     * @param source    the source address of the package
     * @param msgBuf    the buffer containing a full CHIP message (except for the optional length field).
     * @param ctxt      the pointer to additional context on the underlying transport. For TCP, it is a pointer
     *                  to the underlying connection object.
     */
    virtual void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                                   Transport::MessageTransportContext * ctxt = nullptr) = 0;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    /**
     * @brief
     *   Handle connection attempt completion.
     *
     * @param conn      the connection object
     * @param conErr    the connection error on the attempt, or CHIP_NO_ERROR.
     */
    virtual void HandleConnectionAttemptComplete(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr){};

    virtual void HandleConnectionClosed(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr){};

    virtual void HandleConnectionReceived(Transport::ActiveTCPConnectionState * conn){};
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
};

template <typename... TransportTypes>
class TransportMgr : public TransportMgrBase
{
public:
    template <typename... Args>
    CHIP_ERROR Init(Args &&... transportInitArgs)
    {
        ReturnErrorOnFailure(mTransport.Init(this, std::forward<Args>(transportInitArgs)...));
        return TransportMgrBase::Init(&mTransport);
    }

    template <typename... Args>
    CHIP_ERROR ResetTransport(Args &&... transportInitArgs)
    {
        return mTransport.Init(this, std::forward<Args>(transportInitArgs)...);
    }

    void Close()
    {
        TransportMgrBase::Close();
        mTransport.Close();
    };

private:
    Transport::Tuple<TransportTypes...> mTransport;

public:
    auto & GetTransport() { return mTransport; }
};

} // namespace chip
