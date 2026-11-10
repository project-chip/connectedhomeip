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

#include <platform/Linux/OTAImageProcessorImpl.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <crypto/CHIPCryptoPAL.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace chip;

namespace {

constexpr uint8_t kHeader[]        = { 0x1e, 0xf1, 0xee, 0x1b, 0xa2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x00,
                                       0x00, 0x00, 0x15, 0x25, 0x00, 0xad, 0xde, 0x25, 0x01, 0xef, 0xbe, 0x26, 0x02, 0xff,
                                       0xff, 0xff, 0xff, 0x2c, 0x03, 0x03, 0x31, 0x2e, 0x30, 0x24, 0x04, 0x40, 0x24, 0x05,
                                       0x01, 0x24, 0x06, 0x02, 0x2c, 0x07, 0x0a, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                                       0x2f, 0x72, 0x6e, 0x24, 0x08, 0x01, 0x30, 0x09, 0x20, 0x81, 0x3c, 0xa5, 0x28, 0x5c,
                                       0x28, 0xcc, 0xee, 0x5c, 0xab, 0x8b, 0x10, 0xeb, 0xda, 0x9c, 0x90, 0x8f, 0xd6, 0xd7,
                                       0x8e, 0xd9, 0xdc, 0x94, 0xcc, 0x65, 0xea, 0x6c, 0xb6, 0x7a, 0x7f, 0x13, 0xae, 0x18 };
constexpr size_t kDigestTypeOffset = 61;
constexpr size_t kDigestOffset     = 65;
constexpr size_t kPayloadSize      = 64;
constexpr size_t kImageSize        = sizeof(kHeader) + kPayloadSize;
constexpr size_t kBlockSize        = 32;

class FakeDownloader : public OTADownloader
{
public:
    CHIP_ERROR BeginPrepareDownload() override { return CHIP_NO_ERROR; }
    CHIP_ERROR OnPreparedForDownload(CHIP_ERROR status) override
    {
        mPrepareStatus = status;
        return CHIP_NO_ERROR;
    }
    void OnDownloadTimeout() override {}
    void EndDownload(CHIP_ERROR reason) override { mEndReasons.push_back(reason); }
    CHIP_ERROR FetchNextData() override { return CHIP_NO_ERROR; }
    CHIP_ERROR OnFinalized(CHIP_ERROR status) override
    {
        mFinalizeResults.push_back(status);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR mPrepareStatus = CHIP_ERROR_INTERNAL;
    std::vector<CHIP_ERROR> mEndReasons;
    std::vector<CHIP_ERROR> mFinalizeResults;
};

class TestOTAImageProcessorImpl : public ::testing::Test
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
        mImage.assign(std::begin(kHeader), std::end(kHeader));
        for (size_t i = 0; i < kPayloadSize; i++)
        {
            mImage.push_back(static_cast<uint8_t>(0x80 + i));
        }
        ASSERT_EQ(Crypto::Hash_SHA256(&mImage[sizeof(kHeader)], kPayloadSize, &mImage[kDigestOffset]), CHIP_NO_ERROR);
        mPath = (std::filesystem::temp_directory_path() / ("TestOTAImageProcessorImpl-" + std::to_string(getpid()))).string();
        mProcessor.SetOTADownloader(&mDownloader);
        mProcessor.SetOTAImageFile(mPath.c_str());
    }

    void TearDown() override
    {
        EXPECT_EQ(mProcessor.Abort(), CHIP_NO_ERROR);
        Drain();
        std::remove(mPath.c_str());
    }

