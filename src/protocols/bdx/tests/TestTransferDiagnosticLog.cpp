#include <string.h>

#include <pw_unit_test/framework.h>

#include <protocols/bdx/BdxTransferDiagnosticLog.h>
#include <system/RAIIMockClock.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

using namespace ::chip;
using namespace ::chip::bdx;
using namespace ::chip::Protocols;
using namespace ::chip::System::Clock::Literals;

namespace {

constexpr uint16_t kMaxBlockSize             = 512;
constexpr chip::FabricIndex kFabricIndex     = 1;
constexpr chip::NodeId kNodeId               = 2;
constexpr uint8_t kMetaDataLength            = 5;
constexpr uint8_t kMetaData[kMetaDataLength] = { 7, 6, 5, 4, 3 }; // This is not TLV but this is fine for the flows we test here

class TestTransferDiagnosticLog : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    TransferSession mTransferSession{};
};

TEST_F(TestTransferDiagnosticLog, InitsDiagnosticLog)
{
    System::Clock::Internal::RAIIMockClock mockClock;

    mTransferSession.WaitForTransfer(TransferRole::kReceiver, TransferControlFlags::kSenderDrive, kMaxBlockSize, 1000_ms);

    TransferInit transferInit{};

    transferInit.TransferCtlOptions.ClearAll().Set(TransferControlFlags::kSenderDrive, true);
    transferInit.Version = 1;

    transferInit.MaxLength = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;

    transferInit.StartOffset  = 42;
    transferInit.MaxBlockSize = 256;

    const char testFileDes[]    = "test.txt";
    transferInit.FileDesLength  = 8;
    transferInit.FileDesignator = reinterpret_cast<const uint8_t *>(testFileDes);

    transferInit.MetadataLength = kMetaDataLength;
    transferInit.Metadata       = kMetaData;

    size_t msgSize = transferInit.MessageSize();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    ASSERT_FALSE(bbuf.IsNull());
    transferInit.WriteToBuffer(bbuf);

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    ASSERT_FALSE(msgBuf.IsNull());
    System::PacketBufferHandle rcvBuf = System::PacketBufferHandle::NewWithData(msgBuf->Start(), msgSize);
    ASSERT_FALSE(rcvBuf.IsNull());

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(Protocols::BDX::Id, to_underlying(MessageType::SendInit));

    CHIP_ERROR r = mTransferSession.HandleMessageReceived(payloadHeader, std::move(rcvBuf), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    BDXTransferProxyDiagnosticLog proxyDiagnosticLog{};

    r = proxyDiagnosticLog.Init(&mTransferSession);

    EXPECT_EQ(r, CHIP_NO_ERROR);
}

TEST_F(TestTransferDiagnosticLog, AccpetsTransferActingAsReceiverWhileInititatorDrivesTransfer)
{

    BDXTransferProxyDiagnosticLog proxyDiagnosticLog{};

    TransferSession initiator;
    TransferSession::TransferInitData transferInitData{};
    transferInitData.TransferCtlFlags = TransferControlFlags::kSenderDrive;
    transferInitData.MaxBlockSize     = kMaxBlockSize;
    transferInitData.StartOffset      = 0;
    transferInitData.Length           = 1024;
    const char testFileDes[]          = "test.txt";
    transferInitData.FileDesLength    = 9;
    transferInitData.FileDesignator   = reinterpret_cast<const uint8_t *>(testFileDes);
    transferInitData.MetadataLength   = kMetaDataLength;
    transferInitData.Metadata         = kMetaData;

    /// Init initiator (and sender) transfer session
    CHIP_ERROR r = initiator.StartTransfer(TransferRole::kSender, transferInitData, System::Clock::Seconds16(10));

    EXPECT_EQ(r, CHIP_NO_ERROR);

    TransferSession::OutputEvent initiatorEvent;
    initiator.PollOutput(initiatorEvent, System::Clock::kZero);
    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kMsgToSend);

    /// Init responder (and receiver) transfer session
    r = mTransferSession.WaitForTransfer(TransferRole::kReceiver, TransferControlFlags::kSenderDrive, kMaxBlockSize,
                                         System::Clock::Seconds16(20));
    EXPECT_EQ(r, CHIP_NO_ERROR);

    TransferSession::OutputEvent responderEvent;
    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);
    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kNone);

    chip::PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(initiatorEvent.msgTypeData.ProtocolId, initiatorEvent.msgTypeData.MessageType);

    r = mTransferSession.HandleMessageReceived(payloadHeader, std::move(initiatorEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kInitReceived);

    r = proxyDiagnosticLog.Init(&mTransferSession);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    proxyDiagnosticLog.SetFabricIndex(kFabricIndex);
    proxyDiagnosticLog.SetPeerNodeId(kNodeId);
    r = proxyDiagnosticLog.Accept();

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kMsgToSend); // responder sends Accept message
}

