/**
 *    @file
 *      Implementation for the TransferSession class.
 *      // TODO: Support Asynchronous mode. Currently, only Synchronous mode is supported.
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <support/BufferReader.h>
#include <support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>

#include <type_traits>

namespace {
constexpr uint8_t kBdxVersion = 0; ///< The version of this implementation of the BDX spec

/**
 * @brief
 *   Allocate a new PacketBuffer and write data from a BDX message struct.
 */
CHIP_ERROR WriteToPacketBuffer(const ::chip::bdx::BdxMessage & msgStruct, ::chip::System::PacketBufferHandle & msgBuf)
{
    size_t msgDataSize = msgStruct.MessageSize();
    ::chip::Encoding::LittleEndian::PacketBufferWriter bbuf(chip::MessagePacketBuffer::New(msgDataSize), msgDataSize);
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

// We could make this whole method a template, but it's probably smaller code to
// share the implementation across all message types.
CHIP_ERROR AttachHeader(chip::Protocols::Id protocolId, uint8_t msgType, ::chip::System::PacketBufferHandle & msgBuf)
{
    ::chip::PayloadHeader payloadHeader;

    payloadHeader.SetMessageType(protocolId, msgType);

    CHIP_ERROR err = payloadHeader.EncodeBeforeData(msgBuf);
    SuccessOrExit(err);

exit:
    return err;
}

template <typename MessageType>
inline CHIP_ERROR AttachHeader(MessageType msgType, ::chip::System::PacketBufferHandle & msgBuf)
{
    return AttachHeader(chip::Protocols::MessageTypeTraits<MessageType>::ProtocolId(), static_cast<uint8_t>(msgType), msgBuf);
}
} // anonymous namespace

