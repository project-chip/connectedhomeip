/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines a Bluetooth Low Energy (BLE) connection
 *      endpoint abstraction for the byte-streaming,
 *      connection-oriented Weave over Bluetooth Low Energy (WoBLE)
 *      Bluetooth Transport Protocol (BTP).
 *
 */

#ifndef BLEENDPOINT_H_
#define BLEENDPOINT_H_

#include <SystemLayer/SystemMutex.h>

#include <BleLayer/BleLayer.h>
#include <BleLayer/WoBle.h>
#if WEAVE_ENABLE_WOBLE_TEST
#include <BleLayer/WoBleTest.h>
#endif

namespace nl {
namespace Ble {

using ::nl::Weave::System::PacketBuffer;

enum
{
    kBleCloseFlag_SuppressCallback  = 0x01,
    kBleCloseFlag_AbortTransmission = 0x02
};

// Forward declarations
class BleLayer;
class BleEndPointPool;
#if WEAVE_ENABLE_WOBLE_TEST
class WoBleTest;
#endif

class NL_DLL_EXPORT BLEEndPoint : public BleLayerObject
{
    friend class BleLayer;
    friend class BleEndPointPool;
#if WEAVE_ENABLE_WOBLE_TEST
    friend class WoBleTest;
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
    } mState; // [READ-ONLY] End point connection state. Refers to state of Weave over
              // BLE transport protocol connection, not of underlying BLE connection.

    // Public function pointers:
    typedef void (*OnConnectCompleteFunct)(BLEEndPoint * endPoint, BLE_ERROR err);
    OnConnectCompleteFunct OnConnectComplete;

    typedef void (*OnMessageReceivedFunct)(BLEEndPoint * endPoint, PacketBuffer * msg);
    OnMessageReceivedFunct OnMessageReceived;

    typedef void (*OnConnectionClosedFunct)(BLEEndPoint * endPoint, BLE_ERROR err);
    OnConnectionClosedFunct OnConnectionClosed;

#if WEAVE_ENABLE_WOBLE_TEST
    typedef void (*OnCommandReceivedFunct)(BLEEndPoint * endPoint, PacketBuffer * msg);
    OnCommandReceivedFunct OnCommandReceived;
    inline void SetOnCommandReceivedCB(OnCommandReceivedFunct cb) { OnCommandReceived = cb; };
    WoBleTest mWoBleTest;
    inline void SetTxWindowSize(uint8_t size) { mRemoteReceiveWindowSize = size; };
    inline void SetRxWindowSize(uint8_t size) { mReceiveWindowMaxSize = size; };
#endif

public:
    // Public functions:
    BLE_ERROR Send(PacketBuffer * data);
    BLE_ERROR Receive(PacketBuffer * data);
    BLE_ERROR StartConnect(void);

    bool IsUnsubscribePending(void) const;
    void Close(void);
    void Abort(void);

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
#if WEAVE_ENABLE_WOBLE_TEST
        kTimerState_UnderTestTimerRunnung = 0x80 // running throughput Tx test
#endif
    };

    // BLE connection to which an end point is uniquely bound. Type BLE_CONNECTION_OBJECT is defined by the platform or
    // void* by default. This object is passed back to the platform delegate with each call to send traffic over or
    // modify the state of the underlying BLE connection.
    BLE_CONNECTION_OBJECT mConnObj;

    // Queue of outgoing messages to send when current WoBle transmission completes.
    //
    // Re-used during connection setup to cache capabilities request and response payloads; payloads are freed when
    // connection is established.
    PacketBuffer * mSendQueue;

    // Pending stand-alone BTP acknolwedgement. Pre-empts regular send queue or fragmented message transmission in
    // progress.
    PacketBuffer * mAckToSend;

    WoBle mWoBle;
    BleRole mRole;
    uint8_t mConnStateFlags;
    uint8_t mTimerStateFlags;
    SequenceNumber_t mLocalReceiveWindowSize;
    SequenceNumber_t mRemoteReceiveWindowSize;
    SequenceNumber_t mReceiveWindowMaxSize;
#if WEAVE_ENABLE_WOBLE_TEST
    nl::Weave::System::Mutex mTxQueueMutex; // For MT-safe Tx queuing
#endif

private:
    // Private functions:
    BLEEndPoint(void);  // not defined
    ~BLEEndPoint(void); // not defined

