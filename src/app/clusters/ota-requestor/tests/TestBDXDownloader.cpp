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
#include <lib/core/CHIPEncoding.h>
#include <lib/core/OTAImageHeader.h>
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
    CHIP_ERROR Finalize() override
    {
        mFinalizeCount++;
        // A processor that checks the image inside Finalize() reports the result before returning.
        if (mReportInsideFinalize != nullptr)
        {
            EXPECT_EQ(mReportInsideFinalize->OnFinalized(mFinalizeResult), CHIP_NO_ERROR);
        }
        return CHIP_NO_ERROR;
    }
    bool ReportsFinalizeResult() const override { return mReportsFinalizeResult; }
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
        mProcessBlockCount++;
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

    uint32_t mAbortCount        = 0;
    uint32_t mFinalizeCount     = 0;
    bool mReportsFinalizeResult = false;
    // When set, Finalize() reports mFinalizeResult to this downloader before it returns.
    BDXDownloader * mReportInsideFinalize = nullptr;
    CHIP_ERROR mFinalizeResult            = CHIP_NO_ERROR;
    uint32_t mProcessBlockCount           = 0;
    uint64_t mBytesReceived               = 0;
    bool mSuspended                       = false;
    bool mAsync                           = false;
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
        VerifyOrReturnError(!(mFailBlockAckEOF && event.msgTypeData.HasMessageType(MessageType::BlockAckEOF)),
                            CHIP_ERROR_SENDING_BLOCKED);
        mBlockAckEOFCount += event.msgTypeData.HasMessageType(MessageType::BlockAckEOF) ? 1 : 0;
        mOutbox.push_back({ event.msgTypeData, event.MsgData.CloneData() });
        return CHIP_NO_ERROR;
    }

    std::deque<QueuedMessage> mOutbox;
    bool mFailBlockAckEOF      = false;
    uint32_t mBlockAckEOFCount = 0;
};

class RecordingStateDelegate : public BDXDownloader::StateDelegate
{
public:
    void OnDownloadStateChanged(OTADownloader::State state, OTAChangeReasonEnum reason) override
    {
        mState  = state;
        mReason = reason;
        mIdleCount += (state == OTADownloader::State::kIdle) ? 1 : 0;
        mCompleteCount += (state == OTADownloader::State::kComplete) ? 1 : 0;
    }
    void OnUpdateProgressChanged(app::DataModel::Nullable<uint8_t> percent) override {}
    void OnDownloadRestartRequested() override { mRestartCount++; }

    OTADownloader::State mState = OTADownloader::State::kIdle;
    OTAChangeReasonEnum mReason = OTAChangeReasonEnum::kUnknown;
    uint32_t mIdleCount         = 0;
    uint32_t mCompleteCount     = 0;
    uint32_t mRestartCount      = 0;
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
        // Both images start with the file identifier of an OTA image header, like a real image.
        Encoding::LittleEndian::Put32(mImage, kOTAImageFileIdentifier);
        Encoding::LittleEndian::Put32(mOtherImage, kOTAImageFileIdentifier);
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

