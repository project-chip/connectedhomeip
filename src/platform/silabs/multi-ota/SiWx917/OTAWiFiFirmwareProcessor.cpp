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
    sl_status_t status = SL_STATUS_OK;

    ChipLogProgress(SoftwareUpdate, "Process WiFi Block");
    // The first block is expected to be the OTA descriptor.
    // `ProcessDescriptor` handles it and sets `mDescriptorProcessed` to avoid reprocessing.
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
        // Validate block size
        VerifyOrReturnError(block.size() >= kAlignmentBytes, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(SoftwareUpdate, "Block too small for RPS header"));

        // Use spans to reference header and content directly
        ByteSpan rpsHeaderSpan(block.data(), kAlignmentBytes);
        ByteSpan rpsContentSpan(block.data() + kAlignmentBytes, block.size() - kAlignmentBytes);

        // Send RPS header
        status       = sl_si91x_fwup_start(rpsHeaderSpan.data());
        status       = sl_si91x_fwup_load(rpsHeaderSpan.data(), rpsHeaderSpan.size());
        mFWchunktype = SL_FWUP_RPS_CONTENT;

        // Send the rest of the block as content, if any
        if (rpsContentSpan.size() > 0)
        {
            status = sl_si91x_fwup_load(rpsContentSpan.data(), rpsContentSpan.size());
        }
    }
    else if (mFWchunktype == SL_FWUP_RPS_CONTENT)
    {
        // Send RPS content
        status = sl_si91x_fwup_load(block.data(), block.size());
        // When TA received all the blocks it will return SL_STATUS_SI91X_FW_UPDATE_DONE status
        VerifyOrReturnError(status == SL_STATUS_OK || status == SL_STATUS_SI91X_FW_UPDATE_DONE, CHIP_ERROR_INTERNAL,
                            ChipLogError(SoftwareUpdate, "sl_si91x_fwup_load() failed  0x%lx", static_cast<uint32_t>(status)));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::ProcessDescriptor(ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(static_cast<void *>(mAccumulator.GetData())));

    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::FinalizeAction()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAWiFiFirmwareProcessor::ApplyAction()
{
    mReset = true;
    return CHIP_NO_ERROR;
}

} // namespace chip
