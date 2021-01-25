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
    enum class NetworkPreference
    {
        kNetwork_Default,
        kNetwork_HighBandwidth,
        kNetwork_LowLatency,
        kNetwork_LowPowerConsumption,
    };

    enum class TransportPreference
    {
        kTransport_Connectionless,
        kTransport_PreferConnectionOriented, // will fallback to connectionless TCP is not supported
        kTransport_ConnectionOriented,       // will fail if TCP is not supported

        kTransport_Default = kTransport_Connectionless,
    };

    enum class SessionType
    {
        kSession_PASE, // Use SPAKE2 key exchange
        kSession_CASE, // Use SIGMA key exchange

        kSession_Default = kSession_CASE,
    };

    ChannelBuilder & SetPeerNodeId(NodeId peerNodeId)
    {
        mPeerNodeId = peerNodeId;
        return *this;
    }
    NodeId GetPeerNodeId() const { return mPeerNodeId; }

    ChannelBuilder & SetNetworkPreference(NetworkPreference preference)
    {
        mNetworkPreference = preference;
        return *this;
    }
    NetworkPreference GetNetworkPreference() const { return mNetworkPreference; }

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

    uint16_t GetPeerKeyID() const { return mCaseParameters.mPeerKeyId; }
    ChannelBuilder & SetPeerKeyID(uint16_t keyId)
    {
        if (mSessionType == SessionType::kSession_CASE)
            mCaseParameters.mPeerKeyId = keyId;
        return *this;
    }

    uint32_t GetPeerSetUpPINCode() const { return mPaseParameters.mPeerSetUpPINCode; }
    ChannelBuilder & SetPeerSetUpPINCode(uint32_t peerPINCode)
    {
        if (mSessionType == SessionType::kSession_PASE)
            mPaseParameters.mPeerSetUpPINCode = peerPINCode;
        return *this;
    }

#ifndef NDEBUG
    Optional<Inet::IPAddress> GetHintPeerAddress() const { return mHintPeerAddr; }
    ChannelBuilder & SetHintPeerAddress(Inet::IPAddress peerAddr)
    {
        mHintPeerAddr.SetValue(peerAddr);
        return *this;
    }
#endif // NDEBUG

private:
    NodeId mPeerNodeId                       = kUndefinedNodeId;
    NetworkPreference mNetworkPreference     = NetworkPreference::kNetwork_Default;
    TransportPreference mTransportPreference = TransportPreference::kTransport_Default;
    SessionType mSessionType                 = SessionType::kSession_Default;
    union
    {
        struct
        {
            uint16_t mPeerKeyId;
        } mCaseParameters;
        struct
        {
            uint32_t mPeerSetUpPINCode;
        } mPaseParameters;
    };

#ifndef NDEBUG
    Optional<Inet::IPAddress> mHintPeerAddr;
#endif // NDEBUG
};

class ExchangeContext;
class ExchangeDelegate;

enum class ChannelState
{
    kChanneState_None,
    kChanneState_Preparing,
    kChanneState_Ready,
    kChanneState_Closed,
    kChanneState_Failed,
};

class ChannelContextHandleAssociation;

/**
 *  @brief
 *    ChannelHandle is a reference to a channel. An active ChannelHandle will
 *    keep the channel available and ready for use, such that a message can be
 *    sent immediately to the peer.
 *
 *    The ChannelHandle controls the lifespan of the channel. The ChannelHandle
 *    objects are not copyable, so there can be only one instance referring to
 *    the channel. When the handler is released, the channel will be flagged as
 *    pending close, and if there is no active exchange which is using the
 *    channel, the channel will be closed.
 *
 *    The ChannelHandle will track channel status, and notify applications
 *    when the channel state changes via ChannelDelegate.
 */
class ChannelHandle
{
public:
    ChannelHandle(ChannelContextHandleAssociation * association = nullptr) : mAssociation(association) {}
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

    ExchangeContext * NewExchange(ExchangeDelegate * delegate);

    // public APIs
    ChannelState GetState() const;
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
