/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <pw_unit_test/framework.h>

#include <algorithm>
#include <optional>

#include <lib/support/Defer.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/bdx/AsyncTransferFacilitator.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/StatusCode.h>
#include <system/SystemClock.h>

using namespace chip::System::Clock::Literals;

namespace chip {
namespace {

using bdx::StatusCode;
using bdx::TransferSession;

constexpr System::Clock::Seconds16 kBdxTimeout = System::Clock::Seconds16(5 * 60);
constexpr uint16_t kBlockSize                  = 64;

System::Clock::Internal::MockClock gMockClock;
System::Clock::ClockBase * gRealClock = nullptr;

System::Clock::Timestamp Now()
{
    return System::SystemClock().GetMonotonicTimestamp();
}

// What the responder saw, kept outside it because it deletes itself when the transfer ends.
struct ResponderResult
{
    bool mDestroyed              = false;
    CHIP_ERROR mTransferEndError = CHIP_NO_ERROR;
    uint32_t mBlocksServed       = 0;
};

// A BDX sender that serves every BlockQuery, set up the way MTROTAImageTransferHandler is: an AsyncResponder that is the
// exchange delegate for the transfer and deletes itself in DestroySelf().
class TestResponder : public bdx::AsyncResponder
{
public:
    TestResponder(System::Layer & systemLayer, ResponderResult & result, uint32_t blocksInImage, System::Clock::Timeout timeout) :
        mSystemLayer(systemLayer), mResult(result), mBlocksInImage(blocksInImage), mTimeout(timeout)
    {}

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        if (payloadHeader.HasMessageType(bdx::MessageType::ReceiveInit))
        {
            BitFlags<bdx::TransferControlFlags> controlFlags(bdx::TransferControlFlags::kReceiverDrive);
            ReturnErrorOnFailure(Init(&mSystemLayer, ec, bdx::TransferRole::kSender, controlFlags, kBlockSize, mTimeout));
        }
        return AsyncTransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));
    }

private:
    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        switch (event.EventType)
        {
        case TransferSession::OutputEventType::kInitReceived: {
            TransferSession::TransferAcceptData acceptData;
            acceptData.ControlMode  = bdx::TransferControlFlags::kReceiverDrive;
            acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
            acceptData.StartOffset  = mTransfer.GetStartOffset();
            acceptData.Length       = mTransfer.GetTransferLength();
            err                     = mTransfer.AcceptTransfer(acceptData);
            break;
        }
        case TransferSession::OutputEventType::kQueryReceived: {
            mResult.mBlocksServed++;
            TransferSession::BlockData blockData;
            blockData.Data   = mBlock;
            blockData.Length = sizeof(mBlock);
            blockData.IsEof  = (mResult.mBlocksServed == mBlocksInImage);
            err              = mTransfer.PrepareBlock(blockData);
            break;
        }
        default:
            break;
        }
        NotifyEventHandled(event.EventType, err);
    }

    void DestroySelf() override
    {
        mResult.mDestroyed        = true;
        mResult.mTransferEndError = mTransferEndError;
        delete this;
    }

    System::Layer & mSystemLayer;
    ResponderResult & mResult;
    const uint32_t mBlocksInImage;
    const System::Clock::Timeout mTimeout;
    uint8_t mBlock[kBlockSize] = {};
};

class TestResponderFactory : public Messaging::UnsolicitedMessageHandler
{
public:
    TestResponderFactory(System::Layer & systemLayer, ResponderResult & result, uint32_t blocksInImage,
                         System::Clock::Timeout timeout = kBdxTimeout) :
        mSystemLayer(systemLayer),
        mResult(result), mBlocksInImage(blocksInImage), mTimeout(timeout)
    {}

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader &, Messaging::ExchangeDelegate *& newDelegate) override
    {
        newDelegate = new TestResponder(mSystemLayer, mResult, mBlocksInImage, mTimeout);
        return CHIP_NO_ERROR;
    }

