/**
 *    @file
 *      Implementation for the TransferSession class.
 *      // TODO: Support Asynchronous mode. Currently, only Synchronous mode is supported.
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/common/Constants.h>
#include <support/BufferReader.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <system/SystemPacketBuffer.h>

namespace {
constexpr uint8_t kBdxVersion         = 0;         ///< The version of this implementation of the BDX spec
constexpr size_t kStatusReportMinSize = 2 + 4 + 2; ///< 16 bits for GeneralCode, 32 bits for ProtocolId, 16 bits for ProtocolCode

/**
 * @brief
 *   Allocate a new PacketBuffer and write data from a BDX message struct.
 */
CHIP_ERROR WriteToPacketBuffer(const ::chip::bdx::BdxMessage & msgStruct, ::chip::System::PacketBufferHandle & msgBuf)
{
    size_t msgDataSize = msgStruct.MessageSize();
    ::chip::System::PacketBufferWriter bbuf(msgDataSize);
    if (bbuf.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    msgStruct.WriteToBuffer(bbuf);
    msgBuf = bbuf.Finalize();
    if (msgBuf.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AttachHeader(uint16_t protocolId, uint8_t msgType, ::chip::System::PacketBufferHandle & msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ::chip::PayloadHeader payloadHeader;

    payloadHeader.SetMessageType(protocolId, msgType);

    uint16_t headerSize              = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = payloadHeader.Encode(msgBuf->Start(), headerSize, &actualEncodedHeaderSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}
} // anonymous namespace

namespace chip {
namespace bdx {

TransferSession::TransferSession()
{
    mSuppportedXferOpts.SetRaw(0);
}

void TransferSession::PollOutput(OutputEvent & event, uint64_t curTimeMs)
{
    event = OutputEvent(kNone);

    if (mShouldInitTimeoutStart)
    {
        mTimeoutStartTimeMs     = curTimeMs;
        mShouldInitTimeoutStart = false;
    }

    if (mAwaitingResponse && ((curTimeMs - mTimeoutStartTimeMs) >= mTimeoutMs))
    {
        event             = OutputEvent(kTransferTimeout);
        mState            = kErrorState;
        mAwaitingResponse = false;
        return;
    }

    switch (mPendingOutput)
    {
    case kNone:
        event = OutputEvent(kNone);
        break;
    case kInternalError:
        event = OutputEvent::StatusReportEvent(kInternalError, mStatusReportData);
        break;
    case kStatusReceived:
        event = OutputEvent::StatusReportEvent(kStatusReceived, mStatusReportData);
        break;
    case kMsgToSend:
        event               = OutputEvent(kMsgToSend);
        event.MsgData       = std::move(mPendingMsgHandle);
        mTimeoutStartTimeMs = curTimeMs;
        break;
    case kInitReceived:
        event = OutputEvent::TransferInitEvent(mTransferRequestData, std::move(mPendingMsgHandle));
        break;
    case kAcceptReceived:
        event = OutputEvent::TransferAcceptEvent(mTransferAcceptData, std::move(mPendingMsgHandle));
        break;
    case kQueryReceived:
        event = OutputEvent(kQueryReceived);
        break;
    case kBlockReceived:
        event = OutputEvent::BlockDataEvent(mBlockEventData, std::move(mPendingMsgHandle));
        break;
    case kAckReceived:
        event = OutputEvent(kAckReceived);
        break;
    case kAckEOFReceived:
        event = OutputEvent(kAckEOFReceived);
        break;
    default:
        event = OutputEvent(kNone);
        break;
    }

    // If there's no other pending output but an error occured or was received, then continue to output the error.
    // This ensures that when the TransferSession encounters an error and needs to send a StatusReport, both a kMsgToSend and a
    // kInternalError output event will be emitted.
    if (event.EventType == kNone && mState == kErrorState)
    {
        event = OutputEvent::StatusReportEvent(kInternalError, mStatusReportData);
    }

    mPendingOutput = kNone;
}

CHIP_ERROR TransferSession::StartTransfer(TransferRole role, const TransferInitData & initData, uint32_t timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageType msgType;
    TransferInit initMsg;

    VerifyOrExit(mState == kUnitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mRole      = role;
    mTimeoutMs = timeoutMs;

    // Set transfer parameters. They may be overridden later by an Accept message
    mSuppportedXferOpts.SetRaw(initData.TransferCtlFlagsRaw);
    mMaxSupportedBlockSize = initData.MaxBlockSize;
    mStartOffset           = initData.StartOffset;
    mTransferLength        = initData.Length;

    // Prepare TransferInit message
    initMsg.TransferCtlOptions.SetRaw(initData.TransferCtlFlagsRaw);
    initMsg.Version        = kBdxVersion;
    initMsg.MaxBlockSize   = mMaxSupportedBlockSize;
    initMsg.StartOffset    = mStartOffset;
    initMsg.MaxLength      = mTransferLength;
    initMsg.FileDesignator = initData.FileDesignator;
    initMsg.FileDesLength  = initData.FileDesLength;
    initMsg.Metadata       = initData.Metadata;
    initMsg.MetadataLength = initData.MetadataLength;

    err = WriteToPacketBuffer(initMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    msgType = (mRole == kRole_Sender) ? kBdxMsg_SendInit : kBdxMsg_ReceiveInit;
    err     = AttachHeader(Protocols::kProtocol_BDX, msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    mState            = kAwaitingAccept;
    mAwaitingResponse = true;

    mPendingOutput = kMsgToSend;

exit:
    return err;
}

CHIP_ERROR TransferSession::WaitForTransfer(TransferRole role, BitFlags<uint8_t, TransferControlFlags> xferControlOpts,
                                            uint16_t maxBlockSize, uint32_t timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kUnitialized, err = CHIP_ERROR_INCORRECT_STATE);

    // Used to determine compatibility with any future TransferInit parameters
    mRole                  = role;
    mTimeoutMs             = timeoutMs;
    mSuppportedXferOpts    = xferControlOpts;
    mMaxSupportedBlockSize = maxBlockSize;

    mState = kAwaitingInitMsg;

exit:
    return err;
}

CHIP_ERROR TransferSession::AcceptTransfer(const TransferAcceptData & acceptData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle outMsgBuf;
    BitFlags<uint8_t, TransferControlFlags> proposedControlOpts;

    VerifyOrExit(mState == kNegotiateTransferParams, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == kNone, err = CHIP_ERROR_INCORRECT_STATE);

    // Don't allow a Control method that wasn't supported by the initiator
    // MaxBlockSize can't be larger than the proposed value
    proposedControlOpts.SetRaw(mTransferRequestData.TransferCtlFlagsRaw);
    VerifyOrExit(proposedControlOpts.Has(acceptData.ControlMode), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(acceptData.MaxBlockSize <= mTransferRequestData.MaxBlockSize, err = CHIP_ERROR_INVALID_ARGUMENT);

    mTransferMaxBlockSize = acceptData.MaxBlockSize;

    if (mRole == kRole_Sender)
    {
        mStartOffset    = acceptData.StartOffset;
        mTransferLength = acceptData.Length;

        ReceiveAccept acceptMsg;
        acceptMsg.TransferCtlFlags.Set(acceptData.ControlMode);
        acceptMsg.Version        = mTransferVersion;
        acceptMsg.MaxBlockSize   = acceptData.MaxBlockSize;
        acceptMsg.StartOffset    = acceptData.StartOffset;
        acceptMsg.Length         = acceptData.Length;
        acceptMsg.Metadata       = acceptData.Metadata;
        acceptMsg.MetadataLength = acceptData.MetadataLength;

        err = WriteToPacketBuffer(acceptMsg, mPendingMsgHandle);
        SuccessOrExit(err);

        err = AttachHeader(Protocols::kProtocol_BDX, kBdxMsg_ReceiveAccept, mPendingMsgHandle);
        SuccessOrExit(err);
    }
    else
    {
        SendAccept acceptMsg;
        acceptMsg.TransferCtlFlags.Set(acceptData.ControlMode);
        acceptMsg.Version        = mTransferVersion;
        acceptMsg.MaxBlockSize   = acceptData.MaxBlockSize;
        acceptMsg.Metadata       = acceptData.Metadata;
        acceptMsg.MetadataLength = acceptData.MetadataLength;

        err = WriteToPacketBuffer(acceptMsg, mPendingMsgHandle);
        SuccessOrExit(err);

        err = AttachHeader(Protocols::kProtocol_BDX, kBdxMsg_SendAccept, mPendingMsgHandle);
        SuccessOrExit(err);
    }

    mPendingOutput = kMsgToSend;

    mState = kTransferInProgress;

    if ((mRole == kRole_Receiver && mControlMode == kControl_SenderDrive) ||
        (mRole == kRole_Sender && mControlMode == kControl_ReceiverDrive))
    {
        mAwaitingResponse = true;
    }

exit:
    return err;
}

CHIP_ERROR TransferSession::PrepareBlockQuery()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockQuery queryMsg;

    VerifyOrExit(mState == kTransferInProgress, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRole == kRole_Receiver, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == kNone, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    queryMsg.BlockCounter = mNextQueryNum;

    err = WriteToPacketBuffer(queryMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    err = AttachHeader(Protocols::kProtocol_BDX, kBdxMsg_BlockQuery, mPendingMsgHandle);
    SuccessOrExit(err);

    mPendingOutput = kMsgToSend;

    mAwaitingResponse = true;
    mLastQueryNum     = mNextQueryNum++;

exit:
    return err;
}

CHIP_ERROR TransferSession::PrepareBlock(const BlockData & inData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataBlock blockMsg;
    MessageType msgType;

    VerifyOrExit(mState == kTransferInProgress, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRole == kRole_Sender, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == kNone, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    // Verify non-zero data is provided and is no longer than MaxBlockSize (BlockEOF may contain 0 length data)
    VerifyOrExit((inData.Data != nullptr) && (inData.Length <= mTransferMaxBlockSize), err = CHIP_ERROR_INVALID_ARGUMENT);

    blockMsg.BlockCounter = mNextBlockNum;
    blockMsg.Data         = inData.Data;
    blockMsg.DataLength   = inData.Length;

    err = WriteToPacketBuffer(blockMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    msgType = inData.IsEof ? kBdxMsg_BlockEOF : kBdxMsg_Block;
    err     = AttachHeader(Protocols::kProtocol_BDX, msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    mPendingOutput = kMsgToSend;

    if (msgType == kBdxMsg_BlockEOF)
    {
        mState = kAwaitingEOFAck;
    }

    mAwaitingResponse = true;
    mLastBlockNum     = mNextBlockNum++;

exit:
    return err;
}

CHIP_ERROR TransferSession::PrepareBlockAck()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CounterMessage ackMsg;
    MessageType msgType;

    VerifyOrExit(mRole == kRole_Receiver, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit((mState == kTransferInProgress) || (mState == kReceivedEOF), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == kNone, err = CHIP_ERROR_INCORRECT_STATE);

    ackMsg.BlockCounter = mLastBlockNum;
    msgType             = (mState == kReceivedEOF) ? kBdxMsg_BlockAckEOF : kBdxMsg_BlockAck;

    err = WriteToPacketBuffer(ackMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    err = AttachHeader(Protocols::kProtocol_BDX, msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    if (mState == kTransferInProgress)
    {
        if (mControlMode == kControl_SenderDrive)
        {
            // In Sender Drive, a BlockAck is implied to also be a query for the next Block, so expect to receive a Block
            // message.
            mLastQueryNum     = ackMsg.BlockCounter + 1;
            mAwaitingResponse = true;
        }
    }
    else if (mState == kReceivedEOF)
    {
        mState            = kTransferDone;
        mAwaitingResponse = false;
    }

    mPendingOutput = kMsgToSend;

exit:
    return err;
}

CHIP_ERROR TransferSession::AbortTransfer(StatusCode reason)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit((mState != kUnitialized) && (mState != kTransferDone) && (mState != kErrorState),
                 err = CHIP_ERROR_INCORRECT_STATE);

    PrepareStatusReport(reason);

exit:
    return err;
}

void TransferSession::Reset()
{
    mPendingOutput = kNone;
    mState         = kUnitialized;
    mSuppportedXferOpts.SetRaw(0);
    mTransferVersion       = 0;
    mMaxSupportedBlockSize = 0;
    mStartOffset           = 0;
    mTransferLength        = 0;
    mTransferMaxBlockSize  = 0;

    mPendingMsgHandle = nullptr;

    mNumBytesProcessed = 0;
    mLastBlockNum      = 0;
    mNextBlockNum      = 0;
    mLastQueryNum      = 0;
    mNextQueryNum      = 0;

    mTimeoutMs              = 0;
    mTimeoutStartTimeMs     = 0;
    mShouldInitTimeoutStart = true;
    mAwaitingResponse       = false;
}

CHIP_ERROR TransferSession::HandleMessageReceived(System::PacketBufferHandle msg, uint64_t curTimeMs)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    PayloadHeader payloadHeader;

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
    SuccessOrExit(err);

    msg->ConsumeHead(headerSize);

    if (payloadHeader.GetProtocolID() == Protocols::kProtocol_BDX)
    {
        err = HandleBdxMessage(payloadHeader, std::move(msg));
        SuccessOrExit(err);

        mTimeoutStartTimeMs = curTimeMs;
    }
    else if (payloadHeader.GetProtocolID() == Protocols::kProtocol_Protocol_Common &&
             payloadHeader.GetMessageType() == static_cast<uint8_t>(Protocols::Common::MsgType::StatusReport))
    {
        err = HandleStatusReportMessage(payloadHeader, std::move(msg));
        SuccessOrExit(err);
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    return err;
}

// Return CHIP_ERROR only if there was a problem decoding the message. Otherwise, call PrepareStatusReport().
CHIP_ERROR TransferSession::HandleBdxMessage(PayloadHeader & header, System::PacketBufferHandle msg)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    MessageType msgType = static_cast<MessageType>(header.GetMessageType());

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mPendingOutput == kNone, err = CHIP_ERROR_INCORRECT_STATE);

    switch (msgType)
    {
    case kBdxMsg_SendInit:
    case kBdxMsg_ReceiveInit:
        HandleTransferInit(msgType, std::move(msg));
        break;
    case kBdxMsg_SendAccept:
        HandleSendAccept(std::move(msg));
        break;
    case kBdxMsg_ReceiveAccept:
        HandleReceiveAccept(std::move(msg));
        break;
    case kBdxMsg_BlockQuery:
        HandleBlockQuery(std::move(msg));
        break;
    case kBdxMsg_Block:
        HandleBlock(std::move(msg));
        break;
    case kBdxMsg_BlockEOF:
        HandleBlockEOF(std::move(msg));
        break;
    case kBdxMsg_BlockAck:
        HandleBlockAck(std::move(msg));
        break;
    case kBdxMsg_BlockAckEOF:
        HandleBlockAckEOF(std::move(msg));
        break;
    default:
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        break;
    }

exit:
    return err;
}

/**
 * @brief
 *   Parse a StatusReport message and prepare to emit an OutputEvent with the message data.
 *
 *   NOTE: BDX does not currently expect to ever use a "Success" general code, so it will be treated as an error along with any
 *         other code.
 */
CHIP_ERROR TransferSession::HandleStatusReportMessage(PayloadHeader & header, System::PacketBufferHandle msg)
{
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    mState            = kErrorState;
    mAwaitingResponse = false;

    uint16_t generalCode  = 0;
    uint32_t protocolId   = 0;
    uint16_t protocolCode = 0;
    Encoding::LittleEndian::Reader reader(msg->Start(), msg->DataLength());
    ReturnErrorOnFailure(reader.Read16(&generalCode).Read32(&protocolId).Read16(&protocolCode).StatusCode());
    VerifyOrReturnError((protocolId == Protocols::kProtocol_BDX), CHIP_ERROR_INVALID_MESSAGE_TYPE);

    mStatusReportData.StatusCode = protocolCode;

    mPendingOutput = kStatusReceived;

    return CHIP_NO_ERROR;
}

void TransferSession::HandleTransferInit(MessageType msgType, System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferInit transferInit;

    VerifyOrExit(mState == kAwaitingInitMsg, PrepareStatusReport(kStatus_ServerBadState));

    if (mRole == kRole_Sender)
    {
        VerifyOrExit(msgType == kBdxMsg_ReceiveInit, PrepareStatusReport(kStatus_ServerBadState));
    }
    else
    {
        VerifyOrExit(msgType == kBdxMsg_SendInit, PrepareStatusReport(kStatus_ServerBadState));
    }

    err = transferInit.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));

    ResolveTransferControlOptions(transferInit.TransferCtlOptions);
    mTransferVersion      = ::chip::min(kBdxVersion, transferInit.Version);
    mTransferMaxBlockSize = ::chip::min(mMaxSupportedBlockSize, transferInit.MaxBlockSize);

    // Accept for now, they may be changed or rejected by the peer if this is a ReceiveInit
    mStartOffset    = transferInit.StartOffset;
    mTransferLength = transferInit.MaxLength;

    // Store the Request data to share with the caller for verification
    mTransferRequestData.TransferCtlFlagsRaw = transferInit.TransferCtlOptions.Raw(),
    mTransferRequestData.MaxBlockSize        = transferInit.MaxBlockSize;
    mTransferRequestData.StartOffset         = transferInit.StartOffset;
    mTransferRequestData.Length              = transferInit.MaxLength;
    mTransferRequestData.FileDesignator      = transferInit.FileDesignator;
    mTransferRequestData.FileDesLength       = transferInit.FileDesLength;
    mTransferRequestData.Metadata            = transferInit.Metadata;
    mTransferRequestData.MetadataLength      = transferInit.MetadataLength;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = kInitReceived;

    mState = kNegotiateTransferParams;

exit:
    return;
}

void TransferSession::HandleReceiveAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveAccept rcvAcceptMsg;

    VerifyOrExit(mRole == kRole_Receiver, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kAwaitingAccept, PrepareStatusReport(kStatus_ServerBadState));

    err = rcvAcceptMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));

    // Verify that Accept parameters are compatible with the original proposed parameters
    err = VerifyProposedMode(rcvAcceptMsg.TransferCtlFlags);
    SuccessOrExit(err);

    mTransferMaxBlockSize = rcvAcceptMsg.MaxBlockSize;
    mStartOffset          = rcvAcceptMsg.StartOffset;
    mTransferLength       = rcvAcceptMsg.Length;

    // Note: if VerifyProposedMode() returned with no error, then mControlMode must match the proposed mode in the ReceiveAccept
    // message
    mTransferAcceptData.ControlMode    = mControlMode;
    mTransferAcceptData.MaxBlockSize   = rcvAcceptMsg.MaxBlockSize;
    mTransferAcceptData.StartOffset    = rcvAcceptMsg.StartOffset;
    mTransferAcceptData.Length         = rcvAcceptMsg.Length;
    mTransferAcceptData.Metadata       = rcvAcceptMsg.Metadata;
    mTransferAcceptData.MetadataLength = rcvAcceptMsg.MetadataLength;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = kAcceptReceived;

    mAwaitingResponse = (mControlMode == kControl_SenderDrive);
    mState            = kTransferInProgress;

exit:
    return;
}

void TransferSession::HandleSendAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SendAccept sendAcceptMsg;

    VerifyOrExit(mRole == kRole_Sender, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kAwaitingAccept, PrepareStatusReport(kStatus_ServerBadState));

    err = sendAcceptMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));

    // Verify that Accept parameters are compatible with the original proposed parameters
    err = VerifyProposedMode(sendAcceptMsg.TransferCtlFlags);
    SuccessOrExit(err);

    // Note: if VerifyProposedMode() returned with no error, then mControlMode must match the proposed mode in the SendAccept
    // message
    mTransferMaxBlockSize = sendAcceptMsg.MaxBlockSize;

    mTransferAcceptData.ControlMode    = mControlMode;
    mTransferAcceptData.MaxBlockSize   = sendAcceptMsg.MaxBlockSize;
    mTransferAcceptData.StartOffset    = mStartOffset;    // Not included in SendAccept msg, so use member
    mTransferAcceptData.Length         = mTransferLength; // Not included in SendAccept msg, so use member
    mTransferAcceptData.Metadata       = sendAcceptMsg.Metadata;
    mTransferAcceptData.MetadataLength = sendAcceptMsg.MetadataLength;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = kAcceptReceived;

    mAwaitingResponse = (mControlMode == kControl_ReceiverDrive);
    mState            = kTransferInProgress;

exit:
    return;
}

void TransferSession::HandleBlockQuery(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockQuery query;

    VerifyOrExit(mRole == kRole_Sender, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kTransferInProgress, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(kStatus_ServerBadState));

    err = query.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));

    VerifyOrExit(query.BlockCounter == mNextBlockNum, PrepareStatusReport(kStatus_BadBlockCounter));

    mPendingOutput = kQueryReceived;

    mAwaitingResponse = false;
    mLastQueryNum     = query.BlockCounter;

exit:
    return;
}

void TransferSession::HandleBlock(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Block blockMsg;

    VerifyOrExit(mRole == kRole_Receiver, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kTransferInProgress, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(kStatus_ServerBadState));

    err = blockMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));

    VerifyOrExit(blockMsg.BlockCounter == mLastQueryNum, PrepareStatusReport(kStatus_BadBlockCounter));
    VerifyOrExit((blockMsg.DataLength > 0) && (blockMsg.DataLength <= mTransferMaxBlockSize),
                 PrepareStatusReport(kStatus_BadMessageContents));

    if (IsTransferLengthDefinite())
    {
        VerifyOrExit(mNumBytesProcessed + blockMsg.DataLength <= mTransferLength, PrepareStatusReport(kStatus_LengthMismatch));
    }

    mBlockEventData.Data   = blockMsg.Data;
    mBlockEventData.Length = blockMsg.DataLength;
    mBlockEventData.IsEof  = false;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = kBlockReceived;

    mNumBytesProcessed += blockMsg.DataLength;
    mLastBlockNum = blockMsg.BlockCounter;

    mAwaitingResponse = false;

