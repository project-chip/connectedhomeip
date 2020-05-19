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

#include <support/DLLUtil.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPUdpExchange.h>

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
     * @param deviceId              A device identifier. Currently unused and can be set to any value
     * @param deviceAddr            The IPAddress of the requested Device
     * @param appReqState           Application specific context to be passed back when a message is received or on error
     * @param onMessageReceived     Callback for when a message is received
     * @param onError               Callback for when an error occurs
     * @param devicePort            [Optional] The CHIP Device's port, defaults to CHIP_PORT
     * @return CHIP_ERROR           The connection status
     */
    CHIP_ERROR ConnectDevice(uint64_t deviceId, IPAddress deviceAddr, void * appReqState, MessageReceiveHandler onMessageReceived,
                             ErrorHandler onError, uint16_t devicePort = CHIP_PORT);

    // ----- Messaging -----
    /**
     * @brief
     *   Send a message to a connected CHIP device
     *
     * @param appReqState   Application specific context to be passed back when a message is received or on error
     * @param buffer        The Data Buffer to trasmit to the deviec
     * @return CHIP_ERROR   The return status
     */
    CHIP_ERROR SendMessage(void * appReqState, PacketBuffer * buffer);

    /**
     * @brief
     *   Disconnect from a connected device
     *
     * @return CHIP_ERROR   If the device was disconnected successfully
     */
    CHIP_ERROR DisconnectDevice();

    // ----- IO -----
    /**
     * @brief
     *   Allow the CHIP Stack to process any pending events
     *   This can be called in an event handler loop to tigger callbacks within the CHIP stack
     *   Note - Some platforms might need to implement their own event handler
     */
    void ServiceEvents();

    System::Layer * SystemLayer() { return mSystemLayer; }
    Inet::InetLayer * InetLayer() { return mInetLayer; }

private:
    enum
    {
        kState_NotInitialized = 0,
        kState_Initialized    = 1
    } mState;

    enum ConnectionState
    {
        kConnectionState_NotConnected = 0,
        kConnectionState_Connected    = 1,
    };

    System::Layer * mSystemLayer;
    Inet::InetLayer * mInetLayer;
    ChipUdpExchange * mUdpExchange;

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

    static void OnReceiveMessage(ChipUdpExchange * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo);
    static void OnReceiveError(ChipUdpExchange * con, CHIP_ERROR err, const IPPacketInfo * pktInfo);
};

} // namespace DeviceController
} // namespace chip

#endif // __CHIPDEVICECONTROLLER_H
