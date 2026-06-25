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

#include <algorithm>

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/ESP32/ESP32Utils.h>

#include "MultiImageOTAProcessorImpl.h"
#include "lib/core/CHIPError.h"

#define TAG "MultiImageOTAProcessor"

using namespace chip::System;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace {

void PostOTAStateChangeEvent(DeviceLayer::OtaState newState)
{
    DeviceLayer::ChipDeviceEvent otaChange;
    otaChange.Type                     = DeviceLayer::DeviceEventType::kOtaStateChanged;
    otaChange.OtaStateChanged.newState = newState;
    CHIP_ERROR error                   = DeviceLayer::PlatformMgr().PostEvent(&otaChange);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Error while posting OtaChange event %" CHIP_ERROR_FORMAT, error.Format());
    }
}

} // namespace

bool MultiImageOTAProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR MultiImageOTAProcessorImpl::ConfirmCurrentImage()
{
    OTARequestorInterface * requestor = GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != requestor->GetTargetVersion())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::PrepareDownload()
{
    LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::Finalize()
{
    LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::Apply()
{
    LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::Abort()
{
    LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::ProcessBlock(ByteSpan & block)
{
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    LogErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::RegisterProcessor(ImageProcessorEntry & entry)
{
    VerifyOrReturnError(entry.processor != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(entry.tag != 0, CHIP_ERROR_INVALID_ARGUMENT);

    for (auto * current = mRegistryHead; current != nullptr; current = current->next)
    {
        VerifyOrReturnError(current->tag != entry.tag, CHIP_ERROR_DUPLICATE_KEY_ID);
    }
    entry.next    = mRegistryHead;
    mRegistryHead = &entry;
    return CHIP_NO_ERROR;
}

void MultiImageOTAProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<MultiImageOTAProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    imageProcessor->mHeaderParser.Init();
    imageProcessor->mMultiOTAImageHeaderParser.Init();
    imageProcessor->mCurrentSubImageCursor = 0;
    imageProcessor->mCurrentEntryStarted   = false;
    imageProcessor->mActiveProcessor       = nullptr;
    imageProcessor->mLastErr               = CHIP_NO_ERROR;
    imageProcessor->mSubImageResultCount   = 0;
    LogErrorOnFailure(imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR));
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadInProgress);
}

void MultiImageOTAProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<MultiImageOTAProcessorImpl *>(context);
    VerifyOrReturn(imageProcessor, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));

    // Download complete; let the application decide whether to apply the update.
    Span<const SubImageResult> results(imageProcessor->mSubImageResults.Get(), imageProcessor->mSubImageResultCount);
    if (!imageProcessor->ShouldApplyUpdate(results))
    {
        // Abort OTA for all the sub-processors and discard the update.
        ChipLogProgress(SoftwareUpdate, "ShouldApplyUpdate vetoed apply; discarding update");
        imageProcessor->AbortSubProcessors(AbortReason::kCancelled, CHIP_NO_ERROR);
        LogErrorOnFailure(imageProcessor->ReleaseBlock());
        imageProcessor->mMultiOTAImageHeaderParser.Clear();
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    LogErrorOnFailure(imageProcessor->ReleaseBlock());
    imageProcessor->mMultiOTAImageHeaderParser.Clear();
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadComplete);
}

void MultiImageOTAProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<MultiImageOTAProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    const AbortReason reason = (imageProcessor->mLastErr != CHIP_NO_ERROR) ? AbortReason::kError : AbortReason::kCancelled;
    imageProcessor->AbortSubProcessors(reason, imageProcessor->mLastErr);
    LogErrorOnFailure(imageProcessor->ReleaseBlock());
    imageProcessor->mMultiOTAImageHeaderParser.Clear();
    PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadAborted);
}

