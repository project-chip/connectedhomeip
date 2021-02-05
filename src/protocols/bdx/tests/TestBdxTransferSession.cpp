#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>

#include <string.h>

#include <nlunit-test.h>

#include <core/CHIPTLV.h>
#include <protocols/common/Constants.h>
#include <support/BufferReader.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;
using namespace ::chip::bdx;

namespace {
// Use this as a timestamp if not needing to test BDX timeouts.
constexpr uint64_t kNoAdvanceTime = 0;

const uint64_t tlvStrTag  = TLV::ContextTag(4);
const uint64_t tlvListTag = TLV::ProfileTag(7777, 8888);
} // anonymous namespace

// Helper method for generating a complete TLV structure with a list containing a single tag and string
CHIP_ERROR WriteChipTLVString(uint8_t * buf, uint32_t bufLen, const char * data, uint32_t & written)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    written        = 0;
    TLV::TLVWriter writer;
    writer.Init(buf, bufLen);

    {
        TLV::TLVWriter listWriter;
        err = writer.OpenContainer(tlvListTag, TLV::kTLVType_List, listWriter);
        SuccessOrExit(err);
        err = listWriter.PutString(tlvStrTag, data);
        SuccessOrExit(err);
        err = writer.CloseContainer(listWriter);
        SuccessOrExit(err);
    }

    err = writer.Finalize();
    SuccessOrExit(err);
    written = writer.GetLengthWritten();

exit:
    return err;
}

// Helper method: read a TLV structure with a single tag and string and verify it matches expected string.
CHIP_ERROR ReadAndVerifyTLVString(nlTestSuite * inSuite, void * inContext, const uint8_t * dataStart, uint32_t len,
                                  const char * expected, uint16_t expectedLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    char tmp[64]        = { 0 };
    uint32_t readLength = 0;
    VerifyOrExit(sizeof(tmp) > len, err = CHIP_ERROR_INTERNAL);

    reader.Init(dataStart, len);
    err = reader.Next();

    VerifyOrExit(reader.GetTag() == tlvListTag, err = CHIP_ERROR_INTERNAL);

    // Metadata must have a top-level list
    {
        TLV::TLVReader listReader;
        err = reader.OpenContainer(listReader);
        SuccessOrExit(err);

        err = listReader.Next();
        SuccessOrExit(err);

        VerifyOrExit(listReader.GetTag() == tlvStrTag, err = CHIP_ERROR_INTERNAL);
        readLength = listReader.GetLength();
        VerifyOrExit(readLength == expectedLen, err = CHIP_ERROR_INTERNAL);
        err = listReader.GetString(tmp, sizeof(tmp));
        SuccessOrExit(err);
        VerifyOrExit(!memcmp(expected, tmp, readLength), err = CHIP_ERROR_INTERNAL);

        err = reader.CloseContainer(listReader);
        SuccessOrExit(err);
    }

exit:
    return err;
}

// Helper method for verifying that a PacketBufferHandle contains a valid BDX header and message type matches expected.
void VerifyBdxMessageType(nlTestSuite * inSuite, void * inContext, const System::PacketBufferHandle & msg, MessageType expected)
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

// Helper method for verifying that a PacketBufferHandle contains a valid StatusReport message and contains a specific StatusCode.
void VerifyStatusReport(nlTestSuite * inSuite, void * inContext, const System::PacketBufferHandle & msg, StatusCode code)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    PayloadHeader payloadHeader;
    uint16_t generalCode  = 0;
    uint32_t protocolId   = 0;
    uint16_t protocolCode = 0;

    if (msg.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }

    err = payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, payloadHeader.GetProtocolID() == Protocols::kProtocol_Protocol_Common);
    NL_TEST_ASSERT(inSuite, payloadHeader.GetMessageType() == static_cast<uint8_t>(Protocols::Common::MsgType::StatusReport));

    Encoding::LittleEndian::Reader reader(msg->Start() + headerSize, msg->DataLength());
    err = reader.Read16(&generalCode).Read32(&protocolId).Read16(&protocolCode).StatusCode();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, generalCode == static_cast<uint16_t>(Protocols::Common::StatusCode::Failure));
    NL_TEST_ASSERT(inSuite, protocolId == Protocols::kProtocol_BDX);
    NL_TEST_ASSERT(inSuite, protocolCode == code);
}

