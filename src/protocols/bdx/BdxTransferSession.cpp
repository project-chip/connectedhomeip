/**
 *    @file
 *
 *
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <support/CodeUtils.h>

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
namespace BDX {

TransferSession::TransferSession()
{
    mState = kIdle;
}

CHIP_ERROR TransferSession::StartTransfer(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate, TransferRole role,
                                          const TransferInit & initMsg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf;
    MessageType msgType;

    VerifyOrExit(mState == kIdle, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(msgDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(platformDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    mMessagingDelegate = msgDelegate;
    mPlatformDelegate  = platformDelegate;
    mRole              = role;

    err = WriteToPacketBuffer<TransferInit>(initMsg, msgBuf);
    SuccessOrExit(err);

    msgType = (mRole == kSender) ? kSendInit : kReceiveInit;
    err     = AttachHeaderAndSend(msgType, std::move(msgBuf));
    SuccessOrExit(err);

    mState = (mRole == kSender) ? kNegotiateSend : kNegotiateReceive;

exit:
    return err;
}

CHIP_ERROR TransferSession::WaitForTransfer(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate, TransferRole role,
                                            BitFlags<uint8_t, TransferControlFlags> xferControlOpts, uint16_t maxBlockSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kIdle, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(msgDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(platformDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    mMessagingDelegate     = msgDelegate;
    mPlatformDelegate      = platformDelegate;
    mRole                  = role;
    mSuppportedXferOpts    = xferControlOpts;
    mMaxSupportedBlockSize = maxBlockSize;
    mState                 = kIdle;

exit:
    return err;
}

CHIP_ERROR TransferSession::HandleMessageReceived(System::PacketBufferHandle msg)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    uint8_t msgType;
    PayloadHeader payloadHeader;

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
    SuccessOrExit(err);

    msg->ConsumeHead(headerSize);

    // TODO: handle StatusReport
    VerifyOrExit(payloadHeader.GetProtocolID() == Protocols::kProtocol_BDX, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    msgType = payloadHeader.GetMessageType();

    switch (msgType)
    {
    /*        case kSendInit:
                HandleSendInit();
                break;
            case kSendAccept:
                HandleSendAccept();
                break;
    */
    case kReceiveInit:
        HandleReceiveInit(std::move(msg));
        break;
    case kReceiveAccept:
        HandleReceiveAccept(std::move(msg));
        break;
        /*
                case kBlockQuery:
                    HandleBlockQuery();
                    break;
                case kBlock:
                    HandleBlock();
                    break;
                case kBlockEOF:
                    HandleBlockEOF();
                    break;
                case kBlockAck:
                    HandleBlockAck();
                    break;
                case kBlockAckEOF:
                    HandleBlockAckEOF();
        */
        break;
    default:
        printf("unknown message type\n");
        // unknown message type
        break;
    }

exit:
    printf("err = %d\n", err);
    return err;
}

void TransferSession::EndTransfer(CHIP_ERROR error)
{
    // TODO: send status report?
    mState = kIdle;
}
/*
void TransferSession::HandleSendInit()
{

}

void TransferSession::HandleSendAccept()
{

}
*/
void TransferSession::HandleReceiveInit(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveInit rcvInit;
    ReceiveAccept acceptMsg;
    System::PacketBufferHandle outMsgBuf;

    // TODO: (spec) correct errors?
    VerifyOrExit(mRole == kSender, mMessagingDelegate->OnTransferError(kTransferMethodNotSupported));
    VerifyOrExit(mState == kIdle, mMessagingDelegate->OnTransferError(kServerBadState));

    err = rcvInit.Parse(msgData.Retain());
    SuccessOrExit(err);

    // TODO: (spec) Version - will it be hardcoded into the image or chosen at runtime?
    mVersion = ::chip::min(mVersion, rcvInit.Version);

    err = ResolveTransferControlOptions(rcvInit.TransferCtlOptions);
    SuccessOrExit(err);

    // Range Control
    //      - just accept all values
    //      TODO: (spec) do we need to consider widerange? is it assumed all devices support 32 and 64?)
    mStartOffset = rcvInit.StartOffset;
    mMaxLength   = rcvInit.MaxLength;

    mTransferMaxBlockSize = ::chip::min(mMaxSupportedBlockSize, rcvInit.MaxBlockSize);

    // File designator
    //      - TODO: do we need to store backup in this object to protect null access?
    err = mPlatformDelegate->OnFileDesignatorReceived(rcvInit.FileDesignator, rcvInit.FileDesLength);
    SuccessOrExit(err);

    // Metadata
    //      - TODO: do we need to store backup in this object to protect null access?
    if (rcvInit.Metadata != nullptr && rcvInit.MetadataLength > 0)
    {
        err = mPlatformDelegate->OnMetadataReceived(rcvInit.Metadata, rcvInit.MetadataLength);
        SuccessOrExit(err);
    }

    // TODO: move ReceiveAccept sending to a public method?????
    acceptMsg.TransferCtlFlags.SetRaw(0).Set(mControlMode, true);
    acceptMsg.Version      = mVersion;
    acceptMsg.MaxBlockSize = mTransferMaxBlockSize;
    acceptMsg.StartOffset  = mStartOffset;
    acceptMsg.Length       = mMaxLength;

    // TODO: does this work?
    mPlatformDelegate->ProvideResponseMetadata(acceptMsg.Metadata, acceptMsg.MetadataLength);

    err = WriteToPacketBuffer<ReceiveAccept>(acceptMsg, outMsgBuf);
    SuccessOrExit(err);

    err = AttachHeaderAndSend(kReceiveAccept, std::move(outMsgBuf));
    SuccessOrExit(err);

    // TODO: states, or expected message?
    mState = kTransferInProgress;

exit:
    printf("HandeReceiveInit err %d\n", err);
    return;
}

