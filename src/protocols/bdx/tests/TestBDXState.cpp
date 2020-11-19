#include <transport/bdx/BDXMessages.h>
#include <transport/bdx/BDXState.h>
#include <transport/raw/MessageHeader.h>

#include <nlunit-test.h>

#include <support/CodeUtils.h>
#include <support/TestUtils.h>

using namespace ::chip;

static const uint16_t kTestMaxBlockSize = 256;

class DLL_EXPORT MockBDXStateDelegate : public BDX::BDXStateDelegate
{
public:
    CHIP_ERROR SendMessage(BDXMsgType msgType, System::PacketBuffer * msgBuf) override
    {
        mLastMessageSent = msgType;
        return CHIP_NO_ERROR;
    }

    uint16_t mLastMessageSent;
};

void TestHandleReceiveInit(nlTestSuite * inSuite, void * inContext)
{
    MockBDXStateDelegate mockDelegate;
    System::PacketBuffer * pEmptyMsg = System::PacketBuffer::New(10);

    // Support Sender and Receiver drive, no async, no range offsets
    BDX::TransferControlParams transferParams = { false, true, true };
    BDX::RangeControlParams rangeParams       = { false, 0, 0 };

    // Initialize Receiver state machine
    BDX::TransferManager bdxReceiver;
    bdxReceiver.Init(BDX::kReceiver, transferParams, rangeParams, kTestMaxBlockSize, &mockDelegate);

    // Verify Receiver cannot respond to ReceiveInit
    // TODO: is this an actual constraint?
    bdxReceiver.HandleMessageReceived(BDX::BDXMsgType::kReceiveInit, pEmptyMsg);
    NL_TEST_ASSERT(inSuite, mockDelegate.mLastMessageSent == BDX::BDXMsgType::kStatusReport_Temp);

    // Initialize Sender state machine
    BDX::TransferManager bdxSender;
    bdxSender.Init(BDX::kSender, transferParams, rangeParams, kTestMaxBlockSize, &mockDelegate);

    // Verify Sender sends ReceiveAccept in response to ReceiveInit
    bdxSender.HandleMessageReceived(BDX::BDXMsgType::kReceiveInit, pEmptyMsg);
    NL_TEST_ASSERT(inSuite, mockDelegate.mLastMessageSent == BDX::BDXMsgType::kReceiveAccept);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("ReceiveInitSenderOnly", TestHandleReceiveInit),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-TransferManager",
    &sTests[0],
    nullptr,
    nullptr
};
// clang-format on

/**
 *  Main
 */
int TestBDXState()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBDXState)
