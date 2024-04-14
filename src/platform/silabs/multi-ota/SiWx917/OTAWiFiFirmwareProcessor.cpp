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
#include <platform/silabs/multi-ota/SiWx917/OTAWiFiFirmwareProcessor.h>

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
bool OTAWiFiFirmwareProcessor::mReset                                                      = false;
uint32_t OTAWiFiFirmwareProcessor::mWriteOffset                                            = 0;
uint16_t OTAWiFiFirmwareProcessor::writeBufOffset                                          = 0;
uint8_t OTAWiFiFirmwareProcessor::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };

CHIP_ERROR OTAWiFiFirmwareProcessor::Init()
{
    ReturnErrorCodeIf(mCallbackProcessDescriptor == nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::ProcessInternal(ByteSpan & block)
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
                        // TODO: add this somewhere
                        // imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                        // TODO: Replace CHIP_ERROR_CANCELLED with new error statement
                        return CHIP_ERROR_CANCELLED;
                    }
                }
            }
            mWriteOffset += kAlignmentBytes;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::ProcessDescriptor(ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(static_cast<void *>(mAccumulator.data())));

    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::ApplyAction()
{
    ChipLogProgress(SoftwareUpdate, "OTA WiFi Firmware Apply Action started");
    // This reboots the device
    if (mReset)
    {
        ChipLogProgress(SoftwareUpdate, "M4 Firmware update complete");
        // send system reset request to reset the MCU and upgrade the m4 image
        ChipLogProgress(SoftwareUpdate, "SoC Soft Reset initiated!");
        // Reboots the device
        sl_si91x_soc_soft_reset();
    }
    ChipLogProgress(SoftwareUpdate, "OTA WiFi Firmware Apply Action completed");
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::FinalizeAction()
{
    int32_t status        = SL_STATUS_OK;
    ChipLogProgress(SoftwareUpdate, "OTA WiFi Firmware Finalize Action started");
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

                // TODO: add this somewhere
                // imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
                // TODO: Replace CHIP_ERROR_CANCELLED with new error statement
                return CHIP_ERROR_CANCELLED;
            }
        }

    }

    ChipLogProgress(SoftwareUpdate, "OTA WiFi Firmware Finalize Action completed");
    return status ? CHIP_ERROR_CANCELLED : CHIP_NO_ERROR;
}

} // namespace chip
