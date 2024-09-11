/**
 *    @file
 *      This file defines a TransferSession state machine that contains the main logic governing a Bulk Data Transfer session. It
 *      provides APIs for starting a transfer or preparing to receive a transfer request, providing input to be processed, and
 *      accessing output data (including messages to be sent, message data received by the TransferSession, or state information).
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <protocols/bdx/BdxMessages.h>
#include <system/SystemClock.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

#include <type_traits>

namespace chip {
namespace bdx {

enum class TransferRole : uint8_t
{
    kReceiver = 0,
    kSender   = 1,
};

class DLL_EXPORT TransferSession
{
public:
    enum class OutputEventType : uint16_t
    {
        kNone = 0,
        kMsgToSend,
        kInitReceived,
        kAcceptReceived,
        kBlockReceived,
        kQueryReceived,
        kQueryWithSkipReceived,
        kAckReceived,
        kAckEOFReceived,
        kStatusReceived,
        kInternalError,
        kTransferTimeout
    };

    struct TransferInitData
    {
        TransferControlFlags TransferCtlFlags;

        uint16_t MaxBlockSize = 0;
        uint64_t StartOffset  = 0;
        uint64_t Length       = 0;

        const uint8_t * FileDesignator = nullptr;
        uint16_t FileDesLength         = 0;

        // Additional metadata (optional, TLV format)
        const uint8_t * Metadata = nullptr;
        size_t MetadataLength    = 0;
    };

    struct TransferAcceptData
    {
        TransferControlFlags ControlMode;

        uint16_t MaxBlockSize = 0;
        uint64_t StartOffset  = 0; ///< Not used for SendAccept message
        uint64_t Length       = 0; ///< Not used for SendAccept message

        // Additional metadata (optional, TLV format)
        const uint8_t * Metadata = nullptr;
        size_t MetadataLength    = 0;
    };

    struct StatusReportData
    {
        StatusCode statusCode;
    };

    struct BlockData
    {
        const uint8_t * Data  = nullptr;
        size_t Length         = 0;
        bool IsEof            = false;
        uint32_t BlockCounter = 0;
    };

    struct MessageTypeData
    {
        Protocols::Id ProtocolId; // Should only ever be SecureChannel or BDX
        uint8_t MessageType;

        MessageTypeData() : ProtocolId(Protocols::NotSpecified), MessageType(0) {}

        bool HasProtocol(Protocols::Id protocol) const { return ProtocolId == protocol; }
        bool HasMessageType(uint8_t type) const { return MessageType == type; }
        template <typename TMessageType, typename = std::enable_if_t<std::is_enum<TMessageType>::value>>
        bool HasMessageType(TMessageType type) const
        {
            return HasProtocol(Protocols::MessageTypeTraits<TMessageType>::ProtocolId()) && HasMessageType(to_underlying(type));
        }
    };

    struct TransferSkipData
    {
        uint64_t BytesToSkip = 0;
    };

    /**
     * @brief
     *   All output data processed by the TransferSession object will be passed to the caller using this struct via PollOutput().
     *
     *   NOTE: Some sub-structs may contain pointers to data in a PacketBuffer (see Blockdata). In this case, the MsgData field MUST
     *   be populated with a PacketBufferHandle that encapsulates the respective PacketBuffer, in order to ensure valid memory
     *   access.
     *
     *   NOTE: MsgData can contain messages that have been received or messages that should be sent by the caller. The underlying
     *   buffer will always start at the data, never at the payload header. Outgoing messages do not have a header prepended.
     */
    struct OutputEvent
    {
        OutputEventType EventType;
        System::PacketBufferHandle MsgData;
        union
        {
            TransferInitData transferInitData;
            TransferAcceptData transferAcceptData;
            BlockData blockdata;
            StatusReportData statusData;
            MessageTypeData msgTypeData;
            TransferSkipData bytesToSkip;
        };

        OutputEvent() : EventType(OutputEventType::kNone) { statusData = { StatusCode::kUnknown }; }
        OutputEvent(OutputEventType type) : EventType(type) { statusData = { StatusCode::kUnknown }; }

        const char * ToString(OutputEventType outputEventType);

        static OutputEvent TransferInitEvent(TransferInitData data, System::PacketBufferHandle msg);
        static OutputEvent TransferAcceptEvent(TransferAcceptData data);
        static OutputEvent TransferAcceptEvent(TransferAcceptData data, System::PacketBufferHandle msg);
        static OutputEvent BlockDataEvent(BlockData data, System::PacketBufferHandle msg);
        static OutputEvent StatusReportEvent(OutputEventType type, StatusReportData data);
        static OutputEvent MsgToSendEvent(MessageTypeData typeData, System::PacketBufferHandle msg);
        static OutputEvent QueryWithSkipEvent(TransferSkipData bytesToSkip);
    };

    /**
     * @brief
     *   Indicates the presence of pending output and includes any data for the caller to take action on.
     *
     *   This method should be called frequently in order to be notified about any messages received. It should also be called after
     *   most other methods in order to notify the user of any message that needs to be sent, or errors that occurred internally.
     *
     *   It is possible that consecutive calls to this method may emit different outputs depending on the state of the
     *   TransferSession object.
     *
     *   Note that if the type outputted is kMsgToSend, the caller is expected to send the message immediately, and the session
     *   timeout timer will begin at curTime.
     *
     *   See OutputEventType for all possible output event types.
     *
     * @param event     Reference to an OutputEvent struct that will be filled out with any pending output data
     * @param curTime   Current time
     */
    void PollOutput(OutputEvent & event, System::Clock::Timestamp curTime);

    /**
     * @brief
     *   Gets the pending output event from the transfer session in the event param passed in by the caller.
     *   The output event may contain some data for the caller to act upon. If there is no pending output event,
     *   the caller will get an event of type OutputEventType::kNone.
     *
     *   It is possible that consecutive calls to this method may emit different outputs depending on the state of the
     *   TransferSession object.  The caller is generally expected to keep calling this method until it gets an event of type
     * OutputEventType::kNone.
     *
     *   If the output event type is kMsgToSend, the caller is expected to send the message immediately on the
     *   relevant exchange.  In this case the BDX session timeout timer will start when GetNextAction is called.
     *
     *   See OutputEventType for all possible output event types.
     *
     * @param event     Reference to an OutputEvent struct that will be filled out with any pending output event data
     */
    void GetNextAction(OutputEvent & event);

    /**
     * @brief
     *   Initializes the TransferSession object and prepares a TransferInit message (emitted via PollOutput()).
     *
     *   A TransferSession object must be initialized with either StartTransfer() or WaitForTransfer().
     *
     * @param role      Inidcates whether this object will be sending or receiving data
     * @param initData  Data for initializing this object and for populating a TransferInit message
     *                  The role parameter will determine whether to populate a ReceiveInit or SendInit
     * @param timeout   The amount of time to wait for a response before considering the transfer failed
     *
     * @return CHIP_ERROR Result of initialization and preparation of a TransferInit message. May also indicate if the
     *                    TransferSession object is unable to handle this request.
     */
    CHIP_ERROR StartTransfer(TransferRole role, const TransferInitData & initData, System::Clock::Timeout timeout);

    /**
     * @brief
     *   Initialize the TransferSession object and prepare to receive a TransferInit message at some point.
     *
     *   A TransferSession object must be initialized with either StartTransfer() or WaitForTransfer().
     *
     * @param role            Inidcates whether this object will be sending or receiving data
     * @param xferControlOpts Indicates all supported control modes. Used to respond to a TransferInit message
     * @param maxBlockSize    The max Block size that this object supports.
     * @param timeout         The amount of time to wait for a response before considering the transfer failed
     *
     * @return CHIP_ERROR Result of initialization. May also indicate if the TransferSession object is unable to handle this
     *                    request.
     */
    CHIP_ERROR WaitForTransfer(TransferRole role, BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                               System::Clock::Timeout timeout);

    /**
     * @brief
     *   Indicate that all transfer parameters are acceptable and prepare a SendAccept or ReceiveAccept message (depending on role).
     *
     * @param acceptData Data used to populate an Accept message (some fields may differ from the original Init message)
     *
     * @return CHIP_ERROR Result of preparation of an Accept message. May also indicate if the TransferSession object is unable to
     *                    handle this request.
     */
    CHIP_ERROR AcceptTransfer(const TransferAcceptData & acceptData);

    /**
     * @brief
     *   Reject a TransferInit message. Use Reset() to prepare this object for another transfer.
     *
     * @param reason A StatusCode indicating the reason for rejecting the transfer
     *
     * @return CHIP_ERROR The result of the preparation of a StatusReport message. May also indicate if the TransferSession object
     *                    is unable to handle this request.
     */
    CHIP_ERROR RejectTransfer(StatusCode reason);

    /**
     * @brief
     *   Prepare a BlockQuery message. The Block counter will be populated automatically.
     *
     * @return CHIP_ERROR The result of the preparation of a BlockQuery message. May also indicate if the TransferSession object
     *                    is unable to handle this request.
     */
    CHIP_ERROR PrepareBlockQuery();

    /**
     * @brief
     *   Prepare a BlockQueryWithSkip message. The Block counter will be populated automatically.
     *
     * @param bytesToSkip Number of bytes to seek skip
     *
     * @return CHIP_ERROR The result of the preparation of a BlockQueryWithSkip message. May also indicate if the TransferSession
     * object is unable to handle this request.
     */
    CHIP_ERROR PrepareBlockQueryWithSkip(const uint64_t & bytesToSkip);

    /**
     * @brief
     *   Prepare a Block message. The Block counter will be populated automatically.
     *
     * @param inData Contains data for filling out the Block message
     *
     * @return CHIP_ERROR The result of the preparation of a Block message. May also indicate if the TransferSession object
     *                    is unable to handle this request.
     */
    CHIP_ERROR PrepareBlock(const BlockData & inData);

    /**
     * @brief
     *   Prepare a BlockAck message. The Block counter will be populated automatically.
     *
     * @return CHIP_ERROR The result of the preparation of a BlockAck message. May also indicate if the TransferSession object
     *                    is unable to handle this request.
     */
    CHIP_ERROR PrepareBlockAck();

    /**
     * @brief
     *   Prematurely end a transfer with a StatusReport. Must still call Reset() to prepare the TransferSession for another
     *   transfer.
     *
     * @param reason The StatusCode reason for ending the transfer.
     *
     * @return CHIP_ERROR May return an error if there is no transfer in progress.
     */
    CHIP_ERROR AbortTransfer(StatusCode reason);

    /**
     * @brief
     *   Reset all TransferSession parameters. The TransferSession object must then be re-initialized with StartTransfer() or
     *   WaitForTransfer().
     */
    void Reset();

    /**
     * @brief
     *   Process a message intended for this TransferSession object.
     *
     * @param payloadHeader A PayloadHeader containing the Protocol type and Message Type
     * @param msg           A PacketBufferHandle pointing to the message buffer to process. May be BDX or StatusReport protocol.
     *                      Buffer is expected to start at data (not header).
     * @param curTime       Current time
     *
     * @return CHIP_ERROR Indicates any problems in decoding the message, or if the message is not of the BDX or StatusReport
     *                    protocols.
     */
    CHIP_ERROR HandleMessageReceived(const PayloadHeader & payloadHeader, System::PacketBufferHandle msg,
                                     System::Clock::Timestamp curTime);

    TransferControlFlags GetControlMode() const { return mControlMode; }
    uint64_t GetStartOffset() const { return mStartOffset; }
    uint64_t GetTransferLength() const { return mTransferLength; }
    uint16_t GetTransferBlockSize() const { return mTransferMaxBlockSize; }
    uint32_t GetNextBlockNum() const { return mNextBlockNum; }
    uint32_t GetNextQueryNum() const { return mNextQueryNum; }
    size_t GetNumBytesProcessed() const { return mNumBytesProcessed; }
    const uint8_t * GetFileDesignator(uint16_t & fileDesignatorLen) const
    {
        fileDesignatorLen = mTransferRequestData.FileDesLength;
        return mTransferRequestData.FileDesignator;
    }

    TransferSession();