        ASSERT_EQ(mDownloader.SetBDXParams(MakeInit(), kTimeout), CHIP_NO_ERROR);
        ASSERT_EQ(mDownloader.BeginPrepareDownload(), CHIP_NO_ERROR);
        mProcessor.RunQueuedWork();
        ASSERT_EQ(mDownloader.OnPreparedForDownload(mProcessor.mPrepareStatus), CHIP_NO_ERROR);
        Pump();
    }

    TransferSession::TransferInitData MakeInit() const
    {
        TransferSession::TransferInitData init;
        init.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
        init.MaxBlockSize     = kBlockSize;
        init.FileDesignator   = reinterpret_cast<const uint8_t *>(mFileDesignator);
        init.FileDesLength    = static_cast<uint16_t>(strlen(mFileDesignator));
        return init;
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
                if ((mRejectStartOffset && event.transferInitData.StartOffset != 0) || mRejectOffsetZero)
                {
                    ASSERT_EQ(mProvider.AbortTransfer(StatusCode::kStartOffsetNotSupported), CHIP_NO_ERROR);
                    break;
                }
                // An SDK provider echoes the requested offset in the accept; mAcceptOffset makes it send another one, and 0
                // leaves the field out as the specification does. Data is served from the requested offset unless mServeFrom
                // is set.
                const uint64_t requested = event.transferInitData.StartOffset;
                TransferSession::TransferAcceptData accept;
                accept.ControlMode  = TransferControlFlags::kReceiverDrive;
                accept.MaxBlockSize = kBlockSize;
                accept.StartOffset  = mAcceptOffset.value_or(requested);
                accept.Length       = kImageSize - requested;
                ASSERT_EQ(mProvider.AcceptTransfer(accept), CHIP_NO_ERROR);
                mServeOffset = mServeFrom.value_or(static_cast<size_t>(requested));
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
    bool mRejectOffsetZero  = false;
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

// A ReceiveAccept that follows the specification has no start offset field, which the SDK parses as offset 0. Accepting the
// transfer accepts the requested offset, so the download resumes.
TEST_F(TestBDXDownloader, AcceptWithoutStartOffsetResumes)
{
    InterruptAfter(5);
    mAcceptOffset = 0u;
    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mStateDelegate.mRestartCount, 0u);
    EXPECT_EQ(mProcessor.mAbortCount, 0u);
    ExpectFileIsServedImage();
}

// The accept carries no offset the provider chose, so another value in it does not end a download served from the request.
TEST_F(TestBDXDownloader, StartOffsetInAcceptIsNotChecked)
{
    InterruptAfter(5);
    mAcceptOffset = kBlockSize;
    StartDownload(kUnlimited);

    EXPECT_EQ(mStateDelegate.mRestartCount, 0u);
    ExpectFileIsServedImage();
}

// A provider that accepts the offset but serves the image from its start is caught by the image header in the first block,
// before any of the block reaches the partial image, and the download restarts from offset 0 in the same attempt.
TEST_F(TestBDXDownloader, ResumeServedFromStartIsCaughtAtTheFirstBlock)
{
    InterruptAfter(5);
    mProcessor.mProcessBlockCount = 0;
    mServeFrom                    = 0u;
    StartDownload(kUnlimited);

    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mProcessor.mProcessBlockCount, 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mStateDelegate.mReason, OTAChangeReasonEnum::kSuccess);
    EXPECT_EQ(mStateDelegate.mRestartCount, 1u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    // What the delegate does on the restart request.
    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mRestartCount, 1u);
    ExpectFileIsServedImage();
}

// The whole image was received and finalized, but BlockAckEOF could not be sent, so the transfer stalls until the timeout. The
// finalized image must not be treated as a partial one and resumed from its end.
TEST_F(TestBDXDownloader, FinalizedImageIsNotResumedWhenBlockAckEOFFails)
{
    mMessenger.mFailBlockAckEOF = true;
    StartDownload(kUnlimited);
    ASSERT_EQ(mDownloader.GetState(), OTADownloader::State::kInProgress);
    ASSERT_EQ(mProcessor.mBytesReceived, kImageSize);
    mDownloader.OnDownloadTimeout();

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    mMessenger.mFailBlockAckEOF = false;
    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 2u);
    EXPECT_EQ(mRequestedOffsets[1], 0u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, StartOffsetNotSupportedDiscardsPartialImage)
{
    InterruptAfter(5);
    mRejectStartOffset = true;
    StartDownload(kUnlimited);

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mStateDelegate.mReason, OTAChangeReasonEnum::kSuccess);
    EXPECT_EQ(mStateDelegate.mRestartCount, 1u);
    EXPECT_EQ(mProcessor.mAbortCount, 1u);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[1], 5u * kBlockSize);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
}

// A provider that cannot continue a partial image costs the update attempt nothing: the downloader asks its delegate to start
// the same image again from offset 0, instead of failing the attempt and waiting for the next query.
TEST_F(TestBDXDownloader, RejectedStartOffsetRestartsFromZeroWithoutFailing)
{
    InterruptAfter(5);
    mRejectStartOffset = true;
    StartDownload(kUnlimited);

    ASSERT_EQ(mStateDelegate.mRestartCount, 1u);
    EXPECT_EQ(mStateDelegate.mReason, OTAChangeReasonEnum::kSuccess);

    // What the delegate does on the restart request.
    StartDownload(kUnlimited);
    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    EXPECT_EQ(mStateDelegate.mRestartCount, 1u);
    ExpectFileIsServedImage();
}

