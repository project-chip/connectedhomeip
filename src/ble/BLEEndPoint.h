/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines a Bluetooth Low Energy (BLE) connection
 *      endpoint abstraction for the byte-streaming,
 *      connection-oriented CHIP over Bluetooth Low Energy (CHIPoBLE)
 *      Bluetooth Transport Protocol (BTP).
 *
 */

#pragma once

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <cstdint>

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include "BleConnectionDelegate.h"
#include "BleLayerDelegate.h"
#include "BlePlatformDelegate.h"
#include "BleRole.h"
#include "BtpEngine.h"

namespace chip {
namespace Ble {

using ::chip::System::PacketBufferHandle;

enum
{
    kBleCloseFlag_SuppressCallback  = 0x01,
    kBleCloseFlag_AbortTransmission = 0x02
};

// Forward declarations
class BleLayer;
class BleEndPointPool;

class DLL_EXPORT BLEEndPoint
{
    friend class BleLayer;
    friend class BleEndPointPool;

public:
    typedef uint64_t AlignT;

    // Public data members:
    enum
    {
        kState_Ready      = 0,
        kState_Connecting = 1,
        kState_Aborting   = 2,
        kState_Connected  = 3,
        kState_Closing    = 4,
        kState_Closed     = 5
    } mState; // [READ-ONLY] End point connection state. Refers to state of CHIP over
              // BLE transport protocol connection, not of underlying BLE connection.

    // Public function pointers:
    typedef void (*OnConnectCompleteFunct)(BLEEndPoint * endPoint, CHIP_ERROR err);
    OnConnectCompleteFunct OnConnectComplete;

    typedef void (*OnMessageReceivedFunct)(BLEEndPoint * endPoint, PacketBufferHandle && msg);
    OnMessageReceivedFunct OnMessageReceived;

    typedef void (*OnConnectionClosedFunct)(BLEEndPoint * endPoint, CHIP_ERROR err);
    OnConnectionClosedFunct OnConnectionClosed;

    // Public functions:
    CHIP_ERROR Send(PacketBufferHandle && data);
    CHIP_ERROR Receive(PacketBufferHandle && data);
    CHIP_ERROR StartConnect();

    bool IsUnsubscribePending() const;
    bool ConnectionObjectIs(BLE_CONNECTION_OBJECT connObj) { return connObj == mConnObj; }
    void Close();
    void Abort();

private:
    BleLayer * mBle; ///< [READ-ONLY] Pointer to the BleLayer object that owns this object.
    BleLayerDelegate * mBleTransport;

    uint32_t mRefCount;

    void AddRef();
    void Release();

    // Private data members:
    enum class ConnectionStateFlag : uint8_t
    {
        kAutoClose                = 0x01, // End point should close underlying BLE conn on BTP close.
        kCapabilitiesConfReceived = 0x02, // GATT confirmation received for sent capabilities req/resp.
        kCapabilitiesMsgReceived  = 0x04, // Capabilities request or response message received.
        kDidBeginSubscribe        = 0x08, // GATT subscribe request sent; must unsubscribe on close.
        kStandAloneAckInFlight    = 0x10, // Stand-alone ack in flight, awaiting GATT confirmation.
        kGattOperationInFlight    = 0x20  // GATT write, indication, subscribe, or unsubscribe in flight,
                                          // awaiting GATT confirmation.
    };

    enum class TimerStateFlag : uint8_t
    {
        kConnectTimerRunning           = 0x01, // BTP connect completion timer running.
        kReceiveConnectionTimerRunning = 0x02, // BTP receive connection completion timer running.
        kAckReceivedTimerRunning       = 0x04, // Ack received timer running due to unacked sent fragment.
        kSendAckTimerRunning           = 0x08, // Send ack timer running; indicates pending ack to send.
        kUnsubscribeTimerRunning       = 0x10, // Unsubscribe completion timer running.
    };

    // BLE connection to which an end point is uniquely bound. Type BLE_CONNECTION_OBJECT is defined by the platform or
    // void* by default. This object is passed back to the platform delegate with each call to send traffic over or
    // modify the state of the underlying BLE connection.
    BLE_CONNECTION_OBJECT mConnObj;

