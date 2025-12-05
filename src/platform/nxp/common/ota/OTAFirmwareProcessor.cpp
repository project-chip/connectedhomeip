/*
 *
 *    Copyright (c) 2023, 2025 Project CHIP Authors
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
#include <platform/nxp/common/ota/OTAFirmwareProcessor.h>
#include <platform/nxp/common/ota/OTAImageProcessorImpl.h>

#include "OtaSupport.h"

namespace chip {

CHIP_ERROR OTAFirmwareProcessor::Init()
{
    VerifyOrReturnError(mCallbackProcessDescriptor != nullptr, CHIP_ERROR_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));

#if OTA_ENCRYPTION_ENABLE
    mUnalignmentNum = 0;
#endif

    VerifyOrReturnError(gOtaSuccess_c == OTA_SelectExternalStoragePartition(), CHIP_ERROR_OTA_PROCESSOR_EXTERNAL_STORAGE);

#if CONFIG_CHIP_OTA_POSTED_OPERATIONS_IN_IDLE
    /* Resume flash write/erase transactions only in the idle task */
    ota_config_t OTAconfig;
    OTA_GetDefaultConfig(&OTAconfig);
    OTAconfig.PostedOpInIdleTask = true;
    OTA_SetConfig(&OTAconfig);
#endif // CHIP_OTA_POSTED_OPERATIONS_IN_IDLE

    otaResult_t ota_status;
    ota_status = OTA_ServiceInit(&mPostedOperationsStorage[0], NB_PENDING_TRANSACTIONS * TRANSACTION_SZ);

    VerifyOrReturnError(ota_status == gOtaSuccess_c, CHIP_ERROR_OTA_PROCESSOR_CLIENT_INIT);
    VerifyOrReturnError(gOtaSuccess_c == OTA_StartImage(mLength - sizeof(Descriptor)), CHIP_ERROR_OTA_PROCESSOR_START_IMAGE);

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
    otaResult_t status;

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

    status = OTA_MakeHeadRoomForNextBlock(block.size(), OTAImageProcessorImpl::FetchNextData, 0);
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

CHIP_ERROR OTAFirmwareProcessor::ApplyAction()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::AbortAction()
{
    OTA_CancelImage();
    OTA_ServiceDeInit();

    Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ExitAction()
{
    /*
     * The image will be commited in the context of HandleApply after all ApplyAction are completed.
     * So here, we return no error.
     */

    return CHIP_NO_ERROR;
}

} // namespace chip
