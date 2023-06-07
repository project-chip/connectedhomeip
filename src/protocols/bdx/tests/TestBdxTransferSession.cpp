#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>

#include <string.h>

#include <nlunit-test.h>

#include <lib/core/TLV.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>

using namespace ::chip;
using namespace ::chip::bdx;
using namespace ::chip::Protocols;

namespace {

const TLV::Tag tlvStrTag  = TLV::ContextTag(4);
const TLV::Tag tlvListTag = TLV::ProfileTag(7777, 8888);
} // anonymous namespace

// Arbitrary values for the test
uint16_t kTestSmallerBlockSize    = 64;
const uint16_t kProposedBlockSize = 128;
uint32_t kNumBlocksSent           = 0;
uint32_t kNumBlockSends           = 10;
const uint64_t kProposedOffset    = 64;
const uint64_t kProposedLength    = 0;
const char kMetadataStr[11]       = { "hi_dad.txt" };

// This flag is used to simulate a bad accept message for the test TestBadAcceptMessageFields
bool kSimulateBadAcceptMessageError = false;

// This flag is used to simulate duplicate block error for the test TestDuplicateBlockError
bool kSimulateDuplicateBlockError = false;

// Saves the expected output event for every BDX message that should be send from the initiator.
// For e.g When we start a BDX transfer, the initiator should send a MsgToSend with MessageType::SendInit message type.
TransferSession::OutputEvent initiatorExpectedOutputEvent = chip::bdx::TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);

// Saves the expected output event for every BDX message that should be send in the state we are in.
// For e.g When we get a BDX SendInit message, the responder should send a MsgToSend with MessageType::ReceiveInit message type.
TransferSession::OutputEvent responderExpectedOutputEvent = chip::bdx::TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);

// Holds a reference to the initiating receiver that initiates the BDX transfer.
TransferSession * bdxSender = nullptr;

// Holds a reference to the responding sender that receives the BDX init and starts the transfer of the file.
TransferSession * bdxReceiver = nullptr;

// The packet buffer used to send the BDX blocks in the transfer.
System::PacketBufferHandle fakeDataBuf;

// Forward declaration
void SendAcceptTransferForInitReceived(nlTestSuite * inSuite);
void SendBlockForBlockQueryReceived(nlTestSuite * inSuite, TransferSession::OutputEvent & event);
void SendPrematureBlockForBlockQueryReceived(nlTestSuite * inSuite);
void SendAndVerifyBlockAck(nlTestSuite * inSuite, TransferSession & ackReceiver, TransferSession & ackSender, bool expectEOF);
void SendAndVerifyQuery(nlTestSuite * inSuite, TransferSession & queryReceiver, TransferSession & querySender);

// Helper method: Generate a complete TLV structure with a list containing a single tag and string
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

// Helper method: Read a TLV structure with a single tag and string and verify it matches expected string.
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

// Helper method: Attach the payload header and send the message to the HandleMessageReceived of the receiver.
// This simulates sending the BDX messages across the Exchange Context.
CHIP_ERROR AttachHeaderAndSend(TransferSession::MessageTypeData typeData, chip::System::PacketBufferHandle msgBuf,
                               TransferSession & receiver)
{
    chip::PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(typeData.ProtocolId, typeData.MessageType);

    //chip::System::PacketBufferHandle buffer = std::move(msgBuf);
    ReturnErrorOnFailure(receiver.HandleMessageReceived(payloadHeader, std::move(msgBuf)));
    
    return CHIP_NO_ERROR;
}

// Helper method: Verify that a PacketBufferHandle contains a valid BDX header and message type matches expected initiator's
// expected output event
void VerifyBdxMessageToSend(nlTestSuite * inSuite, const TransferSession::OutputEvent & outEvent)
{
    NL_TEST_ASSERT(inSuite, outEvent.EventType == TransferSession::OutputEventType::kMsgToSend);
    NL_TEST_ASSERT(inSuite, !outEvent.MsgData.IsNull());
    NL_TEST_ASSERT(inSuite, outEvent.msgTypeData.ProtocolId == Protocols::BDX::Id);
    NL_TEST_ASSERT(inSuite, outEvent.msgTypeData.MessageType == initiatorExpectedOutputEvent.msgTypeData.MessageType);
}