    // Queue of outgoing messages to send when current BtpEngine transmission completes.
    //
    // Re-used during connection setup to cache capabilities request and response payloads; payloads are freed when
    // connection is established.
    PacketBufferHandle mSendQueue;

    // Pending stand-alone BTP acknowledgement. Pre-empts regular send queue or fragmented message transmission in
    // progress.
    PacketBufferHandle mAckToSend;

    BtpEngine mBtpEngine;
    BleRole mRole;
    BitFlags<ConnectionStateFlag> mConnStateFlags;
    BitFlags<TimerStateFlag> mTimerStateFlags;
    SequenceNumber_t mLocalReceiveWindowSize;
    SequenceNumber_t mRemoteReceiveWindowSize;
    SequenceNumber_t mReceiveWindowMaxSize;

    // Private functions:
    BLEEndPoint()  = delete;
    ~BLEEndPoint() = delete;

    CHIP_ERROR Init(BleLayer * bleLayer, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose);
    bool IsConnected(uint8_t state) const;
    void DoClose(uint8_t flags, CHIP_ERROR err);

    // Transmit path:
    CHIP_ERROR DriveSending();
    CHIP_ERROR DriveStandAloneAck();
    bool PrepareNextFragment(PacketBufferHandle && data, bool & sentAck);
    CHIP_ERROR SendNextMessage();
    CHIP_ERROR ContinueMessageSend();
    CHIP_ERROR DoSendStandAloneAck();
    CHIP_ERROR SendCharacteristic(PacketBufferHandle && buf);
    CHIP_ERROR SendIndication(PacketBufferHandle && buf);
    CHIP_ERROR SendWrite(PacketBufferHandle && buf);

    // Receive path:
    CHIP_ERROR HandleConnectComplete();
    CHIP_ERROR HandleReceiveConnectionComplete();
    void HandleSubscribeReceived();
    void HandleSubscribeComplete();
    void HandleUnsubscribeComplete();
    CHIP_ERROR HandleGattSendConfirmationReceived();
    CHIP_ERROR HandleHandshakeConfirmationReceived();
    CHIP_ERROR HandleFragmentConfirmationReceived();
    CHIP_ERROR HandleCapabilitiesRequestReceived(PacketBufferHandle && data);
    CHIP_ERROR HandleCapabilitiesResponseReceived(PacketBufferHandle && data);
    SequenceNumber_t AdjustRemoteReceiveWindow(SequenceNumber_t lastReceivedAck, SequenceNumber_t maxRemoteWindowSize,
                                               SequenceNumber_t newestUnackedSentSeqNum);

    // Timer control functions:
    CHIP_ERROR StartConnectTimer();           // Start connect timer.
    CHIP_ERROR StartReceiveConnectionTimer(); // Start receive connection timer.
    CHIP_ERROR StartAckReceivedTimer();       // Start ack-received timer if it's not already running.
    CHIP_ERROR RestartAckReceivedTimer();     // Restart ack-received timer.
    CHIP_ERROR StartSendAckTimer();           // Start send-ack timer if it's not already running.
    CHIP_ERROR StartUnsubscribeTimer();
    void StopConnectTimer();           // Stop connect timer.
    void StopReceiveConnectionTimer(); // Stop receive connection timer.
    void StopAckReceivedTimer();       // Stop ack-received timer.
    void StopSendAckTimer();           // Stop send-ack timer.
    void StopUnsubscribeTimer();       // Stop unsubscribe timer.

    // Timer expired callbacks:
    static void HandleConnectTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleReceiveConnectionTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleAckReceivedTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleSendAckTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleUnsubscribeTimeout(chip::System::Layer * systemLayer, void * appState);

    // Close functions:
    void DoCloseCallback(uint8_t state, uint8_t flags, CHIP_ERROR err);
    void FinalizeClose(uint8_t state, uint8_t flags, CHIP_ERROR err);
    void ReleaseBleConnection();
    void Free();
    void FreeBtpEngine();

    void QueueTx(PacketBufferHandle && data, PacketType_t type);
};

} /* namespace Ble */
} /* namespace chip */