private:
    System::Layer & mSystemLayer;
    ResponderResult & mResult;
    const uint32_t mBlocksInImage;
    const System::Clock::Timeout mTimeout;
};

// A BDX receiver that drives the transfer with its own TransferSession, as an OTA requestor does. After mQueriesToSend
// BlockQuery messages it stops querying but keeps the exchange open, which is what the responder sees when a BlockQuery is
// lost: the requestor is still waiting for a Block that will never come.
class TestRequestor : public Messaging::ExchangeDelegate
{
public:
    uint32_t mQueriesToSend = 1;

    CHIP_ERROR Start(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session)
    {
        mExchange = exchangeManager.NewContext(session, this);
        VerifyOrReturnError(mExchange != nullptr, CHIP_ERROR_NO_MEMORY);

        TransferSession::TransferInitData initData;
        initData.TransferCtlFlags = bdx::TransferControlFlags::kReceiverDrive;
        initData.MaxBlockSize     = kBlockSize;
        initData.FileDesLength    = static_cast<uint16_t>(strlen(kFileDesignator));
        initData.FileDesignator   = reinterpret_cast<const uint8_t *>(kFileDesignator);
        // The requestor's own timeouts are not under test, so they must not fire first.
        ReturnErrorOnFailure(mTransfer.StartTransfer(bdx::TransferRole::kReceiver, initData, 10 * kBdxTimeout));
        PollTransfer();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        ReturnErrorOnFailure(mTransfer.HandleMessageReceived(payloadHeader, std::move(payload), Now()));
        PollTransfer();
        // Waiting for a Block that will not come, or for the end of the transfer: either way, stay open.
        if (mExchange != nullptr && !mTransferEnded)
        {
            ec->WillSendMessage();
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext *) override {}
    void OnExchangeClosing(Messaging::ExchangeContext *) override { mExchange = nullptr; }

    void Close()
    {
        if (mExchange != nullptr)
        {
            mExchange->Close();
            mExchange = nullptr;
        }
    }

    std::optional<StatusCode> mStatusReceived;
    std::optional<System::Clock::Timestamp> mStoppedQueryingAt;
    bool mTransferEnded = false;

private:
    static constexpr char kFileDesignator[] = "test.ota";

    void PollTransfer()
    {
        TransferSession::OutputEvent event;
        do
        {
            mTransfer.PollOutput(event, Now());
            switch (event.EventType)
            {
            case TransferSession::OutputEventType::kAcceptReceived:
            case TransferSession::OutputEventType::kBlockReceived:
                if (event.EventType == TransferSession::OutputEventType::kBlockReceived && event.blockdata.IsEof)
                {
                    EXPECT_SUCCESS(mTransfer.PrepareBlockAck());
                    mTransferEnded = true;
                }
                else if (mQueriesToSend > 0)
                {
                    mQueriesToSend--;
                    EXPECT_SUCCESS(mTransfer.PrepareBlockQuery());
                }
                else if (!mStoppedQueryingAt.has_value())
                {
                    mStoppedQueryingAt = Now();
                }
                break;
            case TransferSession::OutputEventType::kStatusReceived:
                mStatusReceived = event.statusData.statusCode;
                mTransferEnded  = true;
                // TransferSession is now in its error state and reports kInternalError on every later poll.
                return;
            case TransferSession::OutputEventType::kInternalError:
                mTransferEnded = true;
                return;
            case TransferSession::OutputEventType::kMsgToSend: {
                ASSERT_NE(mExchange, nullptr);
                Messaging::SendFlags sendFlags;
                if (!event.msgTypeData.HasMessageType(bdx::MessageType::BlockAckEOF))
                {
                    sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
                }
                mExchange->SetResponseTimeout(System::Clock::kZero);
                EXPECT_SUCCESS(mExchange->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                      std::move(event.MsgData), sendFlags));
                break;
            }
            default:
                break;
            }
        } while (event.EventType != TransferSession::OutputEventType::kNone);
    }

