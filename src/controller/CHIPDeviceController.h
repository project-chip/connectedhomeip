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

#ifndef __CHIPDEVICECONTROLLER_H
#define __CHIPDEVICECONTROLLER_H

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/DLLUtil.h>
#include <transport/BLE.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

namespace chip {
namespace DeviceController {

class ChipDeviceController;

extern "C" {
typedef void (*NewConnectionHandler)(ChipDeviceController * deviceController, Transport::PeerConnectionState * state,
                                     void * appReqState);
typedef void (*CompleteHandler)(ChipDeviceController * deviceController, void * appReqState);
typedef void (*ErrorHandler)(ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR err,
                             const IPPacketInfo * pktInfo);
typedef void (*MessageReceiveHandler)(ChipDeviceController * deviceController, void * appReqState, System::PacketBuffer * payload);
};

class DLL_EXPORT ChipDeviceController : public SecureSessionMgrCallback, public Transport::BLECallbackHandler
{
    friend class ChipDeviceControllerCallback;

public:
    ChipDeviceController();

    void * AppState;

    /**
     * Init function to be used when there exists a device layer that takes care of initializing
     * System::Layer and InetLayer.
     */
    CHIP_ERROR Init(NodeId localDeviceId);
    /**
     * Init function to be used when already-initialized System::Layer and InetLayer are available.
     */
    CHIP_ERROR Init(NodeId localDeviceId, System::Layer * systemLayer, InetLayer * inetLayer);
    CHIP_ERROR Shutdown();

    // ----- Connection Management -----
    /**
     * @brief
     *   Connect to a CHIP device with a given name for Rendezvous
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] discriminator         The discriminator of the requested Device
     * @param[in] setupPINCode          The setup PIN code of the requested Device
     * @param[in] appReqState           Application specific context to be passed back when a message is received or on error
     * @param[in] onConnected           Callback for when the connection is established
     * @param[in] onMessageReceived     Callback for when a message is received
     * @param[in] onError               Callback for when an error occurs
     * @return CHIP_ERROR               The connection status
     */
    CHIP_ERROR ConnectDevice(NodeId remoteDeviceId, const uint16_t discriminator, const uint32_t setupPINCode, void * appReqState,
                             NewConnectionHandler onConnected, MessageReceiveHandler onMessageReceived, ErrorHandler onError);

    /**
     * @brief
     *   Connect to a CHIP device at a given address and an optional port
     *
     * @param[in] remoteDeviceId        The remote device Id.
     * @param[in] deviceAddr            The IPAddress of the requested Device
     * @param[in] appReqState           Application specific context to be passed back when a message is received or on error
     * @param[in] onConnected           Callback for when the connection is established
     * @param[in] onMessageReceived     Callback for when a message is received
     * @param[in] onError               Callback for when an error occurs
     * @param[in] devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @return CHIP_ERROR           The connection status
     */
    CHIP_ERROR ConnectDevice(NodeId remoteDeviceId, IPAddress deviceAddr, void * appReqState, NewConnectionHandler onConnected,
                             MessageReceiveHandler onMessageReceived, ErrorHandler onError, uint16_t devicePort = CHIP_PORT);

    /**
     * @brief
     *   The keypair for the secure channel. This is a utility function that will be used
     *   until we have automatic key exchange in place. The function is useful only for
     *   example applications for now. It will eventually be removed.
     *
     * @param state  Peer connection for which to establish the key
     * @param remote_public_key  A pointer to peer's public key
     * @param public_key_length  Length of remote_public_key
     * @param local_private_key  A pointer to local private key
     * @param private_key_length Length of local_private_key
     * @return CHIP_ERROR        The result of key derivation
     */
    CHIP_ERROR ManualKeyExchange(Transport::PeerConnectionState * state, const unsigned char * remote_public_key,
                                 const size_t public_key_length, const unsigned char * local_private_key,
                                 const size_t private_key_length);

    /**
     * @brief
     *   Get the PeerAddress of a connected peer
     *
     * @param[in,out] peerAddress  The PeerAddress object which will be populated with the details of the connected peer
     * @return CHIP_ERROR   An error if there's no active connection
     */
    CHIP_ERROR PopulatePeerAddress(Transport::PeerAddress & peerAddress);

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

    void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf,
                           SecureSessionMgrBase * mgr) override;

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override;

    //////////// BLECallbackHandler Implementation ///////////////
    void OnBLEConnectionError(BLE_ERROR err) override;
    void OnBLEConnectionComplete(BLE_ERROR err) override;
    void OnBLEConnectionClosed(BLE_ERROR err) override;
    void OnBLEPacketReceived(PacketBuffer * buffer) override;

private:
#if CONFIG_NETWORK_LAYER_BLE
    friend class Transport::BLE;
#endif

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
    Transport::Base * mUnsecuredTransport = NULL;

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
    IPAddress mDeviceAddr;
    uint16_t mDevicePort;
    Optional<NodeId> mRemoteDeviceId;
    uint32_t mMessageNumber = 0;

    void ClearRequestState();
    void ClearOpState();
};

} // namespace DeviceController
} // namespace chip

#endif // __CHIPDEVICECONTROLLER_H
