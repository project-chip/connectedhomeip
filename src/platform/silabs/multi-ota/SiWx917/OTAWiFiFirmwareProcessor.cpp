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
#ifdef __cplusplus
}
#endif

namespace chip {

CHIP_ERROR OTAWiFiFirmwareProcessor::ProcessInternal(ByteSpan & block)
{
    int32_t status = SL_STATUS_OK;
    // Store the header of the OTA file
    static uint8_t writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };
    // Used to tranfer other block to processor
    static uint8_t writeDataBuffer[kBlockSize] __attribute__((aligned(4))) = { 0 };

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

    if (mFWchunktype == SL_FWUP_RPS_HEADER)
    {
        memcpy(&writeBuffer, block.data(), kAlignmentBytes);
        // Send RPS header which is received as first chunk
        status       = sl_si91x_fwup_start(writeBuffer);
        status       = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
        mFWchunktype = SL_FWUP_RPS_CONTENT;
        memcpy(&writeDataBuffer, block.data() + kAlignmentBytes, (block.size() - kAlignmentBytes));
        status = sl_si91x_fwup_load(writeDataBuffer, (block.size() - kAlignmentBytes));
    }
    else if (mFWchunktype == SL_FWUP_RPS_CONTENT)
    {
        memcpy(&writeDataBuffer, block.data(), block.size());
        // Send RPS content
        status = sl_si91x_fwup_load(writeDataBuffer, block.size());
        if (status != SL_STATUS_OK)
        {
            // When TA received all the blocks it will return SL_STATUS_SI91X_FW_UPDATE_DONE status
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
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::FinalizeAction()
{
    return CHIP_NO_ERROR;
}

} // namespace chip
