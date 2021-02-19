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

#include <inet/IPAddress.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Messaging {

/**
 *  @brief
 *    The ChannelBuilder object provides information to build a Channel.
 *
 *    ChannelBuilder can be used by application to provide information to
 *    request a channel to a peer. When a channel is requested via
 *    ExchangeManager::EstablishChannel, a ChannelHandle will be return to
 *    represent the state of the channel
 *
 *    The ChannelBuilder object should be short-live and it is only used within
 *    ExchangeManager::EstablishChannel call, after the call its state will be
 *    copied into an internal object represented by ChannelHandle, then the
 *    ChannelBuilder can be safely released.
 */
class ChannelBuilder
{
public:
    enum class TransportPreference
    {
        kConnectionless,
        kPreferConnectionOriented, // will fallback to connectionless if TCP is not supported
        kConnectionOriented,       // will fail if TCP is not supported

        kDefault = kConnectionless,
    };

    enum class SessionType
    {
        kPASE, // Use SPAKE2 key exchange
        kCASE, // Use SIGMA key exchange

        kDefault = kCASE,
    };

    ChannelBuilder & SetPeerNodeId(NodeId peerNodeId)
    {
        mPeerNodeId = peerNodeId;
        return *this;
    }
    NodeId GetPeerNodeId() const { return mPeerNodeId; }

    ChannelBuilder & SetTransportPreference(TransportPreference preference)
    {
        mTransportPreference = preference;
        return *this;
    }
    TransportPreference GetTransportPreference() const { return mTransportPreference; }

    ChannelBuilder & SetSessionType(SessionType preference)
    {
        mSessionType = preference;
        return *this;
    }
    SessionType GetSessionType() const { return mSessionType; }

    uint16_t GetPeerKeyID() const { return mSessionParameters.mCaseParameters.mPeerKeyId; }
    ChannelBuilder & SetPeerKeyID(uint16_t keyId)
    {
        assert(mSessionType == SessionType::kCASE);
        mSessionParameters.mCaseParameters.mPeerKeyId = keyId;
        return *this;
    }

    uint32_t GetPeerSetUpPINCode() const { return mSessionParameters.mPaseParameters.mPeerSetUpPINCode; }
    ChannelBuilder & SetPeerSetUpPINCode(uint32_t peerPINCode)
    {
        assert(mSessionType == SessionType::kPASE);
        mSessionParameters.mPaseParameters.mPeerSetUpPINCode = peerPINCode;
        return *this;
    }

    Optional<Inet::IPAddress> GetForcePeerAddress() const { return mForcePeerAddr; }
    ChannelBuilder & SetForcePeerAddress(Inet::IPAddress peerAddr)
    {
        mForcePeerAddr.SetValue(peerAddr);
        return *this;
    }

private:
    NodeId mPeerNodeId                       = kUndefinedNodeId;
    TransportPreference mTransportPreference = TransportPreference::kDefault;
    SessionType mSessionType                 = SessionType::kDefault;
    union SessionParameters
    {
        struct
        {
            uint16_t mPeerKeyId;
        } mCaseParameters;
        struct
        {
            uint32_t mPeerSetUpPINCode;
        } mPaseParameters;
    } mSessionParameters;

    Optional<Inet::IPAddress> mForcePeerAddr;
};

class ExchangeContext;
class ExchangeDelegate;

enum class ChannelState
{
    kNone,
    kPreparing,
    kReady,
    kClosed,
    kFailed,
};

class ChannelContextHandleAssociation;

/**
 *  @brief
 *    ChannelHandle is a reference to a channel. An active ChannelHandle will
 *    keep the channel available and ready for use, such that a message can be
 *    sent immediately to the peer.
 *
 *    The ChannelHandle controls the lifespan of the channel. When the handle
 *    is released, the channel will be flagged as pending close, and if there
 *    is no active exchange which is using the channel, the channel will be
 *    closed.
 *
 *    The ChannelHandle will track channel status, and notify applications
 *    when the channel state changes via ChannelDelegate.
 */
class ChannelHandle
{
public:
    explicit ChannelHandle(ChannelContextHandleAssociation * association = nullptr) : mAssociation(association) {}
    ~ChannelHandle() { Release(); }

    // non copyable
    ChannelHandle(const ChannelHandle &) = delete;
    ChannelHandle & operator=(const ChannelHandle &) = delete;

    // movable
    ChannelHandle(ChannelHandle && that)
    {
        Release();
        this->mAssociation = that.mAssociation;
        that.mAssociation  = nullptr;
    }
    ChannelHandle & operator=(ChannelHandle && that)
    {
        Release();
        this->mAssociation = that.mAssociation;
        that.mAssociation  = nullptr;
        return *this;
    }

    ChannelState GetState() const;

    /*
     * @brief
     *  Create a new exchange on the channel.
     *
     * @pre GetState() == ChannelState::kReady
     */
    ExchangeContext * NewExchange(ExchangeDelegate * delegate);

    void Release();

private:
    ChannelContextHandleAssociation * mAssociation;
};

/**
 *  @brief
 *    Callback receiver interface of channels
 */
class ChannelDelegate
{
public:
    virtual ~ChannelDelegate() {}

    virtual void OnEstablished()        = 0;
    virtual void OnClosed()             = 0;
    virtual void OnFail(CHIP_ERROR err) = 0;
};

} // namespace Messaging
} // namespace chip
