#include <protocols/bdx/BDXMessageUtils.h>
#include <transport/raw/MessageHeader.h>

#include <nlunit-test.h>

#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <support/TestUtils.h>

#include <string>

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

    testMsg.mWideRange   = true;
    testMsg.mStartOffset = true;
    testMsg.mDefLen      = true;

    testMsg.mMaxLength    = 1024;
    testMsg.mMaxBlockSize = 256;

    char testFileDes[9]     = { "test.txt" };
    testMsg.mFileDesLength  = 9;
    testMsg.mFileDesignator = reinterpret_cast<uint8_t *>(testFileDes);

    uint8_t fakeData[5]     = { 7, 6, 5, 4, 3 };
    testMsg.mMetadataLength = 5;
    testMsg.mMetadata       = reinterpret_cast<uint8_t *>(fakeData);

    // Verify pack is successful
    System::PacketBuffer * buf = System::PacketBuffer::NewWithAvailableSize(512);
    err                        = testMsg.Pack(buf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify successful unpack, and unpacked message is identical to original
    TransferInit testMsgRcvd;
    err = TransferInit::Parse(buf, testMsgRcvd);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testMsgRcvd == testMsg);
}

void TestSendAcceptMessage(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SendAccept testMsg;
    testMsg.mVersion      = 1;
    testMsg.mControlMode  = kReceiverDrive;
    testMsg.mMaxBlockSize = 256;

    uint8_t fakeData[5]     = { 7, 6, 5, 4, 3 };
    testMsg.mMetadataLength = 5;
    testMsg.mMetadata       = reinterpret_cast<uint8_t *>(fakeData);

    // Verify pack is successful
    System::PacketBuffer * buf = System::PacketBuffer::NewWithAvailableSize(512);
    err                        = testMsg.Pack(buf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify successful unpack, and unpacked message is identical to original
    SendAccept testMsgRcvd;
    err = SendAccept::Parse(buf, testMsgRcvd);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testMsgRcvd == testMsg);
}

void TestReceiveAcceptMessage(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReceiveAccept testMsg;
    testMsg.mVersion     = 1;
    testMsg.mControlMode = kReceiverDrive;

    testMsg.mWideRange   = true;
    testMsg.mStartOffset = true;
    testMsg.mDefLen      = true;

    testMsg.mLength       = 1024;
    testMsg.mMaxBlockSize = 256;

    uint8_t fakeData[5]     = { 7, 6, 5, 4, 3 };
    testMsg.mMetadataLength = 5;
    testMsg.mMetadata       = reinterpret_cast<uint8_t *>(fakeData);

    // Verify pack is successful
    System::PacketBuffer * buf = System::PacketBuffer::NewWithAvailableSize(512);
    err                        = testMsg.Pack(buf);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify successful unpack, and unpacked message is identical to original
    ReceiveAccept testMsgRcvd;
    err = ReceiveAccept::Parse(buf, testMsgRcvd);
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
