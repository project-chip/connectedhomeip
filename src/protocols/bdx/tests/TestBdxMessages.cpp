#include <protocols/bdx/BdxMessages.h>

#include <nlunit-test.h>

#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>

#include <limits>

using namespace chip;
using namespace chip::bdx;

/**
 * Helper method for testing that WriteToBuffer() and Parse() are successful, and that the parsed message
 * is identical to the origianl.
 */
template <class MsgType>
void TestHelperWrittenAndParsedMatch(nlTestSuite * inSuite, void * inContext, MsgType & testMsg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    size_t msgSize = testMsg.MessageSize();
    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(msgSize));
    NL_TEST_ASSERT(inSuite, !bbuf.IsNull());

    testMsg.WriteToBuffer(bbuf);
    NL_TEST_ASSERT(inSuite, bbuf.Fit());

    System::PacketBufferHandle msgBuf = bbuf.Finalize();
    NL_TEST_ASSERT(inSuite, !msgBuf.IsNull());
    System::PacketBufferHandle rcvBuf = System::PacketBufferHandle::NewWithData(msgBuf->Start(), msgSize);
    NL_TEST_ASSERT(inSuite, !rcvBuf.IsNull());

    MsgType testMsgRcvd;
    err = testMsgRcvd.Parse(std::move(rcvBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testMsgRcvd == testMsg);
}

void TestTransferInitMessage(nlTestSuite * inSuite, void * inContext)
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

    TestHelperWrittenAndParsedMatch<TransferInit>(inSuite, inContext, testMsg);
}

void TestSendAcceptMessage(nlTestSuite * inSuite, void * inContext)
{
    SendAccept testMsg;

    testMsg.Version = 1;
    testMsg.TransferCtlFlags.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);
    testMsg.MaxBlockSize = 256;

    uint8_t fakeData[5]    = { 7, 6, 5, 4, 3 };
    testMsg.MetadataLength = 5;
    testMsg.Metadata       = reinterpret_cast<uint8_t *>(fakeData);

    TestHelperWrittenAndParsedMatch<SendAccept>(inSuite, inContext, testMsg);
}

void TestReceiveAcceptMessage(nlTestSuite * inSuite, void * inContext)
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

    TestHelperWrittenAndParsedMatch<ReceiveAccept>(inSuite, inContext, testMsg);
}

void TestCounterMessage(nlTestSuite * inSuite, void * inContext)
{
    CounterMessage testMsg;

    testMsg.BlockCounter = 4;

    TestHelperWrittenAndParsedMatch<CounterMessage>(inSuite, inContext, testMsg);
}

void TestDataBlockMessage(nlTestSuite * inSuite, void * inContext)
{
    DataBlock testMsg;

    testMsg.BlockCounter = 4;
    uint8_t fakeData[5]  = { 7, 6, 5, 4, 3 };
    testMsg.DataLength   = 5;
    testMsg.Data         = reinterpret_cast<uint8_t *>(fakeData);

    TestHelperWrittenAndParsedMatch<DataBlock>(inSuite, inContext, testMsg);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestTransferInitMessage", TestTransferInitMessage),
    NL_TEST_DEF("TestSendAcceptMessage", TestSendAcceptMessage),
    NL_TEST_DEF("TestReceiveAcceptMessage", TestReceiveAcceptMessage),
    NL_TEST_DEF("TestCounterMessage", TestCounterMessage),
    NL_TEST_DEF("TestDataBlockMessage", TestDataBlockMessage),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-BdxMessages",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

/**
 *  Main
 */
int TestBdxMessages()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBdxMessages)
