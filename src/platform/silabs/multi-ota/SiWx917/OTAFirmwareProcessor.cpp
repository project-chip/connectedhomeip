/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>
#include <platform/silabs/multi-ota/efr32/OTAFirmwareProcessor.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include "wfx_host_events.h"
#include <platform/silabs/SilabsConfig.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "sl_si91x_driver.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif
#ifdef __cplusplus
}
#endif

#define RPS_HEADER 1
#define RPS_DATA 2

#define SL_STATUS_FW_UPDATE_DONE SL_STATUS_SI91X_NO_AP_FOUND
uint8_t flag = RPS_HEADER;

// TODO: more descriptive error codes
#define SL_OTA_ERROR 1L

namespace chip {

// Define static memebers
uint8_t OTAFirmwareProcessor::mReset                                                  = false;
uint32_t OTAFirmwareProcessor::mWriteOffset                                            = 0;
uint16_t OTAFirmwareProcessor::writeBufOffset                                          = 0;
uint8_t OTAFirmwareProcessor::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };

CHIP_ERROR OTAFirmwareProcessor::Init()
{
    ReturnErrorCodeIf(mCallbackProcessDescriptor == nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ProcessInternal(ByteSpan & block)
{
    int32_t status        = SL_STATUS_OK;
    if (!mDescriptorProcessed)
    {
        ReturnErrorOnFailure(ProcessDescriptor(block));
    }

    uint32_t blockReadOffset = 0;
    while (blockReadOffset < block.size())
    {
        writeBuffer[writeBufOffset] = *((block.data()) + blockReadOffset);
        writeBufOffset++;
        blockReadOffset++;
        if (writeBufOffset == kAlignmentBytes)
        {
            writeBufOffset = 0;
            
            if (flag == RPS_HEADER)
            {
                // Send RPS header which is received as first chunk
                status = sl_si91x_fwup_start(writeBuffer);
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
                flag   = RPS_DATA;
            }
            else if (flag == RPS_DATA)
            {
                // Send RPS content
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
                if (status != SL_STATUS_OK)
                {
                    // If the last chunk of last block-writeBufOffset length is exactly kAlignmentBytes(64) bytes then mReset value
                    // should be set to true in HandleProcessBlock
                    if (status == SL_STATUS_FW_UPDATE_DONE)
                    {
                        mReset = true;
                    }
                    else
                    {
                        ChipLogError(SoftwareUpdate, "ERROR: In HandleProcessBlock sl_si91x_fwup_load() error %ld", status);
                        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                        return;
                    }
                }
            }
            if (err)
            {
                ChipLogError(SoftwareUpdate, "bootloader_eraseWriteStorage() error: %ld", err);
                // TODO: add this somewhere
                // imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                // TODO: Replace CHIP_ERROR_CANCELLED with new error statement
                return CHIP_ERROR_CANCELLED;
            }
            mWriteOffset += kAlignmentBytes;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ProcessDescriptor(ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(static_cast<void *>(mAccumulator.data())));

    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ApplyAction()
{
    uint32_t err = SL_BOOTLOADER_OK;
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "bootloader_verifyImage() error: %ld", err);
        // Call the OTARequestor API to reset the state
        GetRequestorInstance()->CancelImageUpdate();

        return SL_GENERIC_OTA_ERROR;
    }

    CORE_CRITICAL_SECTION(err = bootloader_setImageToBootload(mSlotId);)
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "bootloader_setImageToBootload() error: %ld", err);
        // Call the OTARequestor API to reset the state
        GetRequestorInstance()->CancelImageUpdate();
        return SL_GENERIC_OTA_ERROR;
    }
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::FinalizeAction()
{
    int32_t status        = SL_STATUS_OK;

    // Pad the remainder of the write buffer with zeros and write it to bootloader storage
    if (writeBufOffset != 0)
    {

        while (writeBufOffset != kAlignmentBytes)
        {
            writeBuffer[writeBufOffset] = 0;
            writeBufOffset++;
        }
        status = sl_si91x_fwup_load(writeBuffer, writeBufOffset);
        ChipLogProgress(SoftwareUpdate, "status: 0x%lX", status);

        if (status != SL_STATUS_OK)
        {
            if (status == SL_STATUS_FW_UPDATE_DONE)
            {
                mReset = true;
            }
            else
            {
                ChipLogError(SoftwareUpdate, "ERROR: In HandleFinalize for last chunk sl_si91x_fwup_load() error %ld", status);
                imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                return;
            }
        }

    }

    return err ? CHIP_ERROR_WRITE_FAILED : CHIP_NO_ERROR;
}

} // namespace chip
