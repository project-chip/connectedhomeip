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

    CHIP_ERROR OnFileDesignatorReceived(const uint8_t * fileDesignator, uint16_t length) override
    {
        if (fileDesignator == nullptr)
            printf("====== file des null\n");

        if ((fileDesignator != NULL) && (length > 0))
        {
            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INTERNAL;
        }
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

    // Initialize a valid TransferInit message
    ReceiveInit testMsg;
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

void TestInitiatingReceiverFullExchange(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    MockMessagingDelegate receiverMsgDelegate;
    MockMessagingDelegate senderMsgDelegate;
    MockPlatformDelegate platformDelegate; // share platfrom delegate since this test won't query it
    TransferSession initiatingReceiver;
    TransferSession respondingSender;

    // Initialize a valid ReceiveInit message
    ReceiveInit initMsg;
    initMsg.TransferCtlOptions.Set(kReceiverDrive);
    initMsg.MaxBlockSize   = kTestMaxBlockSize;
    char testFileDes[9]    = { "test.txt" };
    initMsg.FileDesLength  = 9;
    initMsg.FileDesignator = reinterpret_cast<uint8_t *>(testFileDes);

    // Initialize RespondingSender to wait for ReceiveInit message
    BitFlags<uint8_t, TransferControlFlags> xferOpts;
    xferOpts.Set(kReceiverDrive);
    respondingSender.WaitForTransfer(&senderMsgDelegate, &platformDelegate, kSender, xferOpts, kTestMaxBlockSize);

    // Verify StartTransfer(kReceiver...) sends ReceiveInit messsage
    initiatingReceiver.StartTransfer(&receiverMsgDelegate, &platformDelegate, kReceiver, initMsg);
    NL_TEST_ASSERT(inSuite, receiverMsgDelegate.mNumSentMsgs == 1);
    NL_TEST_ASSERT(inSuite, receiverMsgDelegate.mLastMessageType == kReceiveInit);

    // Verfiy Sender responds to ReceiveInit with ReceiveAccept
    err = respondingSender.HandleMessageReceived(std::move(receiverMsgDelegate.mLastMessageSent));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, senderMsgDelegate.mNumSentMsgs == 1);
    NL_TEST_ASSERT(inSuite, senderMsgDelegate.mLastMessageType == kReceiveAccept);

    // Since this is ReceiverDrive, the Receiver should now respond to ReceiveAccept with BlockQuery
    err = initiatingReceiver.HandleMessageReceived(std::move(senderMsgDelegate.mLastMessageSent));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, receiverMsgDelegate.mNumSentMsgs == 2);
    NL_TEST_ASSERT(inSuite, receiverMsgDelegate.mLastMessageType == kBlockQuery);

    // Verify no errors during the entire exchange
    NL_TEST_ASSERT(inSuite, senderMsgDelegate.mNumErr == 0);
    NL_TEST_ASSERT(inSuite, receiverMsgDelegate.mNumErr == 0);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestStartTransfer", TestStartTransfer),
    NL_TEST_DEF("TestInitiatingReceiverFullExchange", TestInitiatingReceiverFullExchange),
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
