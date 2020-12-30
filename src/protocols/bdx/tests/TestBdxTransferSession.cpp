#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <transport/raw/MessageHeader.h>

#include <nlunit-test.h>

#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

using namespace ::chip;

static const uint16_t kTestMaxBlockSize = 256;

class DLL_EXPORT MockMessagingDelegate : public BDX::TransferSession::MessagingDelegate
{
public:
    CHIP_ERROR SendMessage(BDX::MessageType msgType, System::PacketBuffer * msgBuf) override
    {
        mLastMessageSent = msgType;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendStatusReport(CHIP_ERROR error) override { mLastStatus = error; }

    BDX::MessageType mLastMessageSent;
    CHIP_ERROR mLastStatus;
};

class DLL_EXPORT MockPlatformDelegate : public BDX::TransferSession::PlatformDelegate
{
public:
    CHIP_ERROR StoreBlock(Encoding::LittleEndian::Reader & blockBuf) override
    {
        ++numStores;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadBlock(BufBound & buffer, uint16_t length) override
    {
        ++numReads;
        return CHIP_NO_ERROR;
    }

    bool AreParametersAcceptable() override { return true; }

    uint16_t numStores = 0;
    uint16_t numReads  = 0;
};

void TestHandleReceiveInit(nlTestSuite * inSuite, void * inContext)
{
    MockMessagingDelegate mockMessagingDelegate;
    MockPlatformDelegate mockPlatformDelegate;
    System::PacketBufferHandle pEmptyMsg = System::PacketBuffer::New(10);

    // Support Sender and Receiver drive, no async, no range offsets
    BDX::TransferControlParams transferParams = { false, true, true };
    BDX::RangeControlParams rangeParams       = { false, 0, 0 };

    // Initialize Receiver state machine
    BDX::TransferSession bdxReceiver(&mockMessagingDelegate, &mockPlatformDelegate);
    bdxReceiver.Init(BDX::kReceiver, transferParams, rangeParams, kTestMaxBlockSize, &mockDelegate);

    // Verify Receiver cannot respond to ReceiveInit
    // TODO: is this an actual constraint?
    bdxReceiver.HandleMessageReceived(BDX::BDXMsgType::kReceiveInit, pEmptyMsg);
    NL_TEST_ASSERT(inSuite, mockDelegate.mLastMessageSent == BDX::BDXMsgType::kStatusReport_Temp);

    // Initialize Sender state machine
    BDX::TransferSession bdxSender;
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
