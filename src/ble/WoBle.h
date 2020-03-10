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
 *      This file defines types and an object for the Weave over
 *      Bluetooth Low Energy (WoBLE) byte-stream, connection-oriented
 *      adaptation of Weave for point-to-point Bluetooth Low Energy
 *      (BLE) links.
 *
 */

#ifndef WOBLE_H_
#define WOBLE_H_

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <string.h>

#include <BleLayer/BleConfig.h>
#include <BleLayer/BleError.h>
#include <SystemLayer/SystemPacketBuffer.h>
#include <Weave/Support/FlagUtils.hpp>

namespace nl {
namespace Ble {

using ::nl::Weave::System::PacketBuffer;

typedef uint8_t SequenceNumber_t; // If type changed from uint8_t, adjust assumptions in WoBle::IsValidAck and
                                  // BLEEndPoint::AdjustReceiveWindow.

#if WEAVE_ENABLE_WOBLE_TEST
class BLEEndPoint;
#endif

// Public data members:
typedef enum
{
    kType_Data    = 0, // Default 0 for data
    kType_Control = 1,
} PacketType_t; // WoBle packet types

class WoBle
{
#if WEAVE_ENABLE_WOBLE_TEST
    friend class BLEEndPoint;
#endif

public:
    // Public data members:
    typedef enum
    {
        kState_Idle       = 0,
        kState_InProgress = 1,
        kState_Complete   = 2,
        kState_Error      = 3
    } State_t; // [READ-ONLY] Current state

    enum
    {
        kHeaderFlag_StartMessage    = 0x01,
        kHeaderFlag_ContinueMessage = 0x02,
        kHeaderFlag_EndMessage      = 0x04,
        kHeaderFlag_FragmentAck     = 0x08,
#if WEAVE_ENABLE_WOBLE_TEST
        kHeaderFlag_CommandMessage = 0x10,
#endif
    }; // Masks for BTP fragment header flag bits.

    static const uint16_t sDefaultFragmentSize;
    static const uint16_t sMaxFragmentSize;

public:
    // Public functions:
    WoBle(void) { };
    ~WoBle(void) { };

    BLE_ERROR Init(void * an_app_state, bool expect_first_ack);

    inline void SetTxFragmentSize(uint8_t size) { mTxFragmentSize = size; };
    inline void SetRxFragmentSize(uint8_t size) { mRxFragmentSize = size; };

    uint16_t GetRxFragmentSize(void) { return mRxFragmentSize; };
    uint16_t GetTxFragmentSize(void) { return mTxFragmentSize; };

    SequenceNumber_t GetAndIncrementNextTxSeqNum(void);
    SequenceNumber_t GetAndRecordRxAckSeqNum(void);

    inline SequenceNumber_t GetLastReceivedSequenceNumber(void) { return mRxNewestUnackedSeqNum; };
    inline SequenceNumber_t GetNewestUnackedSentSequenceNumber(void) { return mTxNewestUnackedSeqNum; };

    inline bool ExpectingAck(void) const { return mExpectingAck; };

    inline State_t RxState(void) { return mRxState; }
    inline State_t TxState(void) { return mTxState; }
#if WEAVE_ENABLE_WOBLE_TEST
    inline PacketType_t SetTxPacketType(PacketType_t type) { return (mTxPacketType = type); };
    inline PacketType_t SetRxPacketType(PacketType_t type) { return (mRxPacketType = type); };
    inline PacketType_t TxPacketType() { return mTxPacketType; };
    inline PacketType_t RxPacketType() { return mRxPacketType; };
    inline SequenceNumber_t SetTxPacketSeq(SequenceNumber_t seq) { return (mTxPacketSeq = seq); };
    inline SequenceNumber_t SetRxPacketSeq(SequenceNumber_t seq) { return (mRxPacketSeq = seq); };
    inline SequenceNumber_t TxPacketSeq() { return mTxPacketSeq; };
    inline SequenceNumber_t RxPacketSeq() { return mRxPacketSeq; };
    inline bool IsCommandPacket(PacketBuffer * p) { return GetFlag(*(p->Start()), kHeaderFlag_CommandMessage); }
    inline void PushPacketTag(PacketBuffer * p, PacketType_t type)
    {
        p->SetStart(p->Start() - sizeof(type));
        memcpy(p->Start(), &type, sizeof(type));
    };
    inline PacketType_t PopPacketTag(PacketBuffer * p)
    {
        PacketType_t type;
        memcpy(&type, p->Start(), sizeof(type));
        p->SetStart(p->Start() + sizeof(type));
        return type;
    };
#endif // WEAVE_ENABLE_WOBLE_TEST

    bool HasUnackedData(void) const;

    BLE_ERROR HandleCharacteristicReceived(PacketBuffer * data, SequenceNumber_t & receivedAck, bool & didReceiveAck);
    bool HandleCharacteristicSend(PacketBuffer * data, bool send_ack);
    BLE_ERROR EncodeStandAloneAck(PacketBuffer * data);

    PacketBuffer * RxPacket(void);
    PacketBuffer * TxPacket(void);

    bool ClearRxPacket(void);
    bool ClearTxPacket(void);

    void LogState(void) const;
    void LogStateDebug(void) const;

private:
    // Private data members:
#if WEAVE_ENABLE_WOBLE_TEST
    PacketType_t mTxPacketType;
    PacketType_t mRxPacketType;
    SequenceNumber_t mTxPacketSeq;
    SequenceNumber_t mRxPacketSeq;
#endif
    State_t mRxState;
    uint16_t mRxLength;
    void * mAppState;
    PacketBuffer * mRxBuf;
    SequenceNumber_t mRxNextSeqNum;
    SequenceNumber_t mRxNewestUnackedSeqNum;
    SequenceNumber_t mRxOldestUnackedSeqNum;
    uint16_t mRxFragmentSize;

    State_t mTxState;
    uint16_t mTxLength;
    PacketBuffer * mTxBuf;
    SequenceNumber_t mTxNextSeqNum;
    SequenceNumber_t mTxNewestUnackedSeqNum;
    SequenceNumber_t mTxOldestUnackedSeqNum;
    bool mExpectingAck;
    uint16_t mTxFragmentSize;

    uint16_t mRxCharCount;
    uint16_t mRxPacketCount;
    uint16_t mTxCharCount;
    uint16_t mTxPacketCount;

private:
    // Private functions:
    bool IsValidAck(SequenceNumber_t ack_num) const;
    BLE_ERROR HandleAckReceived(SequenceNumber_t ack_num);
};

} /* namespace Ble */
} /* namespace nl */

#endif /* WOBLE_H_ */
