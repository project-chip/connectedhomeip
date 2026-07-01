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
#include <crypto/CHIPCryptoPAL.h>
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

enum class SubImageStatus : uint8_t
{
    kWritten,         // sub-processor was kReady; all bytes delivered successfully
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
     * @brief Registers a sub-processor for an image-ID tag.
     *
     * @param entry Entry node to register. Ownership remains with the caller.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if the
     *         processor is null or the tag is 0, or
     *         CHIP_ERROR_DUPLICATE_KEY_ID if the tag is already registered.
     */
    CHIP_ERROR RegisterProcessor(ImageProcessorEntry & entry);

    /**
     * @brief Decides whether to apply a downloaded OTA update.
     *
     * Called after a successful download, before the apply phase. The application
     * can inspect the per-sub-image results and decide whether to proceed.
     *
     * Runs on the Matter thread and must not block. The default implementation
     * always returns true.
     *
     * @return true to apply the update; false to discard it.
     */
    virtual bool ShouldApplyUpdate(Span<const SubImageResult> results) { return true; }

    /**
     * @brief Verifies a newly booted OTA image before it is committed.
     *
     * Called on the first boot after an OTA update. The default implementation
     * confirms the running software version matches the target version. Override
     * this method to perform custom validation instead.
     *
     * Runs on the Matter thread early during boot and must not block.
     *
     * @return CHIP_NO_ERROR to accept the new image; otherwise an error to reject it.
     */
    virtual CHIP_ERROR ConfirmOTASuccess();

private:
    OTADownloader * mDownloader = nullptr;
    MutableByteSpan mBlock;
    MultiOTAImageHeader mMultiOTAImageHeader;

    OTAImageHeaderParser mHeaderParser;
    MultiOTAImageHeaderParser mMultiOTAImageHeaderParser;
    ImageProcessorEntry * mRegistryHead = nullptr;
    CHIP_ERROR mLastErr                 = CHIP_NO_ERROR;

    // Tracker for the current sub-image, and the entry currently being written.
    uint32_t mCurrentSubImageCursor      = 0;
    bool mCurrentEntryStarted            = false;   // readiness for the active entry already decided
    SubImageProcessor * mActiveProcessor = nullptr; // active sub-processor with kReady state
    Crypto::Hash_SHA256_stream mActiveHasher;       // SHA-256 over the active sub-image's wire bytes

    // Per-sub-image result table, reported to ShouldApplyUpdate() callback.
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
