/**
 *    @file
 *
 *
 */

#include <protocols/bdx/BDXState.h>

#include <protocols/bdx/BdxMessages.h>
#include <support/CodeUtils.h>

namespace chip {
namespace BDX {

TransferManager::TransferManager()
{
    mState = kIdle;
}

CHIP_ERROR TransferManager::Init(TransferRole role, TransferControlParams controlParams, RangeControlParams rangeParams,
                                 uint16_t maxBlockSize, BDXStateDelegate * delegate)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    mRole                  = role;
    mControlParams         = controlParams;
    mRangeParams           = rangeParams;
    mMaxSupportedBlockSize = maxBlockSize;
    mDelegate              = delegate;

    return err;
}

// TODO: change parameter to PacketHeader
CHIP_ERROR TransferManager::HandleMessageReceived(uint16_t msgType, System::PacketBuffer * msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (msgData == NULL)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

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
        HandleReceiveInit(msgData);
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

done:
    return CHIP_NO_ERROR;
}
/*
void TransferManager::HandleSendInit()
{

}

void TransferManager::HandleSendAccept()
{

}
*/
void TransferManager::HandleReceiveInit(System::PacketBuffer * msgData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveInit rcvInit;

    // must be configured as a sender
    VerifyOrExit(mRole == kSender, mDelegate->OnTransferError(kTransferMethodNotSupported));

    // must not be in the middle of a transfer
    VerifyOrExit(mState == kIdle, mDelegate->OnTransferError(kServerBadState));

    err = rcvInit.Parse(*msgData);
    SuccessOrExit(err);

    if (AreParametersCompatible())
    {
        // TODO: Send StatusReport message with appropriate error
        mDelegate->SendMessage(kStatusReport_Temp, NULL);
        goto done;
    }

    // TODO: fill out ReceiveAccept message
    mDelegate->SendMessage(kReceiveAccept, NULL);
    mState = kTransferInProgress;

exit:
    return;
}
/*
TransferManager::HandleReceiveAccept(System::PacketBuffer *msgData)
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

TransferManager::HandleBlockQuery()
{

}

TransferManager::HandleBlock()
{

}

TransferManager::HandleBlockEOF()
{

}

TransferManager::HandleBlockAck()
{

}

TransferManager::HandleBlockAckEOF()
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

CHIP_ERROR TransferManager::StartTransfer()
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

void TransferManager::EndTransfer(CHIP_ERROR error)
{
    // TODO:
    mControlMode = kNotSpecified;
    mState       = kIdle;
}

bool TransferManager::AreParametersCompatible()
{
    // TODO:
    mControlMode = kSenderDrive;
    return true;
}

} // namespace BDX
} // namespace chip
