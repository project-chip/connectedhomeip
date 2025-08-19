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
#include <platform/silabs/multi-ota/OTACustomProcessor.h>
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
uint8_t OTACustomProcessor::mSlotId                                                  = 0;
uint32_t OTACustomProcessor::mWriteOffset                                            = 0;
uint16_t OTACustomProcessor::writeBufOffset                                          = 0;
uint8_t OTACustomProcessor::writeBuffer[kAlignmentBytes] __attribute__((aligned(4))) = { 0 };

CHIP_ERROR OTACustomProcessor::Init()
{
    VerifyOrReturnError(mCallbackProcessDescriptor != nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    mAccumulator.Init(sizeof(Descriptor));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTACustomProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mDescriptorProcessed = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTACustomProcessor::ProcessInternal(ByteSpan & block)
{
    if (!mDescriptorProcessed)
    {
        ReturnErrorOnFailure(ProcessDescriptor(block));
    }

    ChipLogError(SoftwareUpdate, "Reached Custom Processor");

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTACustomProcessor::ProcessDescriptor(ByteSpan & block)
{
    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(static_cast<void *>(mAccumulator.data())));

    mDescriptorProcessed = true;
    mAccumulator.Clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTACustomProcessor::ApplyAction()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTACustomProcessor::FinalizeAction()
{
    return CHIP_NO_ERROR;
}

} // namespace chip