private:
    enum class TransferState : uint8_t
    {
        kUnitialized,
        kAwaitingInitMsg,
        kAwaitingAccept,
        kNegotiateTransferParams,
        kTransferInProgress,
        kAwaitingEOFAck,
        kReceivedEOF,
        kTransferDone,
        kErrorState,
    };

    // Incoming message handlers
    CHIP_ERROR HandleBdxMessage(const PayloadHeader & header, System::PacketBufferHandle msg);
    CHIP_ERROR HandleStatusReportMessage(const PayloadHeader & header, System::PacketBufferHandle msg);
    void HandleTransferInit(MessageType msgType, System::PacketBufferHandle msgData);
    void HandleReceiveAccept(System::PacketBufferHandle msgData);
    void HandleSendAccept(System::PacketBufferHandle msgData);
    void HandleBlockQuery(System::PacketBufferHandle msgData);
    void HandleBlockQueryWithSkip(System::PacketBufferHandle msgData);
    void HandleBlock(System::PacketBufferHandle msgData);
    void HandleBlockEOF(System::PacketBufferHandle msgData);
    void HandleBlockAck(System::PacketBufferHandle msgData);
    void HandleBlockAckEOF(System::PacketBufferHandle msgData);

    /**
     * @brief
     *   Used when handling a TransferInit message. Determines if there are any compatible Transfer control modes between the two
     *   transfer peers.
     */
    void ResolveTransferControlOptions(const BitFlags<TransferControlFlags> & proposed);

    /**
     * @brief
     *   Used when handling an Accept message. Verifies that the chosen control mode is compatible with the orignal supported modes.
     */
    CHIP_ERROR VerifyProposedMode(const BitFlags<TransferControlFlags> & proposed);

    void PrepareStatusReport(StatusCode code);
    bool IsTransferLengthDefinite() const;

    OutputEventType mPendingOutput = OutputEventType::kNone;
    TransferState mState           = TransferState::kUnitialized;
    TransferRole mRole;

    // Indicate supported options pre- transfer accept
    BitFlags<TransferControlFlags> mSuppportedXferOpts;
    uint16_t mMaxSupportedBlockSize = 0;

    // Used to govern transfer once it has been accepted
    TransferControlFlags mControlMode;
    uint8_t mTransferVersion       = 0;
    uint64_t mStartOffset          = 0; ///< 0 represents no offset
    uint64_t mTransferLength       = 0; ///< 0 represents indefinite length
    uint16_t mTransferMaxBlockSize = 0;

    // Used to store event data before it is emitted via PollOutput()
    System::PacketBufferHandle mPendingMsgHandle;
    StatusReportData mStatusReportData;
    TransferInitData mTransferRequestData;
    TransferAcceptData mTransferAcceptData;
    BlockData mBlockEventData;
    MessageTypeData mMsgTypeData;
    TransferSkipData mBytesToSkip;

    size_t mNumBytesProcessed = 0;

    uint32_t mLastBlockNum = 0;
    uint32_t mNextBlockNum = 0;
    uint32_t mLastQueryNum = 0;
    uint32_t mNextQueryNum = 0;

    System::Clock::Timeout mTimeout            = System::Clock::kZero;
    System::Clock::Timestamp mTimeoutStartTime = System::Clock::kZero;
    bool mShouldInitTimeoutStart               = true;
    bool mAwaitingResponse                     = false;
};

} // namespace bdx
} // namespace chip
