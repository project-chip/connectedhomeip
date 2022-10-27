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
