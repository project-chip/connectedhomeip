/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
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

class FlashHandler
{
public:
    enum class Action : uint8_t
    {
        WAKE_UP,
        SLEEP
    };
    virtual ~FlashHandler() {}
    virtual void DoAction(Action aAction);
};

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    static constexpr size_t kBufferSize = CONFIG_CHIP_OTA_REQUESTOR_BUFFER_SIZE;

    explicit OTAImageProcessorImpl(FlashHandler * flashHandler = nullptr) : mFlashHandler(flashHandler) {}

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; };
    void TriggerFlashAction(FlashHandler::Action action);

    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR ProcessBlock(ByteSpan & aBlock) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

protected:
    CHIP_ERROR PrepareDownloadImpl();
    CHIP_ERROR ProcessHeader(ByteSpan & aBlock);

    OTADownloader * mDownloader = nullptr;
    OTAImageHeaderParser mHeaderParser;
    uint8_t mBuffer[kBufferSize];
    FlashHandler * mFlashHandler;
};

} // namespace DeviceLayer
} // namespace chip