    BLE_ERROR Init(BleLayer * bleLayer, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose);
    bool IsConnected(uint8_t state) const;
    void DoClose(uint8_t flags, BLE_ERROR err);

    // Transmit path:
    BLE_ERROR DriveSending(void);
    BLE_ERROR DriveStandAloneAck(void);
    bool PrepareNextFragment(PacketBuffer * data, bool & sentAck);
    BLE_ERROR SendNextMessage(void);
    BLE_ERROR ContinueMessageSend(void);
    BLE_ERROR DoSendStandAloneAck(void);
    BLE_ERROR SendCharacteristic(PacketBuffer * buf);
    bool SendIndication(PacketBuffer * buf);
    bool SendWrite(PacketBuffer * buf);

    // Receive path:
    BLE_ERROR HandleConnectComplete(void);
    BLE_ERROR HandleReceiveConnectionComplete(void);
    void HandleSubscribeReceived(void);
    void HandleSubscribeComplete(void);
    void HandleUnsubscribeComplete(void);
    BLE_ERROR HandleGattSendConfirmationReceived(void);
    BLE_ERROR HandleHandshakeConfirmationReceived(void);
    BLE_ERROR HandleFragmentConfirmationReceived(void);
    BLE_ERROR HandleCapabilitiesRequestReceived(PacketBuffer * data);
    BLE_ERROR HandleCapabilitiesResponseReceived(PacketBuffer * data);
    SequenceNumber_t AdjustRemoteReceiveWindow(SequenceNumber_t lastReceivedAck, SequenceNumber_t maxRemoteWindowSize,
                                               SequenceNumber_t newestUnackedSentSeqNum);

    // Timer control functions:
    BLE_ERROR StartConnectTimer(void);           // Start connect timer.
    BLE_ERROR StartReceiveConnectionTimer(void); // Start receive connection timer.
    BLE_ERROR StartAckReceivedTimer(void);       // Start ack-received timer if it's not already running.
    BLE_ERROR RestartAckReceivedTimer(void);     // Restart ack-received timer.
    BLE_ERROR StartSendAckTimer(void);           // Start send-ack timer if it's not already running.
    BLE_ERROR StartUnsubscribeTimer(void);
    void StopConnectTimer(void);           // Stop connect timer.
    void StopReceiveConnectionTimer(void); // Stop receive connection timer.
    void StopAckReceivedTimer(void);       // Stop ack-received timer.
    void StopSendAckTimer(void);           // Stop send-ack timer.
    void StopUnsubscribeTimer(void);       // Stop unsubscribe timer.

    // Timer expired callbacks:
    static void HandleConnectTimeout(Weave::System::Layer * systemLayer, void * appState, Weave::System::Error err);
    static void HandleReceiveConnectionTimeout(Weave::System::Layer * systemLayer, void * appState, Weave::System::Error err);
    static void HandleAckReceivedTimeout(Weave::System::Layer * systemLayer, void * appState, Weave::System::Error err);
    static void HandleSendAckTimeout(Weave::System::Layer * systemLayer, void * appState, Weave::System::Error err);
    static void HandleUnsubscribeTimeout(Weave::System::Layer * systemLayer, void * appState, Weave::System::Error err);

    // Close functions:
    void DoCloseCallback(uint8_t state, uint8_t flags, BLE_ERROR err);
    void FinalizeClose(uint8_t state, uint8_t flags, BLE_ERROR err);
    void ReleaseBleConnection(void);
    void Free(void);
    void FreeWoBle(void);

    // Mutex lock on Tx queue. Used only in WoBle test build for now.
#if WEAVE_ENABLE_WOBLE_TEST
    inline void QueueTxLock() { mTxQueueMutex.Lock(); };
    inline void QueueTxUnlock() { mTxQueueMutex.Unlock(); };
#else
    inline void QueueTxLock() { };
    inline void QueueTxUnlock() { };
#endif
    void QueueTx(PacketBuffer * data, PacketType_t type);
};

} /* namespace Ble */
} /* namespace nl */

#endif /* BLEENDPOINT_H_ */
