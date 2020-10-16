/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *    @file
 *      Declaration of CHIP Device Controller, a common class
 *      that implements connecting and messaging and will later
 *      be expanded to support discovery, pairing and
 *      provisioning of CHIP  devices.
 *
 */

#pragma once

#include <controller/CHIPPersistentStorageDelegate.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/DLLUtil.h>
#include <transport/RendezvousSession.h>
#include <transport/RendezvousSessionDelegate.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

namespace chip {
namespace DeviceController {

class ChipDeviceController;

typedef void (*NewConnectionHandler)(ChipDeviceController * deviceController, Transport::PeerConnectionState * state,
                                     void * appReqState);
typedef void (*CompleteHandler)(ChipDeviceController * deviceController, void * appReqState);
typedef void (*ErrorHandler)(ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR err,
                             const Inet::IPPacketInfo * pktInfo);
typedef void (*MessageReceiveHandler)(ChipDeviceController * deviceController, void * appReqState, System::PacketBuffer * payload);

class DLL_EXPORT DevicePairingDelegate
{
public:
    /**
     * @brief
     *   Called when the pairing reaches a certain stage.
     *
     * @param status Current status of pairing
     */
    virtual void OnStatusUpdate(RendezvousSessionDelegate::Status status) {}

    /**
     * @brief
     *   Called when the network credentials are needed for the remote device
     *
     * @param callback Callback delegate that provisions the network credentials
     */
    virtual void OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback) = 0;

    /**
     * @brief
     *   Called when the operational credentials are needed for the remote device
     *
     * @param csr Certificate signing request from the device
     * @param csr_length The length of CSR
     * @param callback Callback delegate that provisions the operational credentials
     */
    virtual void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                   RendezvousDeviceCredentialsDelegate * callback) = 0;

    /**
     * @brief
     *   Called when the pairing is complete (with success or error)
     *
     * @param error Error cause, if any
     */
    virtual void OnPairingComplete(CHIP_ERROR error) {}

    /**
     * @brief
     *   Called when the pairing is deleted (with success or error)
     *
     * @param error Error cause, if any
     */
    virtual void OnPairingDeleted(CHIP_ERROR error) {}
};

class DLL_EXPORT ChipDeviceController : public SecureSessionMgrDelegate, public RendezvousSessionDelegate
{
    friend class ChipDeviceControllerCallback;

public:
    ChipDeviceController();
    ~ChipDeviceController();

    void * AppState;

    /**
     * Init function to be used when there exists a device layer that takes care of initializing
     * System::Layer and InetLayer.
     */
    CHIP_ERROR Init(NodeId localDeviceId);
    /**
     * Init function to be used when already-initialized System::Layer and InetLayer are available.
     */
    CHIP_ERROR Init(NodeId localDeviceId, System::Layer * systemLayer, Inet::InetLayer * inetLayer);
    CHIP_ERROR Init(NodeId localDeviceId, DevicePairingDelegate * pairingDelegate,
                    PersistentStorageDelegate * storageDelegate = nullptr);
    CHIP_ERROR Shutdown();

    // ----- Connection Management -----
    /**
     * @brief
     *   Connect to a CHIP device with the provided Rendezvous connection parameters
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] params                The Rendezvous connection parameters
     * @param[in] appReqState           Application specific context to be passed back when a message is received or on error
     * @param[in] onConnected           Callback for when the connection is established
     * @param[in] onMessageReceived     Callback for when a message is received
     * @param[in] onError               Callback for when an error occurs
     * @param[in] devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @param[in] interfaceId           [Optional] The interface indicator to use
     *
     * @return CHIP_ERROR               The connection status
     */
    CHIP_ERROR ConnectDevice(NodeId remoteDeviceId, RendezvousParameters & params, void * appReqState,
                             NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived, ErrorHandler onError,
                             uint16_t devicePort = CHIP_PORT, Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID);