TEST_F(TestTransferDiagnosticLog, RejectsInTheMiddleOfTransfer)
{

    BDXTransferProxyDiagnosticLog proxyDiagnosticLog{};

    TransferSession initiator;
    TransferSession::TransferInitData transferInitData{};
    transferInitData.TransferCtlFlags = TransferControlFlags::kSenderDrive;
    transferInitData.MaxBlockSize     = kMaxBlockSize;
    transferInitData.StartOffset      = 0;
    transferInitData.Length           = 1024;
    const char testFileDes[]          = "test.txt";
    transferInitData.FileDesLength    = 8;
    transferInitData.FileDesignator   = reinterpret_cast<const uint8_t *>(testFileDes);
    transferInitData.MetadataLength   = kMetaDataLength;
    transferInitData.Metadata         = kMetaData;

    /// Init initiator (and sender) transfer session
    CHIP_ERROR r = initiator.StartTransfer(TransferRole::kSender, transferInitData, System::Clock::Seconds16(10));

    EXPECT_EQ(r, CHIP_NO_ERROR);

    TransferSession::OutputEvent initiatorEvent;
    initiator.PollOutput(initiatorEvent, System::Clock::kZero);
    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kMsgToSend);

    /// Init responder (and receiver) transfer session
    r = mTransferSession.WaitForTransfer(TransferRole::kReceiver, TransferControlFlags::kSenderDrive, kMaxBlockSize,
                                         System::Clock::Seconds16(20));
    EXPECT_EQ(r, CHIP_NO_ERROR);

    TransferSession::OutputEvent responderEvent;
    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);
    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kNone);

    chip::PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(initiatorEvent.msgTypeData.ProtocolId, initiatorEvent.msgTypeData.MessageType);

    r = mTransferSession.HandleMessageReceived(payloadHeader, std::move(initiatorEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kInitReceived);

    r = proxyDiagnosticLog.Init(&mTransferSession);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    proxyDiagnosticLog.SetFabricIndex(kFabricIndex);
    proxyDiagnosticLog.SetPeerNodeId(kNodeId);
    r = proxyDiagnosticLog.Accept();

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kMsgToSend); // responder sends Accept message

    /// Now, initiator handles SendAccept message
    payloadHeader.SetMessageType(responderEvent.msgTypeData.ProtocolId, responderEvent.msgTypeData.MessageType);

    r = initiator.HandleMessageReceived(payloadHeader, std::move(responderEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    initiator.PollOutput(initiatorEvent, System::Clock::kZero);

    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kAcceptReceived);

    /// initiator prepares block to send
    TransferSession::BlockData blockData;
    const uint8_t data[]   = { 1, 2, 3, 4, 5 };
    blockData.Data         = data;
    blockData.Length       = 5;
    blockData.IsEof        = false;
    blockData.BlockCounter = 0;

    initiator.PrepareBlock(blockData);

    initiator.PollOutput(initiatorEvent, System::Clock::kZero);

    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kMsgToSend);

    EXPECT_EQ(initiatorEvent.msgTypeData.MessageType, to_underlying(MessageType::Block));

    payloadHeader.SetMessageType(initiatorEvent.msgTypeData.ProtocolId, initiatorEvent.msgTypeData.MessageType);

    r = mTransferSession.HandleMessageReceived(payloadHeader, std::move(initiatorEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kBlockReceived);

    /// proxy acknowledges block
    r = proxyDiagnosticLog.Continue();

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kMsgToSend); // responder sends Ack message

    EXPECT_EQ(responderEvent.msgTypeData.MessageType, to_underlying(MessageType::BlockAck));

    /// Now, initiator handles Block Ack message

    payloadHeader.SetMessageType(responderEvent.msgTypeData.ProtocolId, responderEvent.msgTypeData.MessageType);

    r = initiator.HandleMessageReceived(payloadHeader, std::move(responderEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    initiator.PollOutput(initiatorEvent, System::Clock::kZero);

    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kAckReceived);

    /// initiator prepares next block to send
    blockData.BlockCounter = 10; // while respodner expects 1

    initiator.PrepareBlock(blockData);

    initiator.PollOutput(initiatorEvent, System::Clock::kZero);

    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kMsgToSend);

    EXPECT_EQ(initiatorEvent.msgTypeData.MessageType, to_underlying(MessageType::Block));

    /// responder receives 2nd block
    payloadHeader.SetMessageType(initiatorEvent.msgTypeData.ProtocolId, initiatorEvent.msgTypeData.MessageType);

    r = mTransferSession.HandleMessageReceived(payloadHeader, std::move(initiatorEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kBlockReceived);

    /// proxy rejects transfer
    r = proxyDiagnosticLog.Reject(CHIP_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kMsgToSend); // responder sends StatusReport message

    /// Now, initiator handles StatusReport message
    payloadHeader.SetMessageType(responderEvent.msgTypeData.ProtocolId, responderEvent.msgTypeData.MessageType);

    r = initiator.HandleMessageReceived(payloadHeader, std::move(responderEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    initiator.PollOutput(initiatorEvent, System::Clock::kZero);

    EXPECT_EQ(initiatorEvent.EventType,
              TransferSession::OutputEventType::kStatusReceived); /// this implies transfer rejection got received

    initiator.Reset();
    mTransferSession.Reset();
    proxyDiagnosticLog.Reset();
}

TEST_F(TestTransferDiagnosticLog, AccpetsTransferAndReceivesNonNullTerminatedDataCorrectly)
{
    BDXTransferProxyDiagnosticLog proxyDiagnosticLog{};

    TransferSession initiator;
    TransferSession::TransferInitData transferInitData;
    transferInitData.TransferCtlFlags = TransferControlFlags::kSenderDrive;
    transferInitData.MaxBlockSize     = kMaxBlockSize;
    transferInitData.StartOffset      = 0;
    transferInitData.Length           = 1024;
    const char testFileDes[8]         = { 't', 'e', 's', 't', '.', 't', 'x', 't' };
    transferInitData.FileDesLength    = 8;
    transferInitData.FileDesignator   = reinterpret_cast<const uint8_t *>(testFileDes);
    transferInitData.MetadataLength   = kMetaDataLength;
    transferInitData.Metadata         = kMetaData;

    /// Init initiator (and sender) transfer session
    CHIP_ERROR r = initiator.StartTransfer(TransferRole::kSender, transferInitData, System::Clock::Seconds16(10));

    EXPECT_EQ(r, CHIP_NO_ERROR);

    TransferSession::OutputEvent initiatorEvent;
    initiator.PollOutput(initiatorEvent, System::Clock::kZero);
    EXPECT_EQ(initiatorEvent.EventType, TransferSession::OutputEventType::kMsgToSend);

    /// Init responder (and receiver) transfer session
    r = mTransferSession.WaitForTransfer(TransferRole::kReceiver, TransferControlFlags::kSenderDrive, kMaxBlockSize,
                                         System::Clock::Seconds16(20));
    EXPECT_EQ(r, CHIP_NO_ERROR);

    TransferSession::OutputEvent responderEvent;
    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);
    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kNone);

    chip::PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(initiatorEvent.msgTypeData.ProtocolId, initiatorEvent.msgTypeData.MessageType);

    r = mTransferSession.HandleMessageReceived(payloadHeader, std::move(initiatorEvent.MsgData), System::Clock::kZero);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    mTransferSession.PollOutput(responderEvent, System::Clock::kZero);

    EXPECT_EQ(responderEvent.EventType, TransferSession::OutputEventType::kInitReceived);

    /// Proxy fills its file designator data member from transfer session object passed as argument
    r = proxyDiagnosticLog.Init(&mTransferSession);

    EXPECT_EQ(r, CHIP_NO_ERROR);

    CharSpan fileDesignator = proxyDiagnosticLog.GetFileDesignator();

    EXPECT_TRUE(fileDesignator.data_equal(CharSpan{ testFileDes, 8 }));
}

} // namespace
