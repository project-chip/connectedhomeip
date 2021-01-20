/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <system/SystemMutex.h>

#include <ble/BleLayer.h>
#include <ble/BtpEngine.h>
#if CHIP_ENABLE_CHIPOBLE_TEST
#include <ble/BtpEngineTest.h>
#endif

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
#if CHIP_ENABLE_CHIPOBLE_TEST
class BtpEngineTest;
#endif

class DLL_EXPORT BLEEndPoint : public BleLayerObject
{
    friend class BleLayer;
    friend class BleEndPointPool;
#if CHIP_ENABLE_CHIPOBLE_TEST
    friend class BtpEngineTest;
#endif

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
    typedef void (*OnConnectCompleteFunct)(BLEEndPoint * endPoint, BLE_ERROR err);
    OnConnectCompleteFunct OnConnectComplete;

    typedef void (*OnMessageReceivedFunct)(BLEEndPoint * endPoint, PacketBufferHandle msg);
    OnMessageReceivedFunct OnMessageReceived;

    typedef void (*OnConnectionClosedFunct)(BLEEndPoint * endPoint, BLE_ERROR err);
    OnConnectionClosedFunct OnConnectionClosed;

#if CHIP_ENABLE_CHIPOBLE_TEST
    typedef void (*OnCommandReceivedFunct)(BLEEndPoint * endPoint, PacketBufferHandle msg);
    OnCommandReceivedFunct OnCommandReceived;
    inline void SetOnCommandReceivedCB(OnCommandReceivedFunct cb) { OnCommandReceived = cb; };
    BtpEngineTest mBtpEngineTest;
    inline void SetTxWindowSize(uint8_t size) { mRemoteReceiveWindowSize = size; };
    inline void SetRxWindowSize(uint8_t size) { mReceiveWindowMaxSize = size; };
#endif

    // Public functions:
    BLE_ERROR Send(PacketBufferHandle data);
    BLE_ERROR Receive(PacketBufferHandle data);
    BLE_ERROR StartConnect();

    bool IsUnsubscribePending() const;
    void Close();
    void Abort();

private:
    // Private data members:
    enum ConnectionStateFlags
    {
        kConnState_AutoClose                = 0x01, // End point should close underlying BLE conn on BTP close.
        kConnState_CapabilitiesConfReceived = 0x02, // GATT confirmation received for sent capabilities req/resp.
        kConnState_CapabilitiesMsgReceived  = 0x04, // Capabilities request or response message received.
        kConnState_DidBeginSubscribe        = 0x08, // GATT subscribe request sent; must unsubscribe on close.
        kConnState_StandAloneAckInFlight    = 0x10, // Stand-alone ack in flight, awaiting GATT confirmation.
        kConnState_GattOperationInFlight    = 0x20  // GATT write, indication, subscribe, or unsubscribe in flight,
                                                    // awaiting GATT confirmation.
    };

    enum TimerStateFlags
    {
        kTimerState_ConnectTimerRunning           = 0x01, // BTP connect completion timer running.
        kTimerState_ReceiveConnectionTimerRunning = 0x02, // BTP receive connection completion timer running.
        kTimerState_AckReceivedTimerRunning       = 0x04, // Ack received timer running due to unacked sent fragment.
        kTimerState_SendAckTimerRunning           = 0x08, // Send ack timer running; indicates pending ack to send.
        kTimerState_UnsubscribeTimerRunning       = 0x10, // Unsubscribe completion timer running.
#if CHIP_ENABLE_CHIPOBLE_TEST
        kTimerState_UnderTestTimerRunnung = 0x80 // running throughput Tx test
#endif
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

    // Pending stand-alone BTP acknolwedgement. Pre-empts regular send queue or fragmented message transmission in
    // progress.
    PacketBufferHandle mAckToSend;

