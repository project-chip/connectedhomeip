#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>

#include <nlunit-test.h>

#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

using namespace ::chip;
using namespace ::chip::bdx;

// Helper method for generating a complete TLV structure with just a single tag and string
CHIP_ERROR WriteTLVString(uint8_t * buf, uint32_t bufLen, const char * data, uint64_t tag, uint32_t & written)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    written        = 0;
    TLV::TLVWriter writer;
    writer.Init(buf, bufLen);
    err = writer.PutString(tag, data);
    SuccessOrExit(err);
    err = writer.Finalize();
    SuccessOrExit(err);
    written = writer.GetLengthWritten();

exit:
    return err;
}

// Helper method: read a TLV structure with a single tag and string and verify it matches expected string.
CHIP_ERROR ReadAndVerifyTLVString(nlTestSuite * inSuite, void * inContext, const uint8_t * dataStart, uint32_t len, uint64_t tag,
                                  const char * expected, uint16_t expectedLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    char tmp[64]    = { 0 };
    uint32_t strlen = 0;
    VerifyOrExit(sizeof(tmp) > len, err = CHIP_ERROR_INTERNAL);

    reader.Init(dataStart, len);
    err = reader.Next();
    SuccessOrExit(err);
    VerifyOrExit(reader.GetTag() == tag, err = CHIP_ERROR_INTERNAL);
    strlen = reader.GetLength();
    VerifyOrExit(strlen == expectedLen, err = CHIP_ERROR_INTERNAL);
    err = reader.GetString(tmp, sizeof(tmp));
    SuccessOrExit(err);
    VerifyOrExit(!memcmp(expected, tmp, strlen), err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}

// Helper method for verifying that a PacketBufferHandle contains a valid BDX header and message type matches expected.
void VerifyMessageType(nlTestSuite * inSuite, void * inContext, const System::PacketBufferHandle & msg, MessageType expected)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    PayloadHeader payloadHeader;

    if (msg.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }

    err = payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, payloadHeader.GetProtocolID() == Protocols::kProtocol_BDX);
    NL_TEST_ASSERT(inSuite, payloadHeader.GetMessageType() == expected);
}

// Helper method for preparing a sending a BlockQuery message between two TransferSession objects.
void SendAndVerifyQuery(nlTestSuite * inSuite, void * inContext, TransferSession & sender, TransferSession & receiver,
                        TransferSession::OutputEvent & outEvent)
{
    // Verify that receiver emits BlockQuery message
    CHIP_ERROR err = receiver.PrepareBlockQuery();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, kBdxMsg_BlockQuery);

    // Pass BlockQuery to sender and verify sender emits QueryReceived event
    err = sender.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    sender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_QueryReceived);
}

// Helper method for preparing a sending a Block message between two TransferSession objects. The sender refers to the node that is
// sending Blocks. Uses a static counter incremented with each call. Also verifies that block data received matches what was sent.
void SendAndVerifyArbitraryBlock(nlTestSuite * inSuite, void * inContext, TransferSession & sender, TransferSession & receiver,
                                 TransferSession::OutputEvent & outEvent, bool isEof)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    static uint8_t dataCount = 0;
    uint16_t maxBlockSize    = sender.GetTransferBlockSize();

    NL_TEST_ASSERT(inSuite, maxBlockSize > 0);
    System::PacketBufferHandle fakeDataBuf = System::PacketBuffer::NewWithAvailableSize(maxBlockSize);
    if (fakeDataBuf.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }

    uint8_t * fakeBlockData = fakeDataBuf->Start();
    fakeBlockData[0]        = dataCount++;

    TransferSession::BlockData blockData;
    blockData.Data   = fakeBlockData;
    blockData.Length = maxBlockSize;
    blockData.IsEof  = isEof;

    MessageType expected = isEof ? kBdxMsg_BlockEOF : kBdxMsg_Block;

    // Provide Block data and verify sender emits Block message
    err = sender.PrepareBlock(blockData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    sender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, expected);

    // Pass Block message to receiver and verify matching Block is received
    err = receiver.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_BlockReceived);
    NL_TEST_ASSERT(inSuite, outEvent.blockdata.Data != nullptr);
    NL_TEST_ASSERT(inSuite, !memcmp(fakeBlockData, outEvent.blockdata.Data, outEvent.blockdata.Length));
}

