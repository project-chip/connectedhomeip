/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file defines a WiFiPAF connection endpoint abstraction.
 *
 */

#pragma once

#include <cstdint>

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BufferReader.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include "WiFiPAFConfig.h"
#include "WiFiPAFRole.h"
#include "WiFiPAFTP.h"

namespace chip {
namespace WiFiPAF {

using ::chip::System::PacketBufferHandle;

enum
{
    kWiFiPAFCloseFlag_SuppressCallback  = 0x01,
    kWiFiPAFCloseFlag_AbortTransmission = 0x02
};

// Forward declarations
class WiFiPAFLayer;
class WiFiPAFEndPointPool;

class DLL_EXPORT WiFiPAFEndPoint
{
    friend class WiFiPAFLayer;
    friend class WiFiPAFEndPointPool;
    friend class TestWiFiPAFLayer;

public:
    typedef uint64_t AlignT;

    enum
    {
        kState_Ready      = 0,
        kState_Connecting = 1,
        kState_Aborting   = 2,
        kState_Connected  = 3,
        kState_Closing    = 4,
        kState_Closed     = 5
    } mState; // [READ-ONLY] End point connection state. Refers to state of CHIP over
              // PAF transport protocol connection.

    typedef void (*OnSubscribeCompleteFunct)(void * appState);
    typedef void (*OnSubscribeErrorFunct)(void * appState, CHIP_ERROR err);
    OnSubscribeCompleteFunct mOnPafSubscribeComplete;
    OnSubscribeErrorFunct mOnPafSubscribeError;
    void * mAppState;

    typedef void (*OnMessageReceivedFunct)(WiFiPAFEndPoint * endPoint, PacketBufferHandle && msg);
    OnMessageReceivedFunct OnMessageReceived;

    typedef void (*OnConnectionClosedFunct)(WiFiPAFEndPoint * endPoint, CHIP_ERROR err);
    OnConnectionClosedFunct OnConnectionClosed;

    CHIP_ERROR Send(PacketBufferHandle && data);
    CHIP_ERROR Receive(PacketBufferHandle && data);
    CHIP_ERROR StartConnect();
    WiFiPAFEndPoint()  = default;
    ~WiFiPAFEndPoint() = default;

private:
    CHIP_ERROR RxPacketProcess(PacketBufferHandle && data);
    enum class PktDirect_t : uint8_t
    {
        kTx,
        kRx
    };
    CHIP_ERROR DebugPktAckSn(const PktDirect_t PktDirect, Encoding::LittleEndian::Reader & reader, uint8_t * pHead);
    CHIP_ERROR GetPktSn(Encoding::LittleEndian::Reader & reader, uint8_t * pHead, SequenceNumber_t & seqNum);

    WiFiPAFLayer * mWiFiPafLayer; ///< [READ-ONLY] Pointer to the WiFiPAFLayer object that owns this object.
    WiFiPAFSession mSessionInfo;
    SequenceNumber_t mRxAck;
#define PAFTP_REORDER_QUEUE_SIZE PAF_MAX_RECEIVE_WINDOW_SIZE
    System::PacketBuffer * ReorderQueue[PAFTP_REORDER_QUEUE_SIZE];
    uint8_t ItemsInReorderQueue;

    enum class ConnectionStateFlag : uint8_t
    {
        kCapabilitiesConfReceived = 0x02, // Ready for sent capabilities req/resp.
        kCapabilitiesMsgReceived  = 0x04, // Capabilities request or response message received.
        kStandAloneAckInFlight    = 0x10, // Stand-alone ack in flight.
        kOperationInFlight        = 0x20  // Operation in flight,
    };

    enum class TimerStateFlag : uint8_t
    {
        kConnectTimerRunning     = 0x01, // PAFTP connect completion timer running.
        kWaitResTimerRunning     = 0x02, // Wait for resource to be available
        kAckReceivedTimerRunning = 0x04, // Ack received timer running due to unacked sent fragment.
        kSendAckTimerRunning     = 0x08, // Send ack timer running; indicates pending ack to send.
    };

