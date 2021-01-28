/**
 *    @file
 *      Implementation for the TransferSession class.
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <support/CodeUtils.h>

namespace {
const uint8_t kBdxVersion = 0; ///< The version of this implementation of the BDX spec
}

/**
 * @brief
 *   Allocate a new PacketBuffer and write data from a BDX message struct.
 */
template <class BdxMsgType>
CHIP_ERROR WriteToPacketBuffer(const BdxMsgType & msgStruct, ::chip::System::PacketBufferHandle & msgBuf)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    size_t msgDataSize = msgStruct.MessageSize();
    msgBuf             = ::chip::System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgDataSize));
    if (msgBuf.IsNull())
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    ::chip::BufBound bbuf(msgBuf->Start(), msgBuf->AvailableDataLength());
    msgStruct.WriteToBuffer(bbuf);
    msgBuf->SetDataLength(static_cast<uint16_t>(bbuf.Needed()));

    return err;
}

namespace chip {
namespace bdx {

TransferSession::TransferSession()
{
    mState = kState_Idle;
    mPendingOutput.SetRaw(0);
    mSuppportedXferOpts.SetRaw(0);
}

void TransferSession::PollOutput(OutputEvent & event)
{
    event = OutputEvent();

    // Can only output one event at a time.
    if (mPendingOutput.Raw() == 0)
    {
        event = OutputEvent(kOutput_None);
    }
    else if (mPendingOutput.Has(kOutput_InternalError))
    {
        // Error should take priority over any other events.
        event = OutputEvent::TransferErrorEvent(mTransferErrorData);
        mPendingOutput.Clear(kOutput_InternalError);
    }
    else if (mPendingOutput.Has(kOutput_StatusReceived))
    {
        event = OutputEvent::TransferErrorEvent(mTransferErrorData);
        mPendingOutput.Clear(kOutput_StatusReceived);
    }
    else if (mPendingOutput.Has(kOutput_MsgToSend))
    {
        event         = OutputEvent(kOutput_MsgToSend);
        event.MsgData = std::move(mPendingMsgHandle);
        mPendingOutput.Clear(kOutput_MsgToSend);
    }
    else if (mPendingOutput.Has(kOutput_InitReceived))
    {
        event = OutputEvent::TransferInitEvent(mTransferRequestData, std::move(mPendingMsgHandle));
        mPendingOutput.Clear(kOutput_InitReceived);
    }
    else if (mPendingOutput.Has(kOutput_AcceptReceived))
    {
        event = OutputEvent::TransferAcceptEvent(mTransferAcceptData, std::move(mPendingMsgHandle));
        mPendingOutput.Clear(kOutput_AcceptReceived);
    }
    else if (mPendingOutput.Has(kOutput_QueryReceived))
    {
        event = OutputEvent(kOutput_QueryReceived);
        mPendingOutput.Clear(kOutput_QueryReceived);
    }
    else if (mPendingOutput.Has(kOutput_BlockReceived))
    {
        event = OutputEvent::BlockDataEvent(mBlockEventData, std::move(mPendingMsgHandle));
        mPendingOutput.Clear(kOutput_BlockReceived);
    }
    else if (mPendingOutput.Has(kOutput_AckReceived))
    {
        event = OutputEvent(kOutput_AckReceived);
        mPendingOutput.Clear(kOutput_AckReceived);
    }
    else if (mPendingOutput.Has(kOutput_AckEOFReceived))
    {
        event = OutputEvent(kOutput_AckEOFReceived);
        mPendingOutput.Clear(kOutput_AckEOFReceived);
    }
}

CHIP_ERROR TransferSession::StartTransfer(TransferRole role, const TransferInitData & initData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageType msgType;
    TransferInit initMsg;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    // Set transfer parameters. They may be overridden later by an Accept message
    mSuppportedXferOpts.SetRaw(initData.TransferCtlFlagsRaw);
    mRole                  = role;
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

    err = WriteToPacketBuffer<TransferInit>(initMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    msgType = (mRole == kRole_Sender) ? kBdxMsg_SendInit : kBdxMsg_ReceiveInit;
    err     = AttachBdxHeader(msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    mState = kState_AwaitingAccept;

    mPendingOutput.Set(kOutput_MsgToSend);

exit:
    return err;
}

CHIP_ERROR TransferSession::WaitForTransfer(TransferRole role, BitFlags<uint8_t, TransferControlFlags> xferControlOpts,
                                            uint16_t maxBlockSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    // Used to determine compatibility with any future TransferInit parameters
    mRole                  = role;
    mSuppportedXferOpts    = xferControlOpts;
    mMaxSupportedBlockSize = maxBlockSize;

    mState = kState_AwaitingInitMsg;

exit:
    return err;
}

CHIP_ERROR TransferSession::AcceptTransfer(const TransferAcceptData & acceptData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle outMsgBuf;
    BitFlags<uint8_t, TransferControlFlags> proposedControlOpts;

    VerifyOrExit(mState == kState_NegotiateTransferParams, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mPendingOutput.Has(kOutput_MsgToSend), err = CHIP_ERROR_INCORRECT_STATE);

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

        err = WriteToPacketBuffer<ReceiveAccept>(acceptMsg, mPendingMsgHandle);
        SuccessOrExit(err);

        err = AttachBdxHeader(kBdxMsg_ReceiveAccept, mPendingMsgHandle);
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

        err = WriteToPacketBuffer<SendAccept>(acceptMsg, mPendingMsgHandle);
        SuccessOrExit(err);

        err = AttachBdxHeader(kBdxMsg_SendAccept, mPendingMsgHandle);
        SuccessOrExit(err);
    }

    mPendingOutput.Set(kOutput_MsgToSend);

    mState = kState_TransferInProgress;

exit:
    return err;
}

CHIP_ERROR TransferSession::PrepareBlockQuery()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockQuery queryMsg;

    VerifyOrExit(mState == kState_TransferInProgress, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRole == kRole_Receiver, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mPendingOutput.Has(kOutput_MsgToSend), err = CHIP_ERROR_INCORRECT_STATE);

    queryMsg.BlockCounter = mNextQueryNum++;

    err = WriteToPacketBuffer<BlockQuery>(queryMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    err = AttachBdxHeader(kBdxMsg_BlockQuery, mPendingMsgHandle);
    SuccessOrExit(err);

    mPendingOutput.Set(kOutput_MsgToSend);

    mBlockNumInFlight = queryMsg.BlockCounter;

exit:
    return err;
}

CHIP_ERROR TransferSession::PrepareBlock(const BlockData & inData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataBlock blockMsg;
    MessageType msgType;

    VerifyOrExit(mState == kState_TransferInProgress, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mRole == kRole_Sender, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mPendingOutput.Has(kOutput_MsgToSend), err = CHIP_ERROR_INCORRECT_STATE);

    // Verify non-zero data is provided and is no longer than MaxBlockSize (BlockEOF may contain 0 length data)
    VerifyOrExit((inData.Data != nullptr) && (inData.Length <= mTransferMaxBlockSize), err = CHIP_ERROR_INVALID_ARGUMENT);

    blockMsg.BlockCounter = mBlockNumInFlight;
    blockMsg.Data         = inData.Data;
    blockMsg.DataLength   = inData.Length;

    err = WriteToPacketBuffer<DataBlock>(blockMsg, mPendingMsgHandle);
    SuccessOrExit(err);

    msgType = inData.IsEof ? kBdxMsg_BlockEOF : kBdxMsg_Block;
    err     = AttachBdxHeader(msgType, mPendingMsgHandle);
    SuccessOrExit(err);

    mPendingOutput.Set(kOutput_MsgToSend);

    if (msgType == kBdxMsg_BlockEOF)
    {
        mState = kState_AwaitingEOFAck;
    }

exit:
    return err;
}

CHIP_ERROR TransferSession::PrepareBlockAck()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CounterMessage ackMsg;

    VerifyOrExit(mRole == kRole_Receiver, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit((mState == kState_TransferInProgress) || (mState == kState_ReceivedEOF), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!mPendingOutput.Has(kOutput_MsgToSend), err = CHIP_ERROR_INCORRECT_STATE);

    ackMsg.BlockCounter = mBlockNumInFlight;

    if (mState == kState_TransferInProgress)
    {
        err = WriteToPacketBuffer<BlockAck>(ackMsg, mPendingMsgHandle);
        SuccessOrExit(err);

        err = AttachBdxHeader(kBdxMsg_BlockAck, mPendingMsgHandle);
        SuccessOrExit(err);
    }
    else if (mState == kState_ReceivedEOF)
    {
        err = WriteToPacketBuffer<BlockAckEOF>(ackMsg, mPendingMsgHandle);
        SuccessOrExit(err);

        err = AttachBdxHeader(kBdxMsg_BlockAckEOF, mPendingMsgHandle);
        SuccessOrExit(err);
    }

    mPendingOutput.Set(kOutput_MsgToSend);

exit:
    return err;
}

CHIP_ERROR TransferSession::AbortTransfer(StatusCode reason)
{
    // TODO: prepare a StatusReport
    mState = kState_Error;

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void TransferSession::Reset()
{
    mPendingOutput.SetRaw(0);
    mState = kState_Idle;
    mSuppportedXferOpts.SetRaw(0);
    mTransferVersion       = 0;
    mMaxSupportedBlockSize = 0;
    mStartOffset           = 0;
    mTransferLength        = 0;
    mTransferMaxBlockSize  = 0;
    mNextQueryNum          = 0;
    mBlockNumInFlight      = 0;
    mNumBytesProcessed     = 0;
}

CHIP_ERROR TransferSession::HandleMessageReceived(System::PacketBufferHandle msg)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    PayloadHeader payloadHeader;

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
    SuccessOrExit(err);

    msg->ConsumeHead(headerSize);

    // TODO: call HandleStatusReport if message is StatusReport
    if (payloadHeader.GetProtocolID() == Protocols::kProtocol_BDX)
    {
        err = HandleBdxMessage(payloadHeader, std::move(msg));
        SuccessOrExit(err);
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    return err;
}

CHIP_ERROR TransferSession::HandleBdxMessage(PayloadHeader & header, System::PacketBufferHandle msg)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    MessageType msgType = static_cast<MessageType>(header.GetMessageType());

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

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

CHIP_ERROR TransferSession::HandleStatusReportMessage(PayloadHeader & header, System::PacketBufferHandle msg)
{
    mPendingOutput.Set(kOutput_StatusReceived);
    mState = kState_Error;

    // TODO: parse status report message

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void TransferSession::HandleTransferInit(MessageType msgType, System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferInit transferInit;

    VerifyOrExit(mState == kState_AwaitingInitMsg, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(!mPendingOutput.Has(kOutput_InitReceived), SetTransferError(kStatus_ServerBadState));

    if (mRole == kRole_Sender)
    {
        VerifyOrExit(msgType == kBdxMsg_ReceiveInit, SetTransferError(kStatus_ServerBadState));
    }
    else
    {
        VerifyOrExit(msgType == kBdxMsg_SendInit, SetTransferError(kStatus_ServerBadState));
    }

    err = transferInit.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));

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
    mPendingOutput.Set(kOutput_InitReceived);

    mState = kState_NegotiateTransferParams;

exit:
    return;
}

void TransferSession::HandleReceiveAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveAccept rcvAcceptMsg;

    VerifyOrExit(mRole == kRole_Receiver, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_AwaitingAccept, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(!mPendingOutput.Has(kOutput_AcceptReceived), SetTransferError(kStatus_ServerBadState));

    err = rcvAcceptMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));

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
    mPendingOutput.Set(kOutput_AcceptReceived);

    mState = kState_TransferInProgress;

exit:
    return;
}

void TransferSession::HandleSendAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SendAccept sendAcceptMsg;

