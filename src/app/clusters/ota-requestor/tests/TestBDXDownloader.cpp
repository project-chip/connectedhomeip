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

#include <app/clusters/ota-requestor/BDXDownloader.h>

#include <algorithm>
#include <deque>
#include <lib/support/CHIPMem.h>
#include <optional>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <pw_unit_test/framework.h>
#include <transport/raw/MessageHeader.h>
#include <vector>

using namespace chip;
using namespace chip::bdx;
using chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;

namespace {

constexpr uint16_t kBlockSize              = 64;
constexpr size_t kImageSize                = size_t{ 16 } * kBlockSize;
constexpr char kFileDesignator[]           = "image.ota";
constexpr System::Clock::Timeout kTimeout  = System::Clock::Seconds16(300);
constexpr System::Clock::Timestamp kNoTime = System::Clock::kZero;
constexpr size_t kUnlimited                = SIZE_MAX;

class FakeImageProcessor : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override
    {
        mSuspended = false;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Finalize() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Apply() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Abort() override
    {
        mAbortCount++;
        mAbortPending = true;
        if (!mAsyncAbort)
        {
            CompleteAbort();
        }
        return CHIP_NO_ERROR;
    }
    void CompleteAbort()
    {
        mAbortPending  = false;
        mSuspended     = false;
        mBytesReceived = 0;
    }
    CHIP_ERROR ProcessBlock(ByteSpan & block) override
    {
        mBytesReceived += block.size();
        return CHIP_NO_ERROR;
    }
    bool IsFirstImageRun() override { return false; }
    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }
    uint64_t GetResumeOffset() override { return mSuspended ? mBytesReceived : 0; }
    CHIP_ERROR SuspendDownload() override
    {
        mSuspended = true;
        return CHIP_NO_ERROR;
    }

    uint32_t mAbortCount    = 0;
    uint64_t mBytesReceived = 0;
    bool mSuspended         = false;
    bool mAsyncAbort        = false;
    bool mAbortPending      = false;
};

struct QueuedMessage
{
    TransferSession::MessageTypeData type;
    System::PacketBufferHandle data;
};

class CapturingMessenger : public BDXDownloader::MessagingDelegate
{
public:
    CHIP_ERROR SendMessage(const TransferSession::OutputEvent & event) override
    {
        mOutbox.push_back({ event.msgTypeData, event.MsgData.CloneData() });
        return CHIP_NO_ERROR;
    }

    std::deque<QueuedMessage> mOutbox;
};

class RecordingStateDelegate : public BDXDownloader::StateDelegate
{
public:
    void OnDownloadStateChanged(OTADownloader::State state, OTAChangeReasonEnum reason) override
    {
        mState = state;
        mIdleCount += (state == OTADownloader::State::kIdle) ? 1 : 0;
    }
    void OnUpdateProgressChanged(app::DataModel::Nullable<uint8_t> percent) override {}

    OTADownloader::State mState = OTADownloader::State::kIdle;
    uint32_t mIdleCount         = 0;
};

