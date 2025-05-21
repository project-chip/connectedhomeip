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

#include "OTAMultiImageProcessorImpl.h"
#include <platform/silabs/multi-ota/SiWx917/OTAWiFiFirmwareProcessor.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

extern "C" {
#include "sl_si91x_driver.h"
}

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
    MutableByteSpan byteblock = MutableByteSpan(mAccumulator.GetData(), mAccumulator.GetThreshold());
    memcpy(&byteblock[0], &byteblock[requestedOtaMaxBlockSize], mUnalignmentNum);
    memcpy(&byteblock[mUnalignmentNum], block.data(), block.size());

    if (mUnalignmentNum + block.size() < requestedOtaMaxBlockSize)
    {
        uint32_t alignmentnum = (mUnalignmentNum + block.size()) / 16;
        alignmentnum          = alignmentnum * 16;
        mUnalignmentNum       = (mUnalignmentNum + block.size()) % 16;
        memcpy(&byteblock[requestedOtaMaxBlockSize], &byteblock[alignmentnum], mUnalignmentNum);
        byteblock.reduce_size(alignmentnum);
    }
    else
    {
        mUnalignmentNum = mUnalignmentNum + block.size() - requestedOtaMaxBlockSize;
        byteblock.reduce_size(requestedOtaMaxBlockSize);
    }

    OTATlvProcessor::vOtaProcessInternalEncryption(byteblock);
    block = byteblock;
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION

    if (mFWchunktype == SL_FWUP_RPS_HEADER)
    {
        // Validate block size
        VerifyOrReturnError(block.size() >= kAlignmentBytes, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(SoftwareUpdate, "Block too small for RPS header"));

        // Use spans to reference header and content directly
        ByteSpan rpsHeaderSpan  = block.subSpan(0, kAlignmentBytes);
        ByteSpan rpsContentSpan = block.subSpan(kAlignmentBytes);

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

CHIP_ERROR OTAWiFiFirmwareProcessor::ProcessDescriptor(const ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(reinterpret_cast<void *>(mAccumulator.GetData())));

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