    /**
     * @brief
     *   Connect to a CHIP device at a given address and an optional port. This is a test only API
     *   that bypasses Rendezvous and Secure Pairing process.
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] deviceAddr            The IPAddress of the requested Device
     * @param[in] appReqState           Application specific context to be passed back when a message is received or on error
     * @param[in] onConnected           Callback for when the connection is established
     * @param[in] onMessageReceived     Callback for when a message is received
     * @param[in] onError               Callback for when an error occurs
     * @param[in] devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @param[in] interfaceId           [Optional] The interface indicator to use
     * @return CHIP_ERROR           The connection status
     */
    [[deprecated("Available until Rendezvous is implemented")]] CHIP_ERROR
    ConnectDeviceWithoutSecurePairing(NodeId remoteDeviceId, const Inet::IPAddress & deviceAddr, void * appReqState,
                                      NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived,
                                      ErrorHandler onError, uint16_t devicePort = CHIP_PORT,
                                      Inet::InterfaceId interfaceId = INET_NULL_INTERFACEID);

    /**
     * @brief
     *   Disconnect from a connected device
     *
     * @return CHIP_ERROR   If the device was disconnected successfully
     */
    CHIP_ERROR DisconnectDevice();

    /**
     * @brief
     *   Check if there's an active connection
     *
     * @return bool   If there is an active connection
     */
    bool IsConnected();

    /**
     * @brief
     *   Check if the connection is active and security context is established
     *
     * @return bool   If the connection is active and security context is established
     */
    bool IsSecurelyConnected();

    // ----- Messaging -----
    /**
     * @brief
     *   Send a message to a connected CHIP device
     *
     * @param[in] appReqState   Application specific context to be passed back when a message is received or on error
     * @param[in] buffer        The Data Buffer to trasmit to the deviec
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR SendMessage(void * appReqState, System::PacketBuffer * buffer);

    // ----- IO -----
    /**
     * @brief
     * Start the event loop task within the CHIP stack
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR ServiceEvents();

    /**
     * @brief
     *   Allow the CHIP Stack to process any pending events
     *   This can be called in an event handler loop to tigger callbacks within the CHIP stack
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR ServiceEventSignal();

    void OnMessageReceived(const PacketHeader & header, Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf,
                           SecureSessionMgrBase * mgr) override;

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override;

    //////////// RendezvousSessionDelegate Implementation ///////////////
    void OnRendezvousComplete() override;
    void OnRendezvousMessageReceived(System::PacketBuffer * buffer) override;
    void OnRendezvousStatusUpdate(RendezvousSessionDelegate::Status status, CHIP_ERROR err) override;

private:
    enum
    {
        kState_NotInitialized = 0,
        kState_Initialized    = 1
    } mState;

    enum ConnectionState
    {
        kConnectionState_NotConnected    = 0,
        kConnectionState_Connected       = 1,
        kConnectionState_SecureConnected = 2,
    };

    System::Layer * mSystemLayer;
    Inet::InetLayer * mInetLayer;

    SecureSessionMgr<Transport::UDP> * mSessionManager;
    RendezvousSession * mRendezvousSession;

    ConnectionState mConState;
    void * mAppReqState;

    union
    {
        CompleteHandler General;
        MessageReceiveHandler Response;
    } mOnComplete;

    ErrorHandler mOnError;
    NewConnectionHandler mOnNewConnection;
    System::PacketBuffer * mCurReqMsg;

    NodeId mLocalDeviceId;
    Inet::IPAddress mDeviceAddr;
    uint16_t mDevicePort;
    Inet::InterfaceId mInterface;
    Optional<NodeId> mRemoteDeviceId;

    SecurePairingSession mPairingSession;
    SecurePairingUsingTestSecret * mTestSecurePairingSecret = nullptr;

    SecurePairingSession * mSecurePairingSession = nullptr;

    DevicePairingDelegate * mPairingDelegate;

    PersistentStorageDelegate * mStorageDelegate;

    void ClearRequestState();
    void ClearOpState();

    CHIP_ERROR EstablishSecureSession(const NodeId & peer);
    CHIP_ERROR ResumeSecureSession(const NodeId & peer);
};

} // namespace DeviceController
} // namespace chip
