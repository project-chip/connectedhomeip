#include <protocols/bdx/BDXMessageUtils.h>

#include <nlunit-test.h>

#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

#include <limits>

using namespace chip;
using namespace chip::BDX;

void TestTransferInitMessage(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TransferInit testMsg;

    testMsg.mSupportsAsync         = false;
    testMsg.mSupportsReceiverDrive = false;
    testMsg.mSupportsSenderDrive   = true;
    testMsg.mSupportedVersions     = 1;

    // Make sure mMaxLength is greater than UINT32_MAX to test widerange being set
    testMsg.mMaxLength = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;

    testMsg.mStartOffset  = 42;
    testMsg.mMaxBlockSize = 256;

    char testFileDes[9]     = { "test.txt" };
    testMsg.mFileDesLength  = 9;
    testMsg.mFileDesignator = reinterpret_cast<uint8_t *>(testFileDes);

    uint8_t fakeData[5]     = { 7, 6, 5, 4, 3 };
    testMsg.mMetadataLength = 5;
    testMsg.mMetadata       = reinterpret_cast<uint8_t *>(fakeData);

    size_t msgSize = testMsg.PackedSize();

    System::PacketBuffer * buf = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgSize));
    NL_TEST_ASSERT(inSuite, buf != nullptr);

    err = testMsg.Pack(*buf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    System::PacketBuffer * rcvBuf = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgSize));
    NL_TEST_ASSERT(inSuite, rcvBuf != nullptr);
    memcpy(rcvBuf->Start(), buf->Start(), msgSize);
    rcvBuf->SetDataLength(static_cast<uint16_t>(msgSize));

    TransferInit testMsgRcvd;
    err = TransferInit::Parse(*rcvBuf, testMsgRcvd);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testMsgRcvd == testMsg);
}

void TestSendAcceptMessage(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SendAccept testMsg;

    testMsg.mVersion          = 1;
    testMsg.mUseAsync         = false;
    testMsg.mUseReceiverDrive = true;
    testMsg.mUseSenderDrive   = false;
    testMsg.mMaxBlockSize     = 256;

    uint8_t fakeData[5]     = { 7, 6, 5, 4, 3 };
    testMsg.mMetadataLength = 5;
    testMsg.mMetadata       = reinterpret_cast<uint8_t *>(fakeData);

    size_t msgSize = testMsg.PackedSize();

    System::PacketBuffer * buf = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgSize));
    NL_TEST_ASSERT(inSuite, buf != nullptr);

    err = testMsg.Pack(*buf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    System::PacketBuffer * rcvBuf = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgSize));
    NL_TEST_ASSERT(inSuite, rcvBuf != nullptr);
    memcpy(rcvBuf->Start(), buf->Start(), msgSize);
    rcvBuf->SetDataLength(static_cast<uint16_t>(msgSize));

    SendAccept testMsgRcvd;
    err = SendAccept::Parse(*rcvBuf, testMsgRcvd);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testMsgRcvd == testMsg);
}

void TestReceiveAcceptMessage(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReceiveAccept testMsg;

    testMsg.mVersion          = 1;
    testMsg.mUseAsync         = false;
    testMsg.mUseReceiverDrive = true;
    testMsg.mUseSenderDrive   = false;

    // Make sure mLength is greater than UINT32_MAX to test widerange being set
    testMsg.mLength = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;

    testMsg.mStartOffset  = 42;
    testMsg.mMaxBlockSize = 256;

    uint8_t fakeData[5]     = { 7, 6, 5, 4, 3 };
    testMsg.mMetadataLength = 5;
    testMsg.mMetadata       = reinterpret_cast<uint8_t *>(fakeData);

    size_t msgSize = testMsg.PackedSize();

    System::PacketBuffer * buf = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgSize));
    NL_TEST_ASSERT(inSuite, buf != nullptr);

    err = testMsg.Pack(*buf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    System::PacketBuffer * rcvBuf = System::PacketBuffer::NewWithAvailableSize(static_cast<uint16_t>(msgSize));
    NL_TEST_ASSERT(inSuite, rcvBuf != nullptr);
    memcpy(rcvBuf->Start(), buf->Start(), msgSize);
    rcvBuf->SetDataLength(static_cast<uint16_t>(msgSize));

    ReceiveAccept testMsgRcvd;
    err = ReceiveAccept::Parse(*rcvBuf, testMsgRcvd);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testMsgRcvd == testMsg);
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

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-BDXMessageUtils",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

/**
 *  Main
 */
int TestBDXMessageUtils()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBDXMessageUtils)
