/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/core/OTAImageHeader.h>
#include <map>
#include <platform/nxp/k32w/common/OTATlvProcessor.h>
#include <src/app/clusters/ota-requestor/OTADownloader.h>
#include <src/include/platform/CHIPDeviceLayer.h>
#include <src/include/platform/OTAImageProcessor.h>

/*
 * OTA hooks that can be overwritten by application.
 * Default behavior is implemented as WEAK symbols in
 * platform OtaHooks.cpp.
 */
extern "C" CHIP_ERROR OtaHookInit();
extern "C" void OtaHookReset();

namespace chip {

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    CHIP_ERROR Init(OTADownloader * downloader);
    void Clear();

    //////////// OTAImageProcessorInterface Implementation ///////////////
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    CHIP_ERROR ProcessHeader(ByteSpan & block);
    CHIP_ERROR ProcessPayload(ByteSpan & block);
    CHIP_ERROR SelectProcessor(ByteSpan & block);
    CHIP_ERROR RegisterProcessor(uint32_t tag, OTATlvProcessor * processor);

    static void FetchNextData(uint32_t context);
    static OTAImageProcessorImpl & GetDefaultInstance();

private:
    //////////// Actual handlers for the OTAImageProcessorInterface ///////////////
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleApply(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);

    void HandleStatus(CHIP_ERROR status);

    /**
     * Called to allocate memory for mBlock if necessary and set it to block
     */
    CHIP_ERROR SetBlock(ByteSpan & block);

    /**
     * Called to release allocated memory for mBlock
     */
    CHIP_ERROR ReleaseBlock();

    MutableByteSpan mBlock;
    OTADownloader * mDownloader;
    OTAImageHeaderParser mHeaderParser;
    OTATlvProcessor * mCurrentProcessor = nullptr;
    OTADataAccumulator mAccumulator;
    std::map<uint32_t, OTATlvProcessor *> mProcessorMap;
};

} // namespace chip
