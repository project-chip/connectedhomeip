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
 *      Declaration of CHIP Device Manager, a common class
 *      that implements connecting and messaging and will later
 *      be expanded to support discovery, pairing and
 *      provisioning of CHIP  devices.
 *
 */

#ifndef __CHIPDEVICEMANAGER_H
#define __CHIPDEVICEMANAGER_H

#include <support/DLLUtil.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPConnection.h>

namespace chip {
namespace DeviceController {

class ChipDeviceController;

extern "C" {
typedef void (*CompleteFunct)(ChipDeviceController * deviceMgr, void * appReqState);
typedef void (*ErrorFunct)(ChipDeviceController * deviceMgr, void * appReqState, CHIP_ERROR err, const IPPacketInfo * pktInfo);
typedef void (*MessageReceiveFunct)(ChipDeviceController * deviceMgr, void * appReqState, PacketBuffer * payload,
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
    CHIP_ERROR ConnectDevice(uint64_t deviceId, IPAddress deviceAddr, void * appReqState, MessageReceiveFunct OnMessageReceived,
                             ErrorFunct onError, uint16_t peerPort = CHIP_PORT);

    // ----- Messaging -----
    CHIP_ERROR SendMessage(void * appReqState, PacketBuffer * buffer);

    // ----- IO -----
    void ServiceEvents();

private:
    enum
    {
        kState_NotInitialized = 0,
        kState_Initialized    = 1
    } mState;

    enum ConnectionState
    {
        kConnectionState_NotConnected  = 0,
        kConnectionState_ConnectDevice = 1,
        kConnectionState_Connected     = 2,
    };

    System::Layer * mSystemLayer;
    Inet::InetLayer * mInetLayer;
    ChipConnection * mDeviceCon;

    ConnectionState mConState;
    void * mAppReqState;

    union
    {
        CompleteFunct General;
        MessageReceiveFunct Response;
    } mOnComplete;

    ErrorFunct mOnError;
    PacketBuffer * mCurReqMsg;

    uint64_t mDeviceId;
    IPAddress mDeviceAddr;
    uint16_t mDevicePort;

    void ClearRequestState();
    void ClearOpState();

    static void OnReceiveMessage(ChipConnection * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo);
    static void OnReceiveError(ChipConnection * con, CHIP_ERROR err, const IPPacketInfo * pktInfo);
};

} // namespace DeviceController
} // namespace chip

#endif // __CHIPDEVICEMANAGER_H
