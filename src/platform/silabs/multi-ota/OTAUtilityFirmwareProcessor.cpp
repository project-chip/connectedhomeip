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
#include <platform/silabs/SilabsConfig.h>
// TODO: more descriptive error codes

extern "C" {
#if SL_BTLCTRL_MUX
#include "btl_interface.h"
#endif // SL_BTLCTRL_MUX
#include "em_bus.h" // For CORE_CRITICAL_SECTION
#if SL_WIFI
#include "wfx_host_events.h"
#include "sl_si91x_driver.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif
#endif // SL_WIFI
}

#if SL_WIFI
#define RPS_HEADER 1
#define RPS_DATA 2

#define SL_STATUS_FW_UPDATE_DONE SL_STATUS_SI91X_NO_AP_FOUND
uint8_t flag = RPS_HEADER;
#endif // SL_WIFI

/// No error, operation OK
#define SL_BOOTLOADER_OK 0L
#define SL_OTA_ERROR 1L

namespace chip {

OTAUtilityFirmwareProcessor *sOTAUtilityFirmwareProcessor = NULL;

// Define static memebers
uint8_t OTAUtilityFirmwareProcessor::mSlotId                                                  = 0;
//bool OTAFirmwareProcessor::mReset                                                      = false;
uint32_t OTAUtilityFirmwareProcessor::mWriteOffset                                            = 0;
uint16_t OTAUtilityFirmwareProcessor::writeBufOffset                                          = 0;
uint8_t OTAUtilityFirmwareProcessor::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };


CHIP_ERROR OTAUtilityFirmwareProcessor::Init()
{
    mAccumulator.Init(sizeof(Descriptor));
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}


OTAUtilityFirmwareProcessor* OTAUtilityFirmwareProcessor::getInstance() {

    if (sOTAUtilityFirmwareProcessor == NULL) 
    {
      sOTAUtilityFirmwareProcessor = new OTAUtilityFirmwareProcessor(); 
    }
    return sOTAUtilityFirmwareProcessor;
}

CHIP_ERROR OTAUtilityFirmwareProcessor::Clear()
{
    mAccumulator.Clear();
    mDescriptorProcessed = false;
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAUtilityFirmwareProcessor::ProcessDescriptor(ByteSpan & block)
{
    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

#ifndef SL_WIFI
CHIP_ERROR OTAUtilityFirmwareProcessor::ProcessInternalEFR32(ByteSpan & block)
{
    uint32_t err = SL_BOOTLOADER_OK;
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
#if SL_BTLCTRL_MUX
            err = sl_wfx_host_pre_bootloader_spi_transfer();
            if (err != SL_STATUS_OK)
            {
                ChipLogError(SoftwareUpdate, "sl_wfx_host_pre_bootloader_spi_transfer() error: %ld", err);
                return;
            }
#endif // SL_BTLCTRL_MUX
            CORE_CRITICAL_SECTION(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes);)
#if SL_BTLCTRL_MUX
            err = sl_wfx_host_post_bootloader_spi_transfer();
            if (err != SL_STATUS_OK)
            {
                ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
                return;
            }
#endif // SL_BTLCTRL_MUX
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


CHIP_ERROR OTAUtilityFirmwareProcessor::ApplyActionEFR32()
{
    uint32_t err = SL_BOOTLOADER_OK;

#if SL_BTLCTRL_MUX
    err = sl_wfx_host_pre_bootloader_spi_transfer();
    if (err != SL_STATUS_OK)
    {
        ChipLogError(SoftwareUpdate, "sl_wfx_host_pre_bootloader_spi_transfer() error: %ld", err);
        return SL_GENERIC_OTA_ERROR;
    }
#endif // SL_BTLCTRL_MUX
    CORE_CRITICAL_SECTION(err = bootloader_verifyImage(mSlotId, NULL);)
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "bootloader_verifyImage() error: %ld", err);
        // Call the OTARequestor API to reset the state
        GetRequestorInstance()->CancelImageUpdate();
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_post_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
            return SL_GENERIC_OTA_ERROR;
        }
#endif // SL_BTLCTRL_MUX
        return SL_GENERIC_OTA_ERROR;
    }

    CORE_CRITICAL_SECTION(err = bootloader_setImageToBootload(mSlotId);)
    if (err != SL_BOOTLOADER_OK)
    {
        ChipLogError(SoftwareUpdate, "bootloader_setImageToBootload() error: %ld", err);
        // Call the OTARequestor API to reset the state
        GetRequestorInstance()->CancelImageUpdate();
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_post_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
            return SL_GENERIC_OTA_ERROR;
        }
#endif // SL_BTLCTRL_MUX
        return SL_GENERIC_OTA_ERROR;
    }

#if SL_BTLCTRL_MUX
    err = sl_wfx_host_post_bootloader_spi_transfer();
    if (err != SL_STATUS_OK)
    {
        ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
        return SL_GENERIC_OTA_ERROR;
    }
#endif // SL_BTLCTRL_MUX
    // This reboots the device
    // CORE_CRITICAL_SECTION(bootloader_rebootAndInstall();)

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAUtilityFirmwareProcessor::FinalizeActionEFR32()
{
    uint32_t err = SL_BOOTLOADER_OK;

    // Pad the remainder of the write buffer with zeros and write it to bootloader storage
    if (writeBufOffset != 0)
    {

        while (writeBufOffset != kAlignmentBytes)
        {
            writeBuffer[writeBufOffset] = 0;
            writeBufOffset++;
        }
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_pre_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_pre_bootloader_spi_transfer() error: %ld", err);
            return SL_GENERIC_OTA_ERROR;
        }
#endif // SL_BTLCTRL_MUX
        CORE_CRITICAL_SECTION(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes);)
#if SL_BTLCTRL_MUX
        err = sl_wfx_host_post_bootloader_spi_transfer();
        if (err != SL_STATUS_OK)
        {
            ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
            return SL_GENERIC_OTA_ERROR;
        }
#endif // SL_BTLCTRL_MUX
    }

    return err ? CHIP_ERROR_WRITE_FAILED : CHIP_NO_ERROR;
}
#endif


#ifdef SLI_SI91X_MCU_INTERFACE 
CHIP_ERROR OTAUtilityFirmwareProcessor::ProcessInternal917(ByteSpan & block)
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
                        //mReset = true;
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

CHIP_ERROR OTAUtilityFirmwareProcessor::ApplyAction917()
{
    // This reboots the device
    // if (mReset)
    // {
    //     ChipLogProgress(SoftwareUpdate, "M4 Firmware update complete");
    //     // send system reset request to reset the MCU and upgrade the m4 image
    //     ChipLogProgress(SoftwareUpdate, "SoC Soft Reset initiated!");
    //     // Reboots the device
    //     sl_si91x_soc_soft_reset();
    // }  
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAUtilityFirmwareProcessor::FinalizeAction917()
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
                // mReset = true;
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

    return status ? CHIP_ERROR_CANCELLED : CHIP_NO_ERROR;
}
#endif





} // namespace chip