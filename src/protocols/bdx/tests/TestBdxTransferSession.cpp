#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;
using namespace ::chip::bdx;
using namespace ::chip::Protocols;

namespace {
// Use this as a timestamp if not needing to test BDX timeouts.
constexpr System::Clock::Timestamp kNoAdvanceTime = System::Clock::kZero;

const TLV::Tag tlvStrTag  = TLV::ContextTag(4);
const TLV::Tag tlvListTag = TLV::ProfileTag(7777, 8888);
} // anonymous namespace

// Helper method for generating a complete TLV structure with a list containing a single tag and string
CHIP_ERROR WriteTLVString(uint8_t * buf, uint32_t bufLen, const char * data, uint32_t & written)
{
    written = 0;
    TLV::TLVWriter writer;
    writer.Init(buf, bufLen);

    {
        TLV::TLVWriter listWriter;
        ReturnErrorOnFailure(writer.OpenContainer(tlvListTag, TLV::kTLVType_List, listWriter));
        ReturnErrorOnFailure(listWriter.PutString(tlvStrTag, data));
        ReturnErrorOnFailure(writer.CloseContainer(listWriter));
    }

    ReturnErrorOnFailure(writer.Finalize());
    written = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

// Helper method: read a TLV structure with a single tag and string and verify it matches expected string.
CHIP_ERROR ReadAndVerifyTLVString(const uint8_t * dataStart, uint32_t len, const char * expected, size_t expectedLen)
{
    TLV::TLVReader reader;
    char tmp[64]      = { 0 };
    size_t readLength = 0;
    VerifyOrReturnError(sizeof(tmp) > len, CHIP_ERROR_INTERNAL);

    reader.Init(dataStart, len);
    CHIP_ERROR err = reader.Next();

    VerifyOrReturnError(reader.GetTag() == tlvListTag, CHIP_ERROR_INTERNAL);

    // Metadata must have a top-level list
    {
        TLV::TLVReader listReader;
        ReturnErrorOnFailure(reader.OpenContainer(listReader));

        ReturnErrorOnFailure(listReader.Next());

        VerifyOrReturnError(listReader.GetTag() == tlvStrTag, CHIP_ERROR_INTERNAL);
        readLength = listReader.GetLength();
        VerifyOrReturnError(readLength == expectedLen, CHIP_ERROR_INTERNAL);
        ReturnErrorOnFailure(listReader.GetString(tmp, sizeof(tmp)));
        VerifyOrReturnError(!memcmp(expected, tmp, readLength), CHIP_ERROR_INTERNAL);

        ReturnErrorOnFailure(reader.CloseContainer(listReader));
    }

    return err;
}

CHIP_ERROR AttachHeaderAndSend(TransferSession::MessageTypeData typeData, chip::System::PacketBufferHandle msgBuf,
                               TransferSession & receiver)
{
    chip::PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(typeData.ProtocolId, typeData.MessageType);

    ReturnErrorOnFailure(receiver.HandleMessageReceived(payloadHeader, std::move(msgBuf), kNoAdvanceTime));
    return CHIP_NO_ERROR;
}

// Helper method for verifying that a PacketBufferHandle contains a valid BDX header and message type matches expected.
void VerifyBdxMessageToSend(const TransferSession::OutputEvent & outEvent, MessageType expected)
{
    static_assert(std::is_same<std::underlying_type_t<decltype(expected)>, uint8_t>::value, "Cast is not safe");
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    EXPECT_FALSE(outEvent.MsgData.IsNull());
    EXPECT_EQ(outEvent.msgTypeData.ProtocolId, Protocols::BDX::Id);
    EXPECT_EQ(outEvent.msgTypeData.MessageType, static_cast<uint8_t>(expected));
}

// Helper method for verifying that a PacketBufferHandle contains a valid StatusReport message and contains a specific StatusCode.
// The msg argument is expected to begin at the message data start, not at the PayloadHeader.
void VerifyStatusReport(const System::PacketBufferHandle & msg, StatusCode expectedCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASSERT_FALSE(msg.IsNull());

    System::PacketBufferHandle msgCopy = msg.CloneData();
    ASSERT_FALSE(msgCopy.IsNull());

    SecureChannel::StatusReport report;
    err = report.Parse(std::move(msgCopy));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(report.GetGeneralCode(), SecureChannel::GeneralStatusCode::kFailure);
    EXPECT_EQ(report.GetProtocolId(), Protocols::BDX::Id);
    EXPECT_EQ(report.GetProtocolCode(), static_cast<uint16_t>(expectedCode));
}

void VerifyNoMoreOutput(TransferSession & transferSession)
{
    TransferSession::OutputEvent event;
    transferSession.PollOutput(event, kNoAdvanceTime);
    EXPECT_EQ(event.EventType, TransferSession::OutputEventType::kNone);
}

void VerifyInternalError(TransferSession & transferSession)
{
    TransferSession::OutputEvent event;
    transferSession.PollOutput(event, kNoAdvanceTime);
    EXPECT_EQ(event.EventType, TransferSession::OutputEventType::kInternalError);
}

// Helper method for initializing two TransferSession objects, generating a TransferInit message, and passing it to a responding
// TransferSession.
void SendAndVerifyTransferInit(TransferSession::OutputEvent & outEvent, System::Clock::Timeout timeout, TransferSession & initiator,
                               TransferRole initiatorRole, TransferSession::TransferInitData initData, TransferSession & responder,
                               BitFlags<TransferControlFlags> & responderControlOpts, uint16_t responderMaxBlock)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    TransferRole responderRole  = (initiatorRole == TransferRole::kSender) ? TransferRole::kReceiver : TransferRole::kSender;
    MessageType expectedInitMsg = (initiatorRole == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;

    // Initializer responder to wait for transfer
    err = responder.WaitForTransfer(responderRole, responderControlOpts, responderMaxBlock, timeout);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    VerifyNoMoreOutput(responder);

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(initiatorRole, initData, timeout);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    initiator.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    VerifyBdxMessageToSend(outEvent, expectedInitMsg);
    VerifyNoMoreOutput(initiator);

    // Verify that all parsed TransferInit fields match what was sent by the initiator
    err = AttachHeaderAndSend(outEvent.msgTypeData, std::move(outEvent.MsgData), responder);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    responder.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(responder);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kInitReceived);
    EXPECT_EQ(outEvent.transferInitData.TransferCtlFlags, initData.TransferCtlFlags);
    EXPECT_EQ(outEvent.transferInitData.MaxBlockSize, initData.MaxBlockSize);
    EXPECT_EQ(outEvent.transferInitData.StartOffset, initData.StartOffset);
    EXPECT_EQ(outEvent.transferInitData.Length, initData.Length);
    EXPECT_NE(outEvent.transferInitData.FileDesignator, nullptr);
    EXPECT_EQ(outEvent.transferInitData.FileDesLength, initData.FileDesLength);
    if (outEvent.EventType == TransferSession::OutputEventType::kInitReceived &&
        outEvent.transferInitData.FileDesignator != nullptr)
    {
        EXPECT_EQ(
            0, memcmp(initData.FileDesignator, outEvent.transferInitData.FileDesignator, outEvent.transferInitData.FileDesLength));
    }
    if (outEvent.transferInitData.Metadata != nullptr)
    {
        ASSERT_EQ(outEvent.transferInitData.MetadataLength, initData.MetadataLength);
        // Even if initData.MetadataLength is 0, it is still technically undefined behaviour to call memcmp with a null
        bool isNullAndLengthZero = initData.Metadata == nullptr && initData.MetadataLength == 0;
        if (!isNullAndLengthZero)
        {
            // Only check that metadata buffers match. The OutputEvent can still be inspected when this function returns to
            // parse the metadata and verify that it matches.
            EXPECT_EQ(0, memcmp(initData.Metadata, outEvent.transferInitData.Metadata, outEvent.transferInitData.MetadataLength));
        }
    }
}

// Helper method for sending an Accept message and verifying that the received parameters match what was sent.
// This function assumes that the acceptData struct contains transfer parameters that are valid responses to the original
// TransferInit message (for example, MaxBlockSize should be <= the TransferInit MaxBlockSize). If such parameters are invalid, the
// receiver should emit a StatusCode event instead.
//
// The acceptSender is the node that is sending the Accept message (not necessarily the same node that will send Blocks).
void SendAndVerifyAcceptMsg(TransferSession::OutputEvent & outEvent, TransferSession & acceptSender, TransferRole acceptSenderRole,
                            TransferSession::TransferAcceptData acceptData, TransferSession & acceptReceiver,
                            TransferSession::TransferInitData initData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the node sending the Accept message is also the one that will send Blocks, then this should be a ReceiveAccept message.
    MessageType expectedMsg = (acceptSenderRole == TransferRole::kSender) ? MessageType::ReceiveAccept : MessageType::SendAccept;

    err = acceptSender.AcceptTransfer(acceptData);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify Sender emits ReceiveAccept message for sending
    acceptSender.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(acceptSender);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    VerifyBdxMessageToSend(outEvent, expectedMsg);

    // Pass Accept message to acceptReceiver
    err = AttachHeaderAndSend(outEvent.msgTypeData, std::move(outEvent.MsgData), acceptReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify received ReceiveAccept.
    // Client may want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may choose to reject the
    // Transfer at this point.
    acceptReceiver.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(acceptReceiver);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kAcceptReceived);
    EXPECT_EQ(outEvent.transferAcceptData.ControlMode, acceptData.ControlMode);
    EXPECT_EQ(outEvent.transferAcceptData.MaxBlockSize, acceptData.MaxBlockSize);
    EXPECT_EQ(outEvent.transferAcceptData.StartOffset, acceptData.StartOffset);
    EXPECT_EQ(outEvent.transferAcceptData.Length, acceptData.Length);
    if (outEvent.transferAcceptData.Metadata != nullptr)
    {
        ASSERT_EQ(outEvent.transferAcceptData.MetadataLength, acceptData.MetadataLength);
        // Even if acceptData.MetadataLength is 0, it is still technically undefined behaviour to call memcmp with a null
        bool isNullAndLengthZero = acceptData.Metadata == nullptr && acceptData.MetadataLength == 0;
        if (!isNullAndLengthZero)
        {
            // Only check that metadata buffers match. The OutputEvent can still be inspected when this function returns to
            // parse the metadata and verify that it matches.
            EXPECT_EQ(
                0, memcmp(acceptData.Metadata, outEvent.transferAcceptData.Metadata, outEvent.transferAcceptData.MetadataLength));
        }
    }

    // Verify that MaxBlockSize was set appropriately
    EXPECT_LE(acceptReceiver.GetTransferBlockSize(), initData.MaxBlockSize);
}