class TestBDXDownloader : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        for (size_t i = 0; i < kImageSize; i++)
        {
            mImage[i] = static_cast<uint8_t>(i);
        }
        mDownloader.SetImageProcessorDelegate(&mProcessor);
        mDownloader.SetMessageDelegate(&mMessenger);
        mDownloader.SetStateDelegate(&mStateDelegate);
    }

    void TearDown() override { mDownloader.EndDownload(); }

    void StartDownload(size_t blocksToDeliver)
    {
        mMessenger.mOutbox.clear();
        mProvider.Reset();
        mBlocksToDeliver = blocksToDeliver;
        ASSERT_EQ(mProvider.WaitForTransfer(TransferRole::kSender, TransferControlFlags::kReceiverDrive, kBlockSize, kTimeout),
                  CHIP_NO_ERROR);

        TransferSession::TransferInitData init;
        init.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
        init.MaxBlockSize     = kBlockSize;
        init.FileDesignator   = reinterpret_cast<const uint8_t *>(kFileDesignator);
        init.FileDesLength    = static_cast<uint16_t>(sizeof(kFileDesignator) - 1);
        ASSERT_EQ(mDownloader.SetBDXParams(init, kTimeout), CHIP_NO_ERROR);
        ASSERT_EQ(mDownloader.BeginPrepareDownload(), CHIP_NO_ERROR);
        if (mProcessor.mAbortPending)
        {
            mProcessor.CompleteAbort();
        }
        ASSERT_EQ(mDownloader.OnPreparedForDownload(CHIP_NO_ERROR), CHIP_NO_ERROR);
        Pump();
    }

    void Pump()
    {
        while (!mMessenger.mOutbox.empty())
        {
            QueuedMessage msg = std::move(mMessenger.mOutbox.front());
            mMessenger.mOutbox.pop_front();
            PayloadHeader header;
            header.SetMessageType(msg.type.ProtocolId, msg.type.MessageType);
            ASSERT_EQ(mProvider.HandleMessageReceived(header, std::move(msg.data), kNoTime), CHIP_NO_ERROR);
            ServeProvider();
        }
    }

    void ServeProvider()
    {
        TransferSession::OutputEvent event;
        for (mProvider.PollOutput(event, kNoTime); event.EventType != TransferSession::OutputEventType::kNone;
             mProvider.PollOutput(event, kNoTime))
        {
            switch (event.EventType)
            {
            case TransferSession::OutputEventType::kInitReceived: {
                mRequestedOffsets.push_back(event.transferInitData.StartOffset);
                if (mRejectStartOffset && event.transferInitData.StartOffset != 0)
                {
                    ASSERT_EQ(mProvider.AbortTransfer(StatusCode::kStartOffsetNotSupported), CHIP_NO_ERROR);
                    break;
                }
                mServeOffset = static_cast<size_t>(mAcceptOffset.value_or(event.transferInitData.StartOffset));
                TransferSession::TransferAcceptData accept;
                accept.ControlMode  = TransferControlFlags::kReceiverDrive;
                accept.MaxBlockSize = kBlockSize;
                accept.StartOffset  = mServeOffset;
                accept.Length       = kImageSize - mServeOffset;
                ASSERT_EQ(mProvider.AcceptTransfer(accept), CHIP_NO_ERROR);
                mServeOffset = mServeFrom.value_or(mServeOffset);
                break;
            }
            case TransferSession::OutputEventType::kQueryReceived: {
                TransferSession::BlockData block;
                block.Data   = &mImage[mServeOffset];
                block.Length = std::min<size_t>(kBlockSize, kImageSize - mServeOffset);
                block.IsEof  = (mServeOffset + block.Length) == kImageSize;
                mServeOffset += block.Length;
                ASSERT_EQ(mProvider.PrepareBlock(block), CHIP_NO_ERROR);
                break;
            }
            case TransferSession::OutputEventType::kMsgToSend:
                Deliver(event);
                break;
            case TransferSession::OutputEventType::kStatusReceived:
            case TransferSession::OutputEventType::kInternalError:
                mProvider.Reset();
                return;
            default:
                break;
            }
        }
    }

    void Deliver(TransferSession::OutputEvent & event)
    {
        bool isBlock =
            event.msgTypeData.HasMessageType(MessageType::Block) || event.msgTypeData.HasMessageType(MessageType::BlockEOF);
        if (isBlock)
        {
            if (mBlocksToDeliver == 0)
            {
                return;
            }
            mBlocksToDeliver--;
        }
        PayloadHeader header;
        header.SetMessageType(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType);
        mDownloader.OnMessageReceived(header, std::move(event.MsgData));
        if (isBlock && mDownloader.GetState() == OTADownloader::State::kInProgress)
        {
            TEMPORARY_RETURN_IGNORED mDownloader.FetchNextData();
        }
    }

    void InterruptAfter(size_t blocks)
    {
        StartDownload(blocks);
        mDownloader.OnDownloadTimeout();
    }

    uint8_t mImage[kImageSize];
    FakeImageProcessor mProcessor;
    CapturingMessenger mMessenger;
    RecordingStateDelegate mStateDelegate;
    BDXDownloader mDownloader;
    TransferSession mProvider;
    std::vector<uint64_t> mRequestedOffsets;
    size_t mBlocksToDeliver = kUnlimited;
    size_t mServeOffset     = 0;
    std::optional<uint64_t> mAcceptOffset;
    std::optional<size_t> mServeFrom;
    bool mRejectStartOffset = false;
};

