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

    // Reaching Finalize means the download completed: BDX only calls Finalize() on the EOF block, and
    // each sub-processor has already committed its image on its last Write() chunk
    Span<const SubImageResult> results(imageProcessor->mSubImageResults.Get(), imageProcessor->mSubImageResultCount);
    if (!imageProcessor->ShouldApplyUpdate(results))
    {
        // discard the staged update and stay on the old firmware. Aborting the
        // sub-processors clears AppImageProcessor's partition handle, so a subsequent Apply() is a
        // no-op. The next QueryImage retries the bundle.
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

    // Route the block's bytes to the sub-processors. A single block may span several entries
    // (split-block rule); we keep consuming until the block is empty, then request one more block.
    while (!remainingBlock.empty())
    {
        // Locate the sub-image whose byte range covers the current payload cursor.
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

        // Cursor is not in any sub-image, bundle is invalid stop the download
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

        // Decide readiness exactly once, when the entry starts (R2 step 3/4).
        if (!imageProcessor->mCurrentEntryStarted)
        {
            SubImageProcessor * processor = nullptr;
            DeviceState readiness         = DeviceState::kUnknown;

            if (imageProcessor->GetSubProcessor(activeSubImage->imageId, processor) != CHIP_NO_ERROR || processor == nullptr)
            {
                // No processor registered for this image ID — assume already up to date (R2 step 3).
                ChipLogDetail(SoftwareUpdate, "No processor for image ID 0x%" PRIx32 ", skipping", activeSubImage->imageId);
                readiness = DeviceState::kAlreadyUpToDate;
            }
            else
            {
                if (!processor->IsInitialized())
                {
                    imageProcessor->mLastErr = processor->Init(*activeSubImage);
                    VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                                   imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                                   PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));
                }
                imageProcessor->mLastErr = processor->IsReadyForOTA(readiness);
                VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                               imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                               PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));
            }

            // Not being written this cycle: skip the whole entry (R2 step 5). SkipData IS the next
            // block request
            if (readiness != DeviceState::kReady)
            {
                const SubImageStatus status =
                    (readiness == DeviceState::kNotReady) ? SubImageStatus::kSkippedNotReady : SubImageStatus::kSkippedUpToDate;
                imageProcessor->RecordSubImageResult(activeSubImage->imageId, activeSubImage->version, status);
                imageProcessor->mParams.downloadedBytes += bytesLeftInSubImage; // progress counts skipped bytes (step 7)
                imageProcessor->mCurrentSubImageCursor = subImageEnd;
                LogErrorOnFailure(imageProcessor->mDownloader->SkipData(bytesLeftInSubImage));
                return;
            }

            imageProcessor->mActiveProcessor     = processor;
            imageProcessor->mCurrentEntryStarted = true;
        }

        // kReady: stream this entry's bytes to its processor.
        const uint32_t bytesToDeliver = std::min(bytesLeftInSubImage, static_cast<uint32_t>(remainingBlock.size()));
        ByteSpan chunk                = remainingBlock.SubSpan(0, bytesToDeliver);

        imageProcessor->mLastErr = imageProcessor->mActiveProcessor->Write(chunk);
        VerifyOrReturn(imageProcessor->mLastErr == CHIP_NO_ERROR,
                       imageProcessor->mDownloader->EndDownload(imageProcessor->mLastErr);
                       PostOTAStateChangeEvent(DeviceLayer::kOtaDownloadFailed));

        imageProcessor->mParams.downloadedBytes += bytesToDeliver;
        imageProcessor->mCurrentSubImageCursor += bytesToDeliver;
        remainingBlock = remainingBlock.SubSpan(bytesToDeliver);

        // Entry fully delivered — record the outcome; the next entry (if any) starts next iteration.
        if (imageProcessor->mCurrentSubImageCursor == subImageEnd)
        {
            imageProcessor->RecordSubImageResult(activeSubImage->imageId, activeSubImage->version, SubImageStatus::kWritten);
            imageProcessor->mCurrentEntryStarted = false;
            imageProcessor->mActiveProcessor     = nullptr;
        }
    }

    // Request the next block once, unless the last entry's final bytes were just delivered: the EOF
    // block has already queued Finalize(), so a FetchNextData() here would corrupt the BDX session
    // (R2 step 6). The app image is always the last, always-written entry, so the symmetric
    // "skip into EOF" case cannot occur.
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

        // exactly one app image, and it is the last entry
        VerifyOrReturnError(mMultiOTAImageHeader.subImages.back().imageId == kAppImageProcessorTag,
                            CHIP_ERROR_INVALID_FILE_IDENTIFIER);

        size_t appImageCount = 0;
        // every sub image entry must lie within the payload
        for (const auto & subImage : mMultiOTAImageHeader.subImages)
        {
            VerifyOrReturnError(static_cast<uint64_t>(subImage.offset) + subImage.length <= mParams.totalFileBytes,
                                CHIP_ERROR_INVALID_FILE_IDENTIFIER);
            if (subImage.imageId == kAppImageProcessorTag)
            {
                appImageCount++;
            }
        }
        VerifyOrReturnError(appImageCount == 1, CHIP_ERROR_INVALID_FILE_IDENTIFIER);

        // Allocate the per-sub-image result table (one slot per sub-image) for the ShouldApplyUpdate hook.
        VerifyOrReturnError(mSubImageResults.Alloc(mMultiOTAImageHeader.subImages.size()), CHIP_ERROR_NO_MEMORY);
        mSubImageResultCount = 0;

        // The MultiImageHeader is fully decoded. Both progress accounting and the routing cursor
        // start at the first binary's offset, i.e. sizeof(MultiImageHeader) = 8 + numImages * 48.
        const uint32_t headerSize = kFixedHeaderSize + mMultiOTAImageHeader.subImages.size() * kSubImageHeaderSize;
        mParams.downloadedBytes   = headerSize;
        mCurrentSubImageCursor    = headerSize;
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
    // The table is sized to numImages and each entry is recorded exactly once, in order.
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
