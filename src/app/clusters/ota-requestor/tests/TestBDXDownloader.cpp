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
#include <cstring>
#include <deque>
#include <functional>
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
constexpr char kOtherFileDesignator[]      = "image-rebuilt.ota";
constexpr uint32_t kVersion                = 2;
constexpr NodeId kProviderNodeId           = 0x1111;
constexpr NodeId kOtherProviderNodeId      = 0x2222;
constexpr FabricIndex kFabricIndex         = 1;
constexpr System::Clock::Timeout kTimeout  = System::Clock::Seconds16(300);
constexpr System::Clock::Timestamp kNoTime = System::Clock::kZero;
constexpr size_t kUnlimited                = SIZE_MAX;

// A processor that does its work in call order, like one that queues it on the event loop. With mAsync set, Prepare, Suspend
// and Abort only take effect when RunQueuedWork() runs, so a test can let the downloader decide before earlier work completes.
class FakeImageProcessor : public OTAImageProcessorInterface
{
public:
    // The earlier contract, where the processor decided by itself to continue a suspended image.
    CHIP_ERROR PrepareDownload() override
    {
        return Queue([this] { RunPrepare(mSuspended, std::nullopt); });
    }
    CHIP_ERROR PrepareDownloadFrom(uint64_t offset) override
    {
        return Queue([this, offset] { RunPrepare(offset > 0 && mSuspended && mFile.size() == offset, offset); });
    }
    CHIP_ERROR Finalize() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Apply() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Abort() override
    {
        mAbortCount++;
        return Queue([this] {
            mSuspended = false;
            mFile.clear();
        });
    }
    CHIP_ERROR ProcessBlock(ByteSpan & block) override
    {
        RunQueuedWork();
        mFile.insert(mFile.end(), block.begin(), block.end());
        mBytesReceived = mFile.size();
        return CHIP_NO_ERROR;
    }
    bool IsFirstImageRun() override { return false; }
    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }
    uint64_t GetResumeOffset() override { return mSuspended ? mFile.size() : 0; }
    CHIP_ERROR SuspendDownload() override
    {
        return Queue([this] { mSuspended = true; });
    }

    void RunQueuedWork()
    {
        while (!mQueue.empty())
        {
            std::function<void()> work = std::move(mQueue.front());
            mQueue.pop_front();
            work();
        }
    }

    uint32_t mAbortCount    = 0;
    uint64_t mBytesReceived = 0;
    bool mSuspended         = false;
    bool mAsync             = false;
    std::vector<uint8_t> mFile;
    // Bytes of the partial image each prepare kept, to compare with the offset requested from the provider.
    std::vector<uint64_t> mKeptAtPrepare;
    CHIP_ERROR mPrepareStatus = CHIP_NO_ERROR;