    VerifyOrExit(mRole == kRole_Sender, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_AwaitingAccept, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(!mPendingOutput.Has(kOutput_AcceptReceived), SetTransferError(kStatus_ServerBadState));

    err = sendAcceptMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));

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
    mPendingOutput.Set(kOutput_AcceptReceived);

    mState = kState_TransferInProgress;

exit:
    return;
}

void TransferSession::HandleBlockQuery(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockQuery query;

    VerifyOrExit(mRole == kRole_Sender, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_TransferInProgress, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(!mPendingOutput.Has(kOutput_QueryReceived), SetTransferError(kStatus_ServerBadState));

    err = query.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));

    VerifyOrExit(query.BlockCounter == mNextQueryNum, SetTransferError(kStatus_BadBlockCounter));
    mNextQueryNum++;
    mBlockNumInFlight = query.BlockCounter;

    mPendingOutput.Set(kOutput_QueryReceived);

exit:
    return;
}

void TransferSession::HandleBlock(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Block blockMsg;

    VerifyOrExit(mRole == kRole_Receiver, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_TransferInProgress, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(!mPendingOutput.Has(kOutput_BlockReceived), SetTransferError(kStatus_ServerBadState));

    err = blockMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));

    VerifyOrExit(blockMsg.BlockCounter == mBlockNumInFlight, SetTransferError(kStatus_BadBlockCounter));
    VerifyOrExit((blockMsg.DataLength > 0) && (blockMsg.DataLength <= mTransferMaxBlockSize),
                 SetTransferError(kStatus_BadMessageContents));

    if (IsTransferLengthDefinite())
    {
        VerifyOrExit(mNumBytesProcessed + blockMsg.DataLength <= mTransferLength, SetTransferError(kStatus_LengthMismatch));
    }

    mBlockEventData.Data   = blockMsg.Data;
    mBlockEventData.Length = blockMsg.DataLength;
    mBlockEventData.IsEof  = false;

    mNumBytesProcessed += blockMsg.DataLength;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput.Set(kOutput_BlockReceived);