void SendAndVerifyRejectMsg(TransferSession::OutputEvent & outEvent, TransferSession & rejectSender, StatusCode reason,
                            TransferSession & rejectReceiver)
{
    CHIP_ERROR err = rejectSender.RejectTransfer(reason);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify Sender emits status message for sending
    rejectSender.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(rejectSender);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    System::PacketBufferHandle statusReportMsg = outEvent.MsgData.Retain();
    VerifyStatusReport(std::move(outEvent.MsgData), reason);

    // Pass status message to rejectReceiver
    err = AttachHeaderAndSend(outEvent.msgTypeData, std::move(outEvent.MsgData), rejectReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify received status message.
    rejectReceiver.PollOutput(outEvent, kNoAdvanceTime);
    VerifyInternalError(rejectReceiver);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kStatusReceived);
    EXPECT_EQ(outEvent.statusData.statusCode, reason);
}

// Helper method for preparing a sending a BlockQuery message between two TransferSession objects.
void SendAndVerifyQuery(TransferSession & queryReceiver, TransferSession & querySender, TransferSession::OutputEvent & outEvent)
{
    // Verify that querySender emits BlockQuery message
    CHIP_ERROR err = querySender.PrepareBlockQuery();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    querySender.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    VerifyBdxMessageToSend(outEvent, MessageType::BlockQuery);
    VerifyNoMoreOutput(querySender);

    // Pass BlockQuery to queryReceiver and verify queryReceiver emits QueryReceived event
    err = AttachHeaderAndSend(outEvent.msgTypeData, std::move(outEvent.MsgData), queryReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    queryReceiver.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kQueryReceived);
    VerifyNoMoreOutput(queryReceiver);
}

