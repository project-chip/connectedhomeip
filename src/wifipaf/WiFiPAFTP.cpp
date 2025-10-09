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
 *      This module implements encode, decode, fragmentation and reassembly of
 *      PAF Transport Layer (PAFTP) packet types for transport of
 *      CHIP-over-WiFiPAF (CHIPoPAF) links.
 *
 */

#define _CHIP_WIFI_PAFTP_H
#include "WiFiPAFTP.h"

#include <lib/core/CHIPConfig.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <utility>

#include "WiFiPAFConfig.h"
#include "WiFiPAFError.h"

// Define below to enable extremely verbose PAFTP-specific debug logging.
#undef CHIP_PAF_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED

#ifdef CHIP_PAF_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED
#define ChipLogDebugWiFiPAFTP(MOD, MSG, ...) ChipLogError(MOD, MSG, ##__VA_ARGS__)
#define ChipLogDebugBufferWiFiPAFTP(MOD, BUF)                                                                                      \
    ChipLogByteSpan(MOD, ByteSpan((BUF)->Start(), ((BUF)->DataLength() < 8 ? (BUF)->DataLength() : 8u)))
#else
#define ChipLogDebugWiFiPAFTP(MOD, MSG, ...)
#define ChipLogDebugBufferWiFiPAFTP(MOD, BUF)
#endif

namespace chip {
namespace WiFiPAF {
SequenceNumber_t OffsetSeqNum(SequenceNumber_t & tgtSeqNum, SequenceNumber_t & baseSeqNum)
{
    if (tgtSeqNum >= baseSeqNum)
        return static_cast<SequenceNumber_t>(tgtSeqNum - baseSeqNum);
    return static_cast<SequenceNumber_t>((0xff - baseSeqNum) + tgtSeqNum + 1);
}

static inline bool DidReceiveData(BitFlags<WiFiPAFTP::HeaderFlags> rx_flags)
{
    return rx_flags.HasAny(WiFiPAFTP::HeaderFlags::kStartMessage, WiFiPAFTP::HeaderFlags::kContinueMessage,
                           WiFiPAFTP::HeaderFlags::kEndMessage);
}

const uint16_t WiFiPAFTP::sDefaultFragmentSize = CHIP_PAF_DEFAULT_MTU; // minimum MTU - 3 bytes for operation header
const uint16_t WiFiPAFTP::sMaxFragmentSize =
    CHIP_PAF_DEFAULT_MTU; // Maximum size of PAFTP segment. Ref: 4.21.3.1, "Supported Maximum Service Specific Info Length"

CHIP_ERROR WiFiPAFTP::Init(void * an_app_state, bool expect_first_ack)
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

    return CHIP_NO_ERROR;
}

SequenceNumber_t WiFiPAFTP::GetAndIncrementNextTxSeqNum()
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
    mTxNextSeqNum = IncSeqNum(mTxNextSeqNum);

    return ret;
}

SequenceNumber_t WiFiPAFTP::GetAndRecordRxAckSeqNum()
{
    SequenceNumber_t ret = mRxNewestUnackedSeqNum;

    mRxNewestUnackedSeqNum = mRxNextSeqNum;
    mRxOldestUnackedSeqNum = mRxNextSeqNum;

    return ret;
}

bool WiFiPAFTP::HasUnackedData() const
{
    return (mRxOldestUnackedSeqNum != mRxNextSeqNum);
}