exit:
    return;
}

void TransferSession::HandleBlockEOF(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockEOF blockEOFMsg;

    VerifyOrExit(mRole == kRole_Receiver, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_TransferInProgress, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(!mPendingOutput.Has(kOutput_BlockReceived), SetTransferError(kStatus_ServerBadState));

    err = blockEOFMsg.Parse(msgData.Retain());
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));
    VerifyOrExit(blockEOFMsg.BlockCounter == mBlockNumInFlight, SetTransferError(kStatus_BadBlockCounter));
    VerifyOrExit(blockEOFMsg.DataLength <= mTransferMaxBlockSize, SetTransferError(kStatus_BadMessageContents));

    mBlockEventData.Data   = blockEOFMsg.Data;
    mBlockEventData.Length = blockEOFMsg.DataLength;
    mBlockEventData.IsEof  = true;

    mNumBytesProcessed += blockEOFMsg.DataLength;

    mPendingMsgHandle = std::move(msgData);
    mPendingOutput.Set(kOutput_BlockReceived);

    mState = kState_ReceivedEOF;

exit:
    return;
}

void TransferSession::HandleBlockAck(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockAck ackMsg;

    VerifyOrExit(mRole == kRole_Sender, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_TransferInProgress, SetTransferError(kStatus_ServerBadState));

    err = ackMsg.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));
    VerifyOrExit(ackMsg.BlockCounter == mBlockNumInFlight, SetTransferError(kStatus_BadBlockCounter));

    mPendingOutput.Set(kOutput_AckReceived);