void VerifyNoMoreOutput(nlTestSuite * inSuite, void * inContext, TransferSession & transferSession)
{
    TransferSession::OutputEvent event;
    transferSession.PollOutput(event, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, event.EventType == TransferSession::kNone);
}

// Helper method for initializing two TransferSession objects, generating a TransferInit message, and passing it to a responding
// TransferSession.
void SendAndVerifyTransferInit(nlTestSuite * inSuite, void * inContext, TransferSession::OutputEvent & outEvent, uint32_t timeoutMs,
                               TransferSession & initiator, TransferRole initiatorRole, TransferSession::TransferInitData initData,
                               TransferSession & responder, BitFlags<uint8_t, TransferControlFlags> & responderControlOpts,
                               uint16_t responderMaxBlock)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    TransferRole responderRole  = (initiatorRole == kRole_Sender) ? kRole_Receiver : kRole_Sender;
    MessageType expectedInitMsg = (initiatorRole == kRole_Sender) ? kBdxMsg_SendInit : kBdxMsg_ReceiveInit;

    // Initializer responder to wait for transfer
    err = responder.WaitForTransfer(responderRole, responderControlOpts, responderMaxBlock, timeoutMs);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    VerifyNoMoreOutput(inSuite, inContext, responder);

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(initiatorRole, initData, timeoutMs);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiator.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    VerifyBdxMessageType(inSuite, inContext, outEvent.MsgData, expectedInitMsg);
    VerifyNoMoreOutput(inSuite, inContext, initiator);

    // Verify that all parsed TransferInit fields match what was sent by the initiator
    err = responder.HandleMessageReceived(std::move(outEvent.MsgData), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    responder.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(inSuite, inContext, responder);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kInitReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.TransferCtlFlagsRaw == initData.TransferCtlFlagsRaw);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.MaxBlockSize == initData.MaxBlockSize);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.StartOffset == initData.StartOffset);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.Length == initData.Length);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.FileDesignator != nullptr);
    NL_TEST_ASSERT(inSuite, outEvent.transferInitData.FileDesLength == initData.FileDesLength);
    if (outEvent.EventType == TransferSession::kInitReceived && outEvent.transferInitData.FileDesignator != nullptr)
    {
        NL_TEST_ASSERT(
            inSuite,
            !memcmp(initData.FileDesignator, outEvent.transferInitData.FileDesignator, outEvent.transferInitData.FileDesLength));
    }
    if (outEvent.transferInitData.Metadata != nullptr)
    {
        NL_TEST_ASSERT(inSuite, outEvent.transferInitData.MetadataLength == initData.MetadataLength);
        if (outEvent.transferInitData.MetadataLength == initData.MetadataLength)
        {
            // Only check that metadata buffers match. The OutputEvent can still be inspected when this function returns to parse
            // the metadata and verify that it matches.
            NL_TEST_ASSERT(
                inSuite, !memcmp(initData.Metadata, outEvent.transferInitData.Metadata, outEvent.transferInitData.MetadataLength));
        }
        else
        {
            NL_TEST_ASSERT(inSuite, false); // Metadata length mismatch
        }
    }
}

