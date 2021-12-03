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

#include "OTAImageProcessor.h"

#pragma once

namespace chip {

// A class that abstracts the image download functionality from the particular
// protocol used for that (BDX or possibly HTTPS)
class OTADownloader
{
public:
    // API declarations start

    // Application calls this method to direct OTADownloader to begin the download
    void virtual BeginDownload(){};

    // Platform calls this method upon the completion of PrepareDownload() processing
    void virtual OnPreparedForDownload(CHIP_ERROR status){};

    // Action parameter type for the OnBlockProcessed()
    enum BlockActionType
    {
        kGetNext,
        kEnd
    };

    // Platform calls this method upon the completion of ProcessBlock() processing
    void virtual OnBlockProcessed(CHIP_ERROR status, BlockActionType action){};

    // A setter for the delegate class pointer
    void SetImageProcessorDelegate(OTAImageProcessorInterface * delegate) { mImageProcessorDelegate = delegate; }

    // API declarations end

    // Destructor
    virtual ~OTADownloader() = default;

private:
    OTAImageProcessorInterface * mImageProcessorDelegate;
};

// Set the object implementing OTADownloader
void SetDownloaderInstance(OTADownloader * instance);

// Get the object implementing OTADownloaderInterface
OTADownloader * GetDownloaderInstance();

} // namespace chip