exit:
    return;
}

void TransferSession::HandleBlockAckEOF(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BlockAckEOF ackMsg;

    VerifyOrExit(mRole == kRole_Sender, SetTransferError(kStatus_ServerBadState));
    VerifyOrExit(mState == kState_AwaitingEOFAck, SetTransferError(kStatus_ServerBadState));

    err = ackMsg.Parse(std::move(msgData));
    VerifyOrExit(err == CHIP_NO_ERROR, SetTransferError(kStatus_BadMessageContents));
    VerifyOrExit(ackMsg.BlockCounter == mBlockNumInFlight, SetTransferError(kStatus_BadBlockCounter));

    mPendingOutput.Set(kOutput_AckEOFReceived);

exit:
    return;
}

CHIP_ERROR TransferSession::AttachBdxHeader(MessageType msgType, System::PacketBufferHandle & msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PayloadHeader payloadHeader;

    payloadHeader
        .SetMessageType(static_cast<uint8_t>(msgType)) //
        .SetProtocolID(Protocols::kProtocol_BDX);

    uint16_t headerSize              = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = payloadHeader.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}

void TransferSession::ResolveTransferControlOptions(const BitFlags<uint8_t, TransferControlFlags> & proposed)
{
    // Must specify at least one synchronous option
    if (!proposed.Has(kControl_SenderDrive) && !proposed.Has(kControl_ReceiverDrive))
    {
        SetTransferError(kStatus_TransferMethodNotSupported);
        return;
    }

    // Ensure there are options supported by both nodes. Async gets priority.
    // If there is only one common option, choose that one. Otherwise the application must pick.
    BitFlags<uint8_t, TransferControlFlags> commonOpts;
    commonOpts.SetRaw(proposed.Raw() & mSuppportedXferOpts.Raw());
    if (commonOpts.Raw() == 0)
    {
        SetTransferError(kStatus_TransferMethodNotSupported);
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
        SetTransferError(kStatus_BadMessageContents);
        return CHIP_ERROR_INTERNAL;
    }

    // Verify the proposed mode is supported by this instance
    if (mSuppportedXferOpts.Has(mode))
    {
        mControlMode = mode;
    }
    else
    {
        SetTransferError(kStatus_TransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void TransferSession::SetTransferError(StatusCode code)
{
    mTransferErrorData.error = code;
    mPendingOutput.Set(kOutput_InternalError);
    mState = kState_Error;
}

bool TransferSession::IsTransferLengthDefinite()
{
    return (mTransferLength > 0);
}

} // namespace bdx
} // namespace chip