// Helper method for sending an Accept message and verifying that the received parameters match what was sent.
// This function assumes that the acceptData struct contains transfer parameters that are valid responses to the original
// TransferInit message (for example, MaxBlockSize should be <= the TransferInit MaxBlockSize). If such parameters are invalid, the
// receiver should emit a StatusCode event instead.
//
// The acceptSender is the node that is sending the Accept message (not necessarily the same node that will send Blocks).
void SendAndVerifyAcceptMsg(nlTestSuite * inSuite, void * inContext, TransferSession::OutputEvent & outEvent,
                            TransferSession & acceptSender, TransferRole acceptSenderRole,
                            TransferSession::TransferAcceptData acceptData, TransferSession & acceptReceiver,
                            TransferSession::TransferInitData initData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the node sending the Accept message is also the one that will send Blocks, then this should be a ReceiveAccept message.
    MessageType expectedMsg = (acceptSenderRole == kRole_Sender) ? kBdxMsg_ReceiveAccept : kBdxMsg_SendAccept;

    err = acceptSender.AcceptTransfer(acceptData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify Sender emits ReceiveAccept message for sending
    acceptSender.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(inSuite, inContext, acceptSender);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    VerifyBdxMessageType(inSuite, inContext, outEvent.MsgData, expectedMsg);

    // Pass Accept message to acceptReceiver
    err = acceptReceiver.HandleMessageReceived(std::move(outEvent.MsgData), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify received ReceiveAccept.
    // Client may want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may choose to reject the
    // Transfer at this point.
    acceptReceiver.PollOutput(outEvent, kNoAdvanceTime);
    VerifyNoMoreOutput(inSuite, inContext, acceptReceiver);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kAcceptReceived);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.ControlMode == acceptData.ControlMode);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.MaxBlockSize == acceptData.MaxBlockSize);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.StartOffset == acceptData.StartOffset);
    NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.Length == acceptData.Length);
    if (outEvent.transferAcceptData.Metadata != nullptr)
    {
        NL_TEST_ASSERT(inSuite, outEvent.transferAcceptData.MetadataLength == acceptData.MetadataLength);
        if (outEvent.transferAcceptData.MetadataLength == acceptData.MetadataLength)
        {
            // Only check that metadata buffers match. The OutputEvent can still be inspected when this function returns to parse
            // the metadata and verify that it matches.
            NL_TEST_ASSERT(
                inSuite,
                !memcmp(acceptData.Metadata, outEvent.transferAcceptData.Metadata, outEvent.transferAcceptData.MetadataLength));
        }
        else
        {
            NL_TEST_ASSERT(inSuite, false); // Metadata length mismatch
        }
    }

    // Verify that MaxBlockSize was set appropriately
    NL_TEST_ASSERT(inSuite, acceptReceiver.GetTransferBlockSize() <= initData.MaxBlockSize);
}

// Helper method for preparing a sending a BlockQuery message between two TransferSession objects.
void SendAndVerifyQuery(nlTestSuite * inSuite, void * inContext, TransferSession & queryReceiver, TransferSession & querySender,
                        TransferSession::OutputEvent & outEvent)
{
    // Verify that querySender emits BlockQuery message
    CHIP_ERROR err = querySender.PrepareBlockQuery();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    querySender.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    VerifyBdxMessageType(inSuite, inContext, outEvent.MsgData, kBdxMsg_BlockQuery);
    VerifyNoMoreOutput(inSuite, inContext, querySender);

    // Pass BlockQuery to queryReceiver and verify queryReceiver emits QueryReceived event
    err = queryReceiver.HandleMessageReceived(std::move(outEvent.MsgData), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    queryReceiver.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kQueryReceived);
    VerifyNoMoreOutput(inSuite, inContext, queryReceiver);
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
    System::PacketBufferHandle fakeDataBuf = System::PacketBufferHandle::New(maxBlockSize);
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
    sender.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    VerifyBdxMessageType(inSuite, inContext, outEvent.MsgData, expected);
    VerifyNoMoreOutput(inSuite, inContext, sender);

    // Pass Block message to receiver and verify matching Block is received
    err = receiver.HandleMessageReceived(std::move(outEvent.MsgData), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    receiver.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kBlockReceived);
    NL_TEST_ASSERT(inSuite, outEvent.blockdata.Data != nullptr);
    if (outEvent.EventType == TransferSession::kBlockReceived && outEvent.blockdata.Data != nullptr)
    {
        NL_TEST_ASSERT(inSuite, !memcmp(fakeBlockData, outEvent.blockdata.Data, outEvent.blockdata.Length));
    }
    VerifyNoMoreOutput(inSuite, inContext, receiver);
}

