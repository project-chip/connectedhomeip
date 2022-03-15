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

#pragma once

#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

namespace chip {

struct OTAImageProgress
{
    uint64_t downloadedBytes = 0;
    uint64_t totalFileBytes  = 0;
};

/**
 * @class OTAImageProcessorInterface
 *
 * @brief
 *   This is a platform-agnostic interface for processing downloaded
 *   chunks of OTA image data. The data could be raw image data meant for flash or
 *   metadata. Each platform should provide an implementation of this
 *   interface.
 */
class DLL_EXPORT OTAImageProcessorInterface
{
public:
    virtual ~OTAImageProcessorInterface() {}

    /**
     * Called to prepare for an OTA image download. This may include but not limited to opening the file, finding a block of space
     * in persistent memory, and allocating a buffer. This must not be a blocking call.
     */
    virtual CHIP_ERROR PrepareDownload() = 0;

    /**
     * Called when the OTA image download process has completed. This may include but not limited to closing the file and persistent
     * storage. This must not be a blocking call.
     */
    virtual CHIP_ERROR Finalize() = 0;

    /**
     * Called when the OTA image should be applied.
     */
    virtual CHIP_ERROR Apply() = 0;

    /**
     * Called when the OTA image download process is incomplete or cannot continue. This may include but not limited to erasing
     * everything that has been written and releasing buffers. This must not be a blocking call.
     */
    virtual CHIP_ERROR Abort() = 0;

    /**
     * Called to process a downloaded block of data. This must not be a blocking call to support cases that require IO to elements
     * such as external peripherals/radios. This must not be a blocking call.
     */
    virtual CHIP_ERROR ProcessBlock(ByteSpan & block) = 0;

    /**
     * Called to check the current download status of the OTA image download.
     */
    virtual app::DataModel::Nullable<uint8_t> GetPercentComplete()
    {
        return mParams.totalFileBytes > 0
            ? app::DataModel::Nullable<uint8_t>(static_cast<uint8_t>((mParams.downloadedBytes * 100) / mParams.totalFileBytes))
            : app::DataModel::Nullable<uint8_t>{};
    }

    /**
     * Called to check the current number of bytes that have been downloaded of the OTA image
     */
    virtual uint64_t GetBytesDownloaded() { return mParams.downloadedBytes; }

    /**
     * Called to check if the current image is executed for the first time.
     */
    virtual bool IsFirstImageRun() = 0;

    /**
     * Called to confirm the current image in case it is running tentatively after applying
     * a software update.
     */
    virtual CHIP_ERROR ConfirmCurrentImage() = 0;

protected:
    OTAImageProgress mParams;
};

} // namespace chip
