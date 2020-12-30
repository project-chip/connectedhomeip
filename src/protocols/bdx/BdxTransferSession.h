/**
 *    @file
 *      This file defines a state machine class that will contain the main logic
 *      for the Bulk Data Transfer protocol. It provides APIs for initializing a
 *      state machine object, handling incoming messages, and beginning or
 *      ending transfers.
 */

#pragma once

#include <core/CHIPError.h>
#include <protocols/bdx/BdxMessages.h>
#include <support/BufferReader.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

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

enum TransferRole : uint8_t
{
    kReceiver = 0,
    kSender   = 1,
};

class DLL_EXPORT TransferSession
{
public:
    class DLL_EXPORT MessagingDelegate;
    class DLL_EXPORT PlatformDelegate;

    TransferSession(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate);

    CHIP_ERROR StartTransfer(TransferRole role, const TransferInit & initMsg);
    CHIP_ERROR WaitForTransfer(TransferRole role, BitFlags<uint8_t, TransferControlFlags> xferControlOpts, uint16_t maxBlockSize);

    CHIP_ERROR HandleMessageReceived(const PacketHeader & packetHeader, System::PacketBufferHandle msg);
    void HandleReceiveInit(const System::PacketBufferHandle & msgData);

    // TODO: are these the same?
    void HandleMessageError(CHIP_ERROR error);
    void EndTransfer(CHIP_ERROR error);

    TransferStates GetState() const { return mState; }

private:
    MessagingDelegate * mMessagingDelegate;
    PlatformDelegate * mPlatformDelegate;

    TransferRole mRole;

    // pre-transfer negotiation
    BitFlags<uint8_t, TransferControlFlags> mSuppportedXferOpts;
    uint8_t mVersion;
    uint16_t mMaxSupportedBlockSize;
    TransferStates mState;

    // post-negotiation
    TransferControlFlags mControlMode;
    uint64_t mStartOffset = 0; ///< 0 represents no offset
    uint64_t mMaxLength   = 0; ///< 0 represents indefinite length

    uint16_t mTransferMaxBlockSize;
};

class DLL_EXPORT TransferSession::MessagingDelegate
{
public:
    /**
     * @brief
     *   Called when a TransferSession object needs to send a message to the other endpoint.
     *
     */
    virtual CHIP_ERROR SendMessage(MessageType msgType, System::PacketBuffer * msgBuf) = 0;

    /**
     * @brief
     *   TODO: BDX_ERROR ?
     */
    virtual CHIP_ERROR OnTransferError(CHIP_ERROR error) = 0;
};

class DLL_EXPORT TransferSession::PlatformDelegate
{
public:
    virtual CHIP_ERROR StoreBlock(Encoding::LittleEndian::Reader & blockBuf) = 0;

    virtual CHIP_ERROR ReadBlock(BufBound & buffer, uint16_t length) = 0;

    // TODO: how to pass metadata
    virtual void OnMetadataReceived() {}

    virtual CHIP_ERROR ChooseControlMode(const BitFlags<uint8_t, TransferControlFlags> & proposed,
                                         const BitFlags<uint8_t, TransferControlFlags> & supported,
                                         TransferControlFlags & choice) = 0;
};

} // namespace BDX
} // namespace chip
