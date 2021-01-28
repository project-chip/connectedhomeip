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
 *      This file defines types and an object for the chip over
 *      Bluetooth Low Energy (CHIPoBLE) byte-stream, connection-oriented
 *      adaptation of chip for point-to-point Bluetooth Low Energy
 *      (BLE) links.
 *
 */

#pragma once

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <string.h>

#include <ble/BleConfig.h>

#include <ble/BleError.h>
#include <support/BitFlags.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Ble {

using ::chip::System::PacketBufferHandle;

typedef uint8_t SequenceNumber_t; // If type changed from uint8_t, adjust assumptions in BtpEngine::IsValidAck and
                                  // BLEEndPoint::AdjustReceiveWindow.

#if CHIP_ENABLE_CHIPOBLE_TEST
class BLEEndPoint;
#endif

// Public data members:
typedef enum
{
    kType_Data    = 0, // Default 0 for data
    kType_Control = 1,
} PacketType_t; // BTP packet types

class BtpEngine
{
#if CHIP_ENABLE_CHIPOBLE_TEST
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
#if CHIP_ENABLE_CHIPOBLE_TEST
        kHeaderFlag_CommandMessage = 0x10,
#endif
    }; // Masks for BTP fragment header flag bits.

    static const uint16_t sDefaultFragmentSize;
    static const uint16_t sMaxFragmentSize;

    // Public functions:
    BLE_ERROR Init(void * an_app_state, bool expect_first_ack);

    inline void SetTxFragmentSize(uint16_t size) { mTxFragmentSize = size; }
    inline void SetRxFragmentSize(uint16_t size) { mRxFragmentSize = size; }

    uint16_t GetRxFragmentSize() { return mRxFragmentSize; }
    uint16_t GetTxFragmentSize() { return mTxFragmentSize; }

    SequenceNumber_t GetAndIncrementNextTxSeqNum();
    SequenceNumber_t GetAndRecordRxAckSeqNum();

    inline SequenceNumber_t GetLastReceivedSequenceNumber() { return mRxNewestUnackedSeqNum; }
    inline SequenceNumber_t GetNewestUnackedSentSequenceNumber() { return mTxNewestUnackedSeqNum; }

    inline bool ExpectingAck() const { return mExpectingAck; }

    inline State_t RxState() { return mRxState; }
    inline State_t TxState() { return mTxState; }
#if CHIP_ENABLE_CHIPOBLE_TEST
    inline PacketType_t SetTxPacketType(PacketType_t type) { return (mTxPacketType = type); }
    inline PacketType_t SetRxPacketType(PacketType_t type) { return (mRxPacketType = type); }
    inline PacketType_t TxPacketType() { return mTxPacketType; }
    inline PacketType_t RxPacketType() { return mRxPacketType; }
    inline SequenceNumber_t SetTxPacketSeq(SequenceNumber_t seq) { return (mTxPacketSeq = seq); }
    inline SequenceNumber_t SetRxPacketSeq(SequenceNumber_t seq) { return (mRxPacketSeq = seq); }
    inline SequenceNumber_t TxPacketSeq() { return mTxPacketSeq; }
    inline SequenceNumber_t RxPacketSeq() { return mRxPacketSeq; }
    inline bool IsCommandPacket(const PacketBufferHandle & p) { return GetFlag(*(p->Start()), kHeaderFlag_CommandMessage); }
    inline void PushPacketTag(const PacketBufferHandle & p, PacketType_t type)
    {
        p->SetStart(p->Start() - sizeof(type));
        memcpy(p->Start(), &type, sizeof(type));
    }
    inline PacketType_t PopPacketTag(const PacketBufferHandle & p)
    {
        PacketType_t type;
        memcpy(&type, p->Start(), sizeof(type));
        p->SetStart(p->Start() + sizeof(type));
        return type;
    }
#endif // CHIP_ENABLE_CHIPOBLE_TEST

    bool HasUnackedData() const;

    BLE_ERROR HandleCharacteristicReceived(System::PacketBufferHandle data, SequenceNumber_t & receivedAck, bool & didReceiveAck);
    bool HandleCharacteristicSend(System::PacketBufferHandle data, bool send_ack);
    BLE_ERROR EncodeStandAloneAck(const PacketBufferHandle & data);

    PacketBufferHandle TakeRxPacket();
    PacketBufferHandle BorrowRxPacket() { return mRxBuf.Retain(); }
    void ClearRxPacket() { (void) TakeRxPacket(); }
    PacketBufferHandle TakeTxPacket();
    PacketBufferHandle BorrowTxPacket() { return mTxBuf.Retain(); }
    void ClearTxPacket() { (void) TakeTxPacket(); }

    void LogState() const;
    void LogStateDebug() const;

private:
    // Private data members:
#if CHIP_ENABLE_CHIPOBLE_TEST
    PacketType_t mTxPacketType;
    PacketType_t mRxPacketType;
    SequenceNumber_t mTxPacketSeq;
    SequenceNumber_t mRxPacketSeq;
#endif
    State_t mRxState;
    uint16_t mRxLength;
    void * mAppState;
    System::PacketBufferHandle mRxBuf;
    SequenceNumber_t mRxNextSeqNum;
    SequenceNumber_t mRxNewestUnackedSeqNum;
    SequenceNumber_t mRxOldestUnackedSeqNum;
    uint16_t mRxFragmentSize;

    State_t mTxState;
    uint16_t mTxLength;
    System::PacketBufferHandle mTxBuf;
    SequenceNumber_t mTxNextSeqNum;
    SequenceNumber_t mTxNewestUnackedSeqNum;
    SequenceNumber_t mTxOldestUnackedSeqNum;
    bool mExpectingAck;
    uint16_t mTxFragmentSize;

    uint16_t mRxCharCount;
    uint16_t mRxPacketCount;
    uint16_t mTxCharCount;
    uint16_t mTxPacketCount;

    // Private functions:
    bool IsValidAck(SequenceNumber_t ack_num) const;
    BLE_ERROR HandleAckReceived(SequenceNumber_t ack_num);
};

} /* namespace Ble */
} /* namespace chip */