TEST_F(TestBDXDownloader, CompleteDownloadReceivesWholeImage)
{
    StartDownload(kUnlimited);

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
    ASSERT_EQ(mRequestedOffsets.size(), 1u);
    EXPECT_EQ(mRequestedOffsets[0], 0u);
}

TEST_F(TestBDXDownloader, TimeoutKeepsPartialImage)
{
    InterruptAfter(5);

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.mAbortCount, 0u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 5u * kBlockSize);
}

TEST_F(TestBDXDownloader, RetryResumesFromPartialImage)
{
    InterruptAfter(5);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, ResumeAcrossSeveralInterruptions)
{
    InterruptAfter(3);
    InterruptAfter(4);
    InterruptAfter(2);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 4u);
    EXPECT_EQ(mRequestedOffsets[1], 3u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[2], 7u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[3], 9u * kBlockSize);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, ThreeResumesWithoutProgressStartAnew)
{
    InterruptAfter(5);
    InterruptAfter(0);
    InterruptAfter(0);
    InterruptAfter(0);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 5u);
    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[2], 5u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[3], 5u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[4], 0u);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, ProgressResetsTheAttemptsWithoutProgress)
{
    InterruptAfter(5);
    InterruptAfter(0);
    InterruptAfter(2);
    InterruptAfter(0);
    InterruptAfter(0);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 6u);
    EXPECT_EQ(mRequestedOffsets[2], 5u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[5], 7u * kBlockSize);
    EXPECT_EQ(mProcessor.mAbortCount, 0u);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, NewImageVersionStartsAnew)
{
    mDownloader.SetImageVersion(2);
    InterruptAfter(5);
    mDownloader.SetImageVersion(3);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, NewImageVersionStartsAnewWhenAbortIsAsynchronous)
{
    mProcessor.mAsyncAbort = true;
    mDownloader.SetImageVersion(2);
    InterruptAfter(5);
    mDownloader.SetImageVersion(3);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, CancelDiscardsSuspendedImage)
{
    mDownloader.SetImageVersion(2);
    InterruptAfter(5);
    mDownloader.EndDownload(CHIP_ERROR_CONNECTION_ABORTED);
    mDownloader.SetImageVersion(2);
    StartDownload(kUnlimited);

    EXPECT_EQ(mProcessor.mAbortCount, 1u);
    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, AcceptAtOtherOffsetDiscardsPartialImage)
{
    InterruptAfter(5);
    mAcceptOffset = kBlockSize;
    StartDownload(kUnlimited);

    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.mAbortCount, 1u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    mAcceptOffset.reset();
    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
}

TEST_F(TestBDXDownloader, AcceptAtOffsetZeroDiscardsPartialImage)
{
    InterruptAfter(5);
    mAcceptOffset = 0u;
    StartDownload(kUnlimited);

    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.mAbortCount, 1u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    mAcceptOffset.reset();
    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
}

TEST_F(TestBDXDownloader, ResumeServedFromStartWithKnownLengthEndsDownload)
{
    InterruptAfter(5);
    mServeFrom = 0u;
    StartDownload(kUnlimited);

    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.mAbortCount, 1u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    mServeFrom.reset();
    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
    EXPECT_EQ(mProcessor.mBytesReceived, kImageSize);
}

TEST_F(TestBDXDownloader, StartOffsetNotSupportedDiscardsPartialImage)
{
    InterruptAfter(5);
    mRejectStartOffset = true;
    StartDownload(kUnlimited);

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.mAbortCount, 1u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
}

TEST_F(TestBDXDownloader, RejectedAcceptReportsFailureOnce)
{
    InterruptAfter(5);
    mAcceptOffset             = kBlockSize;
    mStateDelegate.mIdleCount = 0;
    StartDownload(kUnlimited);

    EXPECT_EQ(mStateDelegate.mIdleCount, 1u);
}

TEST_F(TestBDXDownloader, LocalFailureDiscardsPartialImage)
{
    StartDownload(5);
    mDownloader.EndDownload(CHIP_ERROR_WRITE_FAILED);

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);
}

} // namespace
