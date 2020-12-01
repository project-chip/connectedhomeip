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
 *      This module implements encode, decode, fragmentation and reassembly of
 *      Bluetooth Transport Layer (BTP) packet types for transport of a
 *      CHIP-over-Bluetooth Low Energy (CHIPoBLE) byte-stream over point-to-point
 *      Bluetooth Low Energy (BLE) links.
 *
 */

#include <ble/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include <ble/BtpEngine.h>
#if CHIP_ENABLE_CHIPOBLE_TEST
#include <ble/BtpEngineTest.h>
#endif

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

// Define below to enable extremely verbose BLE-specific debug logging.
#undef CHIP_BTP_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED

#ifdef CHIP_BTP_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED
#define ChipLogDebugBtpEngine(MOD, MSG, ...) ChipLogError(MOD, MSG, ##__VA_ARGS__)
#else
#define ChipLogDebugBtpEngine(MOD, MSG, ...)
#endif

#define CHIP_BLE_TRANSFER_PROTOCOL_HEADER_FLAGS_SIZE 1 // Size in bytes of enocded BTP fragment header flag bits
#define CHIP_BLE_TRANSFER_PROTOCOL_SEQUENCE_NUM_SIZE 1 // Size in bytes of encoded BTP sequence number
#define CHIP_BLE_TRANSFER_PROTOCOL_ACK_SIZE 1          // Size in bytes of encoded BTP fragment acknowledgement number
#define CHIP_BLE_TRANSFER_PROTOCOL_MSG_LEN_SIZE 2      // Size in byte of encoded BTP total fragmented message length

#define CHIP_BLE_TRANSFER_PROTOCOL_MAX_HEADER_SIZE                                                                                 \
    (CHIP_BLE_TRANSFER_PROTOCOL_HEADER_FLAGS_SIZE + CHIP_BLE_TRANSFER_PROTOCOL_ACK_SIZE +                                          \
     CHIP_BLE_TRANSFER_PROTOCOL_SEQUENCE_NUM_SIZE + CHIP_BLE_TRANSFER_PROTOCOL_MSG_LEN_SIZE)

#define CHIP_BLE_TRANSFER_PROTOCOL_MID_FRAGMENT_MAX_HEADER_SIZE                                                                    \
    (CHIP_BLE_TRANSFER_PROTOCOL_HEADER_FLAGS_SIZE + CHIP_BLE_TRANSFER_PROTOCOL_ACK_SIZE +                                          \
     CHIP_BLE_TRANSFER_PROTOCOL_SEQUENCE_NUM_SIZE)

#define CHIP_BLE_TRANSFER_PROTOCOL_STANDALONE_ACK_HEADER_SIZE                                                                      \
    (CHIP_BLE_TRANSFER_PROTOCOL_HEADER_FLAGS_SIZE + CHIP_BLE_TRANSFER_PROTOCOL_ACK_SIZE +                                          \
     CHIP_BLE_TRANSFER_PROTOCOL_SEQUENCE_NUM_SIZE)

