/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
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

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/OTARequestorDriver.h>
#include <lib/support/Defer.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <messaging/tests/MessagingContext.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <system/SystemClock.h>

using namespace chip::System::Clock::Literals;

namespace chip {
namespace {

using app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;
using bdx::TransferSession;

// Mirrors kDownloadTimeoutSec in DefaultOTARequestor.cpp.
constexpr System::Clock::Seconds32 kBdxTimeout = System::Clock::Seconds32(5 * 60);

// Every block the provider serves is exactly this size; a shorter one would be read as the last block.
constexpr uint16_t kBlockSize = 64;

System::Clock::Internal::MockClock gMockClock;
System::Clock::ClockBase * gRealClock = nullptr;

System::Clock::Timestamp Now()
{
    return System::SystemClock().GetMonotonicTimestamp();
}

class TestOTARequestorDriver : public OTARequestorDriver
{
public:
    bool CanConsent() override { return false; }
    void SetMaxDownloadBlockSize(uint16_t) override {}
    void HandleIdleStateExit() override {}
    void HandleIdleStateEnter(IdleStateReason) override {}
    void UpdateAvailable(const UpdateDescription &, System::Clock::Seconds32) override {}
    CHIP_ERROR UpdateNotFound(UpdateNotFoundReason, System::Clock::Seconds32) override { return CHIP_NO_ERROR; }
    void UpdateDownloaded() override {}
    void UpdateConfirmed(System::Clock::Seconds32) override {}
    void UpdateSuspended(System::Clock::Seconds32) override {}
    void UpdateDiscontinued() override {}
    void UpdateCancelled() override {}
    void OTACommissioningCallback() override {}
    void ProcessAnnounceOTAProviders(const ProviderLocationType &,
                                     app::Clusters::OtaSoftwareUpdateRequestor::AnnouncementReasonEnum) override
    {}
    void SendQueryImage() override {}
    bool GetNextProviderLocation(ProviderLocationType &, bool & listExhausted) override
    {
        listExhausted = true;
        return false;
    }
};

class TestImageProcessor : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Finalize() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Apply() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Abort() override
    {
        mAborted = true;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR ProcessBlock(ByteSpan &) override
    {
        mFetchPending = true;
        return CHIP_NO_ERROR;
    }
    bool IsFirstImageRun() override { return false; }
    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }

    // Set by ProcessBlock; the test fetches the next block on the next tick, as a real processor would asynchronously.
    bool mFetchPending = false;
    bool mAborted      = false;
};

class DownloadStateRecorder : public BDXDownloader::StateDelegate
{
public:
    void OnDownloadStateChanged(OTADownloader::State state, OTAChangeReasonEnum reason) override
    {
        if (state == OTADownloader::State::kIdle && reason == OTAChangeReasonEnum::kTimeOut)
        {
            mTimeOutCount++;
        }
        if (state == OTADownloader::State::kComplete)
        {
            mCompleted = true;
        }
    }
    void OnUpdateProgressChanged(app::DataModel::Nullable<uint8_t>) override {}

    unsigned mTimeOutCount = 0;
    bool mCompleted        = false;
};

// OTA provider configured per test. Messages it doesn't answer are still MRP-acked, so it looks reachable but silent.
class TestProvider : public Messaging::UnsolicitedMessageHandler, public Messaging::ExchangeDelegate
{
public:
    bool mAcceptTransfer = true;
    // How many BlockQuery messages to answer before going silent.
    uint32_t mBlocksToServe = 1;
    bool mEofOnLastBlock    = false;
    // How long to hold each ReceiveInit and BlockQuery before answering it.
    System::Clock::Milliseconds64 mResponseDelay = System::Clock::kZero;

    CHIP_ERROR Init()
    {
        BitFlags<bdx::TransferControlFlags> controlFlags(bdx::TransferControlFlags::kReceiverDrive);
        return mTransfer.WaitForTransfer(bdx::TransferRole::kSender, controlFlags, kBlockSize, kBdxTimeout);
    }

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader &, Messaging::ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        mExchange            = ec;
        const bool isRequest = payloadHeader.HasMessageType(bdx::MessageType::ReceiveInit) ||
            payloadHeader.HasMessageType(bdx::MessageType::BlockQuery);
        ReturnErrorOnFailure(mTransfer.HandleMessageReceived(payloadHeader, std::move(payload), Now()));

