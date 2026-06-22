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
#include "esp_ota_ops.h"
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <include/platform/OTAImageProcessor.h>
#include <lib/core/CHIPError.h>
#include <lib/core/OTAImageHeader.h>

namespace chip {

typedef uint8_t OTAProcessorTag;

struct SubImageProcessorEntry
{
    OTAProcessorTag tag;
    SubImageProcessor * processor;
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

    CHIP_ERROR RegisterProcessor(OTAProcessorTag tag, SubImageProcessor * processor);

private:
    SubImageProcessorEntry mSubImageProcessors;
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    static void HandleApply(intptr_t context);

    CHIP_ERROR SetBlock(ByteSpan & block);
    CHIP_ERROR ReleaseBlock();
    CHIP_ERROR ProcessHeader(ByteSpan & block);

    OTADownloader * mDownloader = nullptr;
    MutableByteSpan mBlock;
    const esp_partition_t * mOTAUpdatePartition = nullptr;
    esp_ota_handle_t mOTAUpdateHandle;

    OTAImageHeaderParser mHeaderParser;
    MultiOTAImageHeaderParser mMultiOTAImageHeaderParser;
};

} // namespace chip
