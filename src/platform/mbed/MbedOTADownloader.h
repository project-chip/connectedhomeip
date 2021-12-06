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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTAImageProcessor.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

namespace chip {

// A class that abstracts the image download functionality from the particular
// protocol used for that (BDX or possibly HTTPS)
class MbedOTADownloader : public bdx::Initiator, public OTADownloader
{
public:
    struct ImageInfo
    {
        MutableCharSpan imageName;
        size_t imageSize;
    };

private:
    typedef void (*DownloadCompletedCallback)(ImageInfo *);

public:
    MbedOTADownloader(DownloadCompletedCallback downloadCompletedCallback = nullptr)
    {
        mDownloadCompletedCallback = downloadCompletedCallback;
    }

    // Application calls this method to direct OTADownloader to begin the download
    CHIP_ERROR BeginPrepareDownload();

    // Platform calls this method upon the completion of PrepareDownload() processing
    CHIP_ERROR OnPreparedForDownload(CHIP_ERROR status);

    // Should be called when it has been determined that the download has timed out.
    void OnDownloadTimeout() {}

    // Not all download protocols will be able to close gracefully from the receiver side.
    // The reason parameter should be used to indicate if this is a graceful end or a forceful abort.
    void EndDownload(CHIP_ERROR reason = CHIP_NO_ERROR) {}

    void SetDownloadImageInfo(MutableCharSpan & imageName);

private:
    // inherited from bdx::Endpoint
    void HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event);

    bool mIsTransferComplete = false;

    ImageInfo mImageInfo;

    static const uint16_t mBlockSize = 1024;

    DownloadCompletedCallback mDownloadCompletedCallback;
};

inline void MbedOTADownloader::SetDownloadImageInfo(MutableCharSpan & imageName)
{
    mImageInfo.imageName = imageName;
}

} // namespace chip

void SetDownloaderInstance(chip::OTADownloader * instance);

chip::OTADownloader * GetDownloaderInstance();
