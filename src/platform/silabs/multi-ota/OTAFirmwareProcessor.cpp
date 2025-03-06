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
#include <platform/silabs/multi-ota/OTAFirmwareProcessor.h>
#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#if SL_WIFI
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#endif // SL_WIFI

extern "C" {
#include "btl_interface.h"
#include "sl_core.h"
}

/// No error, operation OK
#define SL_BOOTLOADER_OK 0L

namespace chip {

// Define static memebers
uint8_t OTAFirmwareProcessor::mSlotId                                                  = 0;
uint32_t OTAFirmwareProcessor::mWriteOffset                                            = 0;
uint16_t OTAFirmwareProcessor::writeBufOffset                                          = 0;
uint8_t OTAFirmwareProcessor::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };

CHIP_ERROR OTAFirmwareProcessor::Init()
{
    VerifyOrReturnError(mCallbackProcessDescriptor != nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
    mUnalignmentNum = 0;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ProcessInternal(ByteSpan & block)
{
    uint32_t err = SL_BOOTLOADER_OK;
    if (!mDescriptorProcessed)
    {
        ReturnErrorOnFailure(ProcessDescriptor(block));
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
        /* 16 bytes to used to store undecrypted data because of unalignment */
        mAccumulator.Init(requestedOtaMaxBlockSize + 16);
#endif
    }
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
    MutableByteSpan mBlock = MutableByteSpan(mAccumulator.data(), mAccumulator.GetThreshold());
    memcpy(&mBlock[0], &mBlock[requestedOtaMaxBlockSize], mUnalignmentNum);
    memcpy(&mBlock[mUnalignmentNum], block.data(), block.size());

    if (mUnalignmentNum + block.size() < requestedOtaMaxBlockSize)
    {
        uint32_t mAlignmentNum = (mUnalignmentNum + block.size()) / 16;
        mAlignmentNum          = mAlignmentNum * 16;
        mUnalignmentNum        = (mUnalignmentNum + block.size()) % 16;
        memcpy(&mBlock[requestedOtaMaxBlockSize], &mBlock[mAlignmentNum], mUnalignmentNum);
        mBlock.reduce_size(mAlignmentNum);
    }
    else
    {
        mUnalignmentNum = mUnalignmentNum + block.size() - requestedOtaMaxBlockSize;
        mBlock.reduce_size(requestedOtaMaxBlockSize);
    }

    OTATlvProcessor::vOtaProcessInternalEncryption(mBlock);
    block = mBlock;
#endif

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
                return CHIP_ERROR_CANCELLED;
            }
#endif // SL_BTLCTRL_MUX
            CORE_CRITICAL_SECTION(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes);)
#if SL_BTLCTRL_MUX
            err = sl_wfx_host_post_bootloader_spi_transfer();
            if (err != SL_STATUS_OK)
            {
                ChipLogError(SoftwareUpdate, "sl_wfx_host_post_bootloader_spi_transfer() error: %ld", err);
                return CHIP_ERROR_CANCELLED;
            }
#endif // SL_BTLCTRL_MUX
            if (err)
            {
                ChipLogError(SoftwareUpdate, "bootloader_eraseWriteStorage() error: %ld", err);
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

CHIP_ERROR OTAFirmwareProcessor::FinalizeAction()
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

} // namespace chip