// Helper method for preparing a sending a Block message between two TransferSession objects. The sender refers to the node that is
// sending Blocks. Uses a static counter incremented with each call. Also verifies that block data received matches what was sent.
void SendAndVerifyArbitraryBlock(TransferSession & sender, TransferSession & receiver, TransferSession::OutputEvent & outEvent,
                                 bool isEof, uint32_t inBlockCounter)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    static uint8_t dataCount = 0;
    uint16_t maxBlockSize    = sender.GetTransferBlockSize();

    EXPECT_GT(maxBlockSize, 0);
    System::PacketBufferHandle fakeDataBuf = System::PacketBufferHandle::New(maxBlockSize);
    ASSERT_FALSE(fakeDataBuf.IsNull());

    uint8_t * fakeBlockData = fakeDataBuf->Start();
    fakeBlockData[0]        = dataCount++;

    TransferSession::BlockData blockData;
    blockData.Data   = fakeBlockData;
    blockData.Length = maxBlockSize;
    blockData.IsEof  = isEof;

    MessageType expected = isEof ? MessageType::BlockEOF : MessageType::Block;

    // Provide Block data and verify sender emits Block message
    err = sender.PrepareBlock(blockData);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    sender.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    VerifyBdxMessageToSend(outEvent, expected);
    VerifyNoMoreOutput(sender);

    // Pass Block message to receiver and verify matching Block is received
    err = AttachHeaderAndSend(outEvent.msgTypeData, std::move(outEvent.MsgData), receiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    receiver.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kBlockReceived);
    EXPECT_NE(outEvent.blockdata.Data, nullptr);
    if (outEvent.EventType == TransferSession::OutputEventType::kBlockReceived && outEvent.blockdata.Data != nullptr)
    {
        EXPECT_EQ(0, memcmp(fakeBlockData, outEvent.blockdata.Data, outEvent.blockdata.Length));
        EXPECT_EQ(outEvent.blockdata.BlockCounter, inBlockCounter);
    }
    VerifyNoMoreOutput(receiver);
}

// Helper method for sending a BlockAck or BlockAckEOF, depending on the state of the receiver.
void SendAndVerifyBlockAck(TransferSession & ackReceiver, TransferSession & ackSender, TransferSession::OutputEvent & outEvent,
                           bool expectEOF)
{
    TransferSession::OutputEventType expectedEventType =
        expectEOF ? TransferSession::OutputEventType::kAckEOFReceived : TransferSession::OutputEventType::kAckReceived;
    MessageType expectedMsgType = expectEOF ? MessageType::BlockAckEOF : MessageType::BlockAck;

    // Verify PrepareBlockAck() outputs message to send
    CHIP_ERROR err = ackSender.PrepareBlockAck();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ackSender.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    VerifyBdxMessageToSend(outEvent, expectedMsgType);
    VerifyNoMoreOutput(ackSender);

    // Pass BlockAck to ackReceiver and verify it was received
    err = AttachHeaderAndSend(outEvent.msgTypeData, std::move(outEvent.MsgData), ackReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ackReceiver.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, expectedEventType);
    VerifyNoMoreOutput(ackReceiver);
}