// Helper method for sending a BlockAck or BlockAckEOF, depending on the state of the receiver. The sender refers to the node that
// is sending Blocks.
void SendAndVerifyBlockAck(nlTestSuite * inSuite, void * inContext, TransferSession & sender, TransferSession & receiver,
                           TransferSession::OutputEvent & outEvent, bool expectEOF)
{
    TransferSession::OutputEventFlags expectedEventType =
        expectEOF ? TransferSession::kOutput_AckEOFReceived : TransferSession::kOutput_AckReceived;
    MessageType expectedMsgType = expectEOF ? kBdxMsg_BlockAckEOF : kBdxMsg_BlockAck;

    // Verify PrepareBlockAck() outputs message to send
    CHIP_ERROR err = receiver.PrepareBlockAck();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, expectedMsgType);

    // Pass BlockAck to sender and verify it was received
    err = sender.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    sender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == expectedEventType);
}

// Test a full transfer using a responding receiver and an initiating sender, receiver drive.
void TestInitiatingReceiverReceiverDrive(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;
    uint32_t numBlocksSent = 0;

    // Chosen arbitrarily for this test
    uint32_t numBlockSends     = 4;
    uint16_t transferBlockSize = 10;
    uint64_t proposedOffset    = 64;
    uint64_t proposedLength    = 0;

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = kControl_ReceiverDrive;
    initOptions.MaxBlockSize        = transferBlockSize;
    char testFileDes[9]             = { "test.txt" };
    initOptions.FileDesLength       = sizeof(testFileDes) - 1;
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);

    // Verify receiver outputs ReceiveInit message after StartTransfer()
    err = initiatingReceiver.StartTransfer(kRole_Receiver, initOptions);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, kBdxMsg_ReceiveInit);

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<uint8_t, TransferControlFlags> senderOpts;
    senderOpts.Set(kControl_ReceiverDrive);
    err = respondingSender.WaitForTransfer(kRole_Sender, senderOpts, transferBlockSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = respondingSender.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Responder may want to read any of the fields of the TransferInit for approval.
    // Verify File Designator matches. Verify that the respondingSenser automatically set ReceiverDrive
    // for the chosen ControlMode.
    respondingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_InitReceived);
    NL_TEST_ASSERT(inSuite, respondingSender.GetControlMode() == kControl_ReceiverDrive);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.FileDesignator != nullptr);
    NL_TEST_ASSERT(inSuite,
                   !memcmp(testFileDes, outEvent.transferInitData.FileDesignator, outEvent.transferInitData.FileDesLength));

    // Create metadata
    uint8_t tlvBuf[64]    = { 0 };
    char metadataStr[11]  = { "hi_dad.txt" };
    uint64_t tlvTag       = TLV::ProfileTag(1234, 5678);
    uint32_t bytesWritten = 0;
    err                   = WriteTLVString(tlvBuf, sizeof(tlvBuf), metadataStr, tlvTag, bytesWritten);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Compose ReceiveAccept parameters struct and give to respondingSender
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingSender.GetControlMode();
    acceptData.StartOffset    = proposedOffset;
    acceptData.Length         = 0; // Indefinite length
    acceptData.MaxBlockSize   = transferBlockSize;
    acceptData.Metadata       = tlvBuf;
    acceptData.MetadataLength = metadataSize;
    err                       = respondingSender.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify Sender emits ReceiveAccept message for sending
    respondingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, kBdxMsg_ReceiveAccept);

    // Pass Accept message to initiatingReceiver
    err = initiatingReceiver.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify received ReceiveAccept.
    // Client may want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may choose to reject the
    // Transfer at this point.
    initiatingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_AcceptReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.ControlMode == kControl_ReceiverDrive);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.StartOffset == proposedOffset);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.Length == proposedLength);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.MaxBlockSize <= transferBlockSize);
    err = ReadAndVerifyTLVString(inSuite, inContext, outEvent.transferAcceptData.Metadata,
                                 outEvent.transferAcceptData.MetadataLength, tlvTag, metadataStr, sizeof(metadataStr) - 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test BlockQuery -> Block -> BlockAck
    SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);
    SendAndVerifyArbitraryBlock(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, false);
    numBlocksSent++;
    SendAndVerifyBlockAck(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, false);

    // Test multiple Blocks sent and received (last Block is BlockEOF)
    while (numBlocksSent < numBlockSends)
    {
        bool isEof = (numBlocksSent == numBlockSends - 1);

        SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);
        SendAndVerifyArbitraryBlock(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, isEof);

        numBlocksSent++;
    }

    // Verify last block was BlockEOF, then verify response BlockAckEOF message
    NL_TEST_ASSERT(inSuite, outEvent.blockdata.IsEof == true);
    SendAndVerifyBlockAck(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, true);
}

