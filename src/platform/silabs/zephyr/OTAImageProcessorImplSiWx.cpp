/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <platform/silabs/zephyr/OTAImageProcessorImplSiWx.h>

#include <cinttypes>

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

extern "C" {
#include "firmware_upgradation.h"
}

static chip::OTAImageProcessorImpl gImageProcessor;

namespace chip {

using namespace ::chip::DeviceLayer;

uint32_t OTAImageProcessorImpl::mWriteOffset                                            = 0;
uint16_t OTAImageProcessorImpl::writeBufOffset                                          = 0;
bool OTAImageProcessorImpl::mReset                                                      = false;
uint8_t OTAImageProcessorImpl::mFwChunkType                                             = kRpsHeader;
uint8_t OTAImageProcessorImpl::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };

CHIP_ERROR OTAImageProcessorImpl::Init(OTADownloader * downloader)
{
    VerifyOrReturnError(downloader != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    gImageProcessor.SetOTADownloader(downloader);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    return PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    return PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    return PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    return PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = SetBlock(block);

    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format()));

    return PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    VerifyOrReturnError(requestor != nullptr, CHIP_ERROR_INTERNAL, ChipLogError(SoftwareUpdate, "OTARequestorInterface is null"));

    uint32_t currentVersion;
    uint32_t targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(ConfigurationMgr().GetSoftwareVersion(currentVersion));
    VerifyOrReturnError(currentVersion == targetVersion, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(SoftwareUpdate, "Current software version = %" PRIu32 ", expected software version = %" PRIu32,
                                     currentVersion, targetVersion));
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));
    VerifyOrReturn(imageProcessor->mDownloader != nullptr, ChipLogError(SoftwareUpdate, "mDownloader is null"));

    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload");

    mReset                                  = false;
    mFwChunkType                            = kRpsHeader;
    writeBufOffset                          = 0;
    mWriteOffset                            = 0;
    imageProcessor->mParams.downloadedBytes = 0;

    imageProcessor->mHeaderParser.Init();

    CHIP_ERROR error = imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
    VerifyOrReturn(error == CHIP_NO_ERROR,
                   ChipLogError(SoftwareUpdate, "OnPreparedForDownload() error: %" CHIP_ERROR_FORMAT, error.Format()));
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    sl_status_t status    = SL_STATUS_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);

    VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));

    if (writeBufOffset != 0)
    {
        imageProcessor->mParams.downloadedBytes += writeBufOffset;
        status = sl_si91x_fwup_load(writeBuffer, writeBufOffset);
        ChipLogProgress(SoftwareUpdate, "HandleFinalize fwup_load status: 0x%lX", static_cast<unsigned long>(status));

        if (status != SL_STATUS_OK)
        {
            if (status == SL_STATUS_SI91X_FW_UPDATE_DONE)
            {
                mReset = true;
            }
            else
            {
                ChipLogError(SoftwareUpdate, "HandleFinalize sl_si91x_fwup_load() error 0x%lx", static_cast<unsigned long>(status));
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
        }
    }

    if (!mReset)
    {
        ChipLogError(SoftwareUpdate, "Firmware update did not reach completion");
        err = imageProcessor->ReleaseBlock();
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Release block failed: %" CHIP_ERROR_FORMAT, err.Format()));
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    err = imageProcessor->ReleaseBlock();
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(SoftwareUpdate, "Release block failed: %" CHIP_ERROR_FORMAT, err.Format()));
    ChipLogProgress(SoftwareUpdate, "OTA image downloaded successfully");
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    (void) context;
    ChipLogProgress(SoftwareUpdate, "OTAImageProcessorImpl::HandleApply()");

    VerifyOrReturn(mReset, ChipLogError(SoftwareUpdate, "Apply called but firmware update was not marked complete"));

    ChipLogProgress(SoftwareUpdate, "M4 firmware update complete; rebooting for Security Bootloader install");

    // Allow state-transition events to flush before reboot.
    PlatformMgr().HandleServerShuttingDown();
    k_msleep(CHIP_DEVICE_CONFIG_SERVER_SHUTDOWN_ACTIONS_SLEEP_MS);
    sys_reboot(SYS_REBOOT_COLD);
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));

    sl_status_t status = sl_si91x_fwup_abort();
    VerifyOrReturn(status == SL_STATUS_OK,
                   ChipLogError(SoftwareUpdate, "sl_si91x_fwup_abort() error 0x%lx", static_cast<unsigned long>(status)));

    mFwChunkType   = kRpsHeader;
    writeBufOffset = 0;
    mReset         = false;

    CHIP_ERROR err = imageProcessor->ReleaseBlock();
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(SoftwareUpdate, "Release block failed: %" CHIP_ERROR_FORMAT, err.Format()));
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    sl_status_t status    = SL_STATUS_OK;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "ImageProcessor context is null"));
    VerifyOrReturn(imageProcessor->mDownloader != nullptr, ChipLogError(SoftwareUpdate, "mDownloader is null"));

    ByteSpan block        = imageProcessor->mBlock;
    CHIP_ERROR chip_error = imageProcessor->ProcessHeader(block);

    if (chip_error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Matter image header parser error: %" CHIP_ERROR_FORMAT, chip_error.Format());
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return;
    }

    // Fill word-aligned writeBuffer; flush when full. Remainder is written in HandleFinalize().
    uint32_t blockReadOffset = 0;
    while (blockReadOffset < block.size())
    {
        writeBuffer[writeBufOffset] = *(block.data() + blockReadOffset);
        writeBufOffset++;
        blockReadOffset++;
        if (writeBufOffset == kAlignmentBytes)
        {
            writeBufOffset = 0;
            if (mFwChunkType == kRpsHeader)
            {
                status = sl_si91x_fwup_start(writeBuffer);
                if (status != SL_STATUS_OK)
                {
                    ChipLogError(SoftwareUpdate, "sl_si91x_fwup_start() error 0x%lx", static_cast<unsigned long>(status));
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                    return;
                }
                status       = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
                mFwChunkType = kRpsData;
            }
            else
            {
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
            }

            if (status != SL_STATUS_OK)
            {
                if (status == SL_STATUS_SI91X_FW_UPDATE_DONE)
                {
                    mReset = true;
                }
                else
                {
                    ChipLogError(SoftwareUpdate, "sl_si91x_fwup_load() error 0x%lx", static_cast<unsigned long>(status));
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                    return;
                }
            }

            imageProcessor->mParams.downloadedBytes += kAlignmentBytes;
        }
    }

    CHIP_ERROR error = imageProcessor->mDownloader->FetchNextData();
    VerifyOrReturn(error == CHIP_NO_ERROR,
                   ChipLogError(SoftwareUpdate, "FetchNextData() error: %" CHIP_ERROR_FORMAT, error.Format()));
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        ChipLogProgress(SoftwareUpdate, "Image Header software version: %" PRIu32 " payload size: %" PRIu64,
                        header.mSoftwareVersion, header.mPayloadSize);
        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_NO_ERROR;
    }

    if (mBlock.size() < block.size())
    {
        CHIP_ERROR error = ReleaseBlock();
        VerifyOrReturnError(error == CHIP_NO_ERROR, error,
                            ChipLogError(SoftwareUpdate, "Release block failed: %" CHIP_ERROR_FORMAT, error.Format()));

        mBlock = MutableByteSpan(static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size())), block.size());
        VerifyOrReturnError(mBlock.data() != nullptr, CHIP_ERROR_NO_MEMORY);
    }

    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

OTAImageProcessorImpl & OTAImageProcessorImpl::GetDefaultInstance()
{
    return gImageProcessor;
}

} // namespace chip
