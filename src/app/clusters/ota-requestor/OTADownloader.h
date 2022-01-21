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

/* This file contains the declarations for the OTADownloader class which
 * abstracts the image download functionality from the particular protocol
 * used for it.
 * Applications and platforms implementing the OTA Requestor functionality
 * must include this file
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <platform/OTAImageProcessor.h>

namespace chip {

// A class that abstracts the image download functionality from the particular
// protocol used for that (BDX or possibly HTTPS)
class OTADownloader
{
public:
    enum class State : uint8_t
    {
        kIdle,
        kPreparing,
        kInProgress,
        kComplete,
    };

    OTADownloader() : mImageProcessor(nullptr), mState(State::kIdle) {}

    // Application calls this method to direct OTADownloader to begin the download.
    // OTADownloader should handle calling into OTAImageProcessorDriver::PrepareDownload().
    CHIP_ERROR virtual BeginPrepareDownload() = 0;

    // Platform calls this method when it is ready to begin processing downloaded image data.
    // Upon this call, the OTADownloader may begin downloading data.
    CHIP_ERROR virtual OnPreparedForDownload(CHIP_ERROR status) = 0;

    // Should be called when it has been determined that the download has timed out.
    void virtual OnDownloadTimeout() = 0;

    // Not all download protocols will be able to close gracefully from the receiver side.
    // The reason parameter should be used to indicate if this is a graceful end or a forceful abort.
    void virtual EndDownload(CHIP_ERROR reason = CHIP_NO_ERROR) = 0;

    // Fetch the next set of data. May be a no-op for asynchronous protocols.
    CHIP_ERROR virtual FetchNextData() { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // Skip ahead some number of bytes in the download of the image file. May not be supported by some transport protocols.
    CHIP_ERROR virtual SkipData(uint32_t numBytes) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // A setter for the delegate class pointer
    void SetImageProcessorDelegate(OTAImageProcessorInterface * delegate) { mImageProcessor = delegate; }
    OTAImageProcessorInterface * GetImageProcessorDelegate() const { return mImageProcessor; }

    State GetState() const { return mState; }

    virtual ~OTADownloader() = default;

protected:
    OTAImageProcessorInterface * mImageProcessor = nullptr;
    State mState;
};

} // namespace chip
