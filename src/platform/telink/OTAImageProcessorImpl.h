/*
 *    Copyright (c) 2022-2026 Project CHIP Authors
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
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
#include <lib/support/ScopedMemoryBuffer.h>
#endif

namespace chip {

class OTADownloader;

namespace DeviceLayer {

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
class ExternalOTAImageHandler
{
public:
    virtual ~ExternalOTAImageHandler() = default;

    // Stage the image without activating it. Write must consume the data before returning.
    virtual CHIP_ERROR Begin(uint32_t imageIndex, uint32_t imageSize) = 0;
    virtual CHIP_ERROR Write(ByteSpan data)                           = 0;
    // Validate the staged image.
    virtual CHIP_ERROR End()   = 0;
    virtual CHIP_ERROR Apply() = 0;
    virtual void Abort()       = 0;
};
#endif

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    static constexpr size_t kBufferSize = CONFIG_CHIP_OTA_REQUESTOR_BUFFER_SIZE;

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    // Register before starting OTA.
    static void SetExternalImageHandler(ExternalOTAImageHandler * handler);
#endif

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; };
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
    CHIP_ERROR InitFlashStream(size_t offset);
    CHIP_ERROR RestoreBytes(ByteSpan & aBlock);
    static void ResumeWatchdogHandler(System::Layer * systemLayer, void * context);

    OTADownloader * mDownloader = nullptr;
    OTAImageHeaderParser mHeaderParser;
    uint8_t mBuffer[kBufferSize];
    // Define non-volatile storage keys for DownloadedBytes and ImageDigest.
    static constexpr char kDownloadedBytes[]           = "DownloadedBytes";
    static constexpr char kImageDigest[]               = "ImageDigest";
    static constexpr uint16_t kResumeWatchdogTimeoutMs = 6000;
    uint64_t downloadedBytesRestored                   = 0;

#ifdef CONFIG_CHIP_OTA_MULTI_IMAGE
    enum class PayloadState : uint8_t
    {
        kHeader,
        kTable,
        kImages,
        kSingleImage,
    };

    static constexpr size_t kContainerHeaderSize = 3 * sizeof(uint32_t);
    static constexpr size_t kImageInfoSize       = 4 * sizeof(uint32_t);
    CHIP_ERROR ProcessPayload(ByteSpan block);
    CHIP_ERROR ProcessContainerHeader(ByteSpan & block);
    CHIP_ERROR ProcessImageData(ByteSpan & block);
    CHIP_ERROR ProcessImageTable();
    CHIP_ERROR FailDownload(CHIP_ERROR error);
    void ResetPayload();

    static ExternalOTAImageHandler * sExternalImageHandler;
    ExternalOTAImageHandler * mExternalImageHandler = nullptr;
    PayloadState mPayloadState                      = PayloadState::kHeader;
    uint8_t mContainerHeader[kContainerHeaderSize];
    Platform::ScopedMemoryBuffer<uint8_t> mImageTable;
    size_t mHeaderBytes         = 0;
    size_t mTableBytes          = 0;
    uint32_t mImageCount        = 0;
    uint32_t mImageIndex        = 0;
    uint32_t mImageBytes        = 0;
    uint32_t mImageCrc          = 0;
    bool mExternalImagesStarted = false;
    bool mDownloadReady         = false;
    bool mPayloadFailed         = false;
#endif
};

} // namespace DeviceLayer
} // namespace chip
