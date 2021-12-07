/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *  @file
 *    This file contains definitions for Device class. The objects of this
 *    class will be used by Controller applications to interact with CHIP
 *    devices. The class provides mechanism to construct, send and receive
 *    messages to and from the corresponding CHIP devices.
 */

#pragma once

#include <app/DeviceProxy.h>
#include <app/util/attribute-filter.h>
#include <app/util/basic-types.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#include <lib/dnssd/Resolver.h>

namespace chip {

struct DeviceProxyInitParams
{
    SessionManager * sessionManager          = nullptr;
    Messaging::ExchangeManager * exchangeMgr = nullptr;
    SessionIDAllocator * idAllocator         = nullptr;
    FabricInfo * fabricInfo                  = nullptr;

    Controller::DeviceControllerInteractionModelDelegate * imDelegate = nullptr;

    CHIP_ERROR Validate()
    {
        ReturnErrorCodeIf(sessionManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(exchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(idAllocator == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(fabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);

        return CHIP_NO_ERROR;
    }
};

class OperationalDeviceProxy;

typedef void (*OnDeviceConnected)(void * context, OperationalDeviceProxy * device);
typedef void (*OnDeviceConnectionFailure)(void * context, NodeId deviceId, CHIP_ERROR error);

class DLL_EXPORT OperationalDeviceProxy : public DeviceProxy, SessionReleaseDelegate, public SessionEstablishmentDelegate
{
public:
    virtual ~OperationalDeviceProxy();
    OperationalDeviceProxy(DeviceProxyInitParams & params, PeerId peerId)
    {
        VerifyOrReturn(params.Validate() == CHIP_NO_ERROR);

        mInitParams = params;
        mPeerId     = peerId;

        mState = State::NeedsAddress;
    }

    OperationalDeviceProxy(DeviceProxyInitParams & params, PeerId peerId, const Dnssd::ResolvedNodeData & nodeResolutionData) :
        OperationalDeviceProxy(params, peerId)
    {
        OnNodeIdResolved(nodeResolutionData);
    }

    void Clear();

    /*
     * This function can be called to establish a secure session with the device.
     *
     * The device is expected to have been commissioned, A CASE session
     * setup will be triggered.
     *
     * On establishing the session, the callback function `onConnection` will be called. If the
     * session setup fails, `onFailure` will be called.
     *
     * If the session already exists, `onConnection` will be called immediately.
     */
    CHIP_ERROR Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                       Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    bool IsConnected() const { return mState == State::SecureConnected; }

    bool IsConnecting() const { return mState == State::Connecting; }

    /**
     *   Called when a connection is closing.
     *   The object releases all resources associated with the connection.
     */
    void OnSessionReleased(SessionHandle session) override;

    void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeResolutionData)
    {
        mDeviceAddress = ToPeerAddress(nodeResolutionData);

        mMRPConfig = nodeResolutionData.GetMRPConfig();

        if (mState == State::NeedsAddress)
        {
            mState = State::Initialized;
        }
    }

    /**
     *  Mark any open session with the device as expired.
     */
    CHIP_ERROR Disconnect() override;

    NodeId GetDeviceId() const override { return mPeerId.GetNodeId(); }

    /**
     *   Update data of the device.
     *   This function will set new IP address, port and MRP retransmission intervals of the device.
     *   Since the device settings might have been moved from RAM to the persistent storage, the function
     *   will load the device settings first, before making the changes.
     */
    CHIP_ERROR UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config);

    PeerId GetPeerId() const { return mPeerId; }

    bool MatchesSession(SessionHandle session) const { return mSecureSession.HasValue() && mSecureSession.Value() == session; }

    uint8_t GetNextSequenceNumber() override { return mSequenceNumber++; };

    CHIP_ERROR ShutdownSubscriptions() override;

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

    CASESession & GetCASESession() { return mCASESession; }

    Controller::DeviceControllerInteractionModelDelegate * GetInteractionModelDelegate() override { return mInitParams.imDelegate; }

    Messaging::ExchangeManager * GetExchangeManager() const override { return mInitParams.exchangeMgr; }

    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession; }

    bool GetAddress(Inet::IPAddress & addr, uint16_t & port) const override;

    Transport::PeerAddress GetPeerAddress() const { return mDeviceAddress; }

    static Transport::PeerAddress ToPeerAddress(const Dnssd::ResolvedNodeData & nodeData)
    {
        Inet::InterfaceId interfaceId = Inet::InterfaceId::Null();

        // TODO - Revisit usage of InterfaceID only for addresses that are IPv6 LLA
        // Only use the DNS-SD resolution's InterfaceID for addresses that are IPv6 LLA.
        // For all other addresses, we should rely on the device's routing table to route messages sent.
        // Forcing messages down an InterfaceId might fail. For example, in bridged networks like Thread,
        // mDNS advertisements are not usually received on the same interface the peer is reachable on.
        if (nodeData.mAddress[0].IsIPv6LinkLocal())
        {
            interfaceId = nodeData.mInterfaceId;
        }

        return Transport::PeerAddress::UDP(nodeData.mAddress[0], nodeData.mPort, interfaceId);
    }

private:
    enum class State
    {
        Uninitialized,
        NeedsAddress,
        Initialized,
        Connecting,
        SecureConnected,
    };

    DeviceProxyInitParams mInitParams;

    CASESession mCASESession;

    PeerId mPeerId;

    Transport::PeerAddress mDeviceAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    State mState = State::Uninitialized;

    Optional<SessionHandle> mSecureSession = Optional<SessionHandle>::Missing();

    uint8_t mSequenceNumber = 0;

    Callback::CallbackDeque mConnectionSuccess;
    Callback::CallbackDeque mConnectionFailure;

    CHIP_ERROR EstablishConnection();

    bool IsSecureConnected() const override { return mState == State::SecureConnected; }

    void EnqueueConnectionCallbacks(Callback::Callback<OnDeviceConnected> * onConnection,
                                    Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    void DequeueConnectionSuccessCallbacks(bool executeCallback);
    void DequeueConnectionFailureCallbacks(CHIP_ERROR error, bool executeCallback);
};

} // namespace chip
