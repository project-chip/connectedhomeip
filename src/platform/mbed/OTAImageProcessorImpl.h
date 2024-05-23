/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/* This file contains the declarations for OTAImageProcessor, a platform-agnostic
 * interface for processing downloaded chunks of OTA image data.
 * Each platform should provide an implementation of this interface.
 */

#pragma once

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/OTAImageProcessor.h>

#include "blockdevice/BlockDevice.h"

namespace chip {

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    OTAImageProcessorImpl();
    /**
     * Called to prepare for an OTA image download. This may include but not limited to opening the file, finding a block of space
     * in persistent memory, and allocating a buffer. This must not be a blocking call.
     */
    CHIP_ERROR PrepareDownload();

    /**
     * Called when the OTA image download process has completed. This may include but not limited to closing the file and persistent
     * storage. This must not be a blocking call.
     */
    CHIP_ERROR Finalize();

    /**
     * Called when the OTA image should be applied.
     */
    CHIP_ERROR Apply();

    /**
     * Called when the OTA image download process is incomplete or cannot continue. This may include but not limited to erasing
     * everything that has been written and releasing buffers. This must not be a blocking call.
     */
    CHIP_ERROR Abort();

    /**
     * Called to process a downloaded block of data. This must not be a blocking call to support cases that require IO to elements
     * such as external peripherals/radios. This must not be a blocking call.
     */
    CHIP_ERROR ProcessBlock(ByteSpan & block);

    bool IsFirstImageRun() override { return false; }

    CHIP_ERROR ConfirmCurrentImage() override { return CHIP_NO_ERROR; }

    /**
     * Check if memory for update image is works correctly.
     */
    int MemoryTest();

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; };

    mbed::BlockDevice * mBlockDevice;

private:
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    static void HandleApply(intptr_t context);

    CHIP_ERROR SetBlock(ByteSpan & block);
    CHIP_ERROR ReleaseBlock();

    int PrepareMemory();
    int ClearMemory();
    int ProgramMemory();
    int CloseMemory();
    void ClearDownloadParams();

    OTADownloader * mDownloader = nullptr;
    MutableByteSpan mBlock;
};

} // namespace chip