CHIP_ERROR TransferSession::ResolveTransferControlOptions(const BitFlags<uint8_t, TransferControlFlags> & proposed)
{
    // Must specify at least one synchronous option
    if (!proposed.Has(kSenderDrive) && !proposed.Has(kReceiverDrive))
    {
        // TODO: (spec) TRANSFER METHOD NOT SUPPORTED ?
        // BAD MESSAGE CONTENTS ?
        mMessagingDelegate->OnTransferError(kTransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    // Ensure there are options supported by both nodes.
    // If there is only one common option, choose that one. Otherwise, let the application pick.
    BitFlags<uint8_t, TransferControlFlags> commonOpts;
    commonOpts.SetRaw(proposed.Raw() & mSuppportedXferOpts.Raw());
    if (commonOpts.Raw() == 0)
    {
        mMessagingDelegate->OnTransferError(kTransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }
    else if (commonOpts.HasOnly(kAsync))
    {
        mControlMode = kAsync;
    }
    else if (commonOpts.HasOnly(kReceiverDrive))
    {
        mControlMode = kReceiverDrive;
    }
    else if (commonOpts.HasOnly(kSenderDrive))
    {
        mControlMode = kSenderDrive;
    }
    else
    {
        return mPlatformDelegate->ChooseControlMode(proposed, mSuppportedXferOpts, mControlMode);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TransferSession::VerifyProposedMode(const BitFlags<uint8_t, TransferControlFlags> & proposed)
{
    TransferControlFlags mode;

    // Must specify only one mode in Accept messages
    if (proposed.HasOnly(kAsync))
    {
        mode = kAsync;
    }
    else if (proposed.HasOnly(kReceiverDrive))
    {
        mode = kReceiverDrive;
    }
    else if (proposed.HasOnly(kSenderDrive))
    {
        mode = kSenderDrive;
    }
    else
    {
        mMessagingDelegate->OnTransferError(kTransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    // Verify the proposed mode is supported by this instance
    if (mSuppportedXferOpts.Has(mode))
    {
        mControlMode = mode;
    }
    else
    {
        mMessagingDelegate->OnTransferError(kTransferMethodNotSupported);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void TransferSession::HandleReceiveAccept(System::PacketBufferHandle msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveAccept rcvAcceptMsg;
    System::PacketBufferHandle outMsgBuf;

    VerifyOrExit(mRole == kReceiver, mMessagingDelegate->OnTransferError(kTransferMethodNotSupported));
    VerifyOrExit(mState == kNegotiateReceive, mMessagingDelegate->OnTransferError(kServerBadState));

    err = rcvAcceptMsg.Parse(msgData.Retain());
    SuccessOrExit(err);

    err = VerifyProposedMode(rcvAcceptMsg.TransferCtlFlags);
    SuccessOrExit(err);

    // acceptable? range control
    // acceptable? max size

    // TODO: metadata

    // if acceptable...
    if (mControlMode == kReceiverDrive)
    {
        // send block query
    }

    else if (mControlMode == kSenderDrive)
    {
        // await Block
    }

    // err = AttachHeaderAndSend(kReceiveAccept, std::move(outMsgBuf));
    SuccessOrExit(err);

exit:
    return;
}

/*
TransferSession::HandleBlockQuery()
{

}

TransferSession::HandleBlock()
{

}

TransferSession::HandleBlockEOF()
{

}

TransferSession::HandleBlockAck()
{

}

TransferSession::HandleBlockAckEOF()
{
    // if role != Receiver
    if (mTransferRole != kReceiver)
    {
        EndTransfer(CHIP_ERROR_INCORRECT_STATE);
    }

    // if state != ending transfer
    if (mState != kFinalizeTransfer)
    {
        // TODO: send StatusReport and end transfer
        EndTransfer(CHIP_ERROR_INCORRECT_STATE);
    }

    // TODO: does counter correspond to BlockEOF counter?
    if (msgData[BlockCounter] !=  mBlockCounter)
    {
        // TODO: send StatusReport and end transfer
    }

    // end transfer and clean up
    EndTransfer(CHIP_NO_ERROR);
}
*/

CHIP_ERROR TransferSession::AttachHeaderAndSend(MessageType msgType, System::PacketBufferHandle msgBuf)
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

    err = mMessagingDelegate->SendMessage(std::move(msgBuf));
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace BDX
} // namespace chip
