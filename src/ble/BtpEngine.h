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
 *      This file defines types and an object for the chip over
 *      Bluetooth Low Energy (CHIPoBLE) byte-stream, connection-oriented
 *      adaptation of chip for point-to-point Bluetooth Low Energy
 *      (BLE) links.
 *
 */

#pragma once

#ifndef _CHIP_BLE_BLE_H
#error "Please include <ble/Ble.h> instead!"
#endif

#include <cstdint>
#include <cstring>

#include <lib/core/CHIPError.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Ble {

inline constexpr size_t kTransferProtocolHeaderFlagsSize = 1; // Size in bytes of encoded BTP fragment header flag bits
inline constexpr size_t kTransferProtocolSequenceNumSize = 1; // Size in bytes of encoded BTP sequence number
inline constexpr size_t kTransferProtocolAckSize         = 1; // Size in bytes of encoded BTP fragment acknowledgement number
inline constexpr size_t kTransferProtocolMsgLenSize      = 2; // Size in byte of encoded BTP total fragmented message length

constexpr size_t kTransferProtocolMaxHeaderSize =
    kTransferProtocolHeaderFlagsSize + kTransferProtocolAckSize + kTransferProtocolSequenceNumSize + kTransferProtocolMsgLenSize;
constexpr size_t kTransferProtocolMidFragmentMaxHeaderSize =
    kTransferProtocolHeaderFlagsSize + kTransferProtocolAckSize + kTransferProtocolSequenceNumSize;
constexpr size_t kTransferProtocolStandaloneAckHeaderSize =
    kTransferProtocolHeaderFlagsSize + kTransferProtocolAckSize + kTransferProtocolSequenceNumSize;

using ::chip::System::PacketBufferHandle;

typedef uint8_t SequenceNumber_t; // If type changed from uint8_t, adjust assumptions in BtpEngine::IsValidAck and
                                  // BLEEndPoint::AdjustReceiveWindow.

// Public data members:
typedef enum
{
    kType_Data    = 0, // Default 0 for data
    kType_Control = 1,
} PacketType_t; // BTP packet types

class BtpEngine
{
public:
    // Public data members:
    typedef enum
    {
        kState_Idle       = 0,
        kState_InProgress = 1,
        kState_Complete   = 2,
        kState_Error      = 3
    } State_t; // [READ-ONLY] Current state

    // Masks for BTP fragment header flag bits.
    enum class HeaderFlags : uint8_t
    {
        kStartMessage    = 0x01,
        kContinueMessage = 0x02,
        kEndMessage      = 0x04,
        kFragmentAck     = 0x08,
    };

    static const uint16_t sDefaultFragmentSize;
    static const uint16_t sMaxFragmentSize;

    // Public functions:
    CHIP_ERROR Init(void * an_app_state, bool expect_first_ack);

    inline void SetTxFragmentSize(uint16_t size) { mTxFragmentSize = size; }
    inline void SetRxFragmentSize(uint16_t size) { mRxFragmentSize = size; }

    uint16_t GetRxFragmentSize() const { return mRxFragmentSize; }
    uint16_t GetTxFragmentSize() const { return mTxFragmentSize; }

    SequenceNumber_t GetAndIncrementNextTxSeqNum();
    SequenceNumber_t GetAndRecordRxAckSeqNum();

    inline SequenceNumber_t GetLastReceivedSequenceNumber() const { return mRxNewestUnackedSeqNum; }
    inline SequenceNumber_t GetNewestUnackedSentSequenceNumber() const { return mTxNewestUnackedSeqNum; }

    inline bool ExpectingAck() const { return mExpectingAck; }

    inline State_t RxState() { return mRxState; }
    inline State_t TxState() { return mTxState; }

    bool HasUnackedData() const;

    CHIP_ERROR HandleCharacteristicReceived(System::PacketBufferHandle && data, SequenceNumber_t & receivedAck,
                                            bool & didReceiveAck);
    bool HandleCharacteristicSend(System::PacketBufferHandle data, bool send_ack);
    CHIP_ERROR EncodeStandAloneAck(const PacketBufferHandle & data);

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
    CHIP_ERROR HandleAckReceived(SequenceNumber_t ack_num);
};

} /* namespace Ble */
} /* namespace chip */
