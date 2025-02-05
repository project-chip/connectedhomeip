/**
 *    @file
 *      Implementation for the TransferSession class.
 *      // TODO: Support Asynchronous mode. Currently, only Synchronous mode is supported.
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>

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

template <typename MessageType>
void PrepareOutgoingMessageEvent(MessageType messageType, chip::bdx::TransferSession::OutputEventType & pendingOutput,
                                 chip::bdx::TransferSession::MessageTypeData & outputMsgType)
{
    static_assert(std::is_same<std::underlying_type_t<decltype(messageType)>, uint8_t>::value, "Cast is not safe");

    pendingOutput             = chip::bdx::TransferSession::OutputEventType::kMsgToSend;
    outputMsgType.ProtocolId  = chip::Protocols::MessageTypeTraits<MessageType>::ProtocolId();
    outputMsgType.MessageType = static_cast<uint8_t>(messageType);
}

} // anonymous namespace

namespace chip {
namespace bdx {

TransferSession::TransferSession()
{
    mSuppportedXferOpts.ClearAll();
}

void TransferSession::PollOutput(OutputEvent & event, System::Clock::Timestamp curTime)
{
    event = OutputEvent(OutputEventType::kNone);

    if (mShouldInitTimeoutStart)
    {
        mTimeoutStartTime       = curTime;
        mShouldInitTimeoutStart = false;
    }

    if (mAwaitingResponse && ((curTime - mTimeoutStartTime) >= mTimeout))
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
        event             = OutputEvent::MsgToSendEvent(mMsgTypeData, std::move(mPendingMsgHandle));
        mTimeoutStartTime = curTime;
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
    case OutputEventType::kQueryWithSkipReceived:
        event = OutputEvent::QueryWithSkipEvent(mBytesToSkip);
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

    // If there's no other pending output but an error occurred or was received, then continue to output the error.
    // This ensures that when the TransferSession encounters an error and needs to send a StatusReport, both a kMsgToSend and a
    // kInternalError output event will be emitted.
    if (event.EventType == OutputEventType::kNone && mState == TransferState::kErrorState)
    {
        event = OutputEvent::StatusReportEvent(OutputEventType::kInternalError, mStatusReportData);
    }

    mPendingOutput = OutputEventType::kNone;
}

void TransferSession::GetNextAction(OutputEvent & event)
{
    PollOutput(event, System::SystemClock().GetMonotonicTimestamp());
}

CHIP_ERROR TransferSession::StartTransfer(TransferRole role, const TransferInitData & initData, System::Clock::Timeout timeout)
{
    VerifyOrReturnError(mState == TransferState::kUnitialized, CHIP_ERROR_INCORRECT_STATE);

    mRole    = role;
    mTimeout = timeout;

    // Set transfer parameters. They may be overridden later by an Accept message
    mSuppportedXferOpts    = initData.TransferCtlFlags;
    mMaxSupportedBlockSize = initData.MaxBlockSize;
    mStartOffset           = initData.StartOffset;
    mTransferLength        = initData.Length;

    // Prepare TransferInit message
    TransferInit initMsg;
    initMsg.TransferCtlOptions = initData.TransferCtlFlags;
    initMsg.Version            = kBdxVersion;
    initMsg.MaxBlockSize       = mMaxSupportedBlockSize;
    initMsg.StartOffset        = mStartOffset;
    initMsg.MaxLength          = mTransferLength;
    initMsg.FileDesignator     = initData.FileDesignator;
    initMsg.FileDesLength      = initData.FileDesLength;
    initMsg.Metadata           = initData.Metadata;
    initMsg.MetadataLength     = initData.MetadataLength;

    ReturnErrorOnFailure(WriteToPacketBuffer(initMsg, mPendingMsgHandle));

    const MessageType msgType = (mRole == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Sending BDX Message");
    initMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

    mState            = TransferState::kAwaitingAccept;
    mAwaitingResponse = true;

    PrepareOutgoingMessageEvent(msgType, mPendingOutput, mMsgTypeData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::WaitForTransfer(TransferRole role, BitFlags<TransferControlFlags> xferControlOpts,
                                            uint16_t maxBlockSize, System::Clock::Timeout timeout)
{
    VerifyOrReturnError(mState == TransferState::kUnitialized, CHIP_ERROR_INCORRECT_STATE);

    // Used to determine compatibility with any future TransferInit parameters
    mRole                  = role;
    mTimeout               = timeout;
    mSuppportedXferOpts    = xferControlOpts;
    mMaxSupportedBlockSize = maxBlockSize;

    mState = TransferState::kAwaitingInitMsg;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::AcceptTransfer(const TransferAcceptData & acceptData)
{
    MessageType msgType;

    const BitFlags<TransferControlFlags> proposedControlOpts(mTransferRequestData.TransferCtlFlags);

    VerifyOrReturnError(mState == TransferState::kNegotiateTransferParams, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);

    // Don't allow a Control method that wasn't supported by the initiator
    // MaxBlockSize can't be larger than the proposed value
    VerifyOrReturnError(proposedControlOpts.Has(acceptData.ControlMode), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(acceptData.MaxBlockSize <= mTransferRequestData.MaxBlockSize, CHIP_ERROR_INVALID_ARGUMENT);

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

        ReturnErrorOnFailure(WriteToPacketBuffer(acceptMsg, mPendingMsgHandle));
        msgType = MessageType::ReceiveAccept;

#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Sending BDX Message");
        acceptMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING
    }
    else
    {
        SendAccept acceptMsg;
        acceptMsg.TransferCtlFlags.Set(acceptData.ControlMode);
        acceptMsg.Version        = mTransferVersion;
        acceptMsg.MaxBlockSize   = acceptData.MaxBlockSize;
        acceptMsg.Metadata       = acceptData.Metadata;
        acceptMsg.MetadataLength = acceptData.MetadataLength;

        ReturnErrorOnFailure(WriteToPacketBuffer(acceptMsg, mPendingMsgHandle));
        msgType = MessageType::SendAccept;

#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Sending BDX Message");
        acceptMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING
    }

    mState = TransferState::kTransferInProgress;

    if ((mRole == TransferRole::kReceiver && mControlMode == TransferControlFlags::kSenderDrive) ||
        (mRole == TransferRole::kSender && mControlMode == TransferControlFlags::kReceiverDrive))
    {
        mAwaitingResponse = true;
    }

    PrepareOutgoingMessageEvent(msgType, mPendingOutput, mMsgTypeData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::RejectTransfer(StatusCode reason)
{
    VerifyOrReturnError(mState == TransferState::kNegotiateTransferParams, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);

    PrepareStatusReport(reason);
    mState = TransferState::kTransferDone;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlockQuery()
{
    const MessageType msgType = MessageType::BlockQuery;

    VerifyOrReturnError(mState == TransferState::kTransferInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRole == TransferRole::kReceiver, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mAwaitingResponse, CHIP_ERROR_INCORRECT_STATE);

    BlockQuery queryMsg;
    queryMsg.BlockCounter = mNextQueryNum;

    ReturnErrorOnFailure(WriteToPacketBuffer(queryMsg, mPendingMsgHandle));

    mAwaitingResponse = true;
    mLastQueryNum     = mNextQueryNum++;

    PrepareOutgoingMessageEvent(msgType, mPendingOutput, mMsgTypeData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlockQueryWithSkip(const uint64_t & bytesToSkip)
{
    const MessageType msgType = MessageType::BlockQueryWithSkip;

    VerifyOrReturnError(mState == TransferState::kTransferInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRole == TransferRole::kReceiver, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mAwaitingResponse, CHIP_ERROR_INCORRECT_STATE);

    BlockQueryWithSkip queryMsg;
    queryMsg.BlockCounter = mNextQueryNum;
    queryMsg.BytesToSkip  = bytesToSkip;

    ReturnErrorOnFailure(WriteToPacketBuffer(queryMsg, mPendingMsgHandle));

    mAwaitingResponse = true;
    mLastQueryNum     = mNextQueryNum++;

    PrepareOutgoingMessageEvent(msgType, mPendingOutput, mMsgTypeData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlock(const BlockData & inData)
{
    VerifyOrReturnError(mState == TransferState::kTransferInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRole == TransferRole::kSender, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mAwaitingResponse, CHIP_ERROR_INCORRECT_STATE);

    // Verify non-zero data is provided and is no longer than MaxBlockSize (BlockEOF may contain 0 length data)
    VerifyOrReturnError((inData.Data != nullptr) && (inData.Length <= mTransferMaxBlockSize), CHIP_ERROR_INVALID_ARGUMENT);

    DataBlock blockMsg;
    blockMsg.BlockCounter = mNextBlockNum;
    blockMsg.Data         = inData.Data;
    blockMsg.DataLength   = inData.Length;

    ReturnErrorOnFailure(WriteToPacketBuffer(blockMsg, mPendingMsgHandle));

    const MessageType msgType = inData.IsEof ? MessageType::BlockEOF : MessageType::Block;

    if (msgType == MessageType::BlockEOF)
    {
#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Sending BDX Message");
        blockMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING
        mState = TransferState::kAwaitingEOFAck;
    }

    mAwaitingResponse = true;
    mLastBlockNum     = mNextBlockNum++;

    PrepareOutgoingMessageEvent(msgType, mPendingOutput, mMsgTypeData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlockAck()
{
    VerifyOrReturnError(mRole == TransferRole::kReceiver, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError((mState == TransferState::kTransferInProgress) || (mState == TransferState::kReceivedEOF),
                        CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);

    CounterMessage ackMsg;
    ackMsg.BlockCounter       = mLastBlockNum;
    const MessageType msgType = (mState == TransferState::kReceivedEOF) ? MessageType::BlockAckEOF : MessageType::BlockAck;

    ReturnErrorOnFailure(WriteToPacketBuffer(ackMsg, mPendingMsgHandle));

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
#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Sending BDX Message");
        ackMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING
        mState            = TransferState::kTransferDone;
        mAwaitingResponse = false;
    }

    PrepareOutgoingMessageEvent(msgType, mPendingOutput, mMsgTypeData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::AbortTransfer(StatusCode reason)
{
    VerifyOrReturnError((mState != TransferState::kUnitialized) && (mState != TransferState::kTransferDone) &&
                            (mState != TransferState::kErrorState),
                        CHIP_ERROR_INCORRECT_STATE);

    PrepareStatusReport(reason);

    return CHIP_NO_ERROR;
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

    mTimeout                = System::Clock::kZero;
    mTimeoutStartTime       = System::Clock::kZero;
    mShouldInitTimeoutStart = true;
    mAwaitingResponse       = false;
}

CHIP_ERROR TransferSession::HandleMessageReceived(const PayloadHeader & payloadHeader, System::PacketBufferHandle msg,
                                                  System::Clock::Timestamp curTime)
{
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    if (payloadHeader.HasProtocol(Protocols::BDX::Id))
    {
        ReturnErrorOnFailure(HandleBdxMessage(payloadHeader, std::move(msg)));

        mTimeoutStartTime = curTime;
    }
    else if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        ReturnErrorOnFailure(HandleStatusReportMessage(payloadHeader, std::move(msg)));
    }
    else
    {
        return CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

    return CHIP_NO_ERROR;
}

// Return CHIP_ERROR only if there was a problem decoding the message. Otherwise, call PrepareStatusReport().
CHIP_ERROR TransferSession::HandleBdxMessage(const PayloadHeader & header, System::PacketBufferHandle msg)
{
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mPendingOutput == OutputEventType::kNone, CHIP_ERROR_INCORRECT_STATE);

    const MessageType msgType = static_cast<MessageType>(header.GetMessageType());

    switch (msgType)
    {
    case MessageType::SendInit:
    case MessageType::ReceiveInit:
#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Handling received BDX Message");
#endif // CHIP_AUTOMATION_LOGGING
        HandleTransferInit(msgType, std::move(msg));
        break;
    case MessageType::SendAccept:
#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Handling received BDX Message");
#endif // CHIP_AUTOMATION_LOGGING
        HandleSendAccept(std::move(msg));
        break;
    case MessageType::ReceiveAccept:
#if CHIP_AUTOMATION_LOGGING
        ChipLogAutomation("Handling received BDX Message");
#endif // CHIP_AUTOMATION_LOGGING
        HandleReceiveAccept(std::move(msg));
        break;
    case MessageType::BlockQuery:
        HandleBlockQuery(std::move(msg));
        break;
    case MessageType::BlockQueryWithSkip:
        HandleBlockQueryWithSkip(std::move(msg));
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
        return CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

    return CHIP_NO_ERROR;
}

/**
 * @brief
 *   Parse a StatusReport message and prepare to emit an OutputEvent with the message data.
 *
 *   NOTE: BDX does not currently expect to ever use a "Success" general code, so it will be treated as an error along with any
 *         other code.
 */