    TransferSession mTransfer;
    Messaging::ExchangeContext * mExchange = nullptr;
};

} // namespace

class TestAsyncTransferFacilitator : public Testing::LoopbackMessagingContext
{
public:
    static void SetUpTestSuite()
    {
        LoopbackMessagingContext::SetUpTestSuite();
        VerifyOrReturn(!HasFailure());
        gRealClock = &System::SystemClock();
        System::Clock::Internal::SetSystemClockForTesting(&gMockClock);
    }

    static void TearDownTestSuite()
    {
        System::Clock::Internal::SetSystemClockForTesting(gRealClock);
        LoopbackMessagingContext::TearDownTestSuite();
    }

protected:
    // Starts a transfer of a blocksInImage-block image from Bob's responder to Alice's requestor.
    CHIP_ERROR BeginTransfer(TestRequestor & requestor, TestResponderFactory & factory)
    {
        ReturnErrorOnFailure(
            GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(bdx::MessageType::ReceiveInit, &factory));
        ReturnErrorOnFailure(requestor.Start(GetExchangeManager(), GetSessionAliceToBob()));
        DrainAndServiceIO();
        return CHIP_NO_ERROR;
    }

    void EndTransfer(TestRequestor & requestor)
    {
        GetLoopback().mNumMessagesToDrop   = 0;
        GetLoopback().mDroppedMessageCount = 0;
        requestor.Close();
        DrainAndServiceIO();
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(bdx::MessageType::ReceiveInit));
    }

    static void AdvanceClock(System::Clock::Milliseconds64 duration, System::Clock::Milliseconds64 step = 1_s)
    {
        while (duration > System::Clock::kZero)
        {
            System::Clock::Milliseconds64 thisStep = std::min(duration, step);
            gMockClock.AdvanceMonotonic(thisStep);
            duration -= thisStep;
            GetIOContext().DriveIO();
            DrainAndServiceIO();
        }
    }

    static void AdvanceClockTo(System::Clock::Timestamp target)
    {
        if (target > Now())
        {
            AdvanceClock(target - Now());
        }
    }
};

TEST_F(TestAsyncTransferFacilitator, ResponderTellsRequestorWhenItGivesUpWaitingForBlockQuery)
{
    ResponderResult result;
    TestResponderFactory factory(GetSystemLayer(), result, /* blocksInImage = */ 100);
    TestRequestor requestor;
    requestor.mQueriesToSend = 3;
    auto cleanup             = MakeDefer([&]() { EndTransfer(requestor); });

    ASSERT_SUCCESS(BeginTransfer(requestor, factory));
    ASSERT_TRUE(requestor.mStoppedQueryingAt.has_value());
    ASSERT_EQ(result.mBlocksServed, 3u);

    // The responder waits the full BDX timeout for the next BlockQuery...
    AdvanceClockTo(requestor.mStoppedQueryingAt.value() + kBdxTimeout - 1_s);
    EXPECT_FALSE(result.mDestroyed);
    EXPECT_FALSE(requestor.mStatusReceived.has_value());

    // ...then ends the transfer and tells the requestor, which is still waiting for a Block.
    AdvanceClock(2_s);
    EXPECT_TRUE(result.mDestroyed);
    EXPECT_EQ(result.mTransferEndError, CHIP_ERROR_TIMEOUT);
    ASSERT_TRUE(requestor.mStatusReceived.has_value());
    EXPECT_EQ(requestor.mStatusReceived.value(), StatusCode::kTransferFailedUnknownError);
}