TEST_F(TestBDXDownloader, RejectedOffsetZeroFailsInsteadOfRestarting)
{
    StartDownload(kUnlimited);
    mDownloader.EndDownload();
    mRejectOffsetZero            = true;
    mStateDelegate.mRestartCount = 0;
    StartDownload(kUnlimited);

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mStateDelegate.mReason, OTAChangeReasonEnum::kFailure);
    EXPECT_EQ(mStateDelegate.mRestartCount, 0u);
}

TEST_F(TestBDXDownloader, ProviderOtherFailureDoesNotRequestRestart)
{
    InterruptAfter(5);
    mStateDelegate.mRestartCount = 0;
    mDownloader.EndDownload(CHIP_ERROR_WRITE_FAILED);

    EXPECT_EQ(mStateDelegate.mRestartCount, 0u);
}

TEST_F(TestBDXDownloader, CompletedImageDownloadsAgainFromStart)
{
    InterruptAfter(5);
    StartDownload(kUnlimited);
    ASSERT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);

    StartDownload(kUnlimited);

    ASSERT_EQ(mRequestedOffsets.size(), 3u);
    EXPECT_EQ(mRequestedOffsets[2], 0u);
    ExpectFileIsServedImage();
}

// The prepare is still queued with the processor when the download ends, so the abort queued after it removes the file the
// prepare opened. The downloader must treat that prepare as stale instead of starting a transfer into the removed file.
TEST_F(TestBDXDownloader, EndDownloadWhilePreparingDoesNotStartTheTransfer)
{
    InterruptAfter(5);
    mProcessor.mAsync = true;
    mMessenger.mOutbox.clear();
    ASSERT_EQ(mDownloader.SetBDXParams(MakeInit(), kTimeout), CHIP_NO_ERROR);
    ASSERT_EQ(mDownloader.BeginPrepareDownload(), CHIP_NO_ERROR);
    ASSERT_EQ(mDownloader.GetState(), OTADownloader::State::kPreparing);

    mDownloader.EndDownload(CHIP_ERROR_CONNECTION_ABORTED);
    mProcessor.RunQueuedWork();

    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_NE(mDownloader.OnPreparedForDownload(mProcessor.mPrepareStatus), CHIP_NO_ERROR);
    EXPECT_EQ(mDownloader.GetState(), OTADownloader::State::kIdle);
    EXPECT_TRUE(mMessenger.mOutbox.empty());
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);
}

TEST_F(TestBDXDownloader, ResumeServedFromStartGoesIdleOnce)
{
    InterruptAfter(5);
    mServeFrom                = 0u;
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

// A processor that checks the finalized image reports a failure before the download is reported complete, so the requestor never
// asks the provider to apply an image that failed the check.
TEST_F(TestBDXDownloader, FailedFinalizeCheckEndsDownloadAsFailedBeforeComplete)
{
    mProcessor.mReportsFinalizeResult = true;
    StartDownload(kUnlimited);
    ASSERT_EQ(mProcessor.mFinalizeCount, 1u);
    ASSERT_EQ(mProcessor.mBytesReceived, kImageSize);
    EXPECT_EQ(mStateDelegate.mCompleteCount, 0u);

    const uint32_t abortsBefore = mProcessor.mAbortCount;
    EXPECT_EQ(mDownloader.OnFinalized(CHIP_ERROR_INTEGRITY_CHECK_FAILED), CHIP_NO_ERROR);

    EXPECT_EQ(mStateDelegate.mCompleteCount, 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mStateDelegate.mReason, OTAChangeReasonEnum::kFailure);
    EXPECT_EQ(mProcessor.mAbortCount, abortsBefore + 1);
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);
}

