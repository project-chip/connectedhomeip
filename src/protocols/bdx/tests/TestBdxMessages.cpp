#include <limits>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <protocols/bdx/BdxMessages.h>

using namespace chip;
using namespace chip::bdx;

/**
 * Helper method for testing that WriteToBuffer() and Parse() are successful, and that the parsed message
 * is identical to the origianl.
 */
template <class MsgType>
void TestHelperWrittenAndParsedMatch(MsgType & testMsg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    size_t msgSize = testMsg.MessageSize();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    ASSERT_FALSE(bbuf.IsNull());

    testMsg.WriteToBuffer(bbuf);
    EXPECT_TRUE(bbuf.Fit());

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    ASSERT_FALSE(msgBuf.IsNull());
    System::PacketBufferHandle rcvBuf = System::PacketBufferHandle::NewWithData(msgBuf->Start(), msgSize);
    ASSERT_FALSE(rcvBuf.IsNull());

    MsgType testMsgRcvd;
    err = testMsgRcvd.Parse(std::move(rcvBuf));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(testMsgRcvd, testMsg);
}

struct TestBdxMessages : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        CHIP_ERROR error = chip::Platform::MemoryInit();
        ASSERT_EQ(error, CHIP_NO_ERROR);
    }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestBdxMessages, TestTransferInitMessage)
{
    TransferInit testMsg;

    testMsg.TransferCtlOptions.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);
    testMsg.Version = 1;

    // Make sure MaxLength is greater than UINT32_MAX to test widerange being set
    testMsg.MaxLength = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;

    testMsg.StartOffset  = 42;
    testMsg.MaxBlockSize = 256;

    char testFileDes[9]    = { "test.txt" };
    testMsg.FileDesLength  = 9;
    testMsg.FileDesignator = reinterpret_cast<uint8_t *>(testFileDes);

    uint8_t fakeData[5]    = { 7, 6, 5, 4, 3 };
    testMsg.MetadataLength = 5;
    testMsg.Metadata       = reinterpret_cast<uint8_t *>(fakeData);

    TestHelperWrittenAndParsedMatch<TransferInit>(testMsg);
}

TEST_F(TestBdxMessages, TestSendAcceptMessage)
{
    SendAccept testMsg;

    testMsg.Version = 1;
    testMsg.TransferCtlFlags.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);
    testMsg.MaxBlockSize = 256;

    uint8_t fakeData[5]    = { 7, 6, 5, 4, 3 };
    testMsg.MetadataLength = 5;
    testMsg.Metadata       = reinterpret_cast<uint8_t *>(fakeData);

    TestHelperWrittenAndParsedMatch<SendAccept>(testMsg);
}

TEST_F(TestBdxMessages, TestReceiveAcceptMessage)
{
    ReceiveAccept testMsg;

    testMsg.Version = 1;
    testMsg.TransferCtlFlags.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);

    // Make sure Length is greater than UINT32_MAX to test widerange being set
    testMsg.Length = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;

    testMsg.StartOffset  = 42;
    testMsg.MaxBlockSize = 256;

    uint8_t fakeData[5]    = { 7, 6, 5, 4, 3 };
    testMsg.MetadataLength = 5;
    testMsg.Metadata       = reinterpret_cast<uint8_t *>(fakeData);

    TestHelperWrittenAndParsedMatch<ReceiveAccept>(testMsg);
}

TEST_F(TestBdxMessages, TestCounterMessage)
{
    CounterMessage testMsg;

    testMsg.BlockCounter = 4;

    TestHelperWrittenAndParsedMatch<CounterMessage>(testMsg);
}

TEST_F(TestBdxMessages, TestDataBlockMessage)
{
    DataBlock testMsg;

    testMsg.BlockCounter = 4;
    uint8_t fakeData[5]  = { 7, 6, 5, 4, 3 };
    testMsg.DataLength   = 5;
    testMsg.Data         = reinterpret_cast<uint8_t *>(fakeData);

    TestHelperWrittenAndParsedMatch<DataBlock>(testMsg);
}

TEST_F(TestBdxMessages, TestBlockQueryWithSkipMessage)
{
    BlockQueryWithSkip testMsg;

    testMsg.BlockCounter = 5;
    testMsg.BytesToSkip  = 16;

    TestHelperWrittenAndParsedMatch<BlockQueryWithSkip>(testMsg);
}
