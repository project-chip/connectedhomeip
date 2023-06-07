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
    size_t msgDataSize                      = msgStruct.MessageSize();
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::New(msgDataSize);
    if (buffer.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    ::chip::Encoding::LittleEndian::PacketBufferWriter bbuf(std::move(buffer), msgDataSize);
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

} // anonymous namespace

namespace chip {
namespace bdx {

TransferSession::TransferSession()
{
    mSuppportedXferOpts.ClearAll();
}

void TransferSession::DispatchOutputEvent(TransferSession::OutputEvent & outputEvent)
{
    if (mOutputEventHandler != nullptr)
    {
        ChipLogProgress(BDX, "DispatchOutputEvent %s", outputEvent.ToString(outputEvent.EventType));
        mOutputEventHandler(mContext, outputEvent);
    }
}

template <typename MessageType>
TransferSession::MessageTypeData TransferSession::PrepareOutgoingMessageEvent(MessageType messageType)
{
    static_assert(std::is_same<std::underlying_type_t<decltype(messageType)>, uint8_t>::value, "Cast is not safe");

    TransferSession::MessageTypeData outputMsgType;
    outputMsgType.ProtocolId  = chip::Protocols::MessageTypeTraits<MessageType>::ProtocolId();
    outputMsgType.MessageType = static_cast<uint8_t>(messageType);

    return outputMsgType;
}

void TransferSession::RegisterOutputEventHandler(OutputEventHandler callback, void * context)
{
    mOutputEventHandler = callback;
    mContext            = context;
}

void TransferSession::UnregisterOutputEventHandler()
{
    mOutputEventHandler = nullptr;
    mContext            = nullptr;
}

CHIP_ERROR TransferSession::StartTransfer(TransferRole role, const TransferInitData & initData, OutputEventHandler callback,
                                          void * context)
{
    VerifyOrReturnError(mState == TransferState::kUnitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    RegisterOutputEventHandler(callback, context);

    mRole = role;

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

    System::PacketBufferHandle msg;
    ReturnErrorOnFailure(WriteToPacketBuffer(initMsg, msg));

    const MessageType msgType = (mRole == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Sending BDX Message");
    initMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

    mState            = TransferState::kAwaitingAccept;
    mAwaitingResponse = true;
   
    TransferSession::MessageTypeData outputMsgType = PrepareOutgoingMessageEvent(msgType);
    OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::WaitForTransfer(TransferRole role, BitFlags<TransferControlFlags> xferControlOpts,
                                            uint16_t maxBlockSize, OutputEventHandler callback, void * context)
{
    VerifyOrReturnError(mState == TransferState::kUnitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    RegisterOutputEventHandler(callback, context);

    // Used to determine compatibility with any future TransferInit parameters
    mRole                  = role;
    mSuppportedXferOpts    = xferControlOpts;
    mMaxSupportedBlockSize = maxBlockSize;

    mState = TransferState::kAwaitingInitMsg;

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::AcceptTransfer(const TransferAcceptData & acceptData)
{
    MessageType msgType;

    const BitFlags<TransferControlFlags> proposedControlOpts(mTransferRequestControlFlags);

    VerifyOrReturnError(mState == TransferState::kNegotiateTransferParams, CHIP_ERROR_INCORRECT_STATE);

    // Don't allow a Control method that wasn't supported by the initiator
    // MaxBlockSize can't be larger than the proposed value
    VerifyOrReturnError(proposedControlOpts.Has(acceptData.ControlMode), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(acceptData.MaxBlockSize <= mTransferRequestMaxBlockSize, CHIP_ERROR_INVALID_ARGUMENT);

    mTransferMaxBlockSize = acceptData.MaxBlockSize;

    System::PacketBufferHandle msg;
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

        ReturnErrorOnFailure(WriteToPacketBuffer(acceptMsg, msg));
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

        ReturnErrorOnFailure(WriteToPacketBuffer(acceptMsg, msg));
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

    TransferSession::MessageTypeData outputMsgType = PrepareOutgoingMessageEvent(msgType);
    OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlockQuery()
{
    const MessageType msgType = MessageType::BlockQuery;

    VerifyOrReturnError(mState == TransferState::kTransferInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRole == TransferRole::kReceiver, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mAwaitingResponse, CHIP_ERROR_INCORRECT_STATE);

    BlockQuery queryMsg;
    queryMsg.BlockCounter = mNextQueryNum;

    System::PacketBufferHandle msg;
    ReturnErrorOnFailure(WriteToPacketBuffer(queryMsg, msg));

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Sending BDX Message");
    queryMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

    mAwaitingResponse = true;
    mLastQueryNum     = mNextQueryNum++;

    TransferSession::MessageTypeData outputMsgType = PrepareOutgoingMessageEvent(msgType);
    OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlockQueryWithSkip(const uint64_t & bytesToSkip)
{
    const MessageType msgType = MessageType::BlockQueryWithSkip;

    VerifyOrReturnError(mState == TransferState::kTransferInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRole == TransferRole::kReceiver, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mAwaitingResponse, CHIP_ERROR_INCORRECT_STATE);

    BlockQueryWithSkip queryMsg;
    queryMsg.BlockCounter = mNextQueryNum;
    queryMsg.BytesToSkip  = bytesToSkip;

    System::PacketBufferHandle msg;
    ReturnErrorOnFailure(WriteToPacketBuffer(queryMsg, msg));

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Sending BDX Message");
    queryMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

    mAwaitingResponse = true;
    mLastQueryNum     = mNextQueryNum++;

    TransferSession::MessageTypeData outputMsgType = PrepareOutgoingMessageEvent(msgType);
    OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlock(const BlockData & inData)
{
    VerifyOrReturnError(mState == TransferState::kTransferInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mRole == TransferRole::kSender, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mAwaitingResponse, CHIP_ERROR_INCORRECT_STATE);

    // Verify non-zero data is provided and is no longer than MaxBlockSize (BlockEOF may contain 0 length data)
    VerifyOrReturnError((inData.Data != nullptr) && (inData.Length <= mTransferMaxBlockSize), CHIP_ERROR_INVALID_ARGUMENT);

    DataBlock blockMsg;
    blockMsg.BlockCounter = mNextBlockNum;
    blockMsg.Data         = inData.Data;
    blockMsg.DataLength   = inData.Length;

    System::PacketBufferHandle msg;
    ReturnErrorOnFailure(WriteToPacketBuffer(blockMsg, msg));

    const MessageType msgType = inData.IsEof ? MessageType::BlockEOF : MessageType::Block;

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Sending BDX Message");
    blockMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

    if (msgType == MessageType::BlockEOF)
    {
        mState = TransferState::kAwaitingEOFAck;
    }

    mAwaitingResponse = true;
    mLastBlockNum     = mNextBlockNum++;

    TransferSession::MessageTypeData outputMsgType = PrepareOutgoingMessageEvent(msgType);
    OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::PrepareBlockAck()
{
    VerifyOrReturnError(mRole == TransferRole::kReceiver, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError((mState == TransferState::kTransferInProgress) || (mState == TransferState::kReceivedEOF),
                        CHIP_ERROR_INCORRECT_STATE);

    CounterMessage ackMsg;
    ackMsg.BlockCounter       = mLastBlockNum;
    const MessageType msgType = (mState == TransferState::kReceivedEOF) ? MessageType::BlockAckEOF : MessageType::BlockAck;

    System::PacketBufferHandle msg;    
    ReturnErrorOnFailure(WriteToPacketBuffer(ackMsg, msg));

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Sending BDX Message");
    ackMsg.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

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

    TransferSession::MessageTypeData outputMsgType = PrepareOutgoingMessageEvent(msgType);
    OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::AbortTransfer(StatusCode reason)
{
    VerifyOrReturnError((mState != TransferState::kUnitialized) && (mState != TransferState::kTransferDone) &&
                            (mState != TransferState::kErrorState),
                        CHIP_ERROR_INCORRECT_STATE);

    SendStatusReport(reason);

    return CHIP_NO_ERROR;
}

void TransferSession::Reset()
{
    mState = TransferState::kUnitialized;
    UnregisterOutputEventHandler();
    mSuppportedXferOpts.ClearAll();
    mTransferVersion       = 0;
    mMaxSupportedBlockSize = 0;
    mStartOffset           = 0;
    mTransferLength        = 0;
    mTransferMaxBlockSize  = 0;
    mTransferRequestMaxBlockSize = 0;

    mFileDesLength = 0;

    mNumBytesProcessed = 0;
    mLastBlockNum      = 0;
    mNextBlockNum      = 0;
    mLastQueryNum      = 0;
    mNextQueryNum      = 0;

    mAwaitingResponse = false;
}

CHIP_ERROR TransferSession::HandleMessageReceived(const PayloadHeader & payloadHeader, System::PacketBufferHandle msg)
{
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    if (payloadHeader.HasProtocol(Protocols::BDX::Id))
    {
        ReturnErrorOnFailure(HandleBdxMessage(payloadHeader, std::move(msg)));
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

// Return CHIP_ERROR only if there was a problem decoding the message. Otherwise, call SendStatusReport().
CHIP_ERROR TransferSession::HandleBdxMessage(const PayloadHeader & header, System::PacketBufferHandle msg)
{
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    const MessageType msgType = static_cast<MessageType>(header.GetMessageType());

#if CHIP_AUTOMATION_LOGGING
    ChipLogAutomation("Handling received BDX Message");
#endif // CHIP_AUTOMATION_LOGGING

    switch (msgType)
    {
    case MessageType::SendInit:
    case MessageType::ReceiveInit: {
        HandleTransferInit(msgType, std::move(msg));
        break;
    }
    case MessageType::SendAccept: {
        HandleSendAccept(std::move(msg));
        break;
    }

    case MessageType::ReceiveAccept: {
        HandleReceiveAccept(std::move(msg));
        break;
    }

    case MessageType::BlockQuery: {
        HandleBlockQuery(std::move(msg));
        break;
    }

    case MessageType::BlockQueryWithSkip: {
        HandleBlockQueryWithSkip(std::move(msg));
        break;
    }

    case MessageType::Block: {
        HandleBlock(std::move(msg));
        break;
    }
    case MessageType::BlockEOF: {
        HandleBlockEOF(std::move(msg));
        break;
    }

    case MessageType::BlockAck: {
        HandleBlockAck(std::move(msg));
        break;
    }

    case MessageType::BlockAckEOF: {
        HandleBlockAckEOF(std::move(msg));
        break;
    }

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

    StatusReportData statusReportData;
    statusReportData.statusCode = static_cast<StatusCode>(report.GetProtocolCode());

    OutputEvent event = OutputEvent::StatusReportEvent(OutputEventType::kStatusReceived, statusReportData);
    DispatchOutputEvent(event);

    return CHIP_NO_ERROR;
}

void TransferSession::HandleTransferInit(MessageType msgType, System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mState == TransferState::kAwaitingInitMsg, SendStatusReport(StatusCode::kUnexpectedMessage));

    if (mRole == TransferRole::kSender)
    {
        VerifyOrReturn(msgType == MessageType::ReceiveInit, SendStatusReport(StatusCode::kUnexpectedMessage));
    }
    else
    {
        VerifyOrReturn(msgType == MessageType::SendInit, SendStatusReport(StatusCode::kUnexpectedMessage));
    }

    TransferInit transferInit;

    const CHIP_ERROR err = transferInit.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    ResolveTransferControlOptions(transferInit.TransferCtlOptions);
    mTransferVersion      = ::chip::min(kBdxVersion, transferInit.Version);
    mTransferMaxBlockSize = ::chip::min(mMaxSupportedBlockSize, transferInit.MaxBlockSize);

    // Accept for now, they may be changed or rejected by the peer if this is a ReceiveInit
    mStartOffset    = transferInit.StartOffset;
    mTransferLength = transferInit.MaxLength;
    mFileDesignator = transferInit.FileDesignator;
    mFileDesLength = transferInit.FileDesLength;
    mTransferRequestMaxBlockSize = transferInit.MaxBlockSize;
    mTransferRequestControlFlags = transferInit.TransferCtlOptions;


    TransferInitData transferRequestData;
    // Store the Request data to share with the caller for verification
    transferRequestData.TransferCtlFlags = transferInit.TransferCtlOptions;
    transferRequestData.MaxBlockSize     = transferInit.MaxBlockSize;
    transferRequestData.StartOffset      = transferInit.StartOffset;
    transferRequestData.Length           = transferInit.MaxLength;
    transferRequestData.FileDesignator   = transferInit.FileDesignator;
    transferRequestData.FileDesLength    = transferInit.FileDesLength;
    transferRequestData.Metadata         = transferInit.Metadata;
    transferRequestData.MetadataLength   = transferInit.MetadataLength;

    mState = TransferState::kNegotiateTransferParams;

#if CHIP_AUTOMATION_LOGGING
    transferInit.LogMessage(msgType);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent::TransferInitEvent(transferRequestData);
    DispatchOutputEvent(event);
}

void TransferSession::HandleReceiveAccept(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kReceiver, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kAwaitingAccept, SendStatusReport(StatusCode::kUnexpectedMessage));

    ReceiveAccept rcvAcceptMsg;
    const CHIP_ERROR err = rcvAcceptMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    // Verify that Accept parameters are compatible with the original proposed parameters
    ReturnOnFailure(VerifyProposedMode(rcvAcceptMsg.TransferCtlFlags));

    mTransferMaxBlockSize = rcvAcceptMsg.MaxBlockSize;
    mStartOffset          = rcvAcceptMsg.StartOffset;
    mTransferLength       = rcvAcceptMsg.Length;

    // Note: if VerifyProposedMode() returned with no error, then mControlMode must match the proposed mode in the ReceiveAccept
    // message
    TransferAcceptData transferAcceptData;
    transferAcceptData.ControlMode    = mControlMode;
    transferAcceptData.MaxBlockSize   = rcvAcceptMsg.MaxBlockSize;
    transferAcceptData.StartOffset    = rcvAcceptMsg.StartOffset;
    transferAcceptData.Length         = rcvAcceptMsg.Length;
    transferAcceptData.Metadata       = rcvAcceptMsg.Metadata;
    transferAcceptData.MetadataLength = rcvAcceptMsg.MetadataLength;

    mAwaitingResponse = (mControlMode == TransferControlFlags::kSenderDrive);
    mState            = TransferState::kTransferInProgress;

#if CHIP_AUTOMATION_LOGGING
    rcvAcceptMsg.LogMessage(MessageType::ReceiveAccept);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent::TransferAcceptEvent(transferAcceptData);
    DispatchOutputEvent(event);
}

void TransferSession::HandleSendAccept(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kAwaitingAccept, SendStatusReport(StatusCode::kUnexpectedMessage));

    SendAccept sendAcceptMsg;
    const CHIP_ERROR err = sendAcceptMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    // Verify that Accept parameters are compatible with the original proposed parameters
    ReturnOnFailure(VerifyProposedMode(sendAcceptMsg.TransferCtlFlags));

    // Note: if VerifyProposedMode() returned with no error, then mControlMode must match the proposed mode in the SendAccept
    // message
    mTransferMaxBlockSize = sendAcceptMsg.MaxBlockSize;
    TransferAcceptData transferAcceptData;
    transferAcceptData.ControlMode    = mControlMode;
    transferAcceptData.MaxBlockSize   = sendAcceptMsg.MaxBlockSize;
    transferAcceptData.StartOffset    = mStartOffset;    // Not included in SendAccept msg, so use member
    transferAcceptData.Length         = mTransferLength; // Not included in SendAccept msg, so use member
    transferAcceptData.Metadata       = sendAcceptMsg.Metadata;
    transferAcceptData.MetadataLength = sendAcceptMsg.MetadataLength;

    mAwaitingResponse = (mControlMode == TransferControlFlags::kReceiverDrive);
    mState            = TransferState::kTransferInProgress;

#if CHIP_AUTOMATION_LOGGING
    sendAcceptMsg.LogMessage(MessageType::SendAccept);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent::TransferAcceptEvent(transferAcceptData);
    DispatchOutputEvent(event);
}

void TransferSession::HandleBlockQuery(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, SendStatusReport(StatusCode::kUnexpectedMessage));

    BlockQuery query;
    const CHIP_ERROR err = query.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(query.BlockCounter == mNextBlockNum, SendStatusReport(StatusCode::kBadBlockCounter));

    mAwaitingResponse = false;
    mLastQueryNum     = query.BlockCounter;

#if CHIP_AUTOMATION_LOGGING
    query.LogMessage(MessageType::BlockQuery);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent(OutputEventType::kQueryReceived);
    DispatchOutputEvent(event);
}

void TransferSession::HandleBlockQueryWithSkip(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, SendStatusReport(StatusCode::kUnexpectedMessage));

    BlockQueryWithSkip query;
    const CHIP_ERROR err = query.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(query.BlockCounter == mNextBlockNum, SendStatusReport(StatusCode::kBadBlockCounter));

    mAwaitingResponse = false;
    mLastQueryNum     = query.BlockCounter;

#if CHIP_AUTOMATION_LOGGING
    query.LogMessage(MessageType::BlockQueryWithSkip);
#endif // CHIP_AUTOMATION_LOGGING

    TransferSkipData bytesToSkip;
    bytesToSkip.BytesToSkip = query.BytesToSkip;
    OutputEvent event = OutputEvent::QueryWithSkipEvent(bytesToSkip);
    DispatchOutputEvent(event);
}

void TransferSession::HandleBlock(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kReceiver, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, SendStatusReport(StatusCode::kUnexpectedMessage));

    Block blockMsg;
    const CHIP_ERROR err = blockMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(blockMsg.BlockCounter == mLastQueryNum, SendStatusReport(StatusCode::kBadBlockCounter));
    VerifyOrReturn((blockMsg.DataLength > 0) && (blockMsg.DataLength <= mTransferMaxBlockSize),
                   SendStatusReport(StatusCode::kBadMessageContents));

    if (IsTransferLengthDefinite())
    {
        VerifyOrReturn(mNumBytesProcessed + blockMsg.DataLength <= mTransferLength, SendStatusReport(StatusCode::kLengthMismatch));
    }

    BlockData blockEventData;
    blockEventData.Data         = blockMsg.Data;
    blockEventData.Length       = blockMsg.DataLength;
    blockEventData.IsEof        = false;
    blockEventData.BlockCounter = blockMsg.BlockCounter;

    mNumBytesProcessed += blockMsg.DataLength;
    mLastBlockNum = blockMsg.BlockCounter;

    mAwaitingResponse = false;

#if CHIP_AUTOMATION_LOGGING
    blockMsg.LogMessage(MessageType::Block);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent::BlockDataEvent(blockEventData);
    DispatchOutputEvent(event);
}

void TransferSession::HandleBlockEOF(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kReceiver, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, SendStatusReport(StatusCode::kUnexpectedMessage));

    BlockEOF blockEOFMsg;
    const CHIP_ERROR err = blockEOFMsg.Parse(msgData.Retain());
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));

    VerifyOrReturn(blockEOFMsg.BlockCounter == mLastQueryNum, SendStatusReport(StatusCode::kBadBlockCounter));
    VerifyOrReturn(blockEOFMsg.DataLength <= mTransferMaxBlockSize, SendStatusReport(StatusCode::kBadMessageContents));

    BlockData blockEventData;
    blockEventData.Data         = blockEOFMsg.Data;
    blockEventData.Length       = blockEOFMsg.DataLength;
    blockEventData.IsEof        = true;
    blockEventData.BlockCounter = blockEOFMsg.BlockCounter;

    mNumBytesProcessed += blockEOFMsg.DataLength;
    mLastBlockNum = blockEOFMsg.BlockCounter;

    mAwaitingResponse = false;
    mState            = TransferState::kReceivedEOF;

#if CHIP_AUTOMATION_LOGGING
    blockEOFMsg.LogMessage(MessageType::BlockEOF);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent::BlockDataEvent(blockEventData);
    DispatchOutputEvent(event);
}

void TransferSession::HandleBlockAck(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kTransferInProgress, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, SendStatusReport(StatusCode::kUnexpectedMessage));

    BlockAck ackMsg;
    const CHIP_ERROR err = ackMsg.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));
    VerifyOrReturn(ackMsg.BlockCounter == mLastBlockNum, SendStatusReport(StatusCode::kBadBlockCounter));

    // In Receiver Drive, the Receiver can send a BlockAck to indicate receipt of the message and reset the timeout.
    // In this case, the Sender should wait to receive a BlockQuery next.
    mAwaitingResponse = (mControlMode == TransferControlFlags::kReceiverDrive);

#if CHIP_AUTOMATION_LOGGING
    ackMsg.LogMessage(MessageType::BlockAck);
#endif // CHIP_AUTOMATION_LOGGING

    OutputEvent event = OutputEvent(OutputEventType::kAckReceived);
    DispatchOutputEvent(event);
}

void TransferSession::HandleBlockAckEOF(System::PacketBufferHandle msgData)
{
    VerifyOrReturn(mRole == TransferRole::kSender, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mState == TransferState::kAwaitingEOFAck, SendStatusReport(StatusCode::kUnexpectedMessage));
    VerifyOrReturn(mAwaitingResponse, SendStatusReport(StatusCode::kUnexpectedMessage));

    BlockAckEOF ackMsg;
    const CHIP_ERROR err = ackMsg.Parse(std::move(msgData));
    VerifyOrReturn(err == CHIP_NO_ERROR, SendStatusReport(StatusCode::kBadMessageContents));
    VerifyOrReturn(ackMsg.BlockCounter == mLastBlockNum, SendStatusReport(StatusCode::kBadBlockCounter));

    mAwaitingResponse = false;

    mState = TransferState::kTransferDone;

#if CHIP_AUTOMATION_LOGGING
    ackMsg.LogMessage(MessageType::BlockAckEOF);
#endif // CHIP_AUTOMATION_LOGGING
    
    OutputEvent event = OutputEvent(OutputEventType::kAckEOFReceived);
    DispatchOutputEvent(event);
}

void TransferSession::ResolveTransferControlOptions(const BitFlags<TransferControlFlags> & proposed)
{
    // Must specify at least one synchronous option
    //
    if (!proposed.HasAny(TransferControlFlags::kSenderDrive, TransferControlFlags::kReceiverDrive))
    {
        SendStatusReport(StatusCode::kTransferMethodNotSupported);
        return;
    }

    // Ensure there are options supported by both nodes. Async gets priority.
    // If there is only one common option, choose that one. Otherwise the application must pick.
    const BitFlags<TransferControlFlags> commonOpts(proposed & mSuppportedXferOpts);
    if (!commonOpts.HasAny())
    {
        SendStatusReport(StatusCode::kTransferMethodNotSupported);
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
        SendStatusReport(StatusCode::kBadMessageContents);
        return CHIP_ERROR_INTERNAL;
    }

    // Verify the proposed mode is supported by this instance
    if (mSuppportedXferOpts.Has(mode))
    {
        mControlMode = mode;
    }
    else
    {
        SendStatusReport(StatusCode::kTransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void TransferSession::SendStatusReportWithError(StatusReportData statusReportData)
{
    OutputEvent event = OutputEvent::StatusReportEvent(OutputEventType::kInternalError, statusReportData);
    DispatchOutputEvent(event);
}

void TransferSession::SendStatusReport(StatusCode code)
{
    StatusReportData statusReportData;
    statusReportData.statusCode = code;

    // We set mState to an error state and prepare the status report. Once the consumer of the status report gets it,
    // they might modify our state appropriately later on.
    mState            = TransferState::kErrorState;
    mAwaitingResponse = false;

    Protocols::SecureChannel::StatusReport report(Protocols::SecureChannel::GeneralStatusCode::kFailure, Protocols::BDX::Id,
                                                  to_underlying(code));
    size_t msgSize                          = report.Size();
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::New(msgSize);
    if (buffer.IsNull())
    {
        ChipLogError(BDX, "%s: Error preparing status report: %" CHIP_ERROR_FORMAT, __FUNCTION__, CHIP_ERROR_NO_MEMORY.Format());
        SendStatusReportWithError(statusReportData);
        return;
    }

    Encoding::LittleEndian::PacketBufferWriter bbuf(std::move(buffer), msgSize);
    if (bbuf.IsNull())
    {
        ChipLogError(BDX, "%s: Error preparing status report: %" CHIP_ERROR_FORMAT, __FUNCTION__, CHIP_ERROR_NO_MEMORY.Format());
        SendStatusReportWithError(statusReportData);
        return;
    }

    report.WriteToBuffer(bbuf);
    System::PacketBufferHandle msg = bbuf.Finalize();
    if (msg.IsNull())
    {
        ChipLogError(BDX, "%s: Error preparing status report: %" CHIP_ERROR_FORMAT, __FUNCTION__, CHIP_ERROR_NO_MEMORY.Format());
        SendStatusReportWithError(statusReportData);
    }
    else
    {
        TransferSession::MessageTypeData outputMsgType =
        PrepareOutgoingMessageEvent(Protocols::SecureChannel::MsgType::StatusReport);
        OutputEvent event = TransferSession::OutputEvent::MsgToSendEvent(outputMsgType, std::move(msg));
        DispatchOutputEvent(event);
    }
}

bool TransferSession::IsTransferLengthDefinite() const
{
    return (mTransferLength > 0);
}

const char * TransferSession::OutputEvent::ToString(OutputEventType outputEventType)
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

TransferSession::OutputEvent TransferSession::OutputEvent::TransferInitEvent(TransferInitData data)
{
    OutputEvent event(OutputEventType::kInitReceived);
    event.transferInitData = data;
    return event;
}

/**
 * @brief
 *   Convenience method for constructing an OutputEvent with TransferAcceptData that contains Metadata
 */
TransferSession::OutputEvent TransferSession::OutputEvent::TransferAcceptEvent(TransferAcceptData data)
{
    OutputEvent event(OutputEventType::kAcceptReceived);
    event.transferAcceptData = data;
    return event;
}

TransferSession::OutputEvent TransferSession::OutputEvent::BlockDataEvent(BlockData data)
{
    OutputEvent event(OutputEventType::kBlockReceived);
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
    event.MsgData = std::move(msg);

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