        if (isRequest && mResponseDelay > System::Clock::kZero)
        {
            // Keeps the exchange open until ServiceDeferredResponse() answers.
            ec->WillSendMessage();
            mRespondAt = Now() + mResponseDelay;
            return CHIP_NO_ERROR;
        }

        PollTransfer();
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext *) override {}
    void OnExchangeClosing(Messaging::ExchangeContext *) override { mExchange = nullptr; }

    void ServiceDeferredResponse()
    {
        if (mRespondAt.has_value() && Now() >= mRespondAt.value())
        {
            mRespondAt.reset();
            PollTransfer();
        }
    }

    bool IsStalled() const { return mStalledAt.has_value(); }
    // Only meaningful once IsStalled() is true.
    System::Clock::Timestamp StalledAt() const { return mStalledAt.value_or(System::Clock::kZero); }

private:
    void MarkStalled()
    {
        if (!mStalledAt.has_value())
        {
            mStalledAt = Now();
        }
    }

    void PollTransfer()
    {
        TransferSession::OutputEvent event;
        do
        {
            mTransfer.PollOutput(event, Now());
            switch (event.EventType)
            {
            case TransferSession::OutputEventType::kInitReceived: {
                if (!mAcceptTransfer)
                {
                    MarkStalled();
                    break;
                }
                TransferSession::TransferAcceptData acceptData;
                acceptData.ControlMode  = bdx::TransferControlFlags::kReceiverDrive;
                acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
                acceptData.StartOffset  = mTransfer.GetStartOffset();
                acceptData.Length       = mTransfer.GetTransferLength();
                EXPECT_SUCCESS(mTransfer.AcceptTransfer(acceptData));
                break;
            }
            case TransferSession::OutputEventType::kQueryReceived: {
                if (mBlocksServed >= mBlocksToServe)
                {
                    MarkStalled();
                    break;
                }
                mBlocksServed++;
                TransferSession::BlockData blockData;
                blockData.Data   = mBlock;
                blockData.Length = sizeof(mBlock);
                blockData.IsEof  = mEofOnLastBlock && (mBlocksServed == mBlocksToServe);
                EXPECT_SUCCESS(mTransfer.PrepareBlock(blockData));
                break;
            }
            case TransferSession::OutputEventType::kMsgToSend: {
                ASSERT_NE(mExchange, nullptr);
                Messaging::SendFlags sendFlags;
                sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
                EXPECT_SUCCESS(mExchange->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                      std::move(event.MsgData), sendFlags));
                break;
            }
            case TransferSession::OutputEventType::kStatusReceived:
            case TransferSession::OutputEventType::kInternalError:
                // The transfer is over, and TransferSession would report kInternalError on every later poll. A request
                // this provider was holding kept its exchange open, so drop that response and close the exchange too:
                // left open, it outlives this provider, its delegate.
                mTransfer.Reset();
                if (mRespondAt.has_value() && mExchange != nullptr)
                {
                    mRespondAt.reset();
                    Messaging::ExchangeContext * exchange = mExchange;
                    mExchange                             = nullptr;
                    exchange->Close();
                }
                return;
            default:
                break;
            }
        } while (event.EventType != TransferSession::OutputEventType::kNone);
    }

    TransferSession mTransfer;
    Messaging::ExchangeContext * mExchange = nullptr;
    uint8_t mBlock[kBlockSize]             = {};
    uint32_t mBlocksServed                 = 0;
    std::optional<System::Clock::Timestamp> mRespondAt;
    std::optional<System::Clock::Timestamp> mStalledAt;
};

struct DownloadHarness
{
    DefaultOTARequestor requestor;
    BDXDownloader downloader;
    TestOTARequestorDriver driver;
    TestImageProcessor processor;
    DownloadStateRecorder recorder;
    TestProvider provider;
    // BDXMessenger's members are only initialized by StartDownload, so cleanup must not touch it before then.
    bool downloadStarted = false;
};

} // namespace

class TestDefaultOTARequestorDownload : public Testing::LoopbackMessagingContext
{
public:
    static void SetUpTestSuite()
    {
        LoopbackMessagingContext::SetUpTestSuite();
        VerifyOrReturn(!HasFailure());

        // Run the downloader's progress timer on this test's system layer, so both timeouts are tested together.
        DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
        gRealClock = &System::SystemClock();
        System::Clock::Internal::SetSystemClockForTesting(&gMockClock);
    }

