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
#include <platform/nxp/common/ota/OTAImageProcessorImpl.h>
#include <platform/nxp/k32w0/CHIPDevicePlatformConfig.h>
#include <platform/nxp/k32w0/OTAFirmwareProcessor.h>

#include "OtaSupport.h"
#include "OtaUtils.h"

namespace chip {

CHIP_ERROR OTAFirmwareProcessor::Init()
{
    ReturnErrorCodeIf(mCallbackProcessDescriptor == nullptr, CHIP_ERROR_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));
#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif
    ReturnErrorCodeIf(gOtaSuccess_c != OTA_ClientInit(), CHIP_ERROR_OTA_PROCESSOR_CLIENT_INIT);

    auto offset = OTA_GetCurrentEepromAddressOffset();
    if (offset != 0)
    {
        offset += 1;
    }

    ReturnErrorCodeIf(OTA_UTILS_IMAGE_INVALID_ADDR == OTA_SetStartEepromOffset(offset), CHIP_ERROR_OTA_PROCESSOR_EEPROM_OFFSET);
    ReturnErrorCodeIf(gOtaSuccess_c != OTA_StartImage(mLength - sizeof(Descriptor)), CHIP_ERROR_OTA_PROCESSOR_START_IMAGE);

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
    if (!mDescriptorProcessed)
    {
        ReturnErrorOnFailure(ProcessDescriptor(block));
#if OTA_ENCRYPTION_ENABLE
        /* 16 bytes to used to store undecrypted data because of unalignment */
        mAccumulator.Init(requestedOtaMaxBlockSize + 16);
#endif
    }
#if OTA_ENCRYPTION_ENABLE
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
#endif

    auto status = OTA_MakeHeadRoomForNextBlock(requestedOtaMaxBlockSize, OTAImageProcessorImpl::FetchNextData, 0);
    if (gOtaSuccess_c != status)
    {
        ChipLogError(SoftwareUpdate, "Failed to make room for next block. Status: %d", status);
        return CHIP_ERROR_OTA_PROCESSOR_MAKE_ROOM;
    }
#if OTA_ENCRYPTION_ENABLE
    status = OTA_PushImageChunk((uint8_t *) mBlock.data(), (uint16_t) mBlock.size(), NULL, NULL);
#else
    status = OTA_PushImageChunk((uint8_t *) block.data(), (uint16_t) block.size(), NULL, NULL);
#endif
    if (gOtaSuccess_c != status)
    {
        ChipLogError(SoftwareUpdate, "Failed to write image block. Status: %d", status);
        return CHIP_ERROR_OTA_PROCESSOR_PUSH_CHUNK;
    }

    return CHIP_ERROR_OTA_FETCH_ALREADY_SCHEDULED;
}

CHIP_ERROR OTAFirmwareProcessor::ProcessDescriptor(ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(static_cast<void *>(mAccumulator.data())));

    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::AbortAction()
{
    OTA_CancelImage();
    OTA_ResetCustomEntries();
    OTA_ResetCurrentEepromAddress();
    OTA_SetStartEepromOffset(0);
    Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ExitAction()
{
    if (OTA_CommitImage(NULL) != gOtaSuccess_c)
    {
        ChipLogError(SoftwareUpdate, "Failed to commit firmware image.");
        mApplyState = ApplyState::kDoNotApply;
        return CHIP_ERROR_OTA_PROCESSOR_IMG_COMMIT;
    }

    if (OTA_ImageAuthenticate() != gOtaImageAuthPass_c)
    {
        ChipLogError(SoftwareUpdate, "Failed to authenticate firmware image.");
        mApplyState = ApplyState::kDoNotApply;
        return CHIP_ERROR_OTA_PROCESSOR_IMG_AUTH;
    }

    OTA_AddNewImageFlag();

    return CHIP_NO_ERROR;
}

} // namespace chip
