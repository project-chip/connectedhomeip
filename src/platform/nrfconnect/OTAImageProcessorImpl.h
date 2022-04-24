/*
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/Span.h>
#include <platform/OTAImageProcessor.h>
#include <platform/nrfconnect/OTAImageContentHeader.h>

namespace chip {

class OTADownloader;

namespace DeviceLayer {

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    static constexpr size_t kBufferSize = CONFIG_CHIP_OTA_REQUESTOR_BUFFER_SIZE;

    enum class ImageType : uint8_t
    {
        kAppImage = 0,
        kNetImage = 1
    };

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; };

    struct OTAImage
    {
        OTAImageContentHeader::FileInfo * mFileInfo;
        ImageType mImageType;
        uint64_t mCurrentOffset;
    };

    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR ProcessBlock(ByteSpan & aBlock) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

private:
    CHIP_ERROR PrepareDownloadImpl();
    CHIP_ERROR ProcessHeader(ByteSpan & aBlock);
    CHIP_ERROR SwitchToNextImage(const ByteSpan & aRemainingData);

    OTADownloader * mDownloader = nullptr;
    OTAImageHeaderParser mHeaderParser;
    OTAImageContentHeaderParser mContentHeaderParser;
    uint8_t mBuffer[kBufferSize];
    OTAImageContentHeader mContentHeader;
    OTAImage mCurrentImage;
};

class ExtFlashHandler
{
public:
    enum class Action : uint8_t
    {
        WAKE_UP,
        SLEEP
    };
    virtual ~ExtFlashHandler() {}
    virtual void DoAction(Action aAction);
};

class OTAImageProcessorImplPMDevice : public OTAImageProcessorImpl
{
public:
    explicit OTAImageProcessorImplPMDevice(ExtFlashHandler & aHandler);
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR Apply() override;

private:
    ExtFlashHandler & mHandler;
};

} // namespace DeviceLayer
} // namespace chip
