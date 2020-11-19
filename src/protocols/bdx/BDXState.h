/**
 *    @file
 *      This file defines a state machine class that will contain the main logic
 *      for the Bulk Data Transfer protocol. It provides APIs for initializing a
 *      state machine object, handling incoming messages, and beginning or
 *      ending transfers.
 */

#pragma once

#include <core/CHIPError.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace BDX {

enum TransferStates : uint8_t
{
    kIdle               = 0,
    kNegotiateReceive   = 1,
    kNegotiateSend      = 2,
    kTransferInProgress = 3,
    kFinalizeTransfer   = 4,
    kError              = 5,
};

enum ControlMode : uint8_t
{
    kNotSpecified  = 0,
    kReceiverDrive = 1,
    kSenderDrive   = 2,
    kAsync         = 3,
};

enum TransferRole : uint8_t
{
    kReceiver = 0,
    kSender   = 1,
};

typedef struct TransferControlParams
{
    bool mSupportsAsync;
    bool mSupportsReceiverDrive;
    bool mSupportsSenderDrive;
    // uint8_t mSupportedVersions; //TODO: add versioning support
} TransferControlParams;

typedef struct RangeControlParams
{
    bool mWiderange;   ///< Set true to indicate mStartOffset and mDefLen are 64-bit, else 32-bit
    bool mStartOffset; ///< Set true if using start offset in transfer
    bool mDefLen;      ///< Set true if transfer file has definite length
} RangeControlParams;

class DLL_EXPORT BDXStateDelegate
{
public:
    /**
     * @brief
     *   Called when a TransferManager object needs to send a message to the other endpoint.
     *
     */
    virtual CHIP_ERROR SendMessage(BDXMsgType msgType, System::PacketBuffer * msgBuf) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * @brief
     *   Expected to send a StatusReport.
     */
    virtual CHIP_ERROR OnTransferError(BDXError error) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual bool AreParametersAcceptable() { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

class DLL_EXPORT TransferManager
{
public:
    TransferManager();

    // TODO: Builder pattern probably works better here
    CHIP_ERROR Init(TransferRole role, TransferControlParams controlParams, RangeControlParams rangeParams, uint16_t maxBlockSize,
                    BDXStateDelegate * delegate);

    CHIP_ERROR HandleMessageReceived(uint16_t msgId, System::PacketBuffer * msgData);
    void HandleReceiveInit(System::PacketBuffer * msgData);
    void HandleMessageError(CHIP_ERROR error);

    CHIP_ERROR StartTransfer();

    TransferStates GetState() { return mState; }

private:
    bool AreParametersCompatible(TransferControlParams transferParams, RangeControlParams rangeParams, uint32_t maxBlockSize, );
    void EndTransfer(CHIP_ERROR error);

    TransferRole mRole;
    TransferControlParams mControlParams;
    RangeControlParams mRangeParams;
    uint16_t mMaxSupportedBlockSize;
    TransferStates mState;

    BDXStateDelegate * mDelegate;

    // per-transfer variables
    ControlMode mControlMode;
    uint16_t mTransferMaxBlockSize;
};

} // namespace BDX
} // namespace chip