    BtpEngine mBtpEngine;
    BleRole mRole;
    uint8_t mConnStateFlags;
    uint8_t mTimerStateFlags;
    SequenceNumber_t mLocalReceiveWindowSize;
    SequenceNumber_t mRemoteReceiveWindowSize;
    SequenceNumber_t mReceiveWindowMaxSize;
#if CHIP_ENABLE_CHIPOBLE_TEST
    chip::System::Mutex mTxQueueMutex; // For MT-safe Tx queuing
#endif

    // Private functions:
    BLEEndPoint()  = delete;
    ~BLEEndPoint() = delete;

    BLE_ERROR Init(BleLayer * bleLayer, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose);
    bool IsConnected(uint8_t state) const;
    void DoClose(uint8_t flags, BLE_ERROR err);

    // Transmit path:
    BLE_ERROR DriveSending();
    BLE_ERROR DriveStandAloneAck();
    bool PrepareNextFragment(PacketBufferHandle && data, bool & sentAck);
    BLE_ERROR SendNextMessage();
    BLE_ERROR ContinueMessageSend();
    BLE_ERROR DoSendStandAloneAck();
    BLE_ERROR SendCharacteristic(PacketBufferHandle && buf);
    bool SendIndication(PacketBufferHandle && buf);
    bool SendWrite(PacketBufferHandle && buf);

    // Receive path:
    BLE_ERROR HandleConnectComplete();
    BLE_ERROR HandleReceiveConnectionComplete();
    void HandleSubscribeReceived();
    void HandleSubscribeComplete();
    void HandleUnsubscribeComplete();
    BLE_ERROR HandleGattSendConfirmationReceived();
    BLE_ERROR HandleHandshakeConfirmationReceived();
    BLE_ERROR HandleFragmentConfirmationReceived();
    BLE_ERROR HandleCapabilitiesRequestReceived(PacketBufferHandle data);
    BLE_ERROR HandleCapabilitiesResponseReceived(PacketBufferHandle data);
    SequenceNumber_t AdjustRemoteReceiveWindow(SequenceNumber_t lastReceivedAck, SequenceNumber_t maxRemoteWindowSize,
                                               SequenceNumber_t newestUnackedSentSeqNum);

    // Timer control functions:
    BLE_ERROR StartConnectTimer();           // Start connect timer.
    BLE_ERROR StartReceiveConnectionTimer(); // Start receive connection timer.
    BLE_ERROR StartAckReceivedTimer();       // Start ack-received timer if it's not already running.
    BLE_ERROR RestartAckReceivedTimer();     // Restart ack-received timer.
    BLE_ERROR StartSendAckTimer();           // Start send-ack timer if it's not already running.
    BLE_ERROR StartUnsubscribeTimer();
    void StopConnectTimer();           // Stop connect timer.
    void StopReceiveConnectionTimer(); // Stop receive connection timer.
    void StopAckReceivedTimer();       // Stop ack-received timer.
    void StopSendAckTimer();           // Stop send-ack timer.
    void StopUnsubscribeTimer();       // Stop unsubscribe timer.

    // Timer expired callbacks:
    static void HandleConnectTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err);
    static void HandleReceiveConnectionTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err);
    static void HandleAckReceivedTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err);
    static void HandleSendAckTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err);
    static void HandleUnsubscribeTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err);

    // Close functions:
    void DoCloseCallback(uint8_t state, uint8_t flags, BLE_ERROR err);
    void FinalizeClose(uint8_t state, uint8_t flags, BLE_ERROR err);
    void ReleaseBleConnection();
    void Free();
    void FreeBtpEngine();

    // Mutex lock on Tx queue. Used only in BtpEngine test build for now.
#if CHIP_ENABLE_CHIPOBLE_TEST
    inline void QueueTxLock() { mTxQueueMutex.Lock(); }
    inline void QueueTxUnlock() { mTxQueueMutex.Unlock(); }
#else
    inline void QueueTxLock() {}
    inline void QueueTxUnlock() {}
#endif
    void QueueTx(PacketBufferHandle && data, PacketType_t type);
};

} /* namespace Ble */
} /* namespace chip */
