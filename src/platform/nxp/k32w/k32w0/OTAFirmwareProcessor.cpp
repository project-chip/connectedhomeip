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
#include <platform/nxp/k32w/common/OTAImageProcessorImpl.h>
#include <platform/nxp/k32w/k32w0/OTAFirmwareProcessor.h>

#include "OtaSupport.h"
#include "OtaUtils.h"

namespace chip {

CHIP_ERROR OTAFirmwareProcessor::Init()
{
    ReturnErrorCodeIf(mCallbackProcessDescriptor == nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));
    ReturnErrorCodeIf(gOtaSuccess_c != OTA_ClientInit(), CHIP_OTA_PROCESSOR_CLIENT_INIT);

    auto offset = OTA_GetCurrentEepromAddressOffset();
    if (offset != 0)
    {
        offset += 1;
    }

    ReturnErrorCodeIf(OTA_UTILS_IMAGE_INVALID_ADDR == OTA_SetStartEepromOffset(offset), CHIP_OTA_PROCESSOR_EEPROM_OFFSET);
    ReturnErrorCodeIf(gOtaSuccess_c != OTA_StartImage(mLength - sizeof(Descriptor)), CHIP_OTA_PROCESSOR_START_IMAGE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFirmwareProcessor::ProcessInternal(ByteSpan & block)
{
    if (!mDescriptorProcessed)
    {
        ReturnErrorOnFailure(ProcessDescriptor(block));
    }

    auto status = OTA_MakeHeadRoomForNextBlock(requestedOtaMaxBlockSize, OTAImageProcessorImpl::FetchNextData, 0);
    if (gOtaSuccess_c != status)
    {
        ChipLogError(SoftwareUpdate, "Failed to make room for next block. Status: %d", status);
        return CHIP_OTA_PROCESSOR_MAKE_ROOM;
    }

    status = OTA_PushImageChunk((uint8_t *) block.data(), (uint16_t) block.size(), NULL, NULL);
    if (gOtaSuccess_c != status)
    {
        ChipLogError(SoftwareUpdate, "Failed to write image block. Status: %d", status);
        return CHIP_OTA_PROCESSOR_PUSH_CHUNK;
    }

    return CHIP_OTA_FETCH_ALREADY_SCHEDULED;
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
        return CHIP_OTA_PROCESSOR_IMG_COMMIT;
    }

    if (OTA_ImageAuthenticate() != gOtaImageAuthPass_c)
    {
        ChipLogError(SoftwareUpdate, "Failed to authenticate firmware image.");
        return CHIP_OTA_PROCESSOR_IMG_AUTH;
    }

    OTA_AddNewImageFlag();

    return CHIP_NO_ERROR;
}

} // namespace chip