// Helper method: Verify that a PacketBufferHandle contains a valid StatusReport message and contains a specific StatusCode.
// The msg argument is expected to begin at the message data start, not at the PayloadHeader.
void VerifyStatusReport(nlTestSuite * inSuite, const System::PacketBufferHandle & msg, StatusCode expectedCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (msg.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }

    /*System::PacketBufferHandle msgCopy = msg.CloneData();
    if (msgCopy.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }*/

    SecureChannel::StatusReport report;
    err = report.Parse(msg.Retain());
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, report.GetGeneralCode() == SecureChannel::GeneralStatusCode::kFailure);
    NL_TEST_ASSERT(inSuite, report.GetProtocolId() == Protocols::BDX::Id);
    NL_TEST_ASSERT(inSuite, report.GetProtocolCode() == static_cast<uint16_t>(expectedCode));
}

// Helper method: Reset the simulate error flags and call reset on the transfer session for both initiatingReceiver
// and respondingSender. Set the expected output events to kNone and clear out the references to the
// initiatingReceiver and respondingSender
void Reset()
{
    kSimulateBadAcceptMessageError = false;
    kSimulateDuplicateBlockError   = false;
    fakeDataBuf = nullptr;
    bdxReceiver->Reset();
    bdxSender->Reset();
    responderExpectedOutputEvent = chip::bdx::TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);
    initiatorExpectedOutputEvent = chip::bdx::TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);
    bdxReceiver                  = nullptr;
    bdxSender                    = nullptr;
}

