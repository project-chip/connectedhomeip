/**
 *    @file
 *
 *
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <support/CodeUtils.h>

namespace chip {
namespace BDX {

TransferSession::TransferSession()
{
    mState = kIdle;
}

CHIP_ERROR TransferSession::StartTransfer(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate, TransferRole role,
                                          const TransferInit & initMsg)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    size_t msgDataSize = 0;
    System::PacketBufferHandle msgBuf;
    MessageType msgType;

    VerifyOrExit(msgDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(platformDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    mMessagingDelegate = msgDelegate;
    mPlatformDelegate  = platformDelegate;
    mRole              = role;

    msgDataSize = initMsg.MessageSize();
    msgBuf      = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgDataSize));
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    msgType = (mRole == kSender) ? kSendInit : kReceiveInit;

    {
        BufBound bbuf(msgBuf->Start(), msgBuf->AvailableDataLength());
        initMsg.WriteToBuffer(bbuf);
        msgBuf->SetDataLength(static_cast<uint16_t>(bbuf.Needed()));
        err = AttachHeaderAndSend(msgType, std::move(msgBuf));
    }

    SuccessOrExit(err);

    mState = (mRole == kSender) ? kNegotiateSend : kNegotiateReceive;

exit:
    return err;
}

CHIP_ERROR TransferSession::WaitForTransfer(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate, TransferRole role,
                                            BitFlags<uint8_t, TransferControlFlags> xferControlOpts, uint16_t maxBlockSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

exit:
    return err;
}

// TODO: change parameter to PacketHeader
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

    // TODO: define Bdx protocol type
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
        /*        case kReceiveAccept:
                    HandleReceiveAccept();
                    break;
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
        // unknown message type
        break;
    }

exit:
    return CHIP_NO_ERROR;
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
    BitFlags<uint8_t, TransferControlFlags> commonOpts;
    size_t msgDataSize = 0;
    System::PacketBufferHandle msgBuf;
    MessageType msgType;

    VerifyOrExit(mRole == kSender, mMessagingDelegate->OnTransferError(kTransferMethodNotSupported));
    VerifyOrExit(mState == kIdle, mMessagingDelegate->OnTransferError(kServerBadState));

    err = rcvInit.Parse(msgData.Retain());
    SuccessOrExit(err);

    // Transfer Control ///////////////////////////////////////////
    //      - version needs to be equal or less than ours
    //      - at least one drive bit is set, otherwise reject
    //      - if async, pick async
    //      - if both sender and receiver drive, platform picks?
    // TODO: Version - will it be hardcoded into the image or chosen at runtime?
    mVersion = ::chip::min(mVersion, rcvInit.Version);

    // Must specify at least one synchronous option
    if (!rcvInit.TransferCtlOptions.Has(kSenderDrive) && !rcvInit.TransferCtlOptions.Has(kReceiverDrive))
    {
        // TODO: TRANSFER METHOD NOT SUPPORTED ?
        // BAD MESSAGE CONTENTS ?
        return;
    }

    commonOpts.SetRaw(rcvInit.TransferCtlOptions.Raw() & mSuppportedXferOpts.Raw());
    if (commonOpts.Raw() == 0)
    {
        // TODO: TRANSFER METHOD NOT SUPPORTED
        return;
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
        // multiple supported options, application chooses
        err = mPlatformDelegate->ChooseControlMode(rcvInit.TransferCtlOptions, mSuppportedXferOpts, mControlMode);
        SuccessOrExit(err);
    }
    // End Transfer control //////////////////////////////////////////

    // Range Control
    //      - just accept all values (TODO: do we need to consider widerange?)
    mStartOffset = rcvInit.StartOffset;
    mMaxLength   = rcvInit.MaxLength;

    // Max Block Size
    //      - respond with minimum, check against supported MBS
    mTransferMaxBlockSize = ::chip::min(mMaxSupportedBlockSize, rcvInit.MaxBlockSize);

    // File designator
    //      - TODO: pass to platform layer

    // Metadata
    //      - TODO: pass to platform layer (if any exists)

    acceptMsg.TransferCtlFlags.SetRaw(0).Set(mControlMode, true);
    acceptMsg.Version      = mVersion;
    acceptMsg.MaxBlockSize = mTransferMaxBlockSize;
    acceptMsg.StartOffset  = mStartOffset;
    acceptMsg.Length       = mMaxLength;
    // TODO: metadata

    msgDataSize = acceptMsg.MessageSize();
    msgBuf      = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgDataSize));
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    msgType = (mRole == kSender) ? kSendInit : kReceiveInit;

    {
        BufBound bbuf(msgBuf->Start(), msgBuf->AvailableDataLength());
        acceptMsg.WriteToBuffer(bbuf);
        msgBuf->SetDataLength(static_cast<uint16_t>(bbuf.Needed()));
        err = AttachHeaderAndSend(msgType, std::move(msgBuf));
    }

    err = AttachHeaderAndSend(kReceiveAccept, std::move(msgBuf));
    SuccessOrExit(err);

    mState = kTransferInProgress;

exit:
    return;
}
/*
TransferSession::HandleReceiveAccept(System::PacketBuffer *msgData)
{
    // was the last message sent a ReceiveInit?
    if (mState != kNegotiatingReceive)
    {
        //TODO
    }

    // double-check that parameters are acceptable:
    //    need to decode message first

    // acceptable? transfer control
    // acceptable? range control
    // acceptable? max size

    // if acceptable...
    if (mControlMode == kReceiverDrive)
    {
        // send block query
    }

    else if (mControlMode == kSenderDrive)
    {
        // await Block
    }

}

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

void TransferSession::EndTransfer(CHIP_ERROR error)
{
    // TODO:
    mState = kIdle;
}

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