exit:
    return;
}

void TransferSession::HandleBlockEOF(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockEOF blockEOFMsg;

    VerifyOrExit(mRole == kRole_Receiver, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kTransferInProgress, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(kStatus_ServerBadState));

    err = blockEOFMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));

    VerifyOrExit(blockEOFMsg.BlockCounter == mLastQueryNum, PrepareStatusReport(kStatus_BadBlockCounter));
    VerifyOrExit(blockEOFMsg.DataLength <= mTransferMaxBlockSize, PrepareStatusReport(kStatus_BadMessageContents));

    mBlockEventData.Data   = blockEOFMsg.Data;
    mBlockEventData.Length = blockEOFMsg.DataLength;
    mBlockEventData.IsEof  = true;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = kBlockReceived;

    mNumBytesProcessed += blockEOFMsg.DataLength;
    mLastBlockNum = blockEOFMsg.BlockCounter;

    mAwaitingResponse = false;
    mState            = kReceivedEOF;

exit:
    return;
}

void TransferSession::HandleBlockAck(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockAck ackMsg;

    VerifyOrExit(mRole == kRole_Sender, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kTransferInProgress, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(kStatus_ServerBadState));

    err = ackMsg.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));
    VerifyOrExit(ackMsg.BlockCounter == mLastBlockNum, PrepareStatusReport(kStatus_BadBlockCounter));

    mPendingOutput = kAckReceived;

    // In Receiver Drive, the Receiver can send a BlockAck to indicate receipt of the message and reset the timeout.
    // In this case, the Sender should wait to receive a BlockQuery next.
    mAwaitingResponse = (mControlMode == kControl_ReceiverDrive);