// The callback for the responderSender where it receives the BDX messages sent by the initiatingReceiver.
// When we get the message, we want to check if the Output event matches the expected output event
// for the responderSender. Based on the BDX message received by the responderSender and the state of the
// transfer session state machine, the appropriate BDX message is sent to continue with the BDX transfer.
void OnResponderOutputEventReceived(void * context, TransferSession::OutputEvent & event)
{
    ChipLogProgress(BDX, "OnResponderOutputEventReceived %s", event.ToString(event.EventType));
    nlTestSuite * inSuite = static_cast<nlTestSuite *>(context);
    if (inSuite == nullptr)
    {
        return;
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kMsgToSend: {

        // Verify the correct BDX message is being sent
        VerifyBdxMessageToSend(inSuite, event);

        CHIP_ERROR err = CHIP_NO_ERROR;

        // If we are not simulating duplicate block error, send the event.MsgData.
        if (!kSimulateDuplicateBlockError)
        {
            err = AttachHeaderAndSend(event.msgTypeData, std::move(event.MsgData), *bdxSender);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }
        else
        {
            // If we are simulating duplicate block error and we are not sending a block, just send the event.MsgData
            if (event.msgTypeData.MessageType != static_cast<uint8_t>(MessageType::Block))
            {
                err = AttachHeaderAndSend(event.msgTypeData, std::move(event.MsgData), *bdxSender);
            }
            else
            {
                // This part of code is for simulating a duplicate block error for the test case TestDuplicateBlockError.
                // We copy the block sent for block counter 0 and then send that same block for block counter 1 so that we
                // can generate the StatusCode::kBadBlockCounter error
                if (kNumBlocksSent == 0)
                {     
                    fakeDataBuf = System::PacketBufferHandle::NewWithData(event.MsgData->Start(), event.MsgData->DataLength());
                    err         = AttachHeaderAndSend(event.msgTypeData, std::move(event.MsgData), *bdxSender);
                }
                else if (kNumBlocksSent == 1)
                {
                    if (!fakeDataBuf.IsNull())
                    {
                        // Send the duplicate block stored in fakeDataBuf to simulate duplicate block error
                        err = AttachHeaderAndSend(event.msgTypeData, std::move(fakeDataBuf), *bdxSender);
                    }
                }
            }
        }
        break;
    }
    case TransferSession::OutputEventType::kInitReceived: {
        // Verify that all parsed TransferInit fields match what was sent by the initiator

        NL_TEST_ASSERT(inSuite, event.EventType == responderExpectedOutputEvent.EventType);
        NL_TEST_ASSERT(inSuite,
                       event.transferInitData.TransferCtlFlags == responderExpectedOutputEvent.transferInitData.TransferCtlFlags);
        NL_TEST_ASSERT(inSuite, event.transferInitData.MaxBlockSize == responderExpectedOutputEvent.transferInitData.MaxBlockSize);
        NL_TEST_ASSERT(inSuite, event.transferInitData.StartOffset == responderExpectedOutputEvent.transferInitData.StartOffset);
        NL_TEST_ASSERT(inSuite, event.transferInitData.Length == responderExpectedOutputEvent.transferInitData.Length);
        NL_TEST_ASSERT(inSuite, event.transferInitData.FileDesignator != nullptr);
        NL_TEST_ASSERT(inSuite,
                       event.transferInitData.FileDesLength == responderExpectedOutputEvent.transferInitData.FileDesLength);

        if (event.transferInitData.FileDesignator != nullptr &&
            responderExpectedOutputEvent.transferInitData.FileDesignator != nullptr)
        {
            NL_TEST_ASSERT(inSuite,
                           !memcmp(responderExpectedOutputEvent.transferInitData.FileDesignator,
                                   event.transferInitData.FileDesignator, event.transferInitData.FileDesLength));
        }
        if (event.transferInitData.Metadata != nullptr && responderExpectedOutputEvent.transferInitData.Metadata != nullptr)
        {
            size_t metadataLength = responderExpectedOutputEvent.transferInitData.MetadataLength;
            NL_TEST_ASSERT(inSuite, event.transferInitData.MetadataLength == metadataLength);
            if (event.transferInitData.MetadataLength == metadataLength)
            {
                // Only check that metadata buffers match. The OutputEvent can still be inspected when this function returns to
                // parse the metadata and verify that it matches.
                NL_TEST_ASSERT(inSuite,
                               !memcmp(responderExpectedOutputEvent.transferInitData.Metadata, event.transferInitData.Metadata,
                                       event.transferInitData.MetadataLength));
            }
            else
            {
                NL_TEST_ASSERT(inSuite, false); // Metadata length mismatch
            }
        }

        // Send the Accept Transfer msg in response to the kInitReceived
        responderExpectedOutputEvent.transferInitData.FileDesignator = nullptr;
        responderExpectedOutputEvent.transferInitData.Metadata = nullptr;
        SendAcceptTransferForInitReceived(inSuite);
        break;
    }
    case TransferSession::OutputEventType::kQueryReceived: {
        SendBlockForBlockQueryReceived(inSuite, event);
        break;
    }
    case TransferSession::OutputEventType::kAckReceived: {
        // If kSimulateDuplicateBlockError is false, send the block query request until
        // all blocks are transferred.
        if (!kSimulateDuplicateBlockError)
        {
            if (kNumBlocksSent < kNumBlockSends)
            {
                SendAndVerifyQuery(inSuite, *bdxReceiver, *bdxSender);
            }
        }
        else
        {
            // If kSimulateDuplicateBlockError is false, send and verify query for the first 2 blocks.
            // The second query response should trigger a status report with StatusCode::kBadBlockCounter.
            if (kNumBlocksSent < 2)
            {
                SendAndVerifyQuery(inSuite, *bdxReceiver, *bdxSender);
            }
        }
        break;
    }
    case TransferSession::OutputEventType::kStatusReceived: {
        if (kSimulateDuplicateBlockError)
        {
            // Verify the status report and the status code for duplicate block being sent
            NL_TEST_ASSERT(inSuite, event.EventType == TransferSession::OutputEventType::kStatusReceived);
            NL_TEST_ASSERT(inSuite, event.statusData.statusCode != StatusCode::kBadBlockCounter);

            // Reset the transfer sessions and clean up to wrap up the test
            Reset();
        }
        break;
    }
    case TransferSession::OutputEventType::kAckEOFReceived: {
        // Reset the transfer sessions and clean up to wrap up the test
        Reset();
        break;
    }
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        break;
    }
    //event = chip::bdx::TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);
}

