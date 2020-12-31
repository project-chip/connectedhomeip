#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <transport/raw/MessageHeader.h>

#include <nlunit-test.h>

#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

using namespace ::chip;
using namespace ::chip::BDX;

static const uint16_t kTestMaxBlockSize = 256;

class DLL_EXPORT MockMessagingDelegate : public TransferSession::MessagingDelegate
{
public:
    CHIP_ERROR SendMessage(System::PacketBufferHandle msg) override
    {
        ++mNumSentMsgs;
        if (!msg.IsNull())
        {
            PayloadHeader payloadHeader;
            uint16_t headerSize;
            payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
            msg->ConsumeHead(headerSize);
            mLastMessageType = payloadHeader.GetMessageType();
            mLastMessageSent = std::move(msg);
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnTransferError(StatusCode code) override
    {
        ++mNumErr;
        mLastError = code;
        return CHIP_NO_ERROR;
    }

    System::PacketBufferHandle mLastMessageSent;
    uint8_t mLastMessageType;
    StatusCode mLastError;
    uint32_t mNumSentMsgs = 0;
    uint32_t mNumErr      = 0;
};

class DLL_EXPORT MockPlatformDelegate : public TransferSession::PlatformDelegate
{
public:
    CHIP_ERROR StoreBlock(Encoding::LittleEndian::Reader & blockBuf) override
    {
        ++mNumStores;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadBlock(BufBound & buffer, uint16_t length) override
    {
        ++mNumReads;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ChooseControlMode(const BitFlags<uint8_t, TransferControlFlags> & proposed,
                                 const BitFlags<uint8_t, TransferControlFlags> & supported, TransferControlFlags & choice) override
    {

        return CHIP_NO_ERROR;
    }

    uint32_t mNumStores = 0;
    uint32_t mNumReads  = 0;
};

void TestStartTransfer(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MockMessagingDelegate mockMsgDelegate;
    MockPlatformDelegate mockPlatformDelegate;
    TransferSession initiatingReceiver;

    // Initialize a correct TransferInit message
    TransferInit testMsg;
    testMsg.TransferCtlOptions.SetRaw(0).Set(kReceiverDrive, true);
    testMsg.MaxBlockSize   = kTestMaxBlockSize;
    char testFileDes[9]    = { "test.txt" };
    testMsg.FileDesLength  = 9;
    testMsg.FileDesignator = reinterpret_cast<uint8_t *>(testFileDes);

    err = initiatingReceiver.StartTransfer(&mockMsgDelegate, &mockPlatformDelegate, kReceiver, testMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify StartTransfer(kReceiver...) sends ReceiveInit messsage
    NL_TEST_ASSERT(inSuite, mockMsgDelegate.mNumSentMsgs == 1);
    NL_TEST_ASSERT(inSuite, mockMsgDelegate.mLastMessageType == kReceiveInit);

    TransferSession initiatingSender;

    err = initiatingSender.StartTransfer(&mockMsgDelegate, &mockPlatformDelegate, kSender, testMsg);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify StartTransfer(kSender...) sends SendInit messsage
    NL_TEST_ASSERT(inSuite, mockMsgDelegate.mNumSentMsgs == 2);
    NL_TEST_ASSERT(inSuite, mockMsgDelegate.mLastMessageType == kSendInit);

    NL_TEST_ASSERT(inSuite, mockMsgDelegate.mNumErr == 0);
}

/*
void TestHandleReceiveInit(nlTestSuite * inSuite, void * inContext)
{
    MockMessagingDelegate mockResponderDelegate;
    MockMessagingDelegate mockInitiatorDelegate;
    MockPlatformDelegate mockPlatformDelegate;
    System::PacketBufferHandle pEmptyMsg = System::PacketBuffer::New(10);

    BitFlags<uint8_t, TransferControlFlags> xferOpts;
    xferOpts.Set(kSenderDrive);

    ReceiveInit testMsg;
    testMsg.TransferCtlOptions.SetRaw(0).Set(kReceiverDrive, true);
    testMsg.MaxBlockSize   = kTestMaxBlockSize;
    char testFileDes[9]    = { "test.txt" };
    testMsg.FileDesLength  = 9;
    testMsg.FileDesignator = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize Receiver state machine
    TransferSession respondingReceiver(&mockResponderDelegate, &mockPlatformDelegate);
    respondingReceiver.WaitForTransfer(kReceiver, xferOpts, kTestMaxBlockSize);

    TransferSession initiatingReceiver(&mockInitiatorDelegate, &mockPlatformDelegate);
    initiatingReceiver.StartTransfer(kReceiver, testMsg);

    // Verify StartTransfer(kReceiver...) sends ReceiveInit messsage
    NL_TEST_ASSERT(inSuite, mockInitiatorDelegate.mLastMessageType == kReceiveInit);

    respondingReceiver.HandleMessageReceived(std::move(mockInitiatorDelegate.mLastMessageSent));

    // Verify Responding Receiver cannot reply to ReceiveInit
    // TODO: is this an actual constraint?
    bdxSender.HandleMessageReceived(kReceiveInit, pEmptyMsg);
    NL_TEST_ASSERT(inSuite, mockDelegate.mLastMessageSent == BDX::BDXMsgType::kStatusReport_Temp);

    // Initialize Sender state machine
    BDX::TransferSession bdxSender;
    bdxSender.Init(BDX::kSender, transferParams, rangeParams, kTestMaxBlockSize, &mockDelegate);

    // Verify Sender sends ReceiveAccept in response to ReceiveInit
    bdxSender.HandleMessageReceived(BDX::BDXMsgType::kReceiveInit, pEmptyMsg);
    NL_TEST_ASSERT(inSuite, mockDelegate.mLastMessageSent == BDX::BDXMsgType::kReceiveAccept);
}*/

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestStartTransfer", TestStartTransfer),
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