exit:
    return;
}

void TransferSession::HandleBlockAckEOF(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockAckEOF ackMsg;

    VerifyOrExit(mRole == kRole_Sender, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mState == kAwaitingEOFAck, PrepareStatusReport(kStatus_ServerBadState));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(kStatus_ServerBadState));

    err = ackMsg.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(kStatus_BadMessageContents));
    VerifyOrExit(ackMsg.BlockCounter == mLastBlockNum, PrepareStatusReport(kStatus_BadBlockCounter));

    mPendingOutput = kAckEOFReceived;

    mAwaitingResponse = false;

    mState = kTransferDone;

exit:
    return;
}

void TransferSession::ResolveTransferControlOptions(const BitFlags<uint8_t, TransferControlFlags> & proposed)
{
    // Must specify at least one synchronous option
    if (!proposed.Has(kControl_SenderDrive) && !proposed.Has(kControl_ReceiverDrive))
    {
        PrepareStatusReport(kStatus_TransferMethodNotSupported);
        return;
    }

    // Ensure there are options supported by both nodes. Async gets priority.
    // If there is only one common option, choose that one. Otherwise the application must pick.
    BitFlags<uint8_t, TransferControlFlags> commonOpts;
    commonOpts.SetRaw(proposed.Raw() & mSuppportedXferOpts.Raw());
    if (commonOpts.Raw() == 0)
    {
        PrepareStatusReport(kStatus_TransferMethodNotSupported);
    }
    else if (commonOpts.HasOnly(kControl_Async))
    {
        mControlMode = kControl_Async;
    }
    else if (commonOpts.HasOnly(kControl_ReceiverDrive))
    {
        mControlMode = kControl_ReceiverDrive;
    }
    else if (commonOpts.HasOnly(kControl_SenderDrive))
    {
        mControlMode = kControl_SenderDrive;
    }
}