namespace chip {
namespace Ble {

static inline void IncSeqNum(SequenceNumber_t & a_seq_num)
{
    a_seq_num = 0xff & ((a_seq_num) + 1);
}

static inline bool DidReceiveData(uint8_t rx_flags)
{
    return (GetFlag(rx_flags, BtpEngine::kHeaderFlag_StartMessage) || GetFlag(rx_flags, BtpEngine::kHeaderFlag_ContinueMessage) ||
            GetFlag(rx_flags, BtpEngine::kHeaderFlag_EndMessage));
}

static void PrintBufDebug(const System::PacketBufferHandle & buf)
{
#ifdef CHIP_BTP_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED
    uint8_t * b = buf->Start();

    for (int i = 0; i < buf->DataLength(); i++)
    {
        ChipLogError(Ble, "\t%02x", b[i]);
    }
#endif
}

const uint16_t BtpEngine::sDefaultFragmentSize = 20;  // 23-byte minimum ATT_MTU - 3 bytes for ATT operation header
const uint16_t BtpEngine::sMaxFragmentSize     = 128; // Size of write and indication characteristics

BLE_ERROR BtpEngine::Init(void * an_app_state, bool expect_first_ack)
{
    mAppState              = an_app_state;
    mRxState               = kState_Idle;
    mRxBuf                 = nullptr;
    mRxNewestUnackedSeqNum = 0;
    mRxOldestUnackedSeqNum = 0;
    mRxFragmentSize        = sDefaultFragmentSize;
    mTxState               = kState_Idle;
    mTxBuf                 = nullptr;
    mTxFragmentSize        = sDefaultFragmentSize;
    mRxCharCount           = 0;
    mRxPacketCount         = 0;
    mTxCharCount           = 0;
    mTxPacketCount         = 0;
    mTxNewestUnackedSeqNum = 0;
    mTxOldestUnackedSeqNum = 0;
#if CHIP_ENABLE_CHIPOBLE_TEST
    mTxPacketType = kType_Data; // Default BtpEngine Data packet
    mRxPacketType = kType_Data; // Default BtpEngine Data packet
#endif

    if (expect_first_ack)
    {
        mTxNextSeqNum = 1;
        mExpectingAck = true;
        mRxNextSeqNum = 0;
    }
    else
    {
        mTxNextSeqNum = 0;
        mExpectingAck = false;
        mRxNextSeqNum = 1;
    }

    return BLE_NO_ERROR;
}

SequenceNumber_t BtpEngine::GetAndIncrementNextTxSeqNum()
{
    SequenceNumber_t ret = mTxNextSeqNum;

    // If not already expecting ack...
    if (!mExpectingAck)
    {
        mExpectingAck          = true;
        mTxOldestUnackedSeqNum = mTxNextSeqNum;
    }

    // Update newest unacknowledged sequence number.
    mTxNewestUnackedSeqNum = mTxNextSeqNum;

    // Increment mTxNextSeqNum.
    IncSeqNum(mTxNextSeqNum);

    return ret;
}

SequenceNumber_t BtpEngine::GetAndRecordRxAckSeqNum()
{
    SequenceNumber_t ret = mRxNewestUnackedSeqNum;

    mRxNewestUnackedSeqNum = mRxNextSeqNum;
    mRxOldestUnackedSeqNum = mRxNextSeqNum;

    return ret;
}

bool BtpEngine::HasUnackedData() const
{
    return (mRxOldestUnackedSeqNum != mRxNextSeqNum);
}

bool BtpEngine::IsValidAck(SequenceNumber_t ack_num) const
{
    ChipLogDebugBtpEngine(Ble, "entered IsValidAck, ack = %u, oldest = %u, newest = %u", ack_num, mTxOldestUnackedSeqNum,
                          mTxNewestUnackedSeqNum);

    // Return false if not awaiting any ack.
    if (!mExpectingAck)
    {
        ChipLogDebugBtpEngine(Ble, "unexpected ack is invalid");
        return false;
    }

    // Assumption: maximum valid sequence number equals maximum value of SequenceNumber_t.

    if (mTxNewestUnackedSeqNum >= mTxOldestUnackedSeqNum) // If current unacked interval does NOT wrap...
    {
        return (ack_num <= mTxNewestUnackedSeqNum && ack_num >= mTxOldestUnackedSeqNum);
    }
    // Else, if current unacked interval DOES wrap...
    return (ack_num <= mTxNewestUnackedSeqNum || ack_num >= mTxOldestUnackedSeqNum);
}

BLE_ERROR BtpEngine::HandleAckReceived(SequenceNumber_t ack_num)
{
    BLE_ERROR err = BLE_NO_ERROR;

    ChipLogDebugBtpEngine(Ble, "entered HandleAckReceived, ack_num = %u", ack_num);

    // Ensure ack_num falls within range of ack values we're expecting.
    VerifyOrExit(IsValidAck(ack_num), err = BLE_ERROR_INVALID_ACK);

    if (mTxNewestUnackedSeqNum == ack_num) // If ack is for newest outstanding unacknowledged fragment...
    {
        mTxOldestUnackedSeqNum = ack_num;

        // All oustanding fragments have been acknowledged.
        mExpectingAck = false;
    }
    else // If ack is valid, but not for newest oustanding unacknowledged fragment...
    {
        // Update newest unacknowledged fragment to one past that which was just acknowledged.
        mTxOldestUnackedSeqNum = ack_num;
        IncSeqNum(mTxOldestUnackedSeqNum);
    }

exit:
    return err;
}

// Calling convention:
//   EncodeStandAloneAck may only be called if data arg is commited for immediate, synchronous subsequent transmission.
BLE_ERROR BtpEngine::EncodeStandAloneAck(const PacketBufferHandle & data)
{
    BLE_ERROR err = BLE_NO_ERROR;
    uint8_t * characteristic;

    // Ensure enough headroom exists for the lower BLE layers.
    VerifyOrExit(data->EnsureReservedSize(CHIP_CONFIG_BLE_PKT_RESERVED_SIZE), err = BLE_ERROR_NO_MEMORY);

    // Ensure enough space for standalone ack payload.
    VerifyOrExit(data->MaxDataLength() >= CHIP_BLE_TRANSFER_PROTOCOL_STANDALONE_ACK_HEADER_SIZE, err = BLE_ERROR_NO_MEMORY);
    characteristic = data->Start();

    // Since there's no preexisting message payload, we can write BTP header without adjusting data start pointer.
    characteristic[0] = kHeaderFlag_FragmentAck;

    // Acknowledge most recently received sequence number.
    characteristic[1] = GetAndRecordRxAckSeqNum();
    ChipLogDebugBtpEngine(Ble, "===> encoded stand-alone ack = %u", characteristic[1]);

    // Include sequence number for stand-alone ack itself.
    characteristic[2] = GetAndIncrementNextTxSeqNum();

    // Set ack payload data length.
    data->SetDataLength(CHIP_BLE_TRANSFER_PROTOCOL_STANDALONE_ACK_HEADER_SIZE);

exit:
    return err;
}

// Calling convention:
//   BtpEngine does not retain ownership of reassembled messages, layer above needs to free when done.
//
//   BtpEngine does not reset itself on error. Upper layer should free outbound message and inbound reassembly buffers
//   if there is a problem.

// HandleCharacteristicReceived():
//
//   Non-NULL characteristic data arg is always either designated as or appended to the message reassembly buffer,
//   or freed if it holds a stand-alone ack. In all cases, caller must clear its reference to data arg when this
//   function returns.
//
//   Upper layer must immediately clean up and reinitialize protocol engine if returned err != BLE_NO_ERROR.
BLE_ERROR BtpEngine::HandleCharacteristicReceived(System::PacketBufferHandle data, SequenceNumber_t & receivedAck,
                                                  bool & didReceiveAck)
{
    BLE_ERROR err            = BLE_NO_ERROR;
    uint8_t rx_flags         = 0;
    uint8_t cursor           = 0;
    uint8_t * characteristic = data->Start();

    VerifyOrExit(!data.IsNull(), err = BLE_ERROR_BAD_ARGS);

    mRxCharCount++;

    // Get header flags, always in first byte.
    rx_flags = characteristic[cursor++];
#if CHIP_ENABLE_CHIPOBLE_TEST
    if (GetFlag(rx_flags, kHeaderFlag_CommandMessage))
        SetRxPacketType(kType_Control);
    else
        SetRxPacketType(kType_Data);
#endif

    didReceiveAck = GetFlag(rx_flags, kHeaderFlag_FragmentAck);

    // Get ack number, if any.
    if (didReceiveAck)
    {
        receivedAck = characteristic[cursor++];

        err = HandleAckReceived(receivedAck);
        SuccessOrExit(err);
    }

    // Get sequence number.
    mRxNewestUnackedSeqNum = characteristic[cursor++];

    // Verify that received sequence number is the next one we'd expect.
    VerifyOrExit(mRxNewestUnackedSeqNum == mRxNextSeqNum, err = BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER);

    // Increment next expected rx sequence number.
    IncSeqNum(mRxNextSeqNum);

    // If fragment was stand-alone ack, we're done here; no payload for message reassembler.
    if (!DidReceiveData(rx_flags))
    {
        ExitNow();
    }

    // Truncate the incoming fragment length by the mRxFragmentSize as the negotiated
    // mRxFragnentSize may be smaller than the characteristic size.
    data->SetDataLength(chip::min(data->DataLength(), mRxFragmentSize));

    ChipLogDebugBtpEngine(Ble, ">>> BTP reassembler received data:");
    PrintBufDebug(data);

    if (mRxState == kState_Idle)
    {
        // Verify StartMessage header flag set.
        VerifyOrExit(rx_flags & kHeaderFlag_StartMessage, err = BLE_ERROR_INVALID_BTP_HEADER_FLAGS);

        mRxLength = (characteristic[(cursor + 1)] << 8) | characteristic[cursor];
        cursor += 2;

        mRxState = kState_InProgress;

        data->SetStart(&(characteristic[cursor]));

        // Create a new buffer for use as the Rx re-assembly area.
        mRxBuf = PacketBuffer::New();

        VerifyOrExit(!mRxBuf.IsNull(), err = BLE_ERROR_NO_MEMORY);

        mRxBuf->AddToEnd(std::move(data));
        mRxBuf->CompactHead(); // will free 'data' and adjust rx buf's end/length
    }
    else if (mRxState == kState_InProgress)
    {
        // Verify StartMessage header flag NOT set, since we're in the middle of receiving a message.
        VerifyOrExit((rx_flags & kHeaderFlag_StartMessage) == 0, err = BLE_ERROR_INVALID_BTP_HEADER_FLAGS);

        // Verify ContinueMessage or EndMessage header flag set.
        VerifyOrExit((rx_flags & kHeaderFlag_ContinueMessage) || (rx_flags & kHeaderFlag_EndMessage),
                     err = BLE_ERROR_INVALID_BTP_HEADER_FLAGS);

        // Add received fragment to reassembled message buffer.
        data->SetStart(&(characteristic[cursor]));
        mRxBuf->AddToEnd(std::move(data));
        mRxBuf->CompactHead(); // will free 'data' and adjust rx buf's end/length

        // For now, limit BtpEngine message size to max length of 1 pbuf, as we do for chip messages sent via IP.
        // TODO add support for BtpEngine messages longer than 1 pbuf
        VerifyOrExit(mRxBuf->Next() == nullptr, err = BLE_ERROR_RECEIVED_MESSAGE_TOO_BIG);
    }
    else
    {
        err = BLE_ERROR_REASSEMBLER_INCORRECT_STATE;
        ExitNow();
    }

    if (rx_flags & kHeaderFlag_EndMessage)
    {
        // Trim remainder, if any, of received PacketBuffer based on sender-specified length of reassembled message.
        int padding = mRxBuf->DataLength() - mRxLength;

        if (padding > 0)
        {
            mRxBuf->SetDataLength(mRxLength);
        }

        // Ensure all received fragments add up to sender-specified total message size.
        VerifyOrExit(mRxBuf->DataLength() == mRxLength, err = BLE_ERROR_REASSEMBLER_MISSING_DATA);

        // We've reassembled the entire message.
        mRxState = kState_Complete;
        mRxPacketCount++;
    }

exit:
    if (err != BLE_NO_ERROR)
    {
        mRxState = kState_Error;

        // Dump protocol engine state, plus header flags and received data length.
        ChipLogError(Ble, "HandleCharacteristicReceived failed, err = %d, rx_flags = %u", err, rx_flags);
        if (didReceiveAck)
        {
            ChipLogError(Ble, "With rx'd ack = %u", receivedAck);
        }
        if (!mRxBuf.IsNull())
        {
            ChipLogError(Ble, "With rx buf data length = %u", mRxBuf->DataLength());
        }
        LogState();

        if (!data.IsNull())
        {
            // Tack received data onto rx buffer, to be freed when end point resets protocol engine on close.
            if (!mRxBuf.IsNull())
            {
                mRxBuf->AddToEnd(std::move(data));
            }
            else
            {
                mRxBuf = std::move(data);
            }
        }
    }

    return err;
}

void BtpEngine::ClearRxPacket()
{
    if (mRxState == kState_Complete)
    {
        mRxState = kState_Idle;
    }
    mRxBuf = nullptr;
}

// Calling convention:
//   May only be called if data arg is commited for immediate, synchronous subsequent transmission.
//   Returns false on error. Caller must free data arg on error.
bool BtpEngine::HandleCharacteristicSend(System::PacketBufferHandle data, bool send_ack)
{
    uint8_t * characteristic;
    mTxCharCount++;

    if (send_ack && !HasUnackedData())
    {
        ChipLogError(Ble, "HandleCharacteristicSend: send_ack true, but nothing to acknowledge.");
        return false;
    }

    if (mTxState == kState_Idle)
    {
        if (data.IsNull())
        {
            return false;
        }

        mTxBuf    = std::move(data);
        mTxState  = kState_InProgress;
        mTxLength = mTxBuf->DataLength();

        ChipLogDebugBtpEngine(Ble, ">>> CHIPoBle preparing to send whole message:");
        PrintBufDebug(data);

        // Determine fragment header size.
        uint8_t header_size = send_ack ? CHIP_BLE_TRANSFER_PROTOCOL_MAX_HEADER_SIZE
                                       : (CHIP_BLE_TRANSFER_PROTOCOL_MAX_HEADER_SIZE - CHIP_BLE_TRANSFER_PROTOCOL_ACK_SIZE);

        // Ensure enough headroom exists for the BTP header, and any headroom needed by the lower BLE layers.
        if (!mTxBuf->EnsureReservedSize(header_size + CHIP_CONFIG_BLE_PKT_RESERVED_SIZE))
        {
            // handle error
            ChipLogError(Ble, "HandleCharacteristicSend: not enough headroom");
            mTxState = kState_Error;
            mTxBuf   = nullptr; // Avoid double-free after assignment above, as caller frees data on error.

            return false;
        }

        // prepend header.
        characteristic = mTxBuf->Start();
        characteristic -= header_size;
        mTxBuf->SetStart(characteristic);
        uint8_t cursor = 1; // first position past header flags byte

        characteristic[0] = kHeaderFlag_StartMessage;

#if CHIP_ENABLE_CHIPOBLE_TEST
        if (TxPacketType() == kType_Control)
            SetFlag(characteristic[0], kHeaderFlag_CommandMessage, true);
#endif

        if (send_ack)
        {
            SetFlag(characteristic[0], kHeaderFlag_FragmentAck, true);
            characteristic[cursor++] = GetAndRecordRxAckSeqNum();
            ChipLogDebugBtpEngine(Ble, "===> encoded piggybacked ack, ack_num = %u", characteristic[cursor - 1]);
        }

        characteristic[cursor++] = GetAndIncrementNextTxSeqNum();
        characteristic[cursor++] = mTxLength & 0xff;
        characteristic[cursor++] = mTxLength >> 8;

        if ((mTxLength + cursor) <= mTxFragmentSize)
        {
            mTxBuf->SetDataLength(mTxLength + cursor);
            mTxLength = 0;
            SetFlag(characteristic[0], kHeaderFlag_EndMessage, true);
            mTxState = kState_Complete;
            mTxPacketCount++;
        }
        else
        {
            mTxBuf->SetDataLength(mTxFragmentSize);
            mTxLength -= mTxFragmentSize - cursor;
        }

        ChipLogDebugBtpEngine(Ble, ">>> CHIPoBle preparing to send first fragment:");
        PrintBufDebug(data);
    }
    else if (mTxState == kState_InProgress)
    {
        if (!data.IsNull())
        {
            return false;
        }

        // advance past the previous fragment
        characteristic = mTxBuf->Start();
        characteristic += mTxFragmentSize;

        // prepend header
        characteristic -= send_ack
            ? CHIP_BLE_TRANSFER_PROTOCOL_MID_FRAGMENT_MAX_HEADER_SIZE
            : (CHIP_BLE_TRANSFER_PROTOCOL_MID_FRAGMENT_MAX_HEADER_SIZE - CHIP_BLE_TRANSFER_PROTOCOL_ACK_SIZE);
        mTxBuf->SetStart(characteristic);
        uint8_t cursor = 1; // first position past header flags byte

        characteristic[0] = kHeaderFlag_ContinueMessage;

#if CHIP_ENABLE_CHIPOBLE_TEST
        if (TxPacketType() == kType_Control)
            SetFlag(characteristic[0], kHeaderFlag_CommandMessage, true);
#endif

        if (send_ack)
        {
            SetFlag(characteristic[0], kHeaderFlag_FragmentAck, true);
            characteristic[cursor++] = GetAndRecordRxAckSeqNum();
            ChipLogDebugBtpEngine(Ble, "===> encoded piggybacked ack, ack_num = %u", characteristic[cursor - 1]);
        }

        characteristic[cursor++] = GetAndIncrementNextTxSeqNum();

        if ((mTxLength + cursor) <= mTxFragmentSize)
        {
            mTxBuf->SetDataLength(mTxLength + cursor);
            mTxLength = 0;
            SetFlag(characteristic[0], kHeaderFlag_EndMessage, true);
            mTxState = kState_Complete;
            mTxPacketCount++;
        }
        else
        {
            mTxBuf->SetDataLength(mTxFragmentSize);
            mTxLength -= mTxFragmentSize - cursor;
        }

        ChipLogDebugBtpEngine(Ble, ">>> CHIPoBle preparing to send additional fragment:");
        PrintBufDebug(mTxBuf);
    }
    else
    {
        // Invalid tx state.
        return false;
    }

    return true;
}

void BtpEngine::ClearTxPacket()
{
    if (mTxState == kState_Complete)
    {
        mTxState = kState_Idle;
    }
    mTxBuf = nullptr;
}

void BtpEngine::LogState() const
{
    ChipLogError(Ble, "mAppState: %p", mAppState);

    ChipLogError(Ble, "mRxFragmentSize: %d", mRxFragmentSize);
    ChipLogError(Ble, "mRxState: %d", mRxState);
    ChipLogError(Ble, "mRxBuf: %p", mRxBuf.Get_ForNow());
    ChipLogError(Ble, "mRxNextSeqNum: %d", mRxNextSeqNum);
    ChipLogError(Ble, "mRxNewestUnackedSeqNum: %d", mRxNewestUnackedSeqNum);
    ChipLogError(Ble, "mRxOldestUnackedSeqNum: %d", mRxOldestUnackedSeqNum);
    ChipLogError(Ble, "mRxCharCount: %d", mRxCharCount);
    ChipLogError(Ble, "mRxPacketCount: %d", mRxPacketCount);

    ChipLogError(Ble, "mTxFragmentSize: %d", mTxFragmentSize);
    ChipLogError(Ble, "mTxState: %d", mTxState);
    ChipLogError(Ble, "mTxBuf: %p", mTxBuf.Get_ForNow());
    ChipLogError(Ble, "mTxNextSeqNum: %d", mTxNextSeqNum);
    ChipLogError(Ble, "mTxNewestUnackedSeqNum: %d", mTxNewestUnackedSeqNum);
    ChipLogError(Ble, "mTxOldestUnackedSeqNum: %d", mTxOldestUnackedSeqNum);
    ChipLogError(Ble, "mTxCharCount: %d", mTxCharCount);
    ChipLogError(Ble, "mTxPacketCount: %d", mTxPacketCount);
}

void BtpEngine::LogStateDebug() const
{
#ifdef CHIP_BTP_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED
    LogState();
#endif
}

} /* namespace Ble */
} /* namespace chip */

#endif /* CONFIG_NETWORK_LAYER_BLE */
