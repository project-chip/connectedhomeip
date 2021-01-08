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

    TransferSession();

    // TODO: should we return error if initMsg breaks spec?
    // TODO: could break this up into StartSend() and StartReceive()
    CHIP_ERROR StartTransfer(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate, TransferRole role,
                             const TransferInit & initMsg);
    CHIP_ERROR WaitForTransfer(MessagingDelegate * msgDelegate, PlatformDelegate * platformDelegate, TransferRole role,
                               BitFlags<uint8_t, TransferControlFlags> xferControlOpts, uint16_t maxBlockSize);

    CHIP_ERROR HandleMessageReceived(System::PacketBufferHandle msg);

    // TODO: are these the same?
    void HandleMessageError(CHIP_ERROR error);
    void EndTransfer(CHIP_ERROR error);

    TransferStates GetState() const { return mState; }

private:
    void HandleReceiveInit(System::PacketBufferHandle msgData);
    void HandleReceiveAccept(System::PacketBufferHandle msgData);

    CHIP_ERROR AttachHeaderAndSend(MessageType msgType, System::PacketBufferHandle msgBuf);

    MessagingDelegate * mMessagingDelegate;
    PlatformDelegate * mPlatformDelegate;

    TransferStates mState;
    TransferRole mRole;

    // pre-transfer negotiation
    BitFlags<uint8_t, TransferControlFlags> mSuppportedXferOpts;
    uint8_t mVersion;
    uint16_t mMaxSupportedBlockSize;

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
    virtual CHIP_ERROR SendMessage(System::PacketBufferHandle msg) = 0;

    /**
     * @brief
     *   TODO: BDX_ERROR ?
     */
    virtual CHIP_ERROR OnTransferError(StatusCode code) = 0;

    virtual ~MessagingDelegate() {}
};

class DLL_EXPORT TransferSession::PlatformDelegate
{
public:
    virtual CHIP_ERROR StoreBlock(Encoding::LittleEndian::Reader & blockBuf) = 0;

    virtual CHIP_ERROR ReadBlock(BufBound & buffer, uint16_t length) = 0;

    // TODO: how to pass file
    virtual void OnFileDesignatorReceived() = 0;

    // TODO: how to pass metadata
    virtual void OnMetadataReceived() {}

    virtual CHIP_ERROR ChooseControlMode(const BitFlags<uint8_t, TransferControlFlags> & proposed,
                                         const BitFlags<uint8_t, TransferControlFlags> & supported,
                                         TransferControlFlags & choice) = 0;

    virtual ~PlatformDelegate() {}
};

} // namespace BDX
} // namespace chip