// Helper method for sending a BlockAck or BlockAckEOF, depending on the state of the receiver.
void SendAndVerifyBlockAck(nlTestSuite * inSuite, void * inContext, TransferSession & ackReceiver, TransferSession & ackSender,
                           TransferSession::OutputEvent & outEvent, bool expectEOF)
{
    TransferSession::OutputEventType expectedEventType =
        expectEOF ? TransferSession::kAckEOFReceived : TransferSession::kAckReceived;
    MessageType expectedMsgType = expectEOF ? kBdxMsg_BlockAckEOF : kBdxMsg_BlockAck;

    // Verify PrepareBlockAck() outputs message to send
    CHIP_ERROR err = ackSender.PrepareBlockAck();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ackSender.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    VerifyBdxMessageType(inSuite, inContext, outEvent.MsgData, expectedMsgType);
    VerifyNoMoreOutput(inSuite, inContext, ackSender);

    // Pass BlockAck to ackReceiver and verify it was received
    err = ackReceiver.HandleMessageReceived(std::move(outEvent.MsgData), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ackReceiver.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == expectedEventType);
    VerifyNoMoreOutput(inSuite, inContext, ackReceiver);
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
    uint32_t numBlockSends        = 10;
    uint16_t proposedBlockSize    = 128;
    uint16_t testSmallerBlockSize = 64;
    uint64_t proposedOffset       = 64;
    uint64_t proposedLength       = 0;
    uint32_t timeoutMs            = 1000 * 24;

    // Chosen specifically for this test
    TransferControlFlags driveMode = kControl_ReceiverDrive;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = driveMode;
    initOptions.MaxBlockSize        = proposedBlockSize;
    char testFileDes[9]             = { "test.txt" };
    initOptions.FileDesLength       = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<uint8_t, TransferControlFlags> senderOpts;
    senderOpts.Set(driveMode);

    SendAndVerifyTransferInit(inSuite, inContext, outEvent, timeoutMs, initiatingReceiver, kRole_Receiver, initOptions,
                              respondingSender, senderOpts, proposedBlockSize);

    // Test metadata for Accept message
    uint8_t tlvBuf[64]    = { 0 };
    char metadataStr[11]  = { "hi_dad.txt" };
    uint32_t bytesWritten = 0;
    err                   = WriteChipTLVString(tlvBuf, sizeof(tlvBuf), metadataStr, bytesWritten);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Compose ReceiveAccept parameters struct and give to respondingSender
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingSender.GetControlMode();
    acceptData.StartOffset    = proposedOffset;
    acceptData.Length         = proposedLength;
    acceptData.MaxBlockSize   = testSmallerBlockSize;
    acceptData.Metadata       = tlvBuf;
    acceptData.MetadataLength = metadataSize;

    SendAndVerifyAcceptMsg(inSuite, inContext, outEvent, respondingSender, kRole_Sender, acceptData, initiatingReceiver,
                           initOptions);

    // Verify that MaxBlockSize was chosen correctly
    NL_TEST_ASSERT(inSuite, respondingSender.GetTransferBlockSize() == testSmallerBlockSize);
    NL_TEST_ASSERT(inSuite, respondingSender.GetTransferBlockSize() == initiatingReceiver.GetTransferBlockSize());

    // Verify parsed TLV metadata matches the original
    err =
        ReadAndVerifyTLVString(inSuite, inContext, outEvent.transferAcceptData.Metadata, outEvent.transferAcceptData.MetadataLength,
                               metadataStr, static_cast<uint16_t>(strlen(metadataStr)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test BlockQuery -> Block -> BlockAck
    SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);
    SendAndVerifyArbitraryBlock(inSuite, inContext, respondingSender, initiatingReceiver, outEvent, false);
    numBlocksSent++;

    // Test only one block can be prepared at a time, without receiving a response to the first
    System::PacketBufferHandle fakeBuf = System::PacketBufferHandle::New(testSmallerBlockSize);
    TransferSession::BlockData prematureBlock;
    if (fakeBuf.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }
    prematureBlock.Data   = fakeBuf->Start();
    prematureBlock.Length = testSmallerBlockSize;
    prematureBlock.IsEof  = false;
    err                   = respondingSender.PrepareBlock(prematureBlock);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
    VerifyNoMoreOutput(inSuite, inContext, respondingSender);

    // Test Ack -> Query -> Block
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
    uint32_t timeoutMs         = 1000 * 24;

    // Initialize respondingReceiver
    BitFlags<uint8_t, TransferControlFlags> receiverOpts;
    receiverOpts.Set(driveMode);

    // Test metadata for TransferInit message
    uint8_t tlvBuf[64]    = { 0 };
    char metadataStr[11]  = { "hi_dad.txt" };
    uint32_t bytesWritten = 0;
    err                   = WriteChipTLVString(tlvBuf, sizeof(tlvBuf), metadataStr, bytesWritten);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = driveMode;
    initOptions.MaxBlockSize        = transferBlockSize;
    char testFileDes[9]             = { "test.txt" };
    initOptions.FileDesLength       = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata            = tlvBuf;
    initOptions.MetadataLength      = metadataSize;

    SendAndVerifyTransferInit(inSuite, inContext, outEvent, timeoutMs, initiatingSender, kRole_Sender, initOptions,
                              respondingReceiver, receiverOpts, transferBlockSize);

    // Verify parsed TLV metadata matches the original
    err = ReadAndVerifyTLVString(inSuite, inContext, outEvent.transferInitData.Metadata, outEvent.transferInitData.MetadataLength,
                                 metadataStr, static_cast<uint16_t>(strlen(metadataStr)));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Compose SendAccept parameters struct and give to respondingSender
    uint16_t proposedBlockSize = transferBlockSize;
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingReceiver.GetControlMode();
    acceptData.MaxBlockSize   = proposedBlockSize;
    acceptData.StartOffset    = 0; // not used in SendAccept
    acceptData.Length         = 0; // not used in SendAccept
    acceptData.Metadata       = nullptr;
    acceptData.MetadataLength = 0;

    SendAndVerifyAcceptMsg(inSuite, inContext, outEvent, respondingReceiver, kRole_Receiver, acceptData, initiatingSender,
                           initOptions);

    // Test multiple Block -> BlockAck -> Block
    for (int i = 0; i < 3; i++)
    {
        SendAndVerifyArbitraryBlock(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, false);
        SendAndVerifyBlockAck(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, false);
    }

    SendAndVerifyArbitraryBlock(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, true);
    SendAndVerifyBlockAck(inSuite, inContext, initiatingSender, respondingReceiver, outEvent, true);
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
    uint32_t timeoutMs             = 1000 * 24;

    // Initialize struct with TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = driveMode;
    initOptions.MaxBlockSize        = maxBlockSize;
    initOptions.StartOffset         = commonOffset;
    initOptions.Length              = commonLength;
    char testFileDes[9]             = { "test.txt" }; // arbitrary file designator
    initOptions.FileDesLength       = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata            = nullptr;
    initOptions.MetadataLength      = 0;

    // Responder parameters
    BitFlags<uint8_t, TransferControlFlags> responderControl;
    responderControl.Set(driveMode);

    SendAndVerifyTransferInit(inSuite, inContext, outEvent, timeoutMs, initiatingReceiver, kRole_Receiver, initOptions,
                              respondingSender, responderControl, maxBlockSize);

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
    err                      = respondingSender.AcceptTransfer(acceptData2);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

// Test that a TransferSession will emit kTransferTimeout if the specified timeout is exceeded while waiting for a response.
void TestTimeout(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferSession initiator;
    TransferSession::OutputEvent outEvent;

    uint32_t timeoutMs   = 24;
    uint64_t startTimeMs = 100;
    uint64_t endTimeMs   = 124;

    // Initialize struct with arbitrary TransferInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = kControl_ReceiverDrive;
    initOptions.MaxBlockSize        = 64;
    initOptions.StartOffset         = 0;
    initOptions.Length              = 0;
    char testFileDes[9]             = { "test.txt" }; // arbitrary file designator
    initOptions.FileDesLength       = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);
    initOptions.Metadata            = nullptr;
    initOptions.MetadataLength      = 0;

    TransferRole role = kRole_Receiver;

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(role, initOptions, timeoutMs);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // First PollOutput() should output the TransferInit message
    initiator.PollOutput(outEvent, startTimeMs);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    MessageType expectedInitMsg = (role == kRole_Sender) ? kBdxMsg_SendInit : kBdxMsg_ReceiveInit;
    VerifyBdxMessageType(inSuite, inContext, outEvent.MsgData, expectedInitMsg);

    // Second PollOutput() with no call to HandleMessageReceived() should result in a timeout.
    initiator.PollOutput(outEvent, endTimeMs);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kTransferTimeout);
}

