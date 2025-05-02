/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

uint8_t flag = RPS_HEADER;

namespace chip {

// Define static memebers
// bool OTAWiFiFirmwareProcessor::mReset = false;

CHIP_ERROR OTAWiFiFirmwareProcessor::Init()
{
    VerifyOrReturnError(mCallbackProcessDescriptor != nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));
#if SL_MATTER_ENABLE_OTA_ENCRYPTION
    mUnalignmentNum = 0;
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;
#if SL_MATTER_ENABLE_OTA_ENCRYPTION
    mUnalignmentNum = 0;
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::ProcessInternal(ByteSpan & block)
{
    int32_t status = SL_STATUS_OK;
    // Store the header of the OTA file
    static uint8_t writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };
    // Used to tranfer other block to processor
    static uint8_t writeDataBuffer[1024] __attribute__((aligned(4))) = { 0 };

    ChipLogProgress(SoftwareUpdate, "ProcessInternal WiFi Block processing");
    if (!mDescriptorProcessed)
    {
        ReturnErrorOnFailure(ProcessDescriptor(block));
#if SL_MATTER_ENABLE_OTA_ENCRYPTION
        /* 16 bytes to used to store undecrypted data because of unalignment */
        mAccumulator.Init(requestedOtaMaxBlockSize + 16);
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION
    }

#if SL_MATTER_ENABLE_OTA_ENCRYPTION
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
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION

    if (flag == RPS_HEADER)
    {
        memcpy(&writeBuffer, block.data(), kAlignmentBytes);
        // Send RPS header which is received as first chunk
        status = sl_si91x_fwup_start(writeBuffer);
        status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
        flag   = RPS_DATA;
        memcpy(&writeDataBuffer, block.data() + kAlignmentBytes, (block.size() - kAlignmentBytes));
        status = sl_si91x_fwup_load(writeDataBuffer, (block.size() - kAlignmentBytes));
    }
    else if (flag == RPS_DATA)
    {
        memcpy(&writeDataBuffer, block.data(), block.size());
        // Send RPS content
        status = sl_si91x_fwup_load(writeDataBuffer, block.size());
        if (status != SL_STATUS_OK)
        {
            // When TA recived all the blocks it will return SL_STATUS_SI91X_FW_UPDATE_DONE status
            if (status == SL_STATUS_SI91X_FW_UPDATE_DONE)
            {
                mReset = true;
            }
            else
            {
                ChipLogError(SoftwareUpdate, "ERROR: In HandleProcessBlock sl_si91x_fwup_load() error %ld", status);
                return CHIP_ERROR_CANCELLED;
            }
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
    ChipLogProgress(SoftwareUpdate, "OTAWiFiFirmwareProcessor::ApplyAction called");
    mReset = true;
    ChipLogProgress(SoftwareUpdate, "mReset set to: %d", mReset);
#if 0                          // Bhavani: commenting because reset should move to handleApply after all TLVs are processed
    // This reboots the device
    if (mReset)
    {
        ChipLogProgress(SoftwareUpdate, "WiFi Device OTA update complete");
#ifdef SLI_SI91X_MCU_INTERFACE // This is not needed for the 917 SoC; it is required for EFR host applications
        // send system reset request to reset the MCU and upgrade the m4 image
        ChipLogProgress(SoftwareUpdate, "SoC Soft Reset initiated!");
        // Reboots the device
        sl_si91x_soc_nvic_reset();
#endif
    }
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::FinalizeAction()
{
    // TODO: Not requied by 917 OTA updated keeping this function to execute any other command before soft reset of the TA
    return CHIP_NO_ERROR;
}

} // namespace chip
