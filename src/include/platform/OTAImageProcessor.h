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

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>

namespace chip {

struct OTAImageProcessorParams
{
    CharSpan imageFile;
    uint64_t downloadedBytes;
    uint64_t totalFileBytes;
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
     * Called to setup params for the OTA image download
     */
    virtual void SetOTAImageProcessorParams(OTAImageProcessorParams & params) { mParams = params; };

    /**
     * Called to check the current download status of the OTA image download.
     */
    virtual uint8_t GetPercentComplete()
    {
        if (mParams.totalFileBytes == 0)
        {
            return 0;
        }
        else
        {
            return static_cast<uint8_t>((mParams.downloadedBytes * 100) / mParams.totalFileBytes);
        }
    }

protected:
    OTAImageProcessorParams mParams;
};

} // namespace chip