// Test that sending the same block twice (with same block counter) results in a StatusReport message with BadBlockCounter. Also
// test that receiving the StatusReport ends the transfer on the other node.
void TestDuplicateBlockError(nlTestSuite * inSuite, void * inContext)
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
    uint64_t proposedOffset = 64;
    uint64_t proposedLength = 0;
    uint32_t timeoutMs      = 1000 * 24;

    // Chosen specifically for this test
    TransferControlFlags driveMode = kControl_ReceiverDrive;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlagsRaw = driveMode;
    initOptions.MaxBlockSize        = blockSize;
    char testFileDes[9]             = { "test.txt" };
    initOptions.FileDesLength       = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator      = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize respondingSender and pass ReceiveInit message
    BitFlags<uint8_t, TransferControlFlags> senderOpts;
    senderOpts.Set(driveMode);

    SendAndVerifyTransferInit(inSuite, inContext, outEvent, timeoutMs, initiatingReceiver, kRole_Receiver, initOptions,
                              respondingSender, senderOpts, blockSize);

    // Compose ReceiveAccept parameters struct and give to respondingSender
    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode    = respondingSender.GetControlMode();
    acceptData.StartOffset    = proposedOffset;
    acceptData.Length         = proposedLength;
    acceptData.MaxBlockSize   = blockSize;
    acceptData.Metadata       = nullptr;
    acceptData.MetadataLength = 0;

    SendAndVerifyAcceptMsg(inSuite, inContext, outEvent, respondingSender, kRole_Sender, acceptData, initiatingReceiver,
                           initOptions);

    SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);

    TransferSession::BlockData blockData;
    blockData.Data   = fakeData;
    blockData.Length = fakeDataLen;
    blockData.IsEof  = false;

    // Provide Block data and verify sender emits Block message
    err = respondingSender.PrepareBlock(blockData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    respondingSender.PollOutput(eventWithBlock, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, eventWithBlock.EventType == TransferSession::kMsgToSend);
    VerifyBdxMessageType(inSuite, inContext, eventWithBlock.MsgData, kBdxMsg_Block);
    VerifyNoMoreOutput(inSuite, inContext, respondingSender);
    System::PacketBufferHandle blockCopy =
        System::PacketBufferHandle::NewWithData(eventWithBlock.MsgData->Start(), eventWithBlock.MsgData->DataLength());

    // Pass Block message to receiver and verify matching Block is received
    err = initiatingReceiver.HandleMessageReceived(std::move(eventWithBlock.MsgData), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kBlockReceived);
    NL_TEST_ASSERT(inSuite, outEvent.blockdata.Data != nullptr);
    VerifyNoMoreOutput(inSuite, inContext, initiatingReceiver);

    SendAndVerifyQuery(inSuite, inContext, respondingSender, initiatingReceiver, outEvent);

    // Verify receiving same Block twice fails and results in StatusReport event, and then InternalError event
    err = initiatingReceiver.HandleMessageReceived(std::move(blockCopy), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    initiatingReceiver.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kMsgToSend);
    System::PacketBufferHandle statusReportMsg = outEvent.MsgData.Retain();
    VerifyStatusReport(inSuite, inContext, std::move(outEvent.MsgData), kStatus_BadBlockCounter);

    // All subsequent PollOutput() calls should return kInternalError
    for (int i = 0; i < 5; ++i)
    {
        initiatingReceiver.PollOutput(outEvent, kNoAdvanceTime);
        NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kInternalError);
        NL_TEST_ASSERT(inSuite, outEvent.statusData.StatusCode == kStatus_BadBlockCounter);
    }

    err = respondingSender.HandleMessageReceived(std::move(statusReportMsg), kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    respondingSender.PollOutput(outEvent, kNoAdvanceTime);
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kStatusReceived);
    NL_TEST_ASSERT(inSuite, outEvent.statusData.StatusCode == kStatus_BadBlockCounter);

    // All subsequent PollOutput() calls should return kInternalError
    for (int i = 0; i < 5; ++i)
    {
        respondingSender.PollOutput(outEvent, kNoAdvanceTime);
        NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::kInternalError);
        NL_TEST_ASSERT(inSuite, outEvent.statusData.StatusCode == kStatus_BadBlockCounter);
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
    NL_TEST_DEF("TestBadAcceptMessageFields", TestBadAcceptMessageFields),
    NL_TEST_DEF("TestTimeout", TestTimeout),
    NL_TEST_DEF("TestDuplicateBlockError", TestDuplicateBlockError),
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