    static void Drain()
    {
        ASSERT_EQ(DeviceLayer::PlatformMgr().ScheduleWork(
                      [](intptr_t) { EXPECT_EQ(DeviceLayer::PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
                  CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().RunEventLoop();
    }

    // Prepares to download from `offset`, the offset a downloader requested from the provider.
    void Prepare(uint64_t offset = 0)
    {
        mDownloader.mPrepareStatus = CHIP_ERROR_INTERNAL;
        ASSERT_EQ(mProcessor.PrepareDownloadFrom(offset), CHIP_NO_ERROR);
        // A resumed prepare hashes the partial image over several work items; each Drain() runs the items queued so far.
        for (int i = 0; i < 64 && mDownloader.mPrepareStatus == CHIP_ERROR_INTERNAL; i++)
        {
            Drain();
        }
        ASSERT_EQ(mDownloader.mPrepareStatus, CHIP_NO_ERROR);
    }

    std::vector<uint8_t> Payload() const { return std::vector<uint8_t>(mImage.begin() + sizeof(kHeader), mImage.end()); }

    // Serves the image from offset `from` to `end` in blocks, as a provider would; stops after `maxBlocks`.
    void Serve(size_t from, size_t maxBlocks = SIZE_MAX, size_t end = kImageSize)
    {
        for (size_t offset = from; offset < end && maxBlocks > 0; offset += kBlockSize, maxBlocks--)
        {
            ByteSpan block(&mImage[offset], std::min(kBlockSize, end - offset));
            ASSERT_EQ(mProcessor.ProcessBlock(block), CHIP_NO_ERROR);
            Drain();
        }
    }

    void Complete()
    {
        ASSERT_EQ(mProcessor.Finalize(), CHIP_NO_ERROR);
        Drain();
    }

    void Suspend()
    {
        ASSERT_EQ(mProcessor.SuspendDownload(), CHIP_NO_ERROR);
        Drain();
    }

    std::vector<uint8_t> FileContents() const
    {
        std::ifstream in(mPath, std::ios::binary);
        return std::vector<uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    std::vector<uint8_t> mImage;
    std::string mPath;
    FakeDownloader mDownloader;
    OTAImageProcessorImpl mProcessor;
};

TEST_F(TestOTAImageProcessorImpl, CompleteImageIsApplied)
{
    Prepare();
    Serve(0);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_EQ(FileContents(), std::vector<uint8_t>(mImage.begin() + sizeof(kHeader), mImage.end()));
    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

// Finalize() reports the result of its check to the downloader, so a corrupt image ends the download as failed before the
// requestor asks the provider to apply it, instead of being found by Apply() after the provider's DelayedActionTime.
TEST_F(TestOTAImageProcessorImpl, ImageFailingTheDigestIsReportedAtFinalize)
{
    mImage[kImageSize - 1] ^= 0xff;
    Prepare();
    Serve(0);
    Complete();

    ASSERT_EQ(mDownloader.mFinalizeResults.size(), 1u);
    EXPECT_EQ(mDownloader.mFinalizeResults[0], CHIP_ERROR_INTEGRITY_CHECK_FAILED);
}

TEST_F(TestOTAImageProcessorImpl, VerifiedImageIsReportedAtFinalize)
{
    EXPECT_TRUE(mProcessor.ReportsFinalizeResult());
    Prepare();
    Serve(0);
    Complete();

    ASSERT_EQ(mDownloader.mFinalizeResults.size(), 1u);
    EXPECT_EQ(mDownloader.mFinalizeResults[0], CHIP_NO_ERROR);
}

// The partial image of a resumed download is hashed in work items of its own, a chunk each, rather than in the prepare's work item,
// so a large partial image does not hold the event loop for its whole length. Work items run in the order they are scheduled, so
// an item scheduled right after the prepare runs before the hashing does.
TEST_F(TestOTAImageProcessorImpl, ResumedPartialImageIsHashedInItsOwnWorkItems)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    ASSERT_EQ(mProcessor.GetResumeOffset(), 4 * kBlockSize);

    mDownloader.mPrepareStatus = CHIP_ERROR_INTERNAL;
    ASSERT_EQ(mProcessor.PrepareDownloadFrom(mProcessor.GetResumeOffset()), CHIP_NO_ERROR);
    static CHIP_ERROR sStatusAfterPrepareItem;
    sStatusAfterPrepareItem = CHIP_NO_ERROR;
    ASSERT_EQ(DeviceLayer::PlatformMgr().ScheduleWork(
                  [](intptr_t arg) { sStatusAfterPrepareItem = reinterpret_cast<FakeDownloader *>(arg)->mPrepareStatus; },
                  reinterpret_cast<intptr_t>(&mDownloader)),
              CHIP_NO_ERROR);
    for (int i = 0; i < 64 && mDownloader.mPrepareStatus == CHIP_ERROR_INTERNAL; i++)
    {
        Drain();
    }

    EXPECT_EQ(sStatusAfterPrepareItem, CHIP_ERROR_INTERNAL);
    EXPECT_EQ(mDownloader.mPrepareStatus, CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, FinalizeTwiceKeepsTheVerifiedResult)
{
    Prepare();
    Serve(0);
    Complete();
    Complete();

    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, ImageFailingTheDigestIsDiscardedWhenApplyRejectsIt)
{
    mImage[kImageSize - 1] ^= 0xff;
    Prepare();
    Serve(0);
    Complete();

    EXPECT_EQ(mProcessor.Apply(), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
    Drain();
    EXPECT_FALSE(std::filesystem::exists(mPath));
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);
    EXPECT_EQ(mProcessor.Apply(), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
}

TEST_F(TestOTAImageProcessorImpl, ResumedImageIsApplied)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    ASSERT_EQ(mProcessor.GetResumeOffset(), 4 * kBlockSize);
    Prepare(mProcessor.GetResumeOffset());
    Serve(4 * kBlockSize);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_EQ(FileContents(), std::vector<uint8_t>(mImage.begin() + sizeof(kHeader), mImage.end()));
    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

// The partial image file lost data between the suspend and the resume. Appending to it would install an image that misses
// that data, even though the bytes counted as received and their digest match the header.
TEST_F(TestOTAImageProcessorImpl, TruncatedPartialImageIsNotResumed)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    ASSERT_EQ(mProcessor.GetResumeOffset(), 4 * kBlockSize);
    const uint64_t requested = mProcessor.GetResumeOffset();
    std::filesystem::resize_file(mPath, 16);

    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);
    ASSERT_EQ(mProcessor.PrepareDownloadFrom(requested), CHIP_NO_ERROR);
    Drain();
    EXPECT_NE(mDownloader.mPrepareStatus, CHIP_NO_ERROR);
}

// The partial image file changed on disk between the suspend and the resume, but kept its size. The blocks that were received
// match the header, the file that would be installed does not.
TEST_F(TestOTAImageProcessorImpl, ImageChangedOnDiskIsNotApplied)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    {
        std::fstream file(mPath, std::ios::in | std::ios::out | std::ios::binary);
        ASSERT_TRUE(file.good());
        file.seekp(0);
        file.put('\0');
        ASSERT_TRUE(file.good());
    }
    Prepare(mProcessor.GetResumeOffset());
    Serve(4 * kBlockSize);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_EQ(mProcessor.GetBytesDownloaded(), kPayloadSize);
    EXPECT_EQ(mProcessor.Apply(), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
}

TEST_F(TestOTAImageProcessorImpl, ResumedImageServedFromStartEndsDownload)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    Prepare(4 * kBlockSize);
    Serve(0, 2);

    ASSERT_EQ(mDownloader.mEndReasons.size(), 1u);
    EXPECT_EQ(mDownloader.mEndReasons[0], CHIP_ERROR_INVALID_FILE_IDENTIFIER);
    EXPECT_LE(mProcessor.GetBytesDownloaded(), kPayloadSize);
}

TEST_F(TestOTAImageProcessorImpl, ImageLongerThanItsHeaderIsNotApplied)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    Prepare(4 * kBlockSize);
    Serve(4 * kBlockSize - 16);
    Complete();

    EXPECT_NE(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, ImageShorterThanItsHeaderIsNotApplied)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    Prepare(4 * kBlockSize);
    Serve(4 * kBlockSize + 16);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_NE(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, ResumedImageFromTheWrongOffsetIsNotApplied)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    Prepare(4 * kBlockSize);
    Serve(0, SIZE_MAX, kImageSize - 4 * kBlockSize);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_EQ(mProcessor.GetBytesDownloaded(), kPayloadSize);
    EXPECT_NE(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, DigestOfTheWrongLengthEndsDownload)
{
    mImage[kDigestTypeOffset] = static_cast<uint8_t>(OTAImageDigestType::kSha256_128);
    Prepare();
    Serve(0, sizeof(kHeader) / kBlockSize + 1);

    ASSERT_EQ(mDownloader.mEndReasons.size(), 1u);
    EXPECT_EQ(mDownloader.mEndReasons[0], CHIP_ERROR_INVALID_FILE_IDENTIFIER);
}

TEST_F(TestOTAImageProcessorImpl, UnverifiableDigestStartsAnew)
{
    mImage[kDigestTypeOffset] = static_cast<uint8_t>(OTAImageDigestType::kSha384);
    Prepare();
    Serve(0, 4);
    Suspend();

    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    Prepare();
    Serve(0);
    Complete();
    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, UnverifiableImageShorterThanItsHeaderIsNotApplied)
{
    mImage[kDigestTypeOffset] = static_cast<uint8_t>(OTAImageDigestType::kSha384);
    Prepare();
    Serve(0, SIZE_MAX, kImageSize - 16);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_NE(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, SuspendInsideHeaderStartsAnew)
{
    Prepare();
    Serve(0);
    Complete();
    Prepare();
    Serve(0, 1);
    Suspend();

    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);

    Prepare();
    Serve(0);
    Complete();
    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, PrepareFromZeroBeforeSuspendCompletesStartsAnew)
{
    Prepare();
    Serve(0, 4);
    // The suspend is still queued when the downloader decides; it saw no partial image, so it asks for offset 0.
    ASSERT_EQ(mProcessor.SuspendDownload(), CHIP_NO_ERROR);
    const uint64_t requested = mProcessor.GetResumeOffset();
    ASSERT_EQ(mProcessor.PrepareDownloadFrom(requested), CHIP_NO_ERROR);
    Drain();
    ASSERT_EQ(mDownloader.mPrepareStatus, CHIP_NO_ERROR);
    Serve(requested);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_EQ(FileContents(), Payload());
    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, PrepareFromOffsetAfterAbortFailsInsteadOfAppending)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    // The downloader saw the partial image and asked to continue it, but a discard queued before the prepare wins.
    const uint64_t requested = mProcessor.GetResumeOffset();
    ASSERT_GT(requested, 0u);
    ASSERT_EQ(mProcessor.Abort(), CHIP_NO_ERROR);
    ASSERT_EQ(mProcessor.PrepareDownloadFrom(requested), CHIP_NO_ERROR);
    Drain();

    EXPECT_NE(mDownloader.mPrepareStatus, CHIP_NO_ERROR);
    EXPECT_TRUE(FileContents().empty());
    EXPECT_EQ(mProcessor.GetResumeOffset(), 0u);
}

TEST_F(TestOTAImageProcessorImpl, PrepareFromZeroDiscardsSuspendedImage)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    ASSERT_GT(mProcessor.GetResumeOffset(), 0u);
    Prepare(0);
    Serve(0);
    Complete();

    EXPECT_TRUE(mDownloader.mEndReasons.empty());
    EXPECT_EQ(FileContents(), Payload());
    EXPECT_EQ(mProcessor.Apply(), CHIP_NO_ERROR);
}

TEST_F(TestOTAImageProcessorImpl, PrepareFromOtherOffsetFailsInsteadOfAppending)
{
    Prepare();
    Serve(0, 4);
    Suspend();
    ASSERT_GT(mProcessor.GetResumeOffset(), 0u);
    ASSERT_EQ(mProcessor.PrepareDownloadFrom(mProcessor.GetResumeOffset() + kBlockSize), CHIP_NO_ERROR);
    Drain();

    EXPECT_NE(mDownloader.mPrepareStatus, CHIP_NO_ERROR);
    EXPECT_TRUE(FileContents().empty());
}

} // namespace