CHIP_ERROR TransferSession::HandleStatusReportMessage(const PayloadHeader & header, System::PacketBufferHandle msg)
{
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    mState            = TransferState::kErrorState;
    mAwaitingResponse = false;

    Protocols::SecureChannel::StatusReport report;
    ReturnErrorOnFailure(report.Parse(std::move(msg)));
    VerifyOrReturnError((report.GetProtocolId() == Protocols::BDX::Id), CHIP_ERROR_INVALID_MESSAGE_TYPE);

    mStatusReportData.statusCode = static_cast<StatusCode>(report.GetProtocolCode());

    mPendingOutput = OutputEventType::kStatusReceived;

    return CHIP_NO_ERROR;
}

void TransferSession::HandleTransferInit(MessageType msgType, System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mState == TransferState::kAwaitingInitMsg, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    if (mRole == TransferRole::kSender)
    {
        VerifyOrReturn(msgType == MessageType::ReceiveInit, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    }
    else
    {
        VerifyOrReturn(msgType == MessageType::SendInit, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    }

    TransferInit transferInit;
    const CHIP_ERROR err = transferInit.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    ResolveTransferControlOptions(transferInit.TransferCtlOptions);
    mTransferVersion      = std::min(kBdxVersion, transferInit.Version);
    mTransferMaxBlockSize = std::min(mMaxSupportedBlockSize, transferInit.MaxBlockSize);

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

#if CHIP_AUTOMATION_LOGGING
    transferInit.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING
}

void TransferSession::HandleReceiveAccept(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kReceiver, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kAwaitingAccept, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    ReceiveAccept rcvAcceptMsg;
    const CHIP_ERROR err = rcvAcceptMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    // Verify that Accept parameters are compatible with the original proposed parameters
    ReturnOnFailure(VerifyProposedMode(rcvAcceptMsg.TransferCtlFlags));

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

#if CHIP_AUTOMATION_LOGGING
    rcvAcceptMsg.LogMessage(MessageType::ReceiveAccept);
#endif // CHIP_AUTOMATION_LOGGING
}

void TransferSession::HandleSendAccept(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kAwaitingAccept, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    SendAccept sendAcceptMsg;
    const CHIP_ERROR err = sendAcceptMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    // Verify that Accept parameters are compatible with the original proposed parameters
    ReturnOnFailure(VerifyProposedMode(sendAcceptMsg.TransferCtlFlags));

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

#if CHIP_AUTOMATION_LOGGING
    sendAcceptMsg.LogMessage(MessageType::SendAccept);
#endif // CHIP_AUTOMATION_LOGGING
}

void TransferSession::HandleBlockQuery(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    BlockQuery query;
    const CHIP_ERROR err = query.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(query.BlockCounter == mNextBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kQueryReceived;

    mAwaitingResponse = false;
    mLastQueryNum     = query.BlockCounter;
}

void TransferSession::HandleBlockQueryWithSkip(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    BlockQueryWithSkip query;
    const CHIP_ERROR err = query.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(query.BlockCounter == mNextBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kQueryWithSkipReceived;

    mAwaitingResponse        = false;
    mLastQueryNum            = query.BlockCounter;
    mBytesToSkip.BytesToSkip = query.BytesToSkip;
}

void TransferSession::HandleBlock(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kReceiver, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    Block blockMsg;
    const CHIP_ERROR err = blockMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(blockMsg.BlockCounter == mLastQueryNum, PrepareStatusReport(StatusCode::kBadBlockCounter));
    VerifyOrReturn((blockMsg.DataLength > 0) && (blockMsg.DataLength <= mTransferMaxBlockSize),
                   PrepareStatusReport(StatusCode::kBadMessageContents));

    if (IsTransferLengthDefinite())
    {
        VerifyOrReturn(mNumBytesProcessed + blockMsg.DataLength <= mTransferLength,
                       PrepareStatusReport(StatusCode::kLengthMismatch));
    }

    mBlockEventData.Data         = blockMsg.Data;
    mBlockEventData.Length       = blockMsg.DataLength;
    mBlockEventData.IsEof        = false;
    mBlockEventData.BlockCounter = blockMsg.BlockCounter;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = OutputEventType::kBlockReceived;

    mNumBytesProcessed += blockMsg.DataLength;
    mLastBlockNum = blockMsg.BlockCounter;

    mAwaitingResponse = false;
}

void TransferSession::HandleBlockEOF(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kReceiver, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    BlockEOF blockEOFMsg;
    const CHIP_ERROR err = blockEOFMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(blockEOFMsg.BlockCounter == mLastQueryNum, PrepareStatusReport(StatusCode::kBadBlockCounter));
    VerifyOrReturn(blockEOFMsg.DataLength <= mTransferMaxBlockSize, PrepareStatusReport(StatusCode::kBadMessageContents));

    mBlockEventData.Data         = blockEOFMsg.Data;
    mBlockEventData.Length       = blockEOFMsg.DataLength;
    mBlockEventData.IsEof        = true;
    mBlockEventData.BlockCounter = blockEOFMsg.BlockCounter;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput    = OutputEventType::kBlockReceived;

    mNumBytesProcessed += blockEOFMsg.DataLength;
    mLastBlockNum = blockEOFMsg.BlockCounter;

    mAwaitingResponse = false;
    mState            = TransferState::kReceivedEOF;

#if CHIP_AUTOMATION_LOGGING
    blockEOFMsg.LogMessage(MessageType::BlockEOF);
#endif // CHIP_AUTOMATION_LOGGING
}

void TransferSession::HandleBlockAck(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    BlockAck ackMsg;
    const CHIP_ERROR err = ackMsg.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));
    VerifyOrReturn(ackMsg.BlockCounter == mLastBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kAckReceived;

    // In Receiver Drive, the Receiver can send a BlockAck to indicate receipt of the message and reset the timeout.
    // In this case, the Sender should wait to receive a BlockQuery next.
    mAwaitingResponse = (mControlMode == TransferControlFlags::kReceiverDrive);
}

void TransferSession::HandleBlockAckEOF(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kAwaitingEOFAck, PrepareStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, PrepareStatusReport(StatusCode::kUnexpectedMessage));

    BlockAckEOF ackMsg;
    const CHIP_ERROR err = ackMsg.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, PrepareStatusReport(StatusCode::kBadMessageContents));
    VerifyOrReturn(ackMsg.BlockCounter == mLastBlockNum, PrepareStatusReport(StatusCode::kBadBlockCounter));

    mPendingOutput = OutputEventType::kAckEOFReceived;

    mAwaitingResponse = false;

    mState = TransferState::kTransferDone;

#if CHIP_AUTOMATION_LOGGING
    ackMsg.LogMessage(MessageType::BlockAckEOF);
#endif // CHIP_AUTOMATION_LOGGING
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
    mStatusReportData.statusCode = code;

    Protocols::SecureChannel::StatusReport report(Protocols::SecureChannel::GeneralStatusCode::kFailure, Protocols::BDX::Id,
                                                  to_underlying(code));
    size_t msgSize = report.Size();
    Encoding::LittleEndian::PacketBufferWriter bbuf(chip::MessagePacketBuffer::New(msgSize), msgSize);
    VerifyOrExit(!bbuf.IsNull(), mPendingOutput = OutputEventType::kInternalError);

    report.WriteToBuffer(bbuf);
    mPendingMsgHandle = bbuf.Finalize();
    if (mPendingMsgHandle.IsNull())
    {
        ChipLogError(BDX, "%s: error preparing message: %" CHIP_ERROR_FORMAT, __FUNCTION__, CHIP_ERROR_NO_MEMORY.Format());
        mPendingOutput = OutputEventType::kInternalError;
    }
    else
    {
        PrepareOutgoingMessageEvent(Protocols::SecureChannel::MsgType::StatusReport, mPendingOutput, mMsgTypeData);
    }

exit:
    mState            = TransferState::kErrorState;
    mAwaitingResponse = false; // Prevent triggering timeout
}

bool TransferSession::IsTransferLengthDefinite() const
{
    return (mTransferLength > 0);
}

const char * TransferSession::OutputEvent::ToString(OutputEventType outputEventType)
{
    return TypeToString(outputEventType);
}

const char * TransferSession::OutputEvent::TypeToString(OutputEventType outputEventType)
{
    switch (outputEventType)
    {
    case OutputEventType::kNone:
        return "None";
    case OutputEventType::kMsgToSend:
        return "MsgToSend";
    case OutputEventType::kInitReceived:
        return "InitReceived";
    case OutputEventType::kAcceptReceived:
        return "AcceptReceived";
    case OutputEventType::kBlockReceived:
        return "BlockReceived";
    case OutputEventType::kQueryReceived:
        return "QueryReceived";
    case OutputEventType::kQueryWithSkipReceived:
        return "QueryWithSkipReceived";
    case OutputEventType::kAckReceived:
        return "AckReceived";
    case OutputEventType::kAckEOFReceived:
        return "AckEOFReceived";
    case OutputEventType::kStatusReceived:
        return "StatusReceived";
    case OutputEventType::kInternalError:
        return "InternalError";
    case OutputEventType::kTransferTimeout:
        return "TransferTimeout";
    default:
        return "Unknown";
    }
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

TransferSession::OutputEvent TransferSession::OutputEvent::MsgToSendEvent(MessageTypeData typeData, System::PacketBufferHandle msg)
{
    OutputEvent event(OutputEventType::kMsgToSend);
    event.MsgData     = std::move(msg);
    event.msgTypeData = typeData;
    return event;
}

TransferSession::OutputEvent TransferSession::OutputEvent::QueryWithSkipEvent(TransferSkipData bytesToSkip)
{
    OutputEvent event(OutputEventType::kQueryWithSkipReceived);
    event.bytesToSkip = bytesToSkip;
    return event;
}

} // namespace bdx
} // namespace chip