// Partial transfer test using Sender Drive to specifically test Block -> BlockAck -> Block sequence
void TestInitiatingSenderSenderDrive(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingSender;
    TransferSession respondingReceiver;

    TransferControlFlags driveMode = kControl_SenderDrive;

    // Chosen arbitrarily for this test
    uint16_t transferBlockSize = 10;

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = driveMode;
    initOptions.MaxBlockSize        = transferBlockSize;
    char testFileDes[9]             = { "test.txt" };
    initOptions.FileDesLength       = sizeof(testFileDes) - 1;
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata            = nullptr;
    initOptions.MetadataLength      = 0;

    // Verify sender outputs SendInit message after StartTransfer()
    err = initiatingSender.StartTransfer(kRole_Sender, initOptions);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, kBdxMsg_SendInit);

    // Initialize respondingReceiver
    BitFlags<uint8_t, TransferControlFlags> receiverOpts;
    receiverOpts.Set(driveMode);
    err = respondingReceiver.WaitForTransfer(kRole_Receiver, receiverOpts, transferBlockSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Responder may want to read any of the fields of the TransferInit for approval.
    // Verify File Designator matches. Verify that the respondingSenser automatically set ReceiverDrive
    // for the chosen ControlMode.
    err = respondingReceiver.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    respondingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_InitReceived);
    NL_TEST_ASSERT(inSuite, respondingReceiver.GetControlMode() == driveMode);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.FileDesignator != nullptr);
    NL_TEST_ASSERT(inSuite,
                   !memcmp(testFileDes, outEvent.transferInitData.FileDesignator, outEvent.transferInitData.FileDesLength));

    // Create metadata
    uint8_t tlvBuf[64]    = { 0 };
    char metadataStr[11]  = { "hi_dad.txt" };
    uint64_t tlvTag       = TLV::ProfileTag(1234, 5678);
    uint32_t bytesWritten = 0;
    err                   = WriteTLVString(tlvBuf, sizeof(tlvBuf), metadataStr, tlvTag, bytesWritten);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Compose SendAccept parameters struct and give to respondingSender
    uint16_t proposedBlockSize = transferBlockSize;
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingReceiver.GetControlMode();
    acceptData.MaxBlockSize   = proposedBlockSize;
    acceptData.StartOffset    = 0; // not used in SendAccept
    acceptData.Length         = 0; // not used in SendAccept
    acceptData.Metadata       = tlvBuf;
    acceptData.MetadataLength = metadataSize;

    err = respondingReceiver.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify Sender emits SendAccept message for sending
    respondingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, kBdxMsg_SendAccept);

    // Pass Accept message to initiatingReceiver
    err = initiatingSender.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify received SendAccept.
    // Client may still want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may still choose to
    // reject the Transfer at this point.
    initiatingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_AcceptReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.ControlMode == driveMode);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.MaxBlockSize == transferBlockSize);
    err = ReadAndVerifyTLVString(inSuite, inContext, outEvent.transferAcceptData.Metadata,
                                 outEvent.transferAcceptData.MetadataLength, tlvTag, metadataStr, sizeof(metadataStr) - 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test multiple Block -> BlockAck -> Block
    for (int i = 0; i < 3; i++)
    {
        SendAndVerifyArbitraryBlock(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, false);
        SendAndVerifyBlockAck(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, false);
    }

    SendAndVerifyArbitraryBlock(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, true);
    SendAndVerifyBlockAck(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, true);
}