private:
    CHIP_ERROR Queue(std::function<void()> work)
    {
        mQueue.push_back(std::move(work));
        if (!mAsync)
        {
            RunQueuedWork();
        }
        return CHIP_NO_ERROR;
    }

    void RunPrepare(bool keep, std::optional<uint64_t> offset)
    {
        if (!keep)
        {
            mFile.clear();
        }
        mSuspended     = false;
        mBytesReceived = mFile.size();
        mKeptAtPrepare.push_back(mFile.size());
        mPrepareStatus = (offset.value_or(0) > 0 && !keep) ? CHIP_ERROR_INCORRECT_STATE : CHIP_NO_ERROR;
    }

    std::deque<std::function<void()>> mQueue;
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
            mImage[i]      = static_cast<uint8_t>(i);
            mOtherImage[i] = static_cast<uint8_t>(~i);
        }
        mServedImage = mImage;
        mDownloader.SetImageProcessorDelegate(&mProcessor);
        mDownloader.SetImageIdentity(kVersion, ScopedNodeId(kProviderNodeId, kFabricIndex));
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
        init.FileDesignator   = reinterpret_cast<const uint8_t *>(mFileDesignator);
        init.FileDesLength    = static_cast<uint16_t>(strlen(mFileDesignator));
        ASSERT_EQ(mDownloader.SetBDXParams(init, kTimeout), CHIP_NO_ERROR);
        ASSERT_EQ(mDownloader.BeginPrepareDownload(), CHIP_NO_ERROR);
        mProcessor.RunQueuedWork();
        ASSERT_EQ(mDownloader.OnPreparedForDownload(mProcessor.mPrepareStatus), CHIP_NO_ERROR);
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
                block.Data   = &mServedImage[mServeOffset];
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

    // The image the provider holds and the partial image kept for it agree: the bytes the processor kept when it prepared are the
    // bytes the downloader asked the provider to skip, and the downloaded file is the served image.
    void ExpectFileIsServedImage()
    {
        ASSERT_FALSE(mRequestedOffsets.empty());
        ASSERT_FALSE(mProcessor.mKeptAtPrepare.empty());
        EXPECT_EQ(mProcessor.mKeptAtPrepare.back(), mRequestedOffsets.back());
        EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
        EXPECT_EQ(mProcessor.mFile, std::vector<uint8_t>(mServedImage, mServedImage + kImageSize));
    }

    uint8_t mImage[kImageSize];
    uint8_t mOtherImage[kImageSize];
    const uint8_t * mServedImage = nullptr;
    const char * mFileDesignator = kFileDesignator;
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
    InterruptAfter(5);
    mDownloader.SetImageIdentity(kVersion + 1, ScopedNodeId(kProviderNodeId, kFabricIndex));
    mServedImage = mOtherImage;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, NewImageVersionStartsAnewWhenAbortIsAsynchronous)
{
    InterruptAfter(5);
    mProcessor.mAsync = true;
    mDownloader.SetImageIdentity(kVersion + 1, ScopedNodeId(kProviderNodeId, kFabricIndex));
    mServedImage = mOtherImage;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, SameImageResumes)
{
    InterruptAfter(5);
    mDownloader.SetImageIdentity(kVersion, ScopedNodeId(kProviderNodeId, kFabricIndex));
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_GT(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, SameVersionWithOtherFileDesignatorStartsAnew)
{
    InterruptAfter(5);
    mFileDesignator = kOtherFileDesignator;
    mServedImage    = mOtherImage;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, SameVersionFromOtherProviderStartsAnew)
{
    InterruptAfter(5);
    mDownloader.SetImageIdentity(kVersion, ScopedNodeId(kOtherProviderNodeId, kFabricIndex));
    mServedImage = mOtherImage;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, SameVersionOnOtherFabricStartsAnew)
{
    InterruptAfter(5);
    mDownloader.SetImageIdentity(kVersion, ScopedNodeId(kProviderNodeId, static_cast<FabricIndex>(kFabricIndex + 1)));
    mServedImage = mOtherImage;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, OtherFileDesignatorStartsAnewWhenAbortIsAsynchronous)
{
    InterruptAfter(5);
    mProcessor.mAsync = true;
    mFileDesignator   = kOtherFileDesignator;
    mServedImage      = mOtherImage;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, RetryBeforeSuspendCompletesStartsAnew)
{
    mProcessor.mAsync = true;
    InterruptAfter(5);
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, CancelBeforeSuspendCompletesDiscardsSuspendedImage)
{
    mProcessor.mAsync = true;
    InterruptAfter(5);
    mDownloader.EndDownload(CHIP_ERROR_CONNECTION_ABORTED);
    mProcessor.RunQueuedWork();
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, CancelDiscardsSuspendedImage)
{
    InterruptAfter(5);
    mDownloader.EndDownload(CHIP_ERROR_CONNECTION_ABORTED);
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

TEST_F(TestBDXDownloader, EndDownloadAfterCompletionKeepsImage)
{
    StartDownload(kUnlimited);
    mDownloader.EndDownload(CHIP_ERROR_CONNECTION_ABORTED);

    EXPECT_EQ(mProcessor.mAbortCount, 0u);
    EXPECT_EQ(mProcessor.mFile, std::vector<uint8_t>(mServedImage, mServedImage + kImageSize));
}

TEST_F(TestBDXDownloader, FailureBeforeAbortCompletesStartsAnew)
{
    InterruptAfter(5);
    mProcessor.mAsync  = true;
    mRejectStartOffset = true;
    StartDownload(kUnlimited);
    ASSERT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);

    mRejectStartOffset = false;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    ExpectFileIsServedImage();
}

// A processor written before PrepareDownloadFrom() existed: it cannot continue a partial image, so it must not be asked to.
class ProcessorWithoutResume : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override
    {
        mPrepareCount++;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Finalize() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Apply() override { return CHIP_NO_ERROR; }
    CHIP_ERROR Abort() override { return CHIP_NO_ERROR; }
    CHIP_ERROR ProcessBlock(ByteSpan & block) override { return CHIP_NO_ERROR; }
    bool IsFirstImageRun() override { return false; }
    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }

    uint32_t mPrepareCount = 0;
};

TEST(TestOTAImageProcessorInterface, DefaultPrepareDownloadFromOnlyStartsAnew)
{
    ProcessorWithoutResume processor;

    EXPECT_EQ(processor.PrepareDownloadFrom(0), CHIP_NO_ERROR);
    EXPECT_EQ(processor.mPrepareCount, 1u);
    EXPECT_NE(processor.PrepareDownloadFrom(kBlockSize), CHIP_NO_ERROR);
    EXPECT_EQ(processor.mPrepareCount, 1u);
}

} // namespace