TEST_F(TestBDXDownloader, PassedFinalizeCheckCompletesTheDownload)
{
    mProcessor.mReportsFinalizeResult = true;
    StartDownload(kUnlimited);
    ASSERT_EQ(mStateDelegate.mCompleteCount, 0u);

    EXPECT_EQ(mDownloader.OnFinalized(CHIP_NO_ERROR), CHIP_NO_ERROR);

    EXPECT_EQ(mStateDelegate.mCompleteCount, 1u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
    ExpectFileIsServedImage();
}

// A processor that checks the image inside Finalize() reports its result before BlockAckEOF is sent. The download completes
// once BlockAckEOF goes out, exactly once.
TEST_F(TestBDXDownloader, FinalizeResultBeforeBlockAckEOFCompletesAfterIt)
{
    mProcessor.mReportsFinalizeResult = true;
    mProcessor.mReportInsideFinalize  = &mDownloader;
    StartDownload(kUnlimited);

    EXPECT_EQ(mProcessor.mFinalizeCount, 1u);
    EXPECT_EQ(mMessenger.mBlockAckEOFCount, 1u);
    EXPECT_EQ(mStateDelegate.mCompleteCount, 1u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kComplete);
    ExpectFileIsServedImage();
}

// The same processor reporting a failure inside Finalize(): the download still sends BlockAckEOF, so the provider sees the
// transfer end, and then ends as failed without completing.
TEST_F(TestBDXDownloader, FailedFinalizeResultBeforeBlockAckEOFStillSendsBlockAckEOF)
{
    mProcessor.mReportsFinalizeResult = true;
    mProcessor.mReportInsideFinalize  = &mDownloader;
    mProcessor.mFinalizeResult        = CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    const uint32_t abortsBefore       = mProcessor.mAbortCount;
    StartDownload(kUnlimited);

    EXPECT_EQ(mProcessor.mFinalizeCount, 1u);
    EXPECT_EQ(mMessenger.mBlockAckEOFCount, 1u);
    EXPECT_EQ(mStateDelegate.mCompleteCount, 0u);
    EXPECT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);
    EXPECT_EQ(mStateDelegate.mReason, OTAChangeReasonEnum::kFailure);
    EXPECT_EQ(mProcessor.mAbortCount, abortsBefore + 1);
}

TEST_F(TestBDXDownloader, FinalizeResultAfterTimeoutIsRejected)
{
    mProcessor.mReportsFinalizeResult = true;
    mMessenger.mFailBlockAckEOF       = true;
    StartDownload(kUnlimited);
    ASSERT_EQ(mProcessor.mFinalizeCount, 1u);
    mDownloader.OnDownloadTimeout();
    ASSERT_EQ(mStateDelegate.mState, OTADownloader::State::kIdle);

    EXPECT_EQ(mDownloader.OnFinalized(CHIP_NO_ERROR), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mStateDelegate.mCompleteCount, 0u);
}

TEST_F(TestBDXDownloader, StaleFinalizeResultDuringTheNextTransferIsRejected)
{
    mProcessor.mReportsFinalizeResult = true;
    mMessenger.mFailBlockAckEOF       = true;
    StartDownload(kUnlimited);
    mDownloader.OnDownloadTimeout();
    mMessenger.mFailBlockAckEOF = false;

    // The next attempt is in progress and has not received its EOF block.
    StartDownload(3);
    ASSERT_EQ(mDownloader.GetState(), OTADownloader::State::kInProgress);
    const uint32_t idleBefore = mStateDelegate.mIdleCount;

    EXPECT_EQ(mDownloader.OnFinalized(CHIP_ERROR_INTEGRITY_CHECK_FAILED), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mDownloader.GetState(), OTADownloader::State::kInProgress);
    EXPECT_EQ(mStateDelegate.mIdleCount, idleBefore);
    EXPECT_EQ(mStateDelegate.mCompleteCount, 0u);
}

TEST_F(TestBDXDownloader, FinalizeResultWithoutADownloadIsRejected)
{
    mProcessor.mReportsFinalizeResult = true;
    EXPECT_EQ(mDownloader.OnFinalized(CHIP_NO_ERROR), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(mStateDelegate.mCompleteCount, 0u);
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