// The callback for the initiatingReceiver where it receives the BDX messages sent by the respondingSender.
// When we get the message, we want to check if the Output event matches the expected output event
// for the initiatingReceiver. Based on the BDX message received by the initiatingReceiver and the state of the
// transfer session state machine, the appropriate BDX message is sent to continue with the BDX transfer.
void OnInitiatorOutputEventReceived(void * context, TransferSession::OutputEvent & event)
{
    ChipLogProgress(BDX, "OnInitiatorOutputEventReceived %s", event.ToString(event.EventType));
    nlTestSuite * inSuite = static_cast<nlTestSuite *>(context);
    if (inSuite == nullptr)
    {
        return;
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kMsgToSend: {
        CHIP_ERROR err = CHIP_NO_ERROR;
        if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
        {
            VerifyStatusReport(inSuite, std::move(event.MsgData), StatusCode::kBadBlockCounter);
            System::PacketBufferHandle statusReportMsg               = event.MsgData.Retain();
            TransferSession::MessageTypeData statusReportMsgTypeData = event.msgTypeData;
            err = AttachHeaderAndSend(statusReportMsgTypeData, std::move(statusReportMsg), *bdxReceiver);
        }
        else
        {
            VerifyBdxMessageToSend(inSuite, event);
            err = AttachHeaderAndSend(event.msgTypeData, std::move(event.MsgData), *bdxReceiver);
        }
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived: {
        // Verify received ReceiveAccept.
        // Client may want to inspect TransferControl, MaxBlockSize, StartOffset, Length, and Metadata, and may choose to reject the
        // Transfer at this point.
        NL_TEST_ASSERT(inSuite, event.EventType == TransferSession::OutputEventType::kAcceptReceived);
        NL_TEST_ASSERT(inSuite,
                       event.transferAcceptData.ControlMode == responderExpectedOutputEvent.transferAcceptData.ControlMode);
        NL_TEST_ASSERT(inSuite,
                       event.transferAcceptData.MaxBlockSize == responderExpectedOutputEvent.transferAcceptData.MaxBlockSize);
        NL_TEST_ASSERT(inSuite,
                       event.transferAcceptData.StartOffset == responderExpectedOutputEvent.transferAcceptData.StartOffset);
        NL_TEST_ASSERT(inSuite, event.transferAcceptData.Length == responderExpectedOutputEvent.transferAcceptData.Length);
        if (event.transferAcceptData.Metadata != nullptr)
        {
            size_t metadatalength = responderExpectedOutputEvent.transferAcceptData.MetadataLength;
            NL_TEST_ASSERT(inSuite, event.transferAcceptData.MetadataLength == metadatalength);
            if (event.transferAcceptData.MetadataLength == metadatalength)
            {
                // Only check that metadata buffers match. The OutputEvent can still be inspected when this function returns to
                // parse the metadata and verify that it matches.
                NL_TEST_ASSERT(inSuite,
                               !memcmp(responderExpectedOutputEvent.transferAcceptData.Metadata, event.transferAcceptData.Metadata,
                                       event.transferAcceptData.MetadataLength));
            }
            else
            {
                NL_TEST_ASSERT(inSuite, false); // Metadata length mismatch
            }
        }

        NL_TEST_ASSERT(inSuite, bdxSender != nullptr && bdxReceiver != nullptr);
        // Verify that MaxBlockSize was set appropriately
        NL_TEST_ASSERT(inSuite, bdxReceiver->GetTransferBlockSize() <= kProposedBlockSize);

        // Verify that MaxBlockSize was chosen correctly
        NL_TEST_ASSERT(inSuite, bdxSender->GetTransferBlockSize() == kTestSmallerBlockSize);
        NL_TEST_ASSERT(inSuite, bdxSender->GetTransferBlockSize() == bdxReceiver->GetTransferBlockSize());

        // Verify parsed TLV metadata matches the original
        CHIP_ERROR err =
            ReadAndVerifyTLVString(responderExpectedOutputEvent.transferAcceptData.Metadata,
                                   static_cast<uint32_t>(responderExpectedOutputEvent.transferAcceptData.MetadataLength),
                                   kMetadataStr, static_cast<uint16_t>(strlen(kMetadataStr)));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        responderExpectedOutputEvent.transferAcceptData.Metadata = nullptr;
        SendAndVerifyQuery(inSuite, *bdxReceiver, *bdxSender);
        
        break;
    }
    case TransferSession::OutputEventType::kBlockReceived: {
        NL_TEST_ASSERT(inSuite, event.blockdata.Data != nullptr);
        if (event.blockdata.Data != nullptr)
        {
            NL_TEST_ASSERT(inSuite,
                           !memcmp(responderExpectedOutputEvent.blockdata.Data, event.blockdata.Data, event.blockdata.Length));
            NL_TEST_ASSERT(inSuite, event.blockdata.BlockCounter == kNumBlocksSent);
        }

        // Test sending a premature block before block ack is received which should result in a failure in PrepareBlock.
        if (kNumBlocksSent == 0 && !kSimulateDuplicateBlockError)
        {
            SendPrematureBlockForBlockQueryReceived(inSuite);
        }

        // Test Ack -> Query -> Block
        bool isEof = (kNumBlocksSent == kNumBlockSends - 1);
        kNumBlocksSent++;
        responderExpectedOutputEvent.blockdata.Data =  nullptr;
        fakeDataBuf = nullptr;
        SendAndVerifyBlockAck(inSuite, *bdxReceiver, *bdxSender, isEof);
        break;
    }
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kStatusReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kAckReceived:
    default:
        break;
    }
    //event = chip::bdx::TransferSession::OutputEvent(TransferSession::OutputEventType::kNone);
}

// Helper method for initializing two TransferSession objects, generating a TransferInit message, and passing it to a responding
// TransferSession.
void SendAndVerifyTransferInit(nlTestSuite * inSuite, TransferSession & initiator, TransferRole initiatorRole,
                               TransferSession::TransferInitData initData, TransferSession & responder,
                               BitFlags<TransferControlFlags> & responderControlOpts, uint16_t responderMaxBlock)
{
    TransferRole responderRole = (initiatorRole == TransferRole::kSender) ? TransferRole::kReceiver : TransferRole::kSender;

    MessageType expectedInitMsg = (initiatorRole == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;

    // Initializer responder to wait for transfer
    CHIP_ERROR err =
        responder.WaitForTransfer(responderRole, responderControlOpts, responderMaxBlock, OnResponderOutputEventReceived, inSuite);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Set the expected output event when init message is sent to the initiator
    initiatorExpectedOutputEvent.EventType               = TransferSession::OutputEventType::kMsgToSend;
    initiatorExpectedOutputEvent.msgTypeData.ProtocolId  = Protocols::BDX::Id;
    initiatorExpectedOutputEvent.msgTypeData.MessageType = static_cast<uint8_t>(expectedInitMsg);

    // Set the expected BDX Init message to the responder
    responderExpectedOutputEvent.EventType        = TransferSession::OutputEventType::kInitReceived;
    responderExpectedOutputEvent.transferInitData = initData;

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(initiatorRole, initData, OnInitiatorOutputEventReceived, inSuite);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Helper method for initializing two TransferSession objects with an incorrect responder role, generating
// a TransferInit message, and passing it to a responding TransferSession so that AcceptTransfer fails.
void SendAndVerifyTransferInitBadResponderRole(nlTestSuite * inSuite, TransferSession & initiator, TransferRole initiatorRole,
                                               TransferSession::TransferInitData initData, TransferSession & responder,
                                               BitFlags<TransferControlFlags> & responderControlOpts, uint16_t responderMaxBlock)
{
    TransferRole responderRole  = TransferRole::kReceiver;
    MessageType expectedInitMsg = (initiatorRole == TransferRole::kSender) ? MessageType::SendInit : MessageType::ReceiveInit;

    // Initializer responder to wait for transfer
    CHIP_ERROR err =
        responder.WaitForTransfer(responderRole, responderControlOpts, responderMaxBlock, OnResponderOutputEventReceived, inSuite);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Set the expected output event when init message is sent to the initiator
    initiatorExpectedOutputEvent.EventType               = TransferSession::OutputEventType::kMsgToSend;
    initiatorExpectedOutputEvent.msgTypeData.ProtocolId  = Protocols::BDX::Id;
    initiatorExpectedOutputEvent.msgTypeData.MessageType = static_cast<uint8_t>(expectedInitMsg);

    // Set the expected BDX Init message to the responder
    responderExpectedOutputEvent.EventType        = TransferSession::OutputEventType::kInitReceived;
    responderExpectedOutputEvent.transferInitData = initData;

    // Verify initiator outputs respective Init message (depending on role) after StartTransfer()
    err = initiator.StartTransfer(initiatorRole, initData, OnInitiatorOutputEventReceived, inSuite);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Helper method for sending an Accept message and verifying that the received parameters match what was sent.
// This function assumes that the acceptData struct contains transfer parameters that are valid responses to the original
// TransferInit message (for example, MaxBlockSize should be <= the TransferInit MaxBlockSize). If such parameters are invalid, the
// receiver should emit a StatusCode event instead.
//
// The acceptSender is the node that is sending the Accept message (not necessarily the same node that will send Blocks).
void SendAndVerifyAcceptMsg(nlTestSuite * inSuite, TransferSession & acceptSender, TransferRole acceptSenderRole,
                            TransferSession::TransferAcceptData acceptData, TransferSession & acceptReceiver,
                            TransferSession::TransferInitData initData)
{
    // If the node sending the Accept message is also the one that will send Blocks, then this should be a ReceiveAccept message.
    MessageType expectedMsg = (acceptSenderRole == TransferRole::kSender) ? MessageType::ReceiveAccept : MessageType::SendAccept;

    // Set the expected output event when init message is sent to the initiator
    initiatorExpectedOutputEvent.msgTypeData.MessageType = static_cast<uint8_t>(expectedMsg);

    // Set the expected BDX accept received message to the responder
    responderExpectedOutputEvent.EventType          = TransferSession::OutputEventType::kAcceptReceived;
    responderExpectedOutputEvent.transferAcceptData = acceptData;

    CHIP_ERROR err = acceptSender.AcceptTransfer(acceptData);
    // If kSimulateBadAcceptMessageError is set to true, we expect that AcceptTransfer will fail due to incorrect parameters being
    // set in the transferAcceptData.
    NL_TEST_ASSERT(inSuite, (kSimulateBadAcceptMessageError ? err != CHIP_NO_ERROR : err == CHIP_NO_ERROR));
    if (kSimulateBadAcceptMessageError)
    {
        Reset();
    }
}

// Helper method for preparing and sending a BlockQuery message between two TransferSession objects.
void SendAndVerifyQuery(nlTestSuite * inSuite, TransferSession & queryReceiver, TransferSession & querySender)
{
    // Set the expected output event message
    initiatorExpectedOutputEvent.msgTypeData.MessageType = static_cast<uint8_t>(MessageType::BlockQuery);

    responderExpectedOutputEvent.EventType = TransferSession::OutputEventType::kQueryReceived;
    // Verify that querySender emits BlockQuery message
    CHIP_ERROR err = querySender.PrepareBlockQuery();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Helper method for preparing and sending the same block data to simulate a duplicate block error and also setting up the expected
// output event for the responder to kBlockReceived for the first block counter and kStatusReceived for the duplicate block
void SendAndVerifyStatusReportForSameBlock(nlTestSuite * inSuite, TransferSession & sender, TransferSession & receiver, bool isEof,
                                           uint32_t inBlockCounter, TransferSession::OutputEvent & event)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    uint8_t fakeData[64] = { 0 };
    uint8_t fakeDataLen  = sizeof(fakeData);

    TransferSession::BlockData blockData;
    blockData.Data   = fakeData;
    blockData.Length = fakeDataLen;
    blockData.IsEof  = false;

    initiatorExpectedOutputEvent.msgTypeData.MessageType = static_cast<uint8_t>(isEof ? MessageType::BlockEOF : MessageType::Block);
    // Set the expected BDX event type
    if (inBlockCounter == 0)
    {
        responderExpectedOutputEvent.EventType = TransferSession::OutputEventType::kBlockReceived;
        responderExpectedOutputEvent.blockdata = blockData;
    }
    else
    {
        responderExpectedOutputEvent.EventType = TransferSession::OutputEventType::kStatusReceived;
    }
    err = sender.PrepareBlock(blockData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Helper method for preparing and sending a Block message between two TransferSession objects. The sender refers to the node that
// is sending Blocks. Uses a static counter incremented with each call. Also verifies that block data received matches what was
// sent.
void SendAndVerifyArbitraryBlock(nlTestSuite * inSuite, TransferSession & sender, TransferSession & receiver, bool isEof,
                                 uint32_t inBlockCounter)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    static uint8_t dataCount = 0;
    uint16_t maxBlockSize    = sender.GetTransferBlockSize();

    NL_TEST_ASSERT(inSuite, maxBlockSize > 0);
    fakeDataBuf = System::PacketBufferHandle::New(maxBlockSize);
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

    initiatorExpectedOutputEvent.msgTypeData.MessageType = static_cast<uint8_t>(isEof ? MessageType::BlockEOF : MessageType::Block);

    // Set the expected BDX accept received message to the responder
    responderExpectedOutputEvent.EventType = TransferSession::OutputEventType::kBlockReceived;
    responderExpectedOutputEvent.blockdata = blockData;

    // Provide Block data and verify sender emits Block message
    err = sender.PrepareBlock(blockData);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Helper method for sending a BlockAck or BlockAckEOF, depending on the state of the receiver.
void SendAndVerifyBlockAck(nlTestSuite * inSuite, TransferSession & ackReceiver, TransferSession & ackSender, bool expectEOF)
{

    initiatorExpectedOutputEvent.msgTypeData.MessageType =
        static_cast<uint8_t>(expectEOF ? MessageType::BlockAckEOF : MessageType::BlockAck);

    // Set the expected BDX accept received message to the responder
    if (kSimulateDuplicateBlockError && kNumBlocksSent == 1)
    {
        responderExpectedOutputEvent.EventType = TransferSession::OutputEventType::kStatusReceived;
    }
    else
    {
        responderExpectedOutputEvent.EventType =
            expectEOF ? TransferSession::OutputEventType::kAckEOFReceived : TransferSession::OutputEventType::kAckReceived;
    }

    CHIP_ERROR err = ackSender.PrepareBlockAck();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Test a BDX transfer end to end. Tests BDX Init, Receive Accept, Block Query, Block and BlockEOF
void TestBDXTransferReceiverDrive(nlTestSuite * inSuite, void * inContext)
{
    TransferSession initiatingReceiver;
    TransferSession respondingSender;
    kNumBlocksSent = 0;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = kProposedBlockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize initiatingReceiver and respondingSender
    BitFlags<TransferControlFlags> receiverOpts;
    receiverOpts.Set(initOptions.TransferCtlFlags);
    bdxSender   = &initiatingReceiver;
    bdxReceiver = &respondingSender;

    // Send and verify transfer init from the initiatingReceiver
    SendAndVerifyTransferInit(inSuite, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender, receiverOpts,
                              kProposedBlockSize);
}

// Test that calls to AcceptTransfer() with bad parameters result in an error.
void TestBadAcceptMessageFields(nlTestSuite * inSuite, void * inContext)
{
    TransferSession initiatingReceiver;
    TransferSession respondingSender;
    kNumBlocksSent = 0;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = kProposedBlockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize initiatingReceiver and respondingSender
    BitFlags<TransferControlFlags> receiverOpts;
    receiverOpts.Set(initOptions.TransferCtlFlags);
    bdxSender   = &initiatingReceiver;
    bdxReceiver = &respondingSender;

    // kSimulateBadAcceptMessageError is set to true as we are simulating a bad transfer accept data payload to test failure for
    // AcceptTransfer.
    kSimulateBadAcceptMessageError = true;

    // Send and verify transfer init from the initiatingReceiver.
    SendAndVerifyTransferInit(inSuite, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender, receiverOpts,
                              kProposedBlockSize);
}

// Test that sending the same block twice (with same block counter) results in a StatusReport message with BadBlockCounter. Also
// test that receiving the StatusReport ends the transfer on the other node.
void TestDuplicateBlockError(nlTestSuite * inSuite, void * inContext)
{
    TransferSession initiatingReceiver;
    TransferSession respondingSender;
    kNumBlocksSent = 0;

    // ReceiveInit parameters
    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = kProposedBlockSize;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize initiatingReceiver and respondingSender
    BitFlags<TransferControlFlags> receiverOpts;
    receiverOpts.Set(initOptions.TransferCtlFlags);
    bdxSender   = &initiatingReceiver;
    bdxReceiver = &respondingSender;

    // kSimulateDuplicateBlockError is set to true to simulate sending a duplicate block and getting a status report with code
    // StatusCode::kBadBlockCounter.
    kSimulateDuplicateBlockError = true;

    // Send and verify transfer init from the initiatingReceiver
    SendAndVerifyTransferInit(inSuite, initiatingReceiver, TransferRole::kReceiver, initOptions, respondingSender, receiverOpts,
                              kProposedBlockSize);
}

// Send an Accept transfer message from respondingSender when an init is received from initiatingReceiver
void SendAcceptTransferForInitReceived(nlTestSuite * inSuite)
{
    // Test metadata for Accept message
    uint8_t tlvBuf[64]    = { 0 };
    uint32_t bytesWritten = 0;
    CHIP_ERROR err        = WriteTLVString(tlvBuf, sizeof(tlvBuf), kMetadataStr, bytesWritten);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    uint16_t metadataSize = static_cast<uint16_t>(bytesWritten & 0x0000FFFF);

    // Compose ReceiveAccept parameters struct and give to respondingSender
    TransferSession::TransferAcceptData acceptData;
    NL_TEST_ASSERT(inSuite, bdxSender != nullptr && bdxReceiver != nullptr);
    if (kSimulateBadAcceptMessageError)
    {
        acceptData.ControlMode = TransferControlFlags::kSenderDrive;
    }
    else
    {
        acceptData.ControlMode = bdxReceiver->GetControlMode();
    }
    TransferRole role;

    if (responderExpectedOutputEvent.transferInitData.TransferCtlFlags == TransferControlFlags::kReceiverDrive)
    {
        acceptData.StartOffset    = kProposedOffset;
        acceptData.Length         = kProposedLength;
        acceptData.MaxBlockSize   = kTestSmallerBlockSize;
        acceptData.Metadata       = tlvBuf;
        acceptData.MetadataLength = metadataSize;
        role                      = TransferRole::kSender;
    }
    else
    {
        // Compose SendAccept parameters struct and give to respondingSender
        acceptData.MaxBlockSize   = 10;
        acceptData.StartOffset    = 0; // not used in SendAccept
        acceptData.Length         = 0; // not used in SendAccept
        acceptData.Metadata       = nullptr;
        acceptData.MetadataLength = 0;
        role                      = TransferRole::kReceiver;
    }

    // Send and verify transfer accept from the respondingSender
    SendAndVerifyAcceptMsg(inSuite, *bdxReceiver, role, acceptData, *bdxSender, responderExpectedOutputEvent.transferInitData);
}

// Send a block from the responder for a block query sent by the initiator
void SendBlockForBlockQueryReceived(nlTestSuite * inSuite, TransferSession::OutputEvent & event)
{
    bool isEof = (kNumBlocksSent == kNumBlockSends - 1);
    if (kSimulateDuplicateBlockError)
    {
        // Send the same block to simulate block error.
        ChipLogError(BDX, "calling sent same block");
        SendAndVerifyStatusReportForSameBlock(inSuite, *bdxReceiver, *bdxSender, isEof, kNumBlocksSent, event);
    }
    else
    {
        SendAndVerifyArbitraryBlock(inSuite, *bdxReceiver, *bdxSender, isEof, kNumBlocksSent);
    }
}

// Send a block before the block ack comes for the previous block send and check that PrepareBlock fails
void SendPrematureBlockForBlockQueryReceived(nlTestSuite * inSuite)
{
    // Test only one block can be prepared at a time, without receiving a response to the first
    fakeDataBuf = System::PacketBufferHandle::New(kTestSmallerBlockSize);
    TransferSession::BlockData prematureBlock;
    if (fakeDataBuf.IsNull())
    {
        NL_TEST_ASSERT(inSuite, false);
        return;
    }
    prematureBlock.Data   = fakeDataBuf->Start();
    prematureBlock.Length = kTestSmallerBlockSize;
    prematureBlock.IsEof  = false;

    initiatorExpectedOutputEvent.msgTypeData.MessageType =
        static_cast<uint8_t>(prematureBlock.IsEof ? MessageType::BlockEOF : MessageType::Block);

    // Set the expected BDX accept received message to the responder
    responderExpectedOutputEvent.EventType = TransferSession::OutputEventType::kBlockReceived;
    responderExpectedOutputEvent.blockdata = prematureBlock;

    CHIP_ERROR err = bdxReceiver->PrepareBlock(prematureBlock);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}
// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestBDXTransferReceiverDrive", TestBDXTransferReceiverDrive),
    NL_TEST_DEF("TestBadAcceptMessageFields", TestBadAcceptMessageFields),
    NL_TEST_DEF("TestDuplicateBlockError", TestDuplicateBlockError),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestBdxTransferSession_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

int TestBdxTransferSession_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-TransferSession",
    &sTests[0],
    TestBdxTransferSession_Setup,
    TestBdxTransferSession_Teardown
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
