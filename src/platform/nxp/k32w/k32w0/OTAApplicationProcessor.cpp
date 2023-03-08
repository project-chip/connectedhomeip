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
#include <platform/nxp/k32w/k32w0/CHIPDevicePlatformConfig.h>
#include <platform/nxp/k32w/k32w0/OTAApplicationProcessor.h>

#include "OtaSupport.h"
#include "OtaUtils.h"

namespace chip {

CHIP_ERROR OTAApplicationProcessor::Init()
{
    mAccumulator.Init(sizeof(AppDescriptor));

    ReturnErrorCodeIf(gOtaSuccess_c != OTA_ClientInit(), CHIP_OTA_PROCESSOR_CLIENT_INIT);
    ReturnErrorCodeIf(gOtaSuccess_c != OTA_StartImage(mLength - sizeof(AppDescriptor)), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAApplicationProcessor::Clear()
{
    mAccumulator.Clear();
    mLength              = 0;
    mProcessedLength     = 0;
    mWasSelected         = false;
    mDescriptorProcessed = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAApplicationProcessor::ProcessInternal(ByteSpan & block)
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

CHIP_ERROR OTAApplicationProcessor::ProcessDescriptor(ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));

    // TODO: use accumulator data in some way. What should be done with AppDescriptor data?

    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAApplicationProcessor::ApplyAction()
{
    if (OTA_CommitImage(NULL) != gOtaSuccess_c)
    {
        ChipLogError(SoftwareUpdate, "Failed to commit application image.");
        return CHIP_OTA_PROCESSOR_IMG_COMMIT;
    }

    if (OTA_ImageAuthenticate() != gOtaImageAuthPass_c)
    {
        ChipLogError(SoftwareUpdate, "Failed to authenticate application image.");
        return CHIP_OTA_PROCESSOR_IMG_AUTH;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAApplicationProcessor::AbortAction()
{
    OTA_CancelImage();
    Clear();

    return CHIP_NO_ERROR;
}

} // namespace chip