// Helper method for initiating a TransferSession object, generating a TransferInit message, and passing it to a responding
// TransferSession. Uses an arbitrary file descriptor and no metadata.
//
// It's xpected that the responder has already been initialized with WaitForTransfer().
void StartInitiatorAndSend(nlTestSuite * inSuite, void * inContext, TransferSession::OutputEvent & outEvent,
                           TransferSession & initiator, TransferSession & responder, TransferRole initiatorRole,
                           uint8_t rawControlFlags, uint16_t maxBlockSize, uint64_t offset, uint64_t maxLength)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = rawControlFlags;
    initOptions.MaxBlockSize        = maxBlockSize;
    initOptions.StartOffset         = offset;
    initOptions.Length              = maxLength;
    char testFileDes[9]             = { "test.txt" }; // arbitrary file designator
    initOptions.FileDesLength       = sizeof(testFileDes) - 1;
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata            = nullptr;
    initOptions.MetadataLength      = 0;

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(initiatorRole, initOptions);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiator.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    MessageType expectedInitMsg = (initiatorRole == kRole_Sender) ? kBdxMsg_SendInit : kBdxMsg_ReceiveInit;
    VerifyMessageType(inSuite, inContext, outEvent.MsgData, expectedInitMsg);

    // Responder may want to read any of the fields of the TransferInit for approval.
    // Verify File Designator matches. Verify that the respondingSenser automatically set ReceiverDrive
    // for the chosen ControlMode.
    err = responder.HandleMessageReceived(std::move(outEvent.MsgData));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    responder.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_InitReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.FileDesignator != nullptr);
    NL_TEST_ASSERT(inSuite,
                   !memcmp(testFileDes, outEvent.transferInitData.FileDesignator, outEvent.transferInitData.FileDesLength));
}

// Test that calls to AcceptTransfer() with bad parameters result in an error.
void TestBadAcceptMessageFields(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;

    uint16_t maxBlockSize          = 64;
    TransferControlFlags driveMode = kControl_ReceiverDrive;
    uint64_t commonLength          = 0;
    uint64_t commonOffset          = 0;

    BitFlags<uint8_t, TransferControlFlags> waitOpts;
    waitOpts.Set(driveMode);
    err = respondingSender.WaitForTransfer(kRole_Sender, waitOpts, maxBlockSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    StartInitiatorAndSend(inSuite, inContext, outEvent, initiatingReceiver, respondingSender, kRole_Receiver, driveMode,
                          maxBlockSize, commonOffset, commonLength);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify AcceptTransfer() returns error for choosing larger max block size
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = driveMode;
    acceptData.MaxBlockSize = static_cast<uint16_t>(maxBlockSize + 1); // invalid if larger than proposed
    acceptData.StartOffset  = commonOffset;
    acceptData.Length       = commonLength;
    err                     = respondingSender.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // Verify AcceptTransfer() returns error for choosing unsupported transfer control mode
    TransferSession::TransferAcceptData acceptData2;
    acceptData2.ControlMode  = (driveMode == kControl_ReceiverDrive) ? kControl_SenderDrive : kControl_ReceiverDrive;
    acceptData2.MaxBlockSize = maxBlockSize;
    acceptData2.StartOffset  = commonOffset;
    acceptData2.Length       = commonLength;
    err                      = respondingSender.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestInitiatingReceiverReceiverDrive", TestInitiatingReceiverReceiverDrive),
    NL_TEST_DEF("TestInitiatingSenderSenderDrive", TestInitiatingSenderSenderDrive),
    NL_TEST_DEF("TestBadAcceptMessageFields", TestBadAcceptMessageFields),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-TransferSession",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

/**
 *  Main
 */
int TestBdxTransferSession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBdxTransferSession)