    static void TearDownTestSuite()
    {
        System::Clock::Internal::SetSystemClockForTesting(gRealClock);
        DeviceLayer::SetSystemLayerForTesting(nullptr);
        LoopbackMessagingContext::TearDownTestSuite();
    }

protected:
    CHIP_ERROR BeginDownload(DownloadHarness & h)
    {
        ReturnErrorOnFailure(h.provider.Init());
        ReturnErrorOnFailure(
            GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(bdx::MessageType::ReceiveInit, &h.provider));

        h.requestor.mOtaRequestorDriver = &h.driver;
        h.requestor.mBdxDownloader      = &h.downloader;
        h.requestor.mFileDesignator     = "test.ota"_span;
        h.downloader.SetImageProcessorDelegate(&h.processor);
        ReturnErrorOnFailure(h.requestor.StartDownload(GetExchangeManager(), GetSessionAliceToBob()));
        h.downloadStarted = true;

        // StartDownload makes the requestor the state delegate, which writes to cluster attributes these tests do not set up.
        h.downloader.SetStateDelegate(&h.recorder);
        ReturnErrorOnFailure(h.downloader.OnPreparedForDownload(CHIP_NO_ERROR));
        DrainAndServiceIO();
        return CHIP_NO_ERROR;
    }

    static void ResetMessenger(DownloadHarness & h) { h.requestor.mBdxMessenger.Reset(); }

    void FinishDownload(DownloadHarness & h)
    {
        if (h.downloadStarted)
        {
            // Only does anything if an assertion failed before the transfer ended.
            if (h.downloader.GetState() == OTADownloader::State::kInProgress)
            {
                h.downloader.EndDownload();
            }
            h.requestor.mBdxMessenger.Reset();
        }
        GetLoopback().mNumMessagesToDrop   = 0;
        GetLoopback().mDroppedMessageCount = 0;
        DrainAndServiceIO();
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(bdx::MessageType::ReceiveInit));
    }

    static void AdvanceClock(System::Clock::Milliseconds64 duration, DownloadHarness & h, System::Clock::Milliseconds64 step = 1_s)
    {
        while (duration > System::Clock::kZero)
        {
            System::Clock::Milliseconds64 thisStep = std::min(duration, step);
            gMockClock.AdvanceMonotonic(thisStep);
            duration -= thisStep;

            GetIOContext().DriveIO();
            DrainAndServiceIO();

            h.provider.ServiceDeferredResponse();
            DrainAndServiceIO();

            if (h.processor.mFetchPending)
            {
                h.processor.mFetchPending = false;
                // The last block completes the transfer instead of asking for another.
                if (h.downloader.GetState() == OTADownloader::State::kInProgress)
                {
                    EXPECT_SUCCESS(h.downloader.FetchNextData());
                    DrainAndServiceIO();
                }
            }
        }
    }

    static void AdvanceClockTo(System::Clock::Timestamp target, DownloadHarness & h)
    {
        if (target > Now())
        {
            AdvanceClock(target - Now(), h);
        }
    }

    static void WaitForProviderToStall(DownloadHarness & h)
    {
        for (int i = 0; i < 50 && !h.provider.IsStalled(); i++)
        {
            AdvanceClock(100_ms, h);
        }
    }
};

TEST_F(TestDefaultOTARequestorDownload, ProviderThatStopsRespondingTimesOutAfterBdxTimeout)
{
    DownloadHarness h;
    auto cleanup = MakeDefer([&]() { FinishDownload(h); });

    ASSERT_SUCCESS(BeginDownload(h));
    WaitForProviderToStall(h);
    ASSERT_TRUE(h.provider.IsStalled());
    ASSERT_EQ(h.downloader.GetState(), OTADownloader::State::kInProgress);

    // The requestor sent the unanswered BlockQuery at StalledAt(). It must keep waiting for the full BDX timeout...
    AdvanceClockTo(h.provider.StalledAt() + kBdxTimeout - 1_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 0u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kInProgress);

    // ...and give up once it has. Without the exchange's response timeout, the progress timer would add up to another full period.
    AdvanceClock(2_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 1u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kIdle);
    EXPECT_TRUE(h.processor.mAborted);
}

