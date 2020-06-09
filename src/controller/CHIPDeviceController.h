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
#include <transport/SecureTransport.h>

namespace chip {
namespace DeviceController {

class ChipDeviceController;

extern "C" {
typedef void (*CompleteHandler)(ChipDeviceController * deviceController, void * appReqState);
typedef void (*ErrorHandler)(ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR err,
                             const IPPacketInfo * pktInfo);
typedef void (*MessageReceiveHandler)(ChipDeviceController * deviceController, void * appReqState, PacketBuffer * payload,
                                      const IPPacketInfo * pktInfo);
};

class DLL_EXPORT ChipDeviceController
{
public:
    ChipDeviceController();

    void * AppState;

    CHIP_ERROR Init();
    CHIP_ERROR Shutdown();

    // ----- Connection Management -----
    /**
     * @brief
     *   Connect to a CHIP device at a given address and an optional port
     *
     * @param[in] deviceId              A device identifier. Currently unused and can be set to any value
     * @param[in] deviceAddr            The IPAddress of the requested Device
     * @param[in] appReqState           Application specific context to be passed back when a message is received or on error
     * @param[in] onMessageReceived     Callback for when a message is received
     * @param[in] onError               Callback for when an error occurs
     * @param[in] devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @return CHIP_ERROR           The connection status
     */
    CHIP_ERROR ConnectDevice(uint64_t deviceId, IPAddress deviceAddr, void * appReqState, MessageReceiveHandler onMessageReceived,
                             ErrorHandler onError, uint16_t devicePort = CHIP_PORT);

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
     *   Get the address and port of a connected device
     *
     * @param[out] deviceAddr   The IPAddress of the connected device
     * @param[out] devicePort   The port of the econnected device
     * @return CHIP_ERROR   An error if there's no active connection
     */
    CHIP_ERROR GetDeviceAddress(IPAddress * deviceAddr, uint16_t * devicePort);

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

    // ----- Messaging -----
    /**
     * @brief
     *   Send a message to a connected CHIP device
     *
     * @param[in] appReqState   Application specific context to be passed back when a message is received or on error
     * @param[in] buffer        The Data Buffer to trasmit to the deviec
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR SendMessage(void * appReqState, PacketBuffer * buffer);

    // ----- IO -----
    /**
     * @brief
     *   Allow the CHIP Stack to process any pending events
     *   This can be called in an event handler loop to tigger callbacks within the CHIP stack
     *   Note - Some platforms might need to implement their own event handler
     */
    void ServiceEvents();

    /**
     * @brief
     *   Get pointers to the Layers ownerd by the controller
     *
     * @param systemLayer[out]   A pointer to the SystemLayer object
     * @param inetLayer[out]     A pointer to the InetLayer object
     * @return CHIP_ERROR   Indicates whether the layers were populated correctly
     */
    CHIP_ERROR GetLayers(Layer ** systemLayer, InetLayer ** inetLayer);

private:
    using StatefulTransport = StatefulSecureTransport<ChipDeviceController *>;

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
    StatefulTransport * mDeviceCon;

    ConnectionState mConState;
    void * mAppReqState;

    union
    {
        CompleteHandler General;
        MessageReceiveHandler Response;
    } mOnComplete;

    ErrorHandler mOnError;
    PacketBuffer * mCurReqMsg;

    uint64_t mDeviceId;
    IPAddress mDeviceAddr;
    uint16_t mDevicePort;

    void ClearRequestState();
    void ClearOpState();

    static void OnReceiveMessage(StatefulTransport * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo);
    static void OnReceiveError(StatefulTransport * con, CHIP_ERROR err, const IPPacketInfo * pktInfo);
};

} // namespace DeviceController
} // namespace chip

#endif // __CHIPDEVICECONTROLLER_H