namespace chip {
namespace bdx {

TransferSession::TransferSession()
{
    mSuppportedXferOpts.ClearAll();
}

void TransferSession::PollOutput(OutputEvent & event, uint64_t curTimeMs)
{
    event = OutputEvent(OutputEventType::kNone);

    if (mShouldInitTimeoutStart)
    {
        mTimeoutStartTimeMs     = curTimeMs;
        mShouldInitTimeoutStart = false;
    }

    if (mAwaitingResponse && ((curTimeMs - mTimeoutStartTimeMs) >= mTimeoutMs))
    {
        event             = OutputEvent(OutputEventType::kTransferTimeout);
        mState            = TransferState::kErrorState;
        mAwaitingResponse = false;
        return;
    }

    switch (mPendingOutput)
    {
    case OutputEventType::kNone:
        event = OutputEvent(OutputEventType::kNone);
        break;
    case OutputEventType::kInternalError:
        event = OutputEvent::StatusReportEvent(OutputEventType::kInternalError, mStatusReportData);
        break;
    case OutputEventType::kStatusReceived:
        event = OutputEvent::StatusReportEvent(OutputEventType::kStatusReceived, mStatusReportData);
        break;
    case OutputEventType::kMsgToSend:
        event               = OutputEvent(OutputEventType::kMsgToSend);
        event.MsgData       = std::move(mPendingMsgHandle);
        mTimeoutStartTimeMs = curTimeMs;
        break;
    case OutputEventType::kInitReceived:
        event = OutputEvent::TransferInitEvent(mTransferRequestData, std::move(mPendingMsgHandle));
        break;
    case OutputEventType::kAcceptReceived:
        event = OutputEvent::TransferAcceptEvent(mTransferAcceptData, std::move(mPendingMsgHandle));
        break;
    case OutputEventType::kQueryReceived:
        event = OutputEvent(OutputEventType::kQueryReceived);
        break;
    case OutputEventType::kBlockReceived:
        event = OutputEvent::BlockDataEvent(mBlockEventData, std::move(mPendingMsgHandle));
        break;
    case OutputEventType::kAckReceived:
        event = OutputEvent(OutputEventType::kAckReceived);
        break;
    case OutputEventType::kAckEOFReceived:
        event = OutputEvent(OutputEventType::kAckEOFReceived);
        break;
    default:
        event = OutputEvent(OutputEventType::kNone);
        break;
    }

    // If there's no other pending output but an error occured or was received, then continue to output the error.
    // This ensures that when the TransferSession encounters an error and needs to send a StatusReport, both a kMsgToSend and a
    // kInternalError output event will be emitted.
    if (event.EventType == OutputEventType::kNone && mState == TransferState::kErrorState)
    {
        event = OutputEvent::StatusReportEvent(OutputEventType::kInternalError, mStatusReportData);
    }

    mPendingOutput = OutputEventType::kNone;
}

CHIP_ERROR TransferSession::StartTransfer(TransferRole role, const TransferInitData & initData, uint32_t timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageType msgType;
    TransferInit initMsg;

    VerifyOrExit(mState == TransferState::kUnitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mRole      = role;
    mTimeoutMs = timeoutMs;

    // Set transfer parameters. They may be overridden later by an Accept message
    mSuppportedXferOpts    = initData.TransferCtlFlags;
    mMaxSupportedBlockSize = initData.MaxBlockSize;
    mStartOffset           = initData.StartOffset;
    mTransferLength        = initData.Length;

    // Prepare TransferInit message
    initMsg.TransferCtlOptions = initData.TransferCtlFlags;
    initMsg.Version            = kBdxVersion;
    initMsg.MaxBlockSize       = mMaxSupportedBlockSize;
    initMsg.StartOffset        = mStartOffset;
    initMsg.MaxLength          = mTransferLength;
    initMsg.FileDesignator     = initData.FileDesignator;
    initMsg.FileDesLength      = initData.FileDesLength;
    initMsg.Metadata           = initData.Metadata;
    initMsg.MetadataLength     = initData.MetadataLength;

    err = WriteToPacketBuffer(initMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    msgType = (mRole == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;
    err     = AttachHeader(msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    mState            = TransferState::kAwaitingAccept;
    mAwaitingResponse = true;

    mPendingOutput = OutputEventType::kMsgToSend;

exit:
    return err;
}

CHIP_ERROR TransferSession::WaitForTransfer(TransferRole role, BitFlags<TransferControlFlags> xferControlOpts,
                                            uint16_t maxBlockSize, uint32_t timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == TransferState::kUnitialized, err = CHIP_ERROR_INCORRECT_STATE);

    // Used to determine compatibility with any future TransferInit parameters
    mRole                  = role;
    mTimeoutMs             = timeoutMs;
    mSuppportedXferOpts    = xferControlOpts;
    mMaxSupportedBlockSize = maxBlockSize;

    mState = TransferState::kAwaitingInitMsg;

exit:
    return err;
}

CHIP_ERROR TransferSession::AcceptTransfer(const TransferAcceptData & acceptData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle outMsgBuf;
    const BitFlags<TransferControlFlags> proposedControlOpts(mTransferRequestData.TransferCtlFlags);

    VerifyOrExit(mState == TransferState::kNegotiateTransferParams, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == OutputEventType::kNone, err = CHIP_ERROR_INCORRECT_STATE);

    // Don't allow a Control method that wasn't supported by the initiator
    // MaxBlockSize can't be larger than the proposed value
    VerifyOrExit(proposedControlOpts.Has(acceptData.ControlMode), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(acceptData.MaxBlockSize <= mTransferRequestData.MaxBlockSize, err = CHIP_ERROR_INVALID_ARGUMENT);

    mTransferMaxBlockSize = acceptData.MaxBlockSize;

    if (mRole == TransferRole::kSender)
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

        err = AttachHeader(MessageType::ReceiveAccept, mPendingMsgHandle);
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

        err = AttachHeader(MessageType::SendAccept, mPendingMsgHandle);
        SuccessOrExit(err);
    }

    mPendingOutput = OutputEventType::kMsgToSend;

    mState = TransferState::kTransferInProgress;

    if ((mRole == TransferRole::kReceiver && mControlMode == TransferControlFlags::kSenderDrive) ||
        (mRole == TransferRole::kSender && mControlMode == TransferControlFlags::kReceiverDrive))
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

    VerifyOrExit(mState == TransferState::kTransferInProgress, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRole == TransferRole::kReceiver, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == OutputEventType::kNone, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    queryMsg.BlockCounter = mNextQueryNum;

    err = WriteToPacketBuffer(queryMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    err = AttachHeader(MessageType::BlockQuery, mPendingMsgHandle);
    SuccessOrExit(err);

    mPendingOutput = OutputEventType::kMsgToSend;

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

    VerifyOrExit(mState == TransferState::kTransferInProgress, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRole == TransferRole::kSender, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == OutputEventType::kNone, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    // Verify non-zero data is provided and is no longer than MaxBlockSize (BlockEOF may contain 0 length data)
    VerifyOrExit((inData.Data != nullptr) && (inData.Length <= mTransferMaxBlockSize), err = CHIP_ERROR_INVALID_ARGUMENT);

    blockMsg.BlockCounter = mNextBlockNum;
    blockMsg.Data         = inData.Data;
    blockMsg.DataLength   = inData.Length;

    err = WriteToPacketBuffer(blockMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    msgType = inData.IsEof ? MessageType::BlockEOF : MessageType::Block;
    err     = AttachHeader(msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    mPendingOutput = OutputEventType::kMsgToSend;

    if (msgType == MessageType::BlockEOF)
    {
        mState = TransferState::kAwaitingEOFAck;
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

    VerifyOrExit(mRole == TransferRole::kReceiver, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit((mState == TransferState::kTransferInProgress) || (mState == TransferState::kReceivedEOF),
                 err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mPendingOutput == OutputEventType::kNone, err = CHIP_ERROR_INCORRECT_STATE);

    ackMsg.BlockCounter = mLastBlockNum;
    msgType             = (mState == TransferState::kReceivedEOF) ? MessageType::BlockAckEOF : MessageType::BlockAck;

    err = WriteToPacketBuffer(ackMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    err = AttachHeader(msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    if (mState == TransferState::kTransferInProgress)
    {
        if (mControlMode == TransferControlFlags::kSenderDrive)
        {
            // In Sender Drive, a BlockAck is implied to also be a query for the next Block, so expect to receive a Block
            // message.
            mLastQueryNum     = ackMsg.BlockCounter + 1;
            mAwaitingResponse = true;
        }
    }
    else if (mState == TransferState::kReceivedEOF)
    {
        mState            = TransferState::kTransferDone;
        mAwaitingResponse = false;
    }

    mPendingOutput = OutputEventType::kMsgToSend;

exit:
    return err;
}

CHIP_ERROR TransferSession::AbortTransfer(StatusCode reason)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit((mState != TransferState::kUnitialized) && (mState != TransferState::kTransferDone) &&
                     (mState != TransferState::kErrorState),
                 err = CHIP_ERROR_INCORRECT_STATE);

    PrepareStatusReport(reason);

exit:
    return err;
}

void TransferSession::Reset()
{
    mPendingOutput = OutputEventType::kNone;
    mState         = TransferState::kUnitialized;
    mSuppportedXferOpts.ClearAll();
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    PayloadHeader payloadHeader;

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.DecodeAndConsume(msg);
    SuccessOrExit(err);

    if (payloadHeader.HasProtocol(Protocols::BDX::Id))
    {
        err = HandleBdxMessage(payloadHeader, std::move(msg));
        SuccessOrExit(err);

        mTimeoutStartTimeMs = curTimeMs;
    }
    else if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
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
    VerifyOrExit(mPendingOutput == OutputEventType::kNone, err = CHIP_ERROR_INCORRECT_STATE);

    switch (msgType)
    {
    case MessageType::SendInit:
    case MessageType::ReceiveInit:
        HandleTransferInit(msgType, std::move(msg));
        break;
    case MessageType::SendAccept:
        HandleSendAccept(std::move(msg));
        break;
    case MessageType::ReceiveAccept:
        HandleReceiveAccept(std::move(msg));
        break;
    case MessageType::BlockQuery:
        HandleBlockQuery(std::move(msg));
        break;
    case MessageType::Block:
        HandleBlock(std::move(msg));
        break;
    case MessageType::BlockEOF:
        HandleBlockEOF(std::move(msg));
        break;
    case MessageType::BlockAck:
        HandleBlockAck(std::move(msg));
        break;
    case MessageType::BlockAckEOF:
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

    mState            = TransferState::kErrorState;
    mAwaitingResponse = false;

    Protocols::SecureChannel::StatusReport report;
    ReturnErrorOnFailure(report.Parse(std::move(msg)));
    VerifyOrReturnError((report.GetProtocolId() == Protocols::BDX::Id.ToFullyQualifiedSpecForm()), CHIP_ERROR_INVALID_MESSAGE_TYPE);

    mStatusReportData.statusCode = static_cast<StatusCode>(report.GetProtocolCode());

    mPendingOutput = OutputEventType::kStatusReceived;

    return CHIP_NO_ERROR;
}

void TransferSession::HandleTransferInit(MessageType msgType, System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferInit transferInit;

    VerifyOrExit(mState == TransferState::kAwaitingInitMsg, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    if (mRole == TransferRole::kSender)
    {
        VerifyOrExit(msgType == MessageType::ReceiveInit, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    }
    else
    {
        VerifyOrExit(msgType == MessageType::SendInit, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    }

    err = transferInit.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    ResolveTransferControlOptions(transferInit.TransferCtlOptions);
    mTransferVersion      = ::chip::min(kBdxVersion, transferInit.Version);
    mTransferMaxBlockSize = ::chip::min(mMaxSupportedBlockSize, transferInit.MaxBlockSize);

    // Accept for now, they may be changed or rejected by the peer if this is a ReceiveInit
    mStartOffset    = transferInit.StartOffset;
    mTransferLength = transferInit.MaxLength;

    // Store the Request data to share with the caller for verification
    mTransferRequestData.TransferCtlFlags = transferInit.TransferCtlOptions;
    mTransferRequestData.MaxBlockSize     = transferInit.MaxBlockSize;
    mTransferRequestData.StartOffset      = transferInit.StartOffset;
    mTransferRequestData.Length           = transferInit.MaxLength;
    mTransferRequestData.FileDesignator   = transferInit.FileDesignator;
    mTransferRequestData.FileDesLength    = transferInit.FileDesLength;
    mTransferRequestData.Metadata         = transferInit.Metadata;
    mTransferRequestData.MetadataLength   = transferInit.MetadataLength;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = OutputEventType::kInitReceived;

    mState = TransferState::kNegotiateTransferParams;

exit:
    return;
}

void TransferSession::HandleReceiveAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveAccept rcvAcceptMsg;

    VerifyOrExit(mRole == TransferRole::kReceiver, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kAwaitingAccept, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = rcvAcceptMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

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
    mPendingOutput    = OutputEventType::kAcceptReceived;

    mAwaitingResponse = (mControlMode == TransferControlFlags::kSenderDrive);
    mState            = TransferState::kTransferInProgress;

exit:
    return;
}

void TransferSession::HandleSendAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SendAccept sendAcceptMsg;

    VerifyOrExit(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kAwaitingAccept, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = sendAcceptMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

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
    mPendingOutput    = OutputEventType::kAcceptReceived;

    mAwaitingResponse = (mControlMode == TransferControlFlags::kReceiverDrive);
    mState            = TransferState::kTransferInProgress;

exit:
    return;
}

void TransferSession::HandleBlockQuery(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockQuery query;

    VerifyOrExit(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = query.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrExit(query.BlockCounter == mNextBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kQueryReceived;

    mAwaitingResponse = false;
    mLastQueryNum     = query.BlockCounter;

exit:
    return;
}

void TransferSession::HandleBlock(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Block blockMsg;

    VerifyOrExit(mRole == TransferRole::kReceiver, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = blockMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrExit(blockMsg.BlockCounter == mLastQueryNum, PrepareStatusReport(StatusCode::kBadBlockCounter));
    VerifyOrExit((blockMsg.DataLength > 0) && (blockMsg.DataLength <= mTransferMaxBlockSize),
                 PrepareStatusReport(StatusCode::kBadMessageContents));

    if (IsTransferLengthDefinite())
    {
        VerifyOrExit(mNumBytesProcessed + blockMsg.DataLength <= mTransferLength, PrepareStatusReport(StatusCode::kLengthMismatch));
    }

    mBlockEventData.Data   = blockMsg.Data;
    mBlockEventData.Length = blockMsg.DataLength;
    mBlockEventData.IsEof  = false;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = OutputEventType::kBlockReceived;

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

    VerifyOrExit(mRole == TransferRole::kReceiver, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = blockEOFMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrExit(blockEOFMsg.BlockCounter == mLastQueryNum, PrepareStatusReport(StatusCode::kBadBlockCounter));
    VerifyOrExit(blockEOFMsg.DataLength <= mTransferMaxBlockSize, PrepareStatusReport(StatusCode::kBadMessageContents));

    mBlockEventData.Data   = blockEOFMsg.Data;
    mBlockEventData.Length = blockEOFMsg.DataLength;
    mBlockEventData.IsEof  = true;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = OutputEventType::kBlockReceived;

    mNumBytesProcessed += blockEOFMsg.DataLength;
    mLastBlockNum = blockEOFMsg.BlockCounter;

    mAwaitingResponse = false;
    mState            = TransferState::kReceivedEOF;

exit:
    return;
}

void TransferSession::HandleBlockAck(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockAck ackMsg;

    VerifyOrExit(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = ackMsg.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));
    VerifyOrExit(ackMsg.BlockCounter == mLastBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kAckReceived;

    // In Receiver Drive, the Receiver can send a BlockAck to indicate receipt of the message and reset the timeout.
    // In this case, the Sender should wait to receive a BlockQuery next.
    mAwaitingResponse = (mControlMode == TransferControlFlags::kReceiverDrive);

exit:
    return;
}

void TransferSession::HandleBlockAckEOF(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockAckEOF ackMsg;

    VerifyOrExit(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mState == TransferState::kAwaitingEOFAck, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrExit(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    err = ackMsg.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));
    VerifyOrExit(ackMsg.BlockCounter == mLastBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kAckEOFReceived;

    mAwaitingResponse = false;

    mState = TransferState::kTransferDone;

exit:
    return;
}

void TransferSession::ResolveTransferControlOptions(const BitFlags<TransferControlFlags> & proposed)
{
    // Must specify at least one synchronous option
    //
    if (!proposed.HasAny(TransferControlFlags::kSenderDrive, TransferControlFlags::kReceiverDrive))
    {
        PrepareStatusReport(StatusCode::kTransferMethodNotSupported);
        return;
    }

    // Ensure there are options supported by both nodes. Async gets priority.
    // If there is only one common option, choose that one. Otherwise the application must pick.
    const BitFlags<TransferControlFlags> commonOpts(proposed & mSuppportedXferOpts);
    if (!commonOpts.HasAny())
    {
        PrepareStatusReport(StatusCode::kTransferMethodNotSupported);
    }
    else if (commonOpts.HasOnly(TransferControlFlags::kAsync))
    {
        mControlMode = TransferControlFlags::kAsync;
    }
    else if (commonOpts.HasOnly(TransferControlFlags::kReceiverDrive))
    {
        mControlMode = TransferControlFlags::kReceiverDrive;
    }
    else if (commonOpts.HasOnly(TransferControlFlags::kSenderDrive))
    {
        mControlMode = TransferControlFlags::kSenderDrive;
    }
}

CHIP_ERROR TransferSession::VerifyProposedMode(const BitFlags<TransferControlFlags> & proposed)
{
    TransferControlFlags mode;

    // Must specify only one mode in Accept messages
    if (proposed.HasOnly(TransferControlFlags::kAsync))
    {
        mode = TransferControlFlags::kAsync;
    }
    else if (proposed.HasOnly(TransferControlFlags::kReceiverDrive))
    {
        mode = TransferControlFlags::kReceiverDrive;
    }
    else if (proposed.HasOnly(TransferControlFlags::kSenderDrive))
    {
        mode = TransferControlFlags::kSenderDrive;
    }
    else
    {
        PrepareStatusReport(StatusCode::kBadMessageContents);
        return CHIP_ERROR_INTERNAL;
    }

    // Verify the proposed mode is supported by this instance
    if (mSuppportedXferOpts.Has(mode))
    {
        mControlMode = mode;
    }
    else
    {
        PrepareStatusReport(StatusCode::kTransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void TransferSession::PrepareStatusReport(StatusCode code)
{
    static_assert(std::is_same<std::underlying_type_t<decltype(code)>, uint16_t>::value, "Cast is not safe");

    mStatusReportData.statusCode = code;

    Protocols::SecureChannel::StatusReport report(Protocols::SecureChannel::GeneralStatusCode::kFailure,
                                                  Protocols::BDX::Id.ToFullyQualifiedSpecForm(), static_cast<uint16_t>(code));
    size_t msgSize = report.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(chip::MessagePacketBuffer::New(msgSize), msgSize);
    VerifyOrExit(!bbuf.IsNull(), mPendingOutput = OutputEventType::kInternalError);

    report.WriteToBuffer(bbuf);
    mPendingMsgHandle = bbuf.Finalize();
    if (mPendingMsgHandle.IsNull())
    {
        mPendingOutput = OutputEventType::kInternalError;
    }
    else
    {
        CHIP_ERROR err = AttachHeader(Protocols::SecureChannel::MsgType::StatusReport, mPendingMsgHandle);
        VerifyOrExit(err == CHIP_NO_ERROR, mPendingOutput = OutputEventType::kInternalError);
        mPendingOutput = OutputEventType::kMsgToSend;
    }

exit:
    mState            = TransferState::kErrorState;
    mAwaitingResponse = false; // Prevent triggering timeout
}

bool TransferSession::IsTransferLengthDefinite()
{
    return (mTransferLength > 0);
}

TransferSession::OutputEvent TransferSession::OutputEvent::TransferInitEvent(TransferInitData data, System::PacketBufferHandle msg)
{
    OutputEvent event(OutputEventType::kInitReceived);
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
    OutputEvent event(OutputEventType::kAcceptReceived);
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
    OutputEvent event(OutputEventType::kBlockReceived);
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