struct TestBdxTransferSession : public ::testing::Test
{
    static void SetUpTestSuite() { EXPECT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Test a full transfer using a responding receiver and an initiating sender, receiver drive.
TEST_F(TestBdxTransferSession, TestInitiatingReceiverReceiverDrive)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;
    uint32_t numBlocksSent = 0;

    // Chosen arbitrarily for this test
    uint32_t numBlockSends         = 10;
    uint16_t proposedBlockSize     = 128;
    uint16_t testSmallerBlockSize  = 64;
    uint64_t proposedOffset        = 64;
    uint64_t proposedLength        = 0;
    System::Clock::Timeout timeout = System::Clock::Seconds16(24);

    // Chosen specifically for this test
    TransferControlFlags driveMode = TransferControlFlags::kReceiverDrive;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = driveMode;
    initOptions.MaxBlockSize     = proposedBlockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<TransferControlFlags> senderOpts;
    senderOpts.Set(driveMode);

    SendAndVerifyTransferInit(outEvent, timeout, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender,
                              senderOpts, proposedBlockSize);

    // Test metadata for Accept message
    uint8_t tlvBuf[64]    = { 0 };
    char metadataStr[11]  = { "hi_dad.txt" };
    uint32_t bytesWritten = 0;
    err                   = WriteTLVString(tlvBuf, sizeof(tlvBuf), metadataStr, bytesWritten);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Compose ReceiveAccept parameters struct and give to respondingSender
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingSender.GetControlMode();
    acceptData.StartOffset    = proposedOffset;
    acceptData.Length         = proposedLength;
    acceptData.MaxBlockSize   = testSmallerBlockSize;
    acceptData.Metadata       = tlvBuf;
    acceptData.MetadataLength = metadataSize;

    SendAndVerifyAcceptMsg(outEvent, respondingSender, TransferRole::kSender, acceptData, initiatingReceiver, initOptions);

    // Verify that MaxBlockSize was chosen correctly
    EXPECT_EQ(respondingSender.GetTransferBlockSize(), testSmallerBlockSize);
    EXPECT_EQ(respondingSender.GetTransferBlockSize(), initiatingReceiver.GetTransferBlockSize());

    // Verify parsed TLV metadata matches the original
    err = ReadAndVerifyTLVString(outEvent.transferAcceptData.Metadata,
                                 static_cast<uint32_t>(outEvent.transferAcceptData.MetadataLength), metadataStr,
                                 static_cast<uint16_t>(strlen(metadataStr)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test BlockQuery -> Block -> BlockAck
    SendAndVerifyQuery(respondingSender, initiatingReceiver, outEvent);
    SendAndVerifyArbitraryBlock(respondingSender, initiatingReceiver, outEvent, false, numBlocksSent);
    numBlocksSent++;

    // Test only one block can be prepared at a time, without receiving a response to the first
    System::PacketBufferHandle fakeBuf = System::PacketBufferHandle::New(testSmallerBlockSize);
    TransferSession::BlockData prematureBlock;
    ASSERT_FALSE(fakeBuf.IsNull());
    prematureBlock.Data   = fakeBuf->Start();
    prematureBlock.Length = testSmallerBlockSize;
    prematureBlock.IsEof  = false;
    err                   = respondingSender.PrepareBlock(prematureBlock);
    EXPECT_NE(err, CHIP_NO_ERROR);
    VerifyNoMoreOutput(respondingSender);

    // Test Ack -> Query -> Block
    SendAndVerifyBlockAck(respondingSender, initiatingReceiver, outEvent, false);

    // Test multiple Blocks sent and received (last Block is BlockEOF)
    while (numBlocksSent < numBlockSends)
    {
        bool isEof = (numBlocksSent == numBlockSends - 1);

        SendAndVerifyQuery(respondingSender, initiatingReceiver, outEvent);
        SendAndVerifyArbitraryBlock(respondingSender, initiatingReceiver, outEvent, isEof, numBlocksSent);

        numBlocksSent++;
    }

    // Verify last block was BlockEOF, then verify response BlockAckEOF message
    EXPECT_TRUE(outEvent.blockdata.IsEof);
    SendAndVerifyBlockAck(respondingSender, initiatingReceiver, outEvent, true);
}

// Partial transfer test using Sender Drive to specifically test Block -> BlockAck -> Block sequence
TEST_F(TestBdxTransferSession, TestInitiatingSenderSenderDrive)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingSender;
    TransferSession respondingReceiver;

    TransferControlFlags driveMode = TransferControlFlags::kSenderDrive;

    // Chosen arbitrarily for this test
    uint16_t transferBlockSize     = 10;
    System::Clock::Timeout timeout = System::Clock::Seconds16(24);

    // Initialize respondingReceiver
    BitFlags<TransferControlFlags> receiverOpts;
    receiverOpts.Set(driveMode);

    // Test metadata for TransferInit message
    uint8_t tlvBuf[64]    = { 0 };
    char metadataStr[11]  = { "hi_dad.txt" };
    uint32_t bytesWritten = 0;
    err                   = WriteTLVString(tlvBuf, sizeof(tlvBuf), metadataStr, bytesWritten);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = driveMode;
    initOptions.MaxBlockSize     = transferBlockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata         = tlvBuf;
    initOptions.MetadataLength   = metadataSize;

    SendAndVerifyTransferInit(outEvent, timeout, initiatingSender, TransferRole::kSender, initOptions, respondingReceiver,
                              receiverOpts, transferBlockSize);

    // Verify parsed TLV metadata matches the original
    err =
        ReadAndVerifyTLVString(outEvent.transferInitData.Metadata, static_cast<uint32_t>(outEvent.transferInitData.MetadataLength),
                               metadataStr, static_cast<uint16_t>(strlen(metadataStr)));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Compose SendAccept parameters struct and give to respondingSender
    uint16_t proposedBlockSize = transferBlockSize;
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingReceiver.GetControlMode();
    acceptData.MaxBlockSize   = proposedBlockSize;
    acceptData.StartOffset    = 0; // not used in SendAccept
    acceptData.Length         = 0; // not used in SendAccept
    acceptData.Metadata       = nullptr;
    acceptData.MetadataLength = 0;

    SendAndVerifyAcceptMsg(outEvent, respondingReceiver, TransferRole::kReceiver, acceptData, initiatingSender, initOptions);

    uint32_t numBlocksSent = 0;
    // Test multiple Block -> BlockAck -> Block
    for (int i = 0; i < 3; i++)
    {
        SendAndVerifyArbitraryBlock(initiatingSender, respondingReceiver, outEvent, false, numBlocksSent);
        SendAndVerifyBlockAck(initiatingSender, respondingReceiver, outEvent, false);
        numBlocksSent++;
    }

    SendAndVerifyArbitraryBlock(initiatingSender, respondingReceiver, outEvent, true, numBlocksSent);
    SendAndVerifyBlockAck(initiatingSender, respondingReceiver, outEvent, true);
}

// Test that calls to AcceptTransfer() with bad parameters result in an error.
TEST_F(TestBdxTransferSession, TestBadAcceptMessageFields)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;

    uint16_t maxBlockSize          = 64;
    TransferControlFlags driveMode = TransferControlFlags::kReceiverDrive;
    uint64_t commonLength          = 0;
    uint64_t commonOffset          = 0;
    System::Clock::Timeout timeout = System::Clock::Seconds16(24);

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = driveMode;
    initOptions.MaxBlockSize     = maxBlockSize;
    initOptions.StartOffset      = commonOffset;
    initOptions.Length           = commonLength;
    char testFileDes[9]          = { "test.txt" }; // arbitrary file designator
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata         = nullptr;
    initOptions.MetadataLength   = 0;

    // Responder parameters
    BitFlags<TransferControlFlags> responderControl;
    responderControl.Set(driveMode);

    SendAndVerifyTransferInit(outEvent, timeout, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender,
                              responderControl, maxBlockSize);

    // Verify AcceptTransfer() returns error for choosing larger max block size
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = driveMode;
    acceptData.MaxBlockSize = static_cast<uint16_t>(maxBlockSize + 1); // invalid if larger than proposed
    acceptData.StartOffset  = commonOffset;
    acceptData.Length       = commonLength;
    err                     = respondingSender.AcceptTransfer(acceptData);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Verify AcceptTransfer() returns error for choosing unsupported transfer control mode
    TransferSession::TransferAcceptData acceptData2;
    acceptData2.ControlMode  = (driveMode == TransferControlFlags::kReceiverDrive) ? TransferControlFlags::kSenderDrive
                                                                                   : TransferControlFlags::kReceiverDrive;
    acceptData2.MaxBlockSize = maxBlockSize;
    acceptData2.StartOffset  = commonOffset;
    acceptData2.Length       = commonLength;
    err                      = respondingSender.AcceptTransfer(acceptData2);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

// Test that a TransferSession will emit kTransferTimeout if the specified timeout is exceeded while waiting for a response.
TEST_F(TestBdxTransferSession, TestTimeout)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession initiator;
    TransferSession::OutputEvent outEvent;

    System::Clock::Timeout timeout     = System::Clock::Milliseconds32(24);
    System::Clock::Timestamp startTime = System::Clock::Milliseconds64(100);
    System::Clock::Timestamp endTime   = System::Clock::Milliseconds64(124);

    // Initialize struct with arbitrary TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = 64;
    initOptions.StartOffset      = 0;
    initOptions.Length           = 0;
    char testFileDes[9]          = { "test.txt" }; // arbitrary file designator
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata         = nullptr;
    initOptions.MetadataLength   = 0;

    TransferRole role = TransferRole::kReceiver;

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(role, initOptions, timeout);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // First PollOutput() should output the TransferInit message
    initiator.PollOutput(outEvent, startTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    MessageType expectedInitMsg = (role == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;
    VerifyBdxMessageToSend(outEvent, expectedInitMsg);

    // Second PollOutput() with no call to HandleMessageReceived() should result in a timeout.
    initiator.PollOutput(outEvent, endTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kTransferTimeout);
}

// Test that sending the same block twice (with same block counter) results in a StatusReport message with BadBlockCounter. Also
// test that receiving the StatusReport ends the transfer on the other node.
TEST_F(TestBdxTransferSession, TestDuplicateBlockError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession::OutputEvent eventWithBlock;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;

    uint8_t fakeData[64] = { 0 };
    uint8_t fakeDataLen  = sizeof(fakeData);
    uint16_t blockSize   = sizeof(fakeData);

    // Chosen arbitrarily for this test
    uint64_t proposedOffset        = 64;
    uint64_t proposedLength        = 0;
    System::Clock::Timeout timeout = System::Clock::Seconds16(24);

    // Chosen specifically for this test
    TransferControlFlags driveMode = TransferControlFlags::kReceiverDrive;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = driveMode;
    initOptions.MaxBlockSize     = blockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<TransferControlFlags> senderOpts;
    senderOpts.Set(driveMode);

    SendAndVerifyTransferInit(outEvent, timeout, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender,
                              senderOpts, blockSize);

    // Compose ReceiveAccept parameters struct and give to respondingSender
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingSender.GetControlMode();
    acceptData.StartOffset    = proposedOffset;
    acceptData.Length         = proposedLength;
    acceptData.MaxBlockSize   = blockSize;
    acceptData.Metadata       = nullptr;
    acceptData.MetadataLength = 0;

    SendAndVerifyAcceptMsg(outEvent, respondingSender, TransferRole::kSender, acceptData, initiatingReceiver, initOptions);

    SendAndVerifyQuery(respondingSender, initiatingReceiver, outEvent);

    TransferSession::BlockData blockData;
    blockData.Data   = fakeData;
    blockData.Length = fakeDataLen;
    blockData.IsEof  = false;

    // Provide Block data and verify sender emits Block message
    err = respondingSender.PrepareBlock(blockData);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    respondingSender.PollOutput(eventWithBlock, kNoAdvanceTime);
    EXPECT_EQ(eventWithBlock.EventType, TransferSession::OutputEventType::kMsgToSend);
    VerifyBdxMessageToSend(eventWithBlock, MessageType::Block);
    VerifyNoMoreOutput(respondingSender);
    System::PacketBufferHandle blockCopy =
        System::PacketBufferHandle::NewWithData(eventWithBlock.MsgData->Start(), eventWithBlock.MsgData->DataLength());

    // Pass Block message to receiver and verify matching Block is received
    err = AttachHeaderAndSend(eventWithBlock.msgTypeData, std::move(eventWithBlock.MsgData), initiatingReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kBlockReceived);
    EXPECT_NE(outEvent.blockdata.Data, nullptr);
    VerifyNoMoreOutput(initiatingReceiver);

    SendAndVerifyQuery(respondingSender, initiatingReceiver, outEvent);

    // Verify receiving same Block twice fails and results in StatusReport event, and then InternalError event
    err = AttachHeaderAndSend(eventWithBlock.msgTypeData, std::move(blockCopy), initiatingReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kMsgToSend);
    System::PacketBufferHandle statusReportMsg               = outEvent.MsgData.Retain();
    TransferSession::MessageTypeData statusReportMsgTypeData = outEvent.msgTypeData;
    VerifyStatusReport(std::move(outEvent.MsgData), StatusCode::kBadBlockCounter);

    // All subsequent PollOutput() calls should return kInternalError
    for (int i = 0; i < 5; ++i)
    {
        initiatingReceiver.PollOutput(outEvent, kNoAdvanceTime);
        EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kInternalError);
        EXPECT_EQ(outEvent.statusData.statusCode, StatusCode::kBadBlockCounter);
    }

    err = AttachHeaderAndSend(statusReportMsgTypeData, std::move(statusReportMsg), respondingSender);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    respondingSender.PollOutput(outEvent, kNoAdvanceTime);
    EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kStatusReceived);
    EXPECT_EQ(outEvent.statusData.statusCode, StatusCode::kBadBlockCounter);

    // All subsequent PollOutput() calls should return kInternalError
    for (int i = 0; i < 5; ++i)
    {
        respondingSender.PollOutput(outEvent, kNoAdvanceTime);
        EXPECT_EQ(outEvent.EventType, TransferSession::OutputEventType::kInternalError);
        EXPECT_EQ(outEvent.statusData.statusCode, StatusCode::kBadBlockCounter);
    }
}

TEST_F(TestBdxTransferSession, TestRejectTransfer)
{
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;

    // Chosen arbitrarily for this test
    uint16_t proposedBlockSize     = 128;
    System::Clock::Timeout timeout = System::Clock::Seconds16(24);
    TransferControlFlags driveMode = TransferControlFlags::kReceiverDrive;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = driveMode;
    initOptions.MaxBlockSize     = proposedBlockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<TransferControlFlags> senderOpts;
    senderOpts.Set(driveMode);

    SendAndVerifyTransferInit(outEvent, timeout, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender,
                              senderOpts, proposedBlockSize);

    // Reject the transfer with a status
    SendAndVerifyRejectMsg(outEvent, respondingSender, StatusCode::kResponderBusy, initiatingReceiver);
}
