/**
 *    @file
 *
 *
 */

#include <protocols/bdx/BdxTransferSession.h>

#include <protocols/bdx/BdxMessages.h>
#include <support/CodeUtils.h>

namespace chip {
namespace BDX {

TransferSession::TransferSession(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate)
{
    mState             = kIdle;
    mMessagingDelegate = msgDelegate;
    mPlatformDelegate  = platformDelegate;
}

// TODO: change parameter to PacketHeader
CHIP_ERROR TransferSession::HandleMessageReceived(const PacketHeader & packetHeader, System::PacketBufferHandle msg)
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
        HandleReceiveInit(msg);
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
void TransferSession::HandleReceiveInit(const System::PacketBufferHandle & msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveInit rcvInit;
    BitFlags<uint8_t, TransferControlFlags> commonOpts;

    // TODO: must be configured as a sender
    VerifyOrExit(mRole == kSender, mMessagingDelegate->OnTransferError(kTransferMethodNotSupported));

    // TODO: must not be in the middle of a transfer
    VerifyOrExit(mState == kIdle, mMessagingDelegate->OnTransferError(kServerBadState));

    // TODO: need to review PacketBuffer ownership
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

    // TODO: fill out ReceiveAccept message
    mMessagingDelegate->SendMessage(kReceiveAccept, NULL);
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

CHIP_ERROR TransferSession::StartTransfer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mRole == kSender && mControlMode == kSenderDrive)
    {
        // TODO: send Block message
    }
    else if (mRole == kReceiver && mControlMode == kReceiverDrive)
    {
        // TODO: send BlockQuery
    }

    return err;
}

void TransferSession::EndTransfer(CHIP_ERROR error)
{
    // TODO:
    mControlMode = kNotSpecified;
    mState       = kIdle;
}

bool TransferSession::AreParametersCompatible()
{
    // TODO:
    mControlMode = kSenderDrive;
    return true;
}

} // namespace BDX
} // namespace chip