TEST_F(TestAsyncTransferFacilitator, ResponderTellsRequestorWhenNoBlockQueryFollowsReceiveAccept)
{
    ResponderResult result;
    TestResponderFactory factory(GetSystemLayer(), result, /* blocksInImage = */ 100);
    TestRequestor requestor;
    requestor.mQueriesToSend = 0;
    auto cleanup             = MakeDefer([&]() { EndTransfer(requestor); });

    ASSERT_SUCCESS(BeginTransfer(requestor, factory));
    ASSERT_TRUE(requestor.mStoppedQueryingAt.has_value());
    ASSERT_EQ(result.mBlocksServed, 0u);

    AdvanceClockTo(requestor.mStoppedQueryingAt.value() + kBdxTimeout - 1_s);
    EXPECT_FALSE(result.mDestroyed);

    AdvanceClock(2_s);
    EXPECT_TRUE(result.mDestroyed);
    EXPECT_EQ(result.mTransferEndError, CHIP_ERROR_TIMEOUT);
    ASSERT_TRUE(requestor.mStatusReceived.has_value());
    EXPECT_EQ(requestor.mStatusReceived.value(), StatusCode::kTransferFailedUnknownError);
}

TEST_F(TestAsyncTransferFacilitator, CompletedTransferSendsNoStatusReport)
{
    ResponderResult result;
    TestResponderFactory factory(GetSystemLayer(), result, /* blocksInImage = */ 3);
    TestRequestor requestor;
    requestor.mQueriesToSend = 3;
    auto cleanup             = MakeDefer([&]() { EndTransfer(requestor); });

    ASSERT_SUCCESS(BeginTransfer(requestor, factory));
    EXPECT_TRUE(requestor.mTransferEnded);
    EXPECT_TRUE(result.mDestroyed);
    EXPECT_EQ(result.mBlocksServed, 3u);

    // Nothing is left waiting, so no timeout can fire later and report a failure.
    AdvanceClock(2 * kBdxTimeout, 10_s);
    EXPECT_FALSE(requestor.mStatusReceived.has_value());
}

TEST_F(TestAsyncTransferFacilitator, SessionReleaseStillEndsTransferWithTimeout)
{
    ResponderResult result;
    TestResponderFactory factory(GetSystemLayer(), result, /* blocksInImage = */ 100);
    TestRequestor requestor;
    requestor.mQueriesToSend = 3;
    auto cleanup             = MakeDefer([&]() { EndTransfer(requestor); });

    ASSERT_SUCCESS(BeginTransfer(requestor, factory));
    ASSERT_TRUE(requestor.mStoppedQueryingAt.has_value());
    ASSERT_FALSE(result.mDestroyed);

    // Releasing the responder's session ends its exchange through the response timeout path, with no session left to send
    // a StatusReport on. The transfer must still end, and still as a timeout.
    ExpireSessionBobToAlice();
    DrainAndServiceIO();
    EXPECT_TRUE(result.mDestroyed);
    EXPECT_EQ(result.mTransferEndError, CHIP_ERROR_TIMEOUT);
}

TEST_F(TestAsyncTransferFacilitator, UnacknowledgedLastMessageStillEndsTransferWithTimeout)
{
    // Shorter than MRP's retransmission window, so the timeout fires while the responder's last Block is still unacknowledged.
    constexpr System::Clock::Seconds16 kShortTimeout = System::Clock::Seconds16(3);
    ResponderResult result;
    TestResponderFactory factory(GetSystemLayer(), result, /* blocksInImage = */ 100, kShortTimeout);
    TestRequestor requestor;
    requestor.mQueriesToSend = 1;
    auto cleanup             = MakeDefer([&]() { EndTransfer(requestor); });

    ASSERT_SUCCESS(BeginTransfer(requestor, factory));
    ASSERT_TRUE(requestor.mStoppedQueryingAt.has_value());

    // The requestor has gone away: nothing reaches it, not even an ack for the last Block.
    GetLoopback().mNumMessagesToDrop = GetLoopback().kUnlimitedMessageCount;
    AdvanceClock(kShortTimeout + 1_s, 100_ms);
    EXPECT_TRUE(result.mDestroyed);
    EXPECT_EQ(result.mTransferEndError, CHIP_ERROR_TIMEOUT);
}

} // namespace chip
