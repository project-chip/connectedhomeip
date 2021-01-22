#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>

#include <nlunit-test.h>

#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

using namespace ::chip;
using namespace ::chip::BDX;

void SendAndVerifyQuery(nlTestSuite * inSuite, void * inContext, TransferSession & sender, TransferSession & receiver,
                        TransferSession::OutputEvent & outEvent)
{
    // Verify that receiver emits BlockQuery message
    CHIP_ERROR err = receiver.PrepareBlockQuery();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle queryMsgToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !queryMsgToSend.IsNull());

    // Pass BlockQuery to sender
    err = sender.HandleMessageReceived(std::move(queryMsgToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify sender emits QueryReceived event
    sender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_QueryReceived);
}

void SendAndVerifyArbitraryBlock(nlTestSuite * inSuite, void * inContext, TransferSession & sender, TransferSession & receiver,
                                 TransferSession::OutputEvent & outEvent, bool isEof)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    uint16_t maxBlockSize                  = sender.GetTransferBlockSize();
    System::PacketBufferHandle fakeDataBuf = System::PacketBuffer::NewWithAvailableSize(maxBlockSize);
    if (fakeDataBuf.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }

    uint8_t * fakeBlockData = fakeDataBuf->Start();

    // Provide Block data and verify sender emits Block message
    TransferSession::BlockData blockData = { .Data = fakeBlockData, .Length = maxBlockSize, .IsEof = isEof };
    err                                  = sender.PrepareBlock(blockData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    sender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle blockMsgToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !blockMsgToSend.IsNull());

    // Pass Block message to receiver and verify Block is received
    err = receiver.HandleMessageReceived(std::move(blockMsgToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_BlockReceived);
    System::PacketBufferHandle receivedBlock = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !receivedBlock.IsNull());
    NL_TEST_ASSERT(inSuite, outEvent.blockdata.Data != nullptr);
}

void SendAndVerifyBlockAck(nlTestSuite * inSuite, void * inContext, TransferSession & sender, TransferSession & receiver,
                           TransferSession::OutputEvent & outEvent)
{
    // Verify PrepareBlockAck() outputs message to send
    CHIP_ERROR err = receiver.PrepareBlockAck();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle ackToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !ackToSend.IsNull());

    // Pass BlockAck to sender and verify it was received
    err = sender.HandleMessageReceived(std::move(ackToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    sender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_AckReceived);
}

