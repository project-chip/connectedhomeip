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

#include <app/CASEClient.h>
#include <app/CASEClientPool.h>
#include <app/DeviceProxy.h>
#include <app/util/attribute-filter.h>
#include <app/util/basic-types.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <system/SystemLayer.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#include <lib/dnssd/ResolverProxy.h>

namespace chip {

struct DeviceProxyInitParams
{
    SessionManager * sessionManager          = nullptr;
    Messaging::ExchangeManager * exchangeMgr = nullptr;
    SessionIDAllocator * idAllocator         = nullptr;
    FabricTable * fabricTable                = nullptr;
    CASEClientPoolDelegate * clientPool      = nullptr;

    Optional<ReliableMessageProtocolConfig> mrpLocalConfig = Optional<ReliableMessageProtocolConfig>::Missing();

    CHIP_ERROR Validate() const
    {
        ReturnErrorCodeIf(sessionManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(exchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(idAllocator == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(fabricTable == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(clientPool == nullptr, CHIP_ERROR_INCORRECT_STATE);

        return CHIP_NO_ERROR;
    }
};

class OperationalDeviceProxy;

typedef void (*OnDeviceConnected)(void * context, OperationalDeviceProxy * device);
typedef void (*OnDeviceConnectionFailure)(void * context, PeerId peerId, CHIP_ERROR error);

class DLL_EXPORT OperationalDeviceProxy : public DeviceProxy, SessionReleaseDelegate, public SessionEstablishmentDelegate
{
public:
    virtual ~OperationalDeviceProxy();
    OperationalDeviceProxy(DeviceProxyInitParams & params, PeerId peerId) : mSecureSession(*this)
    {
        VerifyOrReturn(params.Validate() == CHIP_NO_ERROR);

        mSystemLayer = params.exchangeMgr->GetSessionManager()->SystemLayer();
        mInitParams  = params;
        mPeerId      = peerId;
        mFabricInfo  = params.fabricTable->FindFabricWithCompressedId(peerId.GetCompressedFabricId());

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
     * If the resolver is null and the device state is State::NeedsAddress, CHIP_ERROR_INVALID_ARGUMENT will be
     * returned.
     */
    CHIP_ERROR Connect(Callback::Callback<OnDeviceConnected> * onConnection,
                       Callback::Callback<OnDeviceConnectionFailure> * onFailure, Dnssd::ResolverProxy * resolver);

    bool IsConnected() const { return mState == State::SecureConnected; }

    bool IsConnecting() const { return mState == State::Connecting; }

    /**
     *   Called when a connection is closing.
     *   The object releases all resources associated with the connection.
     */
    void OnSessionReleased() override;

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

    /**
     * Use SetConnectedSession if 'this' object is a newly allocated device proxy.
     * It will take an existing session, such as the one established
     * during commissioning, and use it for this device proxy.
     *
     * Note: Avoid using this function generally as it is Deprecated
     */
    void SetConnectedSession(const SessionHandle & handle);

    NodeId GetDeviceId() const override { return mPeerId.GetNodeId(); }

    /**
     *   Update data of the device.
     *   This function will set new IP address, port and MRP retransmission intervals of the device.
     *   Since the device settings might have been moved from RAM to the persistent storage, the function
     *   will load the device settings first, before making the changes.
     */
    CHIP_ERROR UpdateDeviceData(const Transport::PeerAddress & addr, const ReliableMessageProtocolConfig & config);

    PeerId GetPeerId() const { return mPeerId; }

    bool MatchesSession(const SessionHandle & session) const { return mSecureSession.Contains(session); }

    uint8_t GetNextSequenceNumber() override { return mSequenceNumber++; };

    CHIP_ERROR ShutdownSubscriptions() override;

    Messaging::ExchangeManager * GetExchangeManager() const override { return mInitParams.exchangeMgr; }

    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession.ToOptional(); }

    bool GetAddress(Inet::IPAddress & addr, uint16_t & port) const override;

    Transport::PeerAddress GetPeerAddress() const { return mDeviceAddress; }

    static Transport::PeerAddress ToPeerAddress(const Dnssd::ResolvedNodeData & nodeData)
    {
        Transport::PeerAddress address = Transport::PeerAddress(Transport::Type::kUdp);
        address.SetPort(nodeData.mPort);

        for (unsigned i = 0; i < nodeData.mNumIPs; i++)
        {
            const auto addr = nodeData.mAddress[i];
            // Only use the mDNS resolution's InterfaceID for addresses that are IPv6 LLA.
            // For all other addresses, we should rely on the device's routing table to route messages sent.
            // Forcing messages down an InterfaceId might fail. For example, in bridged networks like Thread,
            // mDNS advertisements are not usually received on the same interface the peer is reachable on.
            // TODO: Right now, just use addr0, but we should really push all the addresses and interfaces to
            // the device and allow it to make a proper decision about which addresses are preferred and reachable.
            CHIP_ERROR err = address.AppendDestination(nodeData.mAddress[i],
                                                       addr.IsIPv6LinkLocal() ? nodeData.mInterfaceId : Inet::InterfaceId::Null());

            if (err != CHIP_NO_ERROR)
            {
                char addr_str[Inet::IPAddress::kMaxStringLength];
                addr.ToString(addr_str);

                ChipLogError(Controller, "Could not append IP address %s: %s", addr_str, err.AsString());
            }
        }

        return address;
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
    FabricInfo * mFabricInfo;
    System::Layer * mSystemLayer;

    CASEClient * mCASEClient = nullptr;

    PeerId mPeerId;

    Transport::PeerAddress mDeviceAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    State mState = State::Uninitialized;

    SessionHolderWithDelegate mSecureSession;

    uint8_t mSequenceNumber = 0;

    Callback::CallbackDeque mConnectionSuccess;
    Callback::CallbackDeque mConnectionFailure;

    CHIP_ERROR EstablishConnection();

    bool IsSecureConnected() const override { return mState == State::SecureConnected; }

    static void HandleCASEConnected(void * context, CASEClient * client);
    static void HandleCASEConnectionFailure(void * context, CASEClient * client, CHIP_ERROR error);

    static void CloseCASESessionTask(System::Layer * layer, void * context);

    void CloseCASESession();

    void EnqueueConnectionCallbacks(Callback::Callback<OnDeviceConnected> * onConnection,
                                    Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    void DequeueConnectionSuccessCallbacks(bool executeCallback);
    void DequeueConnectionFailureCallbacks(CHIP_ERROR error, bool executeCallback);
};

} // namespace chip