    // Queue of outgoing messages to send when current PAFTPEngine transmission completes.
    // Re-used during connection setup to cache capabilities request and response payloads; payloads are freed when
    // connection is established.
    PacketBufferHandle mSendQueue;

    // Pending stand-alone PAFTP acknowledgement. Pre-empts regular send queue or fragmented message transmission in
    // progress.
    PacketBufferHandle mAckToSend;

    WiFiPAFTP mPafTP;
    WiFiPafRole mRole;
    // How many continuing times the resource is unavailable. Will close the session if it's occupied too long
    uint8_t mResourceWaitCount = 0;

    BitFlags<ConnectionStateFlag> mConnStateFlags;
    BitFlags<TimerStateFlag> mTimerStateFlags;
    SequenceNumber_t mLocalReceiveWindowSize;
    SequenceNumber_t mRemoteReceiveWindowSize;
    SequenceNumber_t mReceiveWindowMaxSize;

    CHIP_ERROR Init(WiFiPAFLayer * WiFiPafLayer, WiFiPAFSession & SessionInfo);
    void DoClose(uint8_t flags, CHIP_ERROR err);
    bool IsConnected(uint8_t state) const;

    // Transmit path:
    CHIP_ERROR DriveSending();
    CHIP_ERROR DriveStandAloneAck();
    bool PrepareNextFragment(PacketBufferHandle && data, bool & sentAck);
    CHIP_ERROR SendNextMessage();
    CHIP_ERROR ContinueMessageSend();
    CHIP_ERROR DoSendStandAloneAck();
    CHIP_ERROR SendCharacteristic(PacketBufferHandle && buf);
    CHIP_ERROR SendWrite(PacketBufferHandle && buf);

    // Receive path:
    CHIP_ERROR HandleConnectComplete();
    CHIP_ERROR HandleSendConfirmationReceived(bool result);
    CHIP_ERROR HandleHandshakeConfirmationReceived();
    CHIP_ERROR HandleFragmentConfirmationReceived(bool result);
    CHIP_ERROR HandleCapabilitiesRequestReceived(PacketBufferHandle && data);
    CHIP_ERROR HandleCapabilitiesResponseReceived(PacketBufferHandle && data);
    SequenceNumber_t AdjustRemoteReceiveWindow(SequenceNumber_t lastReceivedAck, SequenceNumber_t maxRemoteWindowSize,
                                               SequenceNumber_t newestUnackedSentSeqNum);

    // Timer control functions:
    CHIP_ERROR StartConnectTimer();       // Start connect timer.
    CHIP_ERROR StartAckReceivedTimer();   // Start ack-received timer if it's not already running.
    CHIP_ERROR RestartAckReceivedTimer(); // Restart ack-received timer.
    CHIP_ERROR StartSendAckTimer();       // Start send-ack timer if it's not already running.
    CHIP_ERROR StartWaitResourceTimer();  // Start wait-resource timer if it's not already running.
    void StopConnectTimer();              // Stop connect timer.
    void StopAckReceivedTimer();          // Stop ack-received timer.
    void StopSendAckTimer();              // Stop send-ack timer.
    void StopWaitResourceTimer();         // Stop wait-resource timer

    // Timer expired callbacks:
    static void HandleConnectTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleAckReceivedTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleSendAckTimeout(chip::System::Layer * systemLayer, void * appState);
    static void HandleWaitResourceTimeout(chip::System::Layer * systemLayer, void * appState);

    // Close functions:
    void DoCloseCallback(uint8_t state, uint8_t flags, CHIP_ERROR err);
    void FinalizeClose(uint8_t state, uint8_t flags, CHIP_ERROR err);
    void Free();
    void FreePAFtpEngine();

    void QueueTx(PacketBufferHandle && data, PacketType_t type);
    void ClearAll();
};

} /* namespace WiFiPAF */
} /* namespace chip */