// Test a full transfer using a responding receiver and an initiating sender, receiver drive.
void TestInitiatingReceiverReceiverDrive(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession::OutputEvent outEvent;
    TransferSession initiatingReceiver;
    TransferSession respondingSender;

    // Chosen arbitrarily for this test
    uint32_t numBlockSends     = 4;
    uint16_t transferBlockSize = 10;

    uint32_t numBlocksSent = 0;

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = kControl_ReceiverDrive;
    initOptions.MaxBlockSize        = transferBlockSize;
    char testFileDes[9]             = { "test.txt" };
    initOptions.FileDesLength       = 9;
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);

    // Verify receiver outputs ReceiveInit message after StartTransfer()
    err = initiatingReceiver.StartTransfer(kRole_Receiver, initOptions);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    NL_TEST_ASSERT(inSuite, !outEvent.MsgData.IsNull());

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<uint8_t, TransferControlFlags> senderOpts;
    senderOpts.Set(kControl_ReceiverDrive);
    err = respondingSender.WaitForTransfer(kRole_Sender, senderOpts, transferBlockSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = respondingSender.HandleMessageReceived(outEvent.MsgData.Retain());
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

    // Compose ReceiveAccept parameters struct and give to respondingSender
    char testMetadata[11]      = { "hi_dad.txt" };
    uint64_t proposedOffset    = 64;
    uint64_t proposedLength    = 0;
    uint16_t proposedBlockSize = transferBlockSize;
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingSender.GetControlMode();
    acceptData.StartOffset    = proposedOffset;
    acceptData.Length         = 0; // Indefinite length
    acceptData.MaxBlockSize   = proposedBlockSize;
    acceptData.Metadata       = reinterpret_cast<uint8_t *>(testMetadata);
    acceptData.MetadataLength = 11;

    err = respondingSender.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify Sender emits ReceiveAccept message for sending
    respondingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle acceptMsgToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !acceptMsgToSend.IsNull());

    // Pass Accept message to initiatingReceiver
    err = initiatingReceiver.HandleMessageReceived(std::move(acceptMsgToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify received ReceiveAccept.
    // Client may want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may choose to reject the
    // Transfer at this point.
    initiatingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_AcceptReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.ControlMode == kControl_ReceiverDrive);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.StartOffset == proposedOffset);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.Length == proposedLength);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.MaxBlockSize == transferBlockSize);
    System::PacketBufferHandle rcvdAcceptMsgData = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !rcvdAcceptMsgData.IsNull());
    NL_TEST_ASSERT(inSuite,
                   !memcmp(testMetadata, outEvent.transferAcceptData.Metadata, outEvent.transferAcceptData.MetadataLength));

    // Test BlockQuery -> Block -> BlockAck
    SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);
    SendAndVerifyArbitraryBlock(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, false);
    numBlocksSent++;
    SendAndVerifyBlockAck(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);

    // Test multiple Blocks sent and received (last Block is BlockEOF)
    while (numBlocksSent < numBlockSends)
    {
        bool isEof = (numBlocksSent == numBlockSends - 1);

        SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);
        SendAndVerifyArbitraryBlock(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, isEof);

        numBlocksSent++;
    }

    // Verify last block was BlockEOF, then verify that PrepareBlockAckEOF() emits BlockAckEOF message
    NL_TEST_ASSERT(inSuite, outEvent.blockdata.IsEof == true);
    err = initiatingReceiver.PrepareBlockAckEOF();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle eofAckToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !eofAckToSend.IsNull());

    // Pass BlockAckEOF message to respondingSender and verify PollOutput emits correct event
    err = respondingSender.HandleMessageReceived(std::move(eofAckToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    respondingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_AckEOFReceived);
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
    initOptions.FileDesLength       = 9;
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);

    // Verify sender outputs SendInit message after StartTransfer()
    err = initiatingSender.StartTransfer(kRole_Sender, initOptions);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle sendInitToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !sendInitToSend.IsNull());

    // Initialize respondingReceiver
    BitFlags<uint8_t, TransferControlFlags> receiverOpts;
    receiverOpts.Set(driveMode);
    err = respondingReceiver.WaitForTransfer(kRole_Receiver, receiverOpts, transferBlockSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Responder may want to read any of the fields of the TransferInit for approval.
    // Verify File Designator matches. Verify that the respondingSenser automatically set ReceiverDrive
    // for the chosen ControlMode.
    err = respondingReceiver.HandleMessageReceived(std::move(sendInitToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    respondingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_InitReceived);
    NL_TEST_ASSERT(inSuite, respondingReceiver.GetControlMode() == driveMode);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.FileDesignator != nullptr);
    NL_TEST_ASSERT(inSuite,
                   !memcmp(testFileDes, outEvent.transferInitData.FileDesignator, outEvent.transferInitData.FileDesLength));

    // Compose SendAccept parameters struct and give to respondingSender
    char testMetadata[9]       = { "metadata" };
    uint16_t proposedBlockSize = transferBlockSize;
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingReceiver.GetControlMode();
    acceptData.MaxBlockSize   = proposedBlockSize;
    acceptData.Metadata       = reinterpret_cast<uint8_t *>(testMetadata);
    acceptData.MetadataLength = 9;

    err = respondingReceiver.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify Sender emits SendAccept message for sending
    respondingReceiver.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_MsgToSend);
    System::PacketBufferHandle acceptMsgToSend = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !acceptMsgToSend.IsNull());

    // Pass Accept message to initiatingReceiver
    err = initiatingSender.HandleMessageReceived(std::move(acceptMsgToSend));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify received SendAccept.
    // Client may still want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may still choose to
    // reject the Transfer at this point.
    initiatingSender.PollOutput(outEvent);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kOutput_AcceptReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.ControlMode == driveMode);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.MaxBlockSize == transferBlockSize);
    System::PacketBufferHandle rcvdAcceptMsgData = std::move(outEvent.MsgData);
    NL_TEST_ASSERT(inSuite, !rcvdAcceptMsgData.IsNull());
    NL_TEST_ASSERT(inSuite,
                   !memcmp(testMetadata, outEvent.transferAcceptData.Metadata, outEvent.transferAcceptData.MetadataLength));

    // Test multiple Block -> BlockAck -> Block
    for (int i = 0; i < 3; i++)
    {
        SendAndVerifyArbitraryBlock(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, false);
        SendAndVerifyBlockAck(inSuite, inContext, initiatingSender, respondingReceiver, outEvent);
    }
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