bool WiFiPAFTP::IsValidAck(SequenceNumber_t ack_num) const
{
    ChipLogDebugWiFiPAFTP(WiFiPAF, "entered IsValidAck, ack = %u, oldest = %u, newest = %u", ack_num, mTxOldestUnackedSeqNum,
                          mTxNewestUnackedSeqNum);

    // Return false if not awaiting any ack.
    if (!mExpectingAck)
    {
        ChipLogDebugWiFiPAFTP(WiFiPAF, "unexpected ack is invalid");
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

CHIP_ERROR WiFiPAFTP::HandleAckReceived(SequenceNumber_t ack_num)
{
    ChipLogDebugWiFiPAFTP(WiFiPAF, "entered HandleAckReceived, ack_num = %u", ack_num);

    // Ensure ack_num falls within range of ack values we're expecting.
    VerifyOrReturnError(IsValidAck(ack_num), WIFIPAF_ERROR_INVALID_ACK);

    if (mTxNewestUnackedSeqNum == ack_num) // If ack is for newest outstanding unacknowledged fragment...
    {
        mTxOldestUnackedSeqNum = ack_num;

        // All outstanding fragments have been acknowledged.
        mExpectingAck = false;
    }
    else // If ack is valid, but not for newest outstanding unacknowledged fragment...
    {
        // Update newest unacknowledged fragment to one past that which was just acknowledged.
        mTxOldestUnackedSeqNum = ack_num;
        mTxOldestUnackedSeqNum = IncSeqNum(mTxOldestUnackedSeqNum);
    }

    return CHIP_NO_ERROR;
}

// Calling convention:
//   EncodeStandAloneAck may only be called if data arg is committed for immediate, synchronous subsequent transmission.
CHIP_ERROR WiFiPAFTP::EncodeStandAloneAck(const PacketBufferHandle & data)
{
    // Ensure enough headroom exists for the lower BLE layers.
    VerifyOrReturnError(data->EnsureReservedSize(CHIP_CONFIG_BLE_PKT_RESERVED_SIZE), CHIP_ERROR_NO_MEMORY);

    // Ensure enough space for standalone ack payload.
    VerifyOrReturnError(data->MaxDataLength() >= kTransferProtocolStandaloneAckHeaderSize, CHIP_ERROR_NO_MEMORY);
    uint8_t * characteristic = data->Start();

    // Since there's no preexisting message payload, we can write BTP header without adjusting data start pointer.
    characteristic[0] = static_cast<uint8_t>(HeaderFlags::kFragmentAck);

    // Acknowledge most recently received sequence number.
    characteristic[1] = GetAndRecordRxAckSeqNum();
    ChipLogDebugWiFiPAFTP(WiFiPAF, "===> encoded stand-alone ack = %u", characteristic[1]);

    // Include sequence number for stand-alone ack itself.
    characteristic[2] = GetAndIncrementNextTxSeqNum();

    // Set ack payload data length.
    data->SetDataLength(kTransferProtocolStandaloneAckHeaderSize);

    return CHIP_NO_ERROR;
}

// Calling convention:
//   WiFiPAFTP does not retain ownership of reassembled messages, layer above needs to free when done.
//
//   WiFiPAFTP does not reset itself on error. Upper layer should free outbound message and inbound reassembly buffers
//   if there is a problem.

// HandleCharacteristicReceived():
//
//   Non-NULL characteristic data arg is always either designated as or appended to the message reassembly buffer,
//   or freed if it holds a stand-alone ack. In all cases, caller must clear its reference to data arg when this
//   function returns.
//
//   Upper layer must immediately clean up and reinitialize protocol engine if returned err != CHIP_NO_ERROR.
CHIP_ERROR WiFiPAFTP::HandleCharacteristicReceived(System::PacketBufferHandle && data, SequenceNumber_t & receivedAck,
                                                   bool & didReceiveAck)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BitFlags<HeaderFlags> rx_flags;

    VerifyOrExit(!data.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    { // Scope for reader, so we can do the VerifyOrExit above.
        // Data uses little-endian byte order.
        Encoding::LittleEndian::Reader reader(data->Start(), data->DataLength());

        mRxCharCount++;

        // Get header flags, always in first byte.
        err = reader.Read8(rx_flags.RawStorage()).StatusCode();
        SuccessOrExit(err);

        didReceiveAck = rx_flags.Has(HeaderFlags::kFragmentAck);

        // Get ack number, if any.
        if (didReceiveAck)
        {
            err = reader.Read8(&receivedAck).StatusCode();
            SuccessOrExit(err);

            err = HandleAckReceived(receivedAck);
            // Multiple-ACK
            if (err != CHIP_NO_ERROR)
            {
                ChipLogDebugWiFiPAFTP(WiFiPAF, "Drop the invalid ack, but update the seq_n and leave");
                err = reader.Read8(&mRxNewestUnackedSeqNum).StatusCode();
                SuccessOrExit(err);
                ChipLogDebugWiFiPAFTP(WiFiPAF, "Update the seq_n: %u", mRxNewestUnackedSeqNum);
                mRxNextSeqNum = mRxNewestUnackedSeqNum;
                mRxNextSeqNum = IncSeqNum(mRxNextSeqNum);
                LogState();
                return CHIP_NO_ERROR;
            }
        }

        // Get sequence number.
        err = reader.Read8(&mRxNewestUnackedSeqNum).StatusCode();
        SuccessOrExit(err);
        ChipLogDebugWiFiPAFTP(WiFiPAF, "(Rx_Seq, mRxNextSeqNum)=(%u, %u), mRxState: %u", mRxNewestUnackedSeqNum, mRxNextSeqNum,
                              mRxState);
        mRxSeqHist[mRxSeqHistId] = mRxNewestUnackedSeqNum;
        mRxSeqHistId             = (mRxSeqHistId + 1) % CHIP_PAFTP_RXHIST_SIZE;
        if (mRxNewestUnackedSeqNum < mRxNextSeqNum)
        {
            // Drop the duplicated rx-pkt
            ChipLogError(WiFiPAF, "Drop the duplicated rx pkt!");
            return CHIP_NO_ERROR;
        }

        // Verify that received sequence number is the next one we'd expect.
        VerifyOrExit(mRxNewestUnackedSeqNum == mRxNextSeqNum, err = WIFIPAF_ERROR_INVALID_PAFTP_SEQUENCE_NUMBER);

        // Increment next expected rx sequence number.
        mRxNextSeqNum = IncSeqNum(mRxNextSeqNum);

        // If fragment was stand-alone ack, we're done here; no payload for message reassembler.
        if (!DidReceiveData(rx_flags))
        {
            ExitNow();
        }

        // Truncate the incoming fragment length by the mRxFragmentSize as the negotiated
        // mRxFragmentSize may be smaller than the characteristic size.  Make sure
        // we're not truncating to a data length smaller than what we have already consumed.
        VerifyOrExit(reader.OctetsRead() <= mRxFragmentSize, err = WIFIPAF_ERROR_REASSEMBLER_INCORRECT_STATE);
        data->SetDataLength(std::min(data->DataLength(), static_cast<size_t>(mRxFragmentSize)));

        // Now mark the bytes we consumed as consumed.
        data->ConsumeHead(static_cast<uint16_t>(reader.OctetsRead()));

        ChipLogDebugWiFiPAFTP(WiFiPAF, ">>> PAFTP reassembler received data:");
        ChipLogDebugBufferWiFiPAFTP(WiFiPAF, data);
    }

    if (mRxState == kState_Idle)
    {
        // We need a new reader, because the state of our outer reader no longer
        // matches the state of the packetbuffer, both in terms of start
        // position and available length.
        Encoding::LittleEndian::Reader startReader(data->Start(), data->DataLength());

        // Verify StartMessage header flag set.
        VerifyOrExit(rx_flags.Has(HeaderFlags::kStartMessage), err = WIFIPAF_ERROR_INVALID_PAFTP_HEADER_FLAGS);

        err = startReader.Read16(&mRxLength).StatusCode();
        SuccessOrExit(err);

        mRxState = kState_InProgress;

        data->ConsumeHead(static_cast<uint16_t>(startReader.OctetsRead()));

        // Create a new buffer for use as the Rx re-assembly area.
        mRxBuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

        VerifyOrExit(!mRxBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        mRxBuf->AddToEnd(std::move(data));
        mRxBuf->CompactHead(); // will free 'data' and adjust rx buf's end/length

        // For now, limit WiFiPAFTP message size to max length of 1 pbuf, as we do for chip messages sent via IP.
        // TODO add support for WiFiPAFTP messages longer than 1 pbuf
        VerifyOrExit(!mRxBuf->HasChainedBuffer(), err = CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    }
    else if (mRxState == kState_InProgress)
    {
        // Verify StartMessage header flag NOT set, since we're in the middle of receiving a message.
        VerifyOrExit(!rx_flags.Has(HeaderFlags::kStartMessage), err = WIFIPAF_ERROR_INVALID_PAFTP_HEADER_FLAGS);

        // Verify ContinueMessage or EndMessage header flag set.
        VerifyOrExit(rx_flags.HasAny(HeaderFlags::kContinueMessage, HeaderFlags::kEndMessage),
                     err = WIFIPAF_ERROR_INVALID_PAFTP_HEADER_FLAGS);

        // Add received fragment to reassembled message buffer.
        mRxBuf->AddToEnd(std::move(data));
        mRxBuf->CompactHead(); // will free 'data' and adjust rx buf's end/length

        // For now, limit WiFiPAFTP message size to max length of 1 pbuf, as we do for chip messages sent via IP.
        // TODO add support for WiFiPAFTP messages longer than 1 pbuf
        VerifyOrExit(!mRxBuf->HasChainedBuffer(), err = CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG);
    }
    else
    {
        err = WIFIPAF_ERROR_REASSEMBLER_INCORRECT_STATE;
        ExitNow();
    }

    if (rx_flags.Has(HeaderFlags::kEndMessage))
    {
        // Trim remainder, if any, of the received packet buffer based on sender-specified length of reassembled message.
        VerifyOrExit(CanCastTo<uint16_t>(mRxBuf->DataLength()), err = CHIP_ERROR_MESSAGE_TOO_LONG);
        int padding = static_cast<uint16_t>(mRxBuf->DataLength()) - mRxLength;

        if (padding > 0)
        {
            mRxBuf->SetDataLength(static_cast<size_t>(mRxLength));
        }

        // Ensure all received fragments add up to sender-specified total message size.
        VerifyOrExit(mRxBuf->DataLength() == mRxLength, err = WIFIPAF_ERROR_REASSEMBLER_MISSING_DATA);

        // We've reassembled the entire message.
        mRxState = kState_Complete;
        mRxPacketCount++;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        mRxState = kState_Error;
        // Dump protocol engine state, plus header flags and received data length.
        ChipLogError(WiFiPAF, "HandleCharacteristicReceived failed, err = %" CHIP_ERROR_FORMAT ", rx_flags = %u", err.Format(),
                     rx_flags.Raw());
        if (didReceiveAck)
        {
            ChipLogError(WiFiPAF, "With rx'd ack = %u", receivedAck);
        }
        if (!mRxBuf.IsNull())
        {
            ChipLogError(WiFiPAF, "With rx buf data length = %u", static_cast<unsigned>(mRxBuf->DataLength()));
        }
        LogState();

        if (!data.IsNull()) // NOLINT(bugprone-use-after-move)
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

PacketBufferHandle WiFiPAFTP::TakeRxPacket()
{
    if (mRxState == kState_Complete)
    {
        mRxState = kState_Idle;
    }
    return std::move(mRxBuf);
}

// Calling convention:
//   May only be called if data arg is committed for immediate, synchronous subsequent transmission.
//   Returns false on error. Caller must free data arg on error.
bool WiFiPAFTP::HandleCharacteristicSend(System::PacketBufferHandle data, bool send_ack)
{
    uint8_t * characteristic;
    mTxCharCount++;

    if (send_ack && !HasUnackedData())
    {
        ChipLogError(Inet, "HandleCharacteristicSend: send_ack true, but nothing to acknowledge.");
        return false;
    }

    if (mTxState == kState_Idle)
    {
        if (data.IsNull())
        {
            return false;
        }

        mTxBuf   = std::move(data);
        mTxState = kState_InProgress;
        VerifyOrReturnError(CanCastTo<uint16_t>(mTxBuf->DataLength()), false);
        mTxLength = static_cast<uint16_t>(mTxBuf->DataLength());

        ChipLogDebugWiFiPAFTP(WiFiPAF, ">>> CHIPoWiFiPAF preparing to send whole message:");
        ChipLogDebugBufferWiFiPAFTP(WiFiPAF, mTxBuf);

        // Determine fragment header size.
        uint8_t header_size =
            send_ack ? kTransferProtocolMaxHeaderSize : (kTransferProtocolMaxHeaderSize - kTransferProtocolAckSize);

        // Ensure enough headroom exists for the PAFTP header
        if (!mTxBuf->EnsureReservedSize(header_size))
        {
            // handle error
            ChipLogError(Inet, "HandleCharacteristicSend: not enough headroom");
            mTxState = kState_Error;
            mTxBuf   = nullptr; // Avoid double-free after assignment above, as caller frees data on error.

            return false;
        }

        // prepend header.
        characteristic = mTxBuf->Start();
        characteristic -= header_size;
        mTxBuf->SetStart(characteristic);
        uint8_t cursor = 1; // first position past header flags byte
        BitFlags<HeaderFlags> headerFlags(HeaderFlags::kStartMessage);

        if (send_ack)
        {
            headerFlags.Set(HeaderFlags::kFragmentAck);
            characteristic[cursor++] = GetAndRecordRxAckSeqNum();
            ChipLogDebugWiFiPAFTP(WiFiPAF, "===> encoded piggybacked ack, ack_num = %u", characteristic[cursor - 1]);
        }

        characteristic[cursor++] = GetAndIncrementNextTxSeqNum();
        characteristic[cursor++] = static_cast<uint8_t>(mTxLength & 0xff);
        characteristic[cursor++] = static_cast<uint8_t>(mTxLength >> 8);

        if ((mTxLength + cursor) <= mTxFragmentSize)
        {
            mTxBuf->SetDataLength(static_cast<uint16_t>(mTxLength + cursor));
            mTxLength = 0;
            headerFlags.Set(HeaderFlags::kEndMessage);
            mTxState = kState_Complete;
            mTxPacketCount++;
        }
        else
        {
            mTxBuf->SetDataLength(mTxFragmentSize);
            mTxLength = static_cast<uint16_t>((mTxLength + cursor) - mTxFragmentSize);
        }

        characteristic[0] = headerFlags.Raw();
        ChipLogDebugWiFiPAFTP(WiFiPAF, ">>> CHIPoWiFiPAF preparing to send first fragment:");
        ChipLogDebugBufferWiFiPAFTP(WiFiPAF, mTxBuf);
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
        characteristic -= send_ack ? kTransferProtocolMidFragmentMaxHeaderSize
                                   : (kTransferProtocolMidFragmentMaxHeaderSize - kTransferProtocolAckSize);
        mTxBuf->SetStart(characteristic);
        uint8_t cursor = 1; // first position past header flags byte

        BitFlags<HeaderFlags> headerFlags(HeaderFlags::kContinueMessage);

        if (send_ack)
        {
            headerFlags.Set(HeaderFlags::kFragmentAck);
            characteristic[cursor++] = GetAndRecordRxAckSeqNum();
            ChipLogDebugWiFiPAFTP(WiFiPAF, "===> encoded piggybacked ack, ack_num = %u", characteristic[cursor - 1]);
        }

        characteristic[cursor++] = GetAndIncrementNextTxSeqNum();

        if ((mTxLength + cursor) <= mTxFragmentSize)
        {
            mTxBuf->SetDataLength(static_cast<uint16_t>(mTxLength + cursor));
            mTxLength = 0;
            headerFlags.Set(HeaderFlags::kEndMessage);
            mTxState = kState_Complete;
            mTxPacketCount++;
        }
        else
        {
            mTxBuf->SetDataLength(mTxFragmentSize);
            mTxLength = static_cast<uint16_t>((mTxLength + cursor) - mTxFragmentSize);
        }

        characteristic[0] = headerFlags.Raw();
        ChipLogDebugWiFiPAFTP(WiFiPAF, ">>> CHIPoWiFiPAF preparing to send additional fragment:");
        ChipLogDebugBufferWiFiPAFTP(WiFiPAF, mTxBuf);
    }
    else
    {
        // Invalid tx state.
        ChipLogError(WiFiPAF, "Invalid tx state: %u", mTxState);
        return false;
    }

    return true;
}

PacketBufferHandle WiFiPAFTP::TakeTxPacket()
{
    if (mTxState == kState_Complete)
    {
        mTxState = kState_Idle;
    }
    return std::move(mTxBuf);
}

void WiFiPAFTP::LogState() const
{
    ChipLogError(WiFiPAF, "mAppState: %p", mAppState);

    ChipLogError(WiFiPAF, "mRxFragmentSize: %d", mRxFragmentSize);
    ChipLogError(WiFiPAF, "mRxState: %d", mRxState);
    ChipLogError(WiFiPAF, "mRxBuf: %d", !mRxBuf.IsNull());
    ChipLogError(WiFiPAF, "mRxNextSeqNum: %d", mRxNextSeqNum);
    ChipLogError(WiFiPAF, "mRxNewestUnackedSeqNum: %d", mRxNewestUnackedSeqNum);
    ChipLogError(WiFiPAF, "mRxOldestUnackedSeqNum: %d", mRxOldestUnackedSeqNum);
    ChipLogError(WiFiPAF, "mRxCharCount: %d", mRxCharCount);
    ChipLogError(WiFiPAF, "mRxPacketCount: %d", mRxPacketCount);

    char RxSeqHistMsg[64];
    memset(RxSeqHistMsg, 0, sizeof(RxSeqHistMsg));
    for (uint8_t idx = 0; idx < CHIP_PAFTP_RXHIST_SIZE; idx++)
    {
        char RxSeq[6];
        snprintf(RxSeq, sizeof(RxSeq), "%03u ", mRxSeqHist[(mRxSeqHistId + 1 + idx) % CHIP_PAFTP_RXHIST_SIZE]);
        strcat(RxSeqHistMsg, RxSeq);
    }
    ChipLogError(WiFiPAF, "Rx_Seq_History: [%s]", RxSeqHistMsg);

    ChipLogError(WiFiPAF, "mTxFragmentSize: %d", mTxFragmentSize);
    ChipLogError(WiFiPAF, "mTxState: %d", mTxState);
    ChipLogError(WiFiPAF, "mTxBuf: %d", !mTxBuf.IsNull());
    ChipLogError(WiFiPAF, "mTxNextSeqNum: %d", mTxNextSeqNum);
    ChipLogError(WiFiPAF, "mTxNewestUnackedSeqNum: %d", mTxNewestUnackedSeqNum);
    ChipLogError(WiFiPAF, "mTxOldestUnackedSeqNum: %d", mTxOldestUnackedSeqNum);
    ChipLogError(WiFiPAF, "mTxCharCount: %d", mTxCharCount);
    ChipLogError(WiFiPAF, "mTxPacketCount: %d", mTxPacketCount);
}

void WiFiPAFTP::LogStateDebug() const
{
#ifdef CHIP_PAF_PROTOCOL_ENGINE_DEBUG_LOGGING_ENABLED
    LogState();
#endif
}

} /* namespace WiFiPAF */
} /* namespace chip */
