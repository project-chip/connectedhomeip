/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/OTAImageHeader.h>
#include <lib/support/Span.h>
#include <platform/OTAImageProcessor.h>

namespace chip {

class OTADownloader;

namespace DeviceLayer {

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    static constexpr size_t kBufferSize = CONFIG_CHIP_OTA_REQUESTOR_BUFFER_SIZE;

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

    OTADownloader * mDownloader = nullptr;
    OTAImageHeaderParser mHeaderParser;
    uint8_t mBuffer[kBufferSize];
};

} // namespace DeviceLayer
} // namespace chip
