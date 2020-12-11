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
 *    @file
 *      This file defines the API classes for to CHIP Channel.
 *
 *      Channel is a object to abstract all low layer dependencies of an
 *      exchange, including secure session, transport connection and network
 *      status.
 *
 *      Channel is not connection. Channel can abstract both connection
 *      oriented and connectionless transports, It contains information to send
 *      and receive messages via exchanges. For example, when using
 *      connectionless transport, channel will contain peer address and session
 *      key; when using connection oriented transport, channel will contain
 *      connection handle and session key.
 *
 *      Channel is not session. Session do persistent through cold reboot, but
 *      channel doesn't. Applications must re-establish channels after a cold
 *      reboot.
 *
 *      Because channel is a local concept, peer device is not able aware of
 *      channel establishment events. Instead, peer device is able to aware
 *      session establishment events, connection establishment events for
 *      connection oriented transport and message received events for
 *      connectionless transport.
 */

#pragma once

#include <messaging/Channel.h>
#include <transport/PeerConnectionState.h>

namespace chip {
namespace Messaging {

class ChannelContext
{
public:
    ChannelState GetState() const { return mState; }
    void Release()
    { /* TODO */
    }

    // events of ResolveDelegate
    // void HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const MdnsService & address);

    // events of SecureSessionManager
    void OnNewConnection(const Transport::PeerConnectionState * state);
    void OnConnectionExpired(const Transport::PeerConnectionState * state);

private:
    ChannelState mState         = ChannelState::kChanneState_Closed;
    ChannelDelegate * mDelegate = nullptr;

    enum PrepareState
    {
        kPrepareState_AddressResolving,
        kPrepareState_SessionEstablishing,
    };

    union
    {
        struct
        {
            PrepareState mState;
            NodeId mPeerNodeId;
        } mPreparing;
        struct
        {
            Transport::PeerConnectionState * mSession;
        } mReady;
    };
};

} // namespace Messaging
} // namespace chip