CHIP_ERROR TransferSession::VerifyProposedMode(const BitFlags<uint8_t, TransferControlFlags> & proposed)
{
    TransferControlFlags mode;

    // Must specify only one mode in Accept messages
    if (proposed.HasOnly(kControl_Async))
    {
        mode = kControl_Async;
    }
    else if (proposed.HasOnly(kControl_ReceiverDrive))
    {
        mode = kControl_ReceiverDrive;
    }
    else if (proposed.HasOnly(kControl_SenderDrive))
    {
        mode = kControl_SenderDrive;
    }
    else
    {
        PrepareStatusReport(kStatus_BadMessageContents);
        return CHIP_ERROR_INTERNAL;
    }

    // Verify the proposed mode is supported by this instance
    if (mSuppportedXferOpts.Has(mode))
    {
        mControlMode = mode;
    }
    else
    {
        PrepareStatusReport(kStatus_TransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void TransferSession::PrepareStatusReport(StatusCode code)
{
    mStatusReportData.StatusCode = code;

    System::PacketBufferWriter bbuf(kStatusReportMinSize);
    VerifyOrReturn(!bbuf.IsNull());

    bbuf.Put16(static_cast<uint16_t>(Protocols::Common::StatusCode::Failure));
    bbuf.Put32(Protocols::kProtocol_BDX);
    bbuf.Put16(mStatusReportData.StatusCode);

    mPendingMsgHandle = bbuf.Finalize();
    if (mPendingMsgHandle.IsNull())
    {
        mPendingOutput = kInternalError;
    }
    else
    {
        CHIP_ERROR err = AttachHeader(Protocols::kProtocol_Protocol_Common,
                                      static_cast<uint8_t>(Protocols::Common::MsgType::StatusReport), mPendingMsgHandle);
        VerifyOrReturn(err == CHIP_NO_ERROR);

        mPendingOutput = kMsgToSend;
    }

    mState            = kErrorState;
    mAwaitingResponse = false; // Prevent triggering timeout
}

bool TransferSession::IsTransferLengthDefinite()
{
    return (mTransferLength > 0);
}

TransferSession::OutputEvent TransferSession::OutputEvent::TransferInitEvent(TransferInitData data, System::PacketBufferHandle msg)
{
    OutputEvent event(kInitReceived);
    event.MsgData          = std::move(msg);
    event.transferInitData = data;
    return event;
}

/**
 * @brief
 *   Convenience method for constructing an OutputEvent with TransferAcceptData that does not contain Metadata
 */
TransferSession::OutputEvent TransferSession::OutputEvent::TransferAcceptEvent(TransferAcceptData data)
{
    OutputEvent event(kAcceptReceived);
    event.transferAcceptData = data;
    return event;
}
/**
 * @brief
 *   Convenience method for constructing an OutputEvent with TransferAcceptData that contains Metadata
 */
TransferSession::OutputEvent TransferSession::OutputEvent::TransferAcceptEvent(TransferAcceptData data,
                                                                               System::PacketBufferHandle msg)
{
    OutputEvent event = TransferAcceptEvent(data);
    event.MsgData     = std::move(msg);
    return event;
}

TransferSession::OutputEvent TransferSession::OutputEvent::BlockDataEvent(BlockData data, System::PacketBufferHandle msg)
{
    OutputEvent event(kBlockReceived);
    event.MsgData   = std::move(msg);
    event.blockdata = data;
    return event;
}

/**
 * @brief
 *   Convenience method for constructing an event with kInternalError or kOutputStatusReceived
 */
TransferSession::OutputEvent TransferSession::OutputEvent::StatusReportEvent(OutputEventType type, StatusReportData data)
{
    OutputEvent event(type);
    event.statusData = data;
    return event;
}

} // namespace bdx
} // namespace chip
