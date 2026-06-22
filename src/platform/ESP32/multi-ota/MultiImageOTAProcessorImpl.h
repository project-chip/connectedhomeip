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

#pragma once
#include "MultiOTAImageHeader.h"
#include "SubImageProcessor.h"
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <include/platform/OTAImageProcessor.h>
#include <lib/core/CHIPError.h>
#include <lib/core/OTAImageHeader.h>

namespace chip {

struct ImageProcessorEntry
{
    OTAProcessorTag tag;
    SubImageProcessor * processor;
    ImageProcessorEntry * next = nullptr;
};

// How each sub-image was processed during the download, reported to ShouldApplyUpdate (§5).
enum class SubImageStatus : uint8_t
{
    kWritten,         // sub-processor was kReady; all bytes delivered
    kSkippedUpToDate, // kAlreadyUpToDate, or no processor registered for the imageId
    kSkippedNotReady, // kNotReady
};

// Result of processing one sub-image.
struct SubImageResult
{
    OTAProcessorTag tag;
    uint32_t version;
    SubImageStatus status;
};

class MultiImageOTAProcessorImpl : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; };
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    /**
     * Register a processor entry to Image processor registry.
     * @param entry The processor entry to register.
     * @return CHIP_NO_ERROR if the processor was registered successfully
     * CHIP_ERROR_INVALID_ARGUMENT if the processor is null or the tag is 0
     * CHIP_ERROR_DUPLICATE_KEY_ID if the processor is already bound to a tag
     * CHIP_ERROR if other error occurred
     */
    CHIP_ERROR RegisterProcessor(ImageProcessorEntry & entry);

    /**
     * Invoked once after a successful download, on the Matter thread, before the apply phase (§5).
     * Inspect the per-sub-image results and return true to apply the staged update, or false to
     * discard it (the device stays on the old firmware and the next QueryImage retries the bundle).
     * Must not block — decide from the in-RAM results only. Default implementation returns true.
     */
    virtual bool ShouldApplyUpdate(Span<const SubImageResult> results) { return true; }

private:
    OTADownloader * mDownloader = nullptr;
    MutableByteSpan mBlock;
    MultiOTAImageHeader mMultiOTAImageHeader;

    OTAImageHeaderParser mHeaderParser;
    MultiOTAImageHeaderParser mMultiOTAImageHeaderParser;
    ImageProcessorEntry * mRegistryHead = nullptr;
    CHIP_ERROR mLastErr                 = CHIP_NO_ERROR;

    // Routing state: where we are in the payload, and the entry currently being written.
    uint32_t mCurrentSubImageCursor      = 0;
    bool mCurrentEntryStarted            = false;   // readiness for the active entry already decided
    SubImageProcessor * mActiveProcessor = nullptr; // processor for the active (kReady) entry

    // Per-sub-image result table (one slot per sub-image), reported to ShouldApplyUpdate (§5).
    Platform::ScopedMemoryBuffer<SubImageResult> mSubImageResults;
    uint8_t mSubImageResultCount = 0;

    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    static void HandleApply(intptr_t context);

    CHIP_ERROR SetBlock(ByteSpan & block);
    CHIP_ERROR ReleaseBlock();
    CHIP_ERROR ProcessHeader(ByteSpan & block);
    CHIP_ERROR ProcessMultiImageHeader(ByteSpan & block);
    CHIP_ERROR GetSubProcessor(OTAProcessorTag tag, SubImageProcessor *& processor) const;

    void RecordSubImageResult(OTAProcessorTag tag, uint32_t version, SubImageStatus status);
    void AbortSubProcessors(AbortReason reason, CHIP_ERROR error);
};

} // namespace chip