void MultiImageOTAProcessorImpl::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<MultiImageOTAProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());

    CHIP_ERROR error = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        imageProcessor->mDownloader->EndDownload(error);
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    error = imageProcessor->ProcessMultiImageHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        imageProcessor->mDownloader->EndDownload(error);
        PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
        return;
    }

    ByteSpan remainingBlock    = block;
    const uint64_t payloadSize = imageProcessor->mParams.totalFileBytes;

    // Consume the entire block before fetching the next one. It may span several entries.
    while (!remainingBlock.empty())
    {
        // Find the sub-image covering the cursor position
        const SubImageHeader * activeSubImage = nullptr;
        for (const auto & subImage : imageProcessor->mMultiOTAImageHeader.subImages)
        {
            const uint64_t subImageEnd = static_cast<uint64_t>(subImage.offset) + subImage.length;
            if (imageProcessor->mCurrentSubImageCursor >= subImage.offset && imageProcessor->mCurrentSubImageCursor < subImageEnd)
            {
                activeSubImage = &subImage;
                break;
            }
        }

        // Cursor outside every sub-image — malformed bundle.
        if (activeSubImage == nullptr)
        {
            imageProcessor->mLastErr = CHIP_ERROR_INVALID_FILE_IDENTIFIER;
            ChipLogError(SoftwareUpdate, "No sub-image covers payload offset %llu",
                         static_cast<unsigned long long>(imageProcessor->mCurrentSubImageCursor));
            imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
            PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
            return;
        }

        const uint32_t subImageEnd         = activeSubImage->offset + activeSubImage->length;
        const uint32_t bytesLeftInSubImage = subImageEnd - imageProcessor->mCurrentSubImageCursor;

        if (!imageProcessor->mCurrentEntryStarted)
        {
            SubImageProcessor * processor = nullptr;
            DeviceState readiness         = DeviceState::kUnknown;

            if (imageProcessor->GetSubProcessor(activeSubImage->imageId, processor) != CHIP_NO_ERROR || processor == nullptr)
            {
                // No processor registered for this image ID, assume already up to date.
                ChipLogError(SoftwareUpdate, "No processor for image ID 0x%" PRIx32 ", skipping", activeSubImage->imageId);
                readiness = DeviceState::kAlreadyUpToDate;
            }
            else
            {
                // Init() unconditionally so each session starts fresh. Called once for each sub-image.
                imageProcessor->mLastErr = processor->Init(*activeSubImage);
                VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                               imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                               PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));

                imageProcessor->mLastErr = processor->IsReadyForOTA(readiness);
                VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                               imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                               PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));
            }

            // OTA will be skipped this cycle: drop the rest of this entry and move to the next one.
            if (readiness != DeviceState::kReady)
            {
                const SubImageStatus status =
                    (readiness == DeviceState::kNotReady) ? SubImageStatus::kSkippedNotReady : SubImageStatus::kSkippedUpToDate;
                imageProcessor->RecordSubImageResult(activeSubImage->imageId, activeSubImage->version, status);

                // OTA provider will skip the bytes from what it has already sent (the end of this block), not from the cursor we're
                // at. So we need to skip only the part of this entry that has not been sent yet; any bytes already in this block
                // are dropped here.
                const uint32_t deliveredEnd = imageProcessor->mCurrentSubImageCursor + static_cast<uint32_t>(remainingBlock.size());
                imageProcessor->mParams.downloadedBytes += bytesLeftInSubImage; // progress counts skipped bytes
                imageProcessor->mCurrentSubImageCursor = subImageEnd;

                if (subImageEnd <= deliveredEnd)
                {
                    // If the skippable required bytes are already in this block, consume them locally No need to call SkipData()
                    remainingBlock = remainingBlock.SubSpan(bytesLeftInSubImage);
                    continue;
                }

                LogErrorOnFailure(imageProcessor->mDownloader->SkipData(subImageEnd - deliveredEnd));
                return;
            }

            imageProcessor->mActiveProcessor     = processor;
            imageProcessor->mCurrentEntryStarted = true;

            // Begin hashing this sub-image's on-wire bytes for the integrity check.
            imageProcessor->mLastErr = imageProcessor->mActiveHasher.Begin();
            VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                           imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                           PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));
        }

        // kReady: stream this entry's bytes to its processor.
        const uint32_t bytesToDeliver = std::min(bytesLeftInSubImage, static_cast<uint32_t>(remainingBlock.size()));
        ByteSpan chunk                = remainingBlock.SubSpan(0, bytesToDeliver);

        // Hash the on-wire bytes, then hand them to the processor (which may decrypt/patch them).
        imageProcessor->mLastErr = imageProcessor->mActiveHasher.AddData(chunk);
        VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                       imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                       PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));

        imageProcessor->mLastErr = imageProcessor->mActiveProcessor->Write(chunk);
        VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                       imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                       PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));

        imageProcessor->mParams.downloadedBytes += bytesToDeliver;
        imageProcessor->mCurrentSubImageCursor += bytesToDeliver;
        remainingBlock = remainingBlock.SubSpan(bytesToDeliver);

        // Entry fully delivered: verify its integrity, close it, then advance to the next.
        if (imageProcessor->mCurrentSubImageCursor == subImageEnd)
        {
            uint8_t digest[Crypto::kSHA256_Hash_Length];
            MutableByteSpan digestSpan(digest);
            imageProcessor->mLastErr = imageProcessor->mActiveHasher.Finish(digestSpan);
            VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                           imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                           PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));

            if (!digestSpan.data_equal(ByteSpan(activeSubImage->sha256)))
            {
                imageProcessor->mLastErr = CHIP_ERROR_INTEGRITY_CHECK_FAILED;
                ChipLogError(SoftwareUpdate, "Sub-image 0x%" PRIx32 " SHA-256 mismatch", activeSubImage->imageId);
                imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed);
                return;
            }

            imageProcessor->mLastErr = imageProcessor->mActiveProcessor->Finish();
            VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                           imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                           PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));

            imageProcessor->RecordSubImageResult(activeSubImage->imageId, activeSubImage->version, SubImageStatus::kWritten);
            imageProcessor->mCurrentEntryStarted = false;
            imageProcessor->mActiveProcessor     = nullptr;
        }
    }

    // Fetch the next block unless the whole payload is delivered: the EOF block already queued
    // Finalize(), so fetching again would corrupt the BDX session.
    const bool payloadConsumed =
        imageProcessor->mMultiOTAImageHeaderParser.IsHeaderParsed() && imageProcessor->mCurrentSubImageCursor >= payloadSize;
    if (!payloadConsumed)
    {
        LogErrorOnFailure(imageProcessor->mDownloader->FetchNextData());
    }
}

void MultiImageOTAProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<MultiImageOTAProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }

    for (ImageProcessorEntry * entry = imageProcessor->mRegistryHead; entry != nullptr; entry = entry->next)
    {
        if (!entry->processor->IsInitialized())
        {
            continue;
        }
        LogErrorOnFailure(entry->processor->Apply());
    }
    PostOTAStateChangeEvent(DeviceLayer::kOtaApplyComplete);
}

CHIP_ERROR MultiImageOTAProcessorImpl::SetBlock(ByteSpan & block)
{
    if (block.empty())
    {
        LogErrorOnFailure(ReleaseBlock());
        return CHIP_NO_ERROR;
    }
    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            LogErrorOnFailure(ReleaseBlock());
        }
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(Platform::MemoryAlloc(block.size()));
        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        Platform::MemoryFree(mBlock.data());
    }
    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Need more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::ProcessMultiImageHeader(ByteSpan & block)
{
    if (mMultiOTAImageHeaderParser.IsInitialized() && !mMultiOTAImageHeaderParser.IsHeaderParsed())
    {
        CHIP_ERROR error = mMultiOTAImageHeaderParser.AccumulateAndDecode(block, mMultiOTAImageHeader);
        // Need more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        // The app image must be the last entry, and there must be exactly one.
        VerifyOrReturnError(mMultiOTAImageHeader.subImages.back().imageId == kAppImageProcessorTag,
                            CHIP_ERROR_INVALID_FILE_IDENTIFIER);

        const uint32_t headerSize = kFixedHeaderSize + mMultiOTAImageHeader.subImages.size() * kSubImageHeaderSize;
        VerifyOrReturnError(mParams.totalFileBytes <= UINT32_MAX, CHIP_ERROR_INVALID_FILE_IDENTIFIER);

        uint64_t expectedOffset = headerSize;
        size_t appImageCount    = 0;
        for (const auto & subImage : mMultiOTAImageHeader.subImages)
        {
            VerifyOrReturnError(subImage.length > 0, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
            VerifyOrReturnError(subImage.offset == expectedOffset, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
            expectedOffset += subImage.length;
            if (subImage.imageId == kAppImageProcessorTag)
            {
                appImageCount++;
            }
        }
        VerifyOrReturnError(expectedOffset == mParams.totalFileBytes, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        VerifyOrReturnError(appImageCount == 1, CHIP_ERROR_INVALID_FILE_IDENTIFIER);

        VerifyOrReturnError(mSubImageResults.Alloc(mMultiOTAImageHeader.subImages.size()), CHIP_ERROR_NO_MEMORY);
        mSubImageResultCount = 0;

        // Cursor and progress start at the first binary's offset.
        mParams.downloadedBytes = headerSize;
        mCurrentSubImageCursor  = headerSize;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiImageOTAProcessorImpl::GetSubProcessor(OTAProcessorTag tag, SubImageProcessor *& processor) const
{
    processor                   = nullptr;
    ImageProcessorEntry * entry = mRegistryHead;
    while (entry != nullptr)
    {
        if (entry->tag == tag)
        {
            processor = entry->processor;
            return CHIP_NO_ERROR;
        }
        entry = entry->next;
    }
    return CHIP_ERROR_NOT_FOUND;
}

void MultiImageOTAProcessorImpl::RecordSubImageResult(OTAProcessorTag tag, uint32_t version, SubImageStatus status)
{
    if (mSubImageResults.Get() != nullptr && mSubImageResultCount < mMultiOTAImageHeader.subImages.size())
    {
        mSubImageResults[mSubImageResultCount++] = SubImageResult{ tag, version, status };
    }
}

void MultiImageOTAProcessorImpl::AbortSubProcessors(AbortReason reason, CHIP_ERROR error)
{
    AbortContext abortContext{ reason, error };
    for (ImageProcessorEntry * entry = mRegistryHead; entry != nullptr; entry = entry->next)
    {
        if (entry->processor->IsInitialized())
        {
            entry->processor->Abort(abortContext);
        }
    }
}

} // namespace chip
