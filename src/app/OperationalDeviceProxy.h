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
#include <credentials/GroupDataProvider.h>
#include <lib/address_resolve/AddressResolve.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/secure_channel/CASESession.h>
#include <system/SystemLayer.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

namespace chip {

struct DeviceProxyInitParams
{
    SessionManager * sessionManager                     = nullptr;
    SessionResumptionStorage * sessionResumptionStorage = nullptr;
    Messaging::ExchangeManager * exchangeMgr            = nullptr;
    FabricTable * fabricTable                           = nullptr;
    CASEClientPoolDelegate * clientPool                 = nullptr;
    Credentials::GroupDataProvider * groupDataProvider  = nullptr;

    Optional<ReliableMessageProtocolConfig> mrpLocalConfig = Optional<ReliableMessageProtocolConfig>::Missing();

    CHIP_ERROR Validate() const
    {
        ReturnErrorCodeIf(sessionManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
        // sessionResumptionStorage can be nullptr when resumption is disabled
        ReturnErrorCodeIf(exchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(fabricTable == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(groupDataProvider == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(clientPool == nullptr, CHIP_ERROR_INCORRECT_STATE);

        return CHIP_NO_ERROR;
    }
};

class OperationalDeviceProxy;

typedef void (*OnDeviceConnected)(void * context, OperationalDeviceProxy * device);
typedef void (*OnDeviceConnectionFailure)(void * context, PeerId peerId, CHIP_ERROR error);

/**
 * Represents a connection path to a device that is in an operational state.
 *
 * Handles the lifetime of communicating with such a device:
 *    - Discover the device using DNSSD (find out what IP address to use and what
 *      communication parameters are appropriate for it)
 *    - Establish a secure channel to it via CASE
 *    - Expose to consumers the secure session for talking to the device.
 */
class DLL_EXPORT OperationalDeviceProxy : public DeviceProxy,
                                          public SessionReleaseDelegate,
                                          public SessionEstablishmentDelegate,
                                          public AddressResolve::NodeListener
{
public:
    ~OperationalDeviceProxy() override;

    //
    // TODO: Should not be PeerId, but rather, ScopedNodeId
    //
    OperationalDeviceProxy(DeviceProxyInitParams & params, PeerId peerId) : mSecureSession(*this)
    {
        mInitParams = params;
        if (params.Validate() != CHIP_NO_ERROR)
        {
            mState = State::Uninitialized;
            return;
        }

        mSystemLayer = params.exchangeMgr->GetSessionManager()->SystemLayer();
        mPeerId      = peerId;
        mFabricInfo  = params.fabricTable->FindFabricWithCompressedId(peerId.GetCompressedFabricId());
        mState       = State::NeedsAddress;
        mAddressLookupHandle.SetListener(this);
    }

    OperationalDeviceProxy(DeviceProxyInitParams & params, PeerId peerId, const Dnssd::ResolvedNodeData & nodeResolutionData) :
        OperationalDeviceProxy(params, peerId)
    {
        mAddressLookupHandle.SetListener(this);
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
                       Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    bool IsConnected() const { return mState == State::SecureConnected; }

    bool IsConnecting() const { return mState == State::Connecting; }

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablished(const SessionHandle & session) override;
    void OnSessionEstablishmentError(CHIP_ERROR error) override;

    /**
     *   Called when a connection is closing.
     *   The object releases all resources associated with the connection.
     */
    void OnSessionReleased() override;

    void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeResolutionData)
    {
        mDeviceAddress = ToPeerAddress(nodeResolutionData);

        mRemoteMRPConfig = nodeResolutionData.GetMRPConfig();

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

    bool MatchesSession(const SessionHandle & session) const { return mSecureSession.Contains(session); }

    CHIP_ERROR ShutdownSubscriptions() override;

    Messaging::ExchangeManager * GetExchangeManager() const override { return mInitParams.exchangeMgr; }

    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession.ToOptional(); }

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

    /**
     * @brief Get the raw Fabric ID assigned to the device.
     */
    FabricIndex GetFabricIndex() const
    {
        if (mFabricInfo != nullptr)
        {
            return mFabricInfo->GetFabricIndex();
        }
        return kUndefinedFabricIndex;
    }

    /**
     * Triggers a DNSSD lookup to find a usable peer address for this operational device.
     */
    CHIP_ERROR LookupPeerAddress();

    // AddressResolve::NodeListener - notifications when dnssd finds a node IP address
    void OnNodeAddressResolved(const PeerId & peerId, const AddressResolve::ResolveResult & result) override;
    void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) override;

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

    // mCASEClient is only non-null if we are in State::Connecting or just
    // allocated it as part of an attempt to enter State::Connecting.
    CASEClient * mCASEClient = nullptr;

    PeerId mPeerId;

    Transport::PeerAddress mDeviceAddress = Transport::PeerAddress::UDP(Inet::IPAddress::Any);

    void MoveToState(State aTargetState);

    State mState = State::Uninitialized;

    SessionHolderWithDelegate mSecureSession;

    Callback::CallbackDeque mConnectionSuccess;
    Callback::CallbackDeque mConnectionFailure;

    /// This is used when a node address is required.
    chip::AddressResolve::NodeLookupHandle mAddressLookupHandle;

    CHIP_ERROR EstablishConnection();

    /*
     * This checks to see if an existing CASE session exists to the peer within the SessionManager
     * and if one exists, to load that into mSecureSession.
     *
     * Returns true if a valid session was found, false otherwise.
     *
     */
    bool AttachToExistingSecureSession();

    bool IsSecureConnected() const override { return mState == State::SecureConnected; }

    void CleanupCASEClient();

    void EnqueueConnectionCallbacks(Callback::Callback<OnDeviceConnected> * onConnection,
                                    Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    /*
     * This dequeues all failure and success callbacks and appropriately
     * invokes either set depending on the value of error.
     *
     * If error == CHIP_NO_ERROR, only success callbacks are invoked.
     * Otherwise, only failure callbacks are invoked.
     *
     */
    void DequeueConnectionCallbacks(CHIP_ERROR error);
};

} // namespace chip