TEST_F(TestDefaultOTARequestorDownload, ProviderThatNeverAcceptsTimesOutOnceAfterBdxTimeout)
{
    DownloadHarness h;
    auto cleanup = MakeDefer([&]() { FinishDownload(h); });

    h.provider.mAcceptTransfer = false;
    ASSERT_SUCCESS(BeginDownload(h));
    WaitForProviderToStall(h);
    ASSERT_TRUE(h.provider.IsStalled());

    AdvanceClockTo(h.provider.StalledAt() + kBdxTimeout - 1_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 0u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kInProgress);

    // Nothing was received, so both timers expire together; only one of them may end the download.
    AdvanceClock(2_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 1u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kIdle);

    AdvanceClock(2 * kBdxTimeout, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 1u);
}

TEST_F(TestDefaultOTARequestorDownload, LostMessagesDoNotEndTransferBeforeBdxTimeout)
{
    DownloadHarness h;
    auto cleanup = MakeDefer([&]() { FinishDownload(h); });

    GetLoopback().mNumMessagesToDrop = GetLoopback().kUnlimitedMessageCount;
    ASSERT_SUCCESS(BeginDownload(h));
    System::Clock::Timestamp receiveInitSentAt = Now();

    // MRP gives up on ReceiveInit within seconds, but the requestor must still wait the full BDX timeout.
    AdvanceClock(60_s, h);
    EXPECT_GT(GetLoopback().mDroppedMessageCount, static_cast<uint32_t>(CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS));
    EXPECT_EQ(h.recorder.mTimeOutCount, 0u);

    AdvanceClockTo(receiveInitSentAt + kBdxTimeout - 1_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 0u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kInProgress);

    AdvanceClock(2_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 1u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kIdle);
}

TEST_F(TestDefaultOTARequestorDownload, SlowProviderGetsFullBdxTimeoutForEachResponse)
{
    DownloadHarness h;
    auto cleanup = MakeDefer([&]() { FinishDownload(h); });

    // Each response takes 240 s, inside the BDX timeout, so the timeout must apply per response, not per transfer.
    h.provider.mResponseDelay  = 240_s;
    h.provider.mBlocksToServe  = 3;
    h.provider.mEofOnLastBlock = true;
    ASSERT_SUCCESS(BeginDownload(h));

    // ReceiveInit and three BlockQuery messages, each answered after mResponseDelay.
    AdvanceClock(4 * 240_s + 60_s, h, 10_s);
    EXPECT_TRUE(h.recorder.mCompleted);
    EXPECT_EQ(h.recorder.mTimeOutCount, 0u);
    EXPECT_FALSE(h.processor.mAborted);
}

TEST_F(TestDefaultOTARequestorDownload, ExchangeIsReleasedWhenFirstBlockQueryIsNeverAnswered)
{
    DownloadHarness h;
    auto cleanup = MakeDefer([&]() { FinishDownload(h); });

    const size_t exchangesBefore = GetExchangeManager().GetNumActiveExchanges();

    // The provider accepts the transfer but never answers the BlockQuery that the requestor sends in reply to ReceiveAccept.
    h.provider.mBlocksToServe = 0;
    ASSERT_SUCCESS(BeginDownload(h));
    WaitForProviderToStall(h);
    ASSERT_TRUE(h.provider.IsStalled());

    AdvanceClockTo(h.provider.StalledAt() + kBdxTimeout + 1_s, h);
    EXPECT_EQ(h.recorder.mTimeOutCount, 1u);
    EXPECT_EQ(h.downloader.GetState(), OTADownloader::State::kIdle);

    // A timed-out transfer must not leave its exchange behind; repeated failures would otherwise exhaust the exchange pool.
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), exchangesBefore);
}

TEST_F(TestDefaultOTARequestorDownload, AbortedDownloadReleasesExchangeProviderWasHoldingOpen)
{
    DownloadHarness h;
    auto cleanup = MakeDefer([&]() { FinishDownload(h); });

    const size_t exchangesBefore = GetExchangeManager().GetNumActiveExchanges();

    // The provider holds the BlockQuery and its exchange open while it waits to answer.
    h.provider.mResponseDelay = 240_s;
    ASSERT_SUCCESS(BeginDownload(h));
    AdvanceClock(260_s, h, 10_s);
    ASSERT_EQ(h.downloader.GetState(), OTADownloader::State::kInProgress);

    // The requestor gives up and sends the provider a StatusReport. Neither side may keep an exchange for it.
    h.downloader.EndDownload();
    // DefaultOTARequestor does this when the download goes idle; this test records state changes in its place.
    ResetMessenger(h);
    DrainAndServiceIO();
    AdvanceClock(2 * 240_s, h, 10_s);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), exchangesBefore);
}

} // namespace chip
