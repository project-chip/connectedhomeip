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
#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>
#include <platform/silabs/multi-ota/efr32/OTAFirmwareProcessor.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include "wfx_host_events.h"
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

// TODO: more descriptive error codes
#define SL_OTA_ERROR 1L

namespace chip {

CHIP_ERROR OTAFirmwareProcessor::Init()
{
    ReturnErrorCodeIf(mCallbackProcessDescriptor == nullptr, CHIP_OTA_PROCESSOR_CB_NOT_REGISTERED);
    return OTAUtilityFirmwareProcessor::getInstance()->Init();
}

CHIP_ERROR OTAFirmwareProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    return OTAUtilityFirmwareProcessor::getInstance()->Clear();
}

CHIP_ERROR OTAFirmwareProcessor::ProcessInternal(ByteSpan & block)
{
    return OTAUtilityFirmwareProcessor::getInstance()->ProcessInternal917(block);
}

CHIP_ERROR OTAFirmwareProcessor::ProcessDescriptor(ByteSpan & block)
{
    OTADataAccumulator *mAccumulator = &OTAUtilityFirmwareProcessor::getInstance()->mAccumulator; 
    ReturnErrorOnFailure(mAccumulator->Accumulate(block));
    ReturnErrorOnFailure(mCallbackProcessDescriptor(static_cast<void *>(mAccumulator->data())));
    return OTAUtilityFirmwareProcessor::getInstance()->ProcessDescriptor(block);
}

CHIP_ERROR OTAFirmwareProcessor::ApplyAction()
{
    return OTAUtilityFirmwareProcessor::getInstance()->ApplyAction917();
}

CHIP_ERROR OTAFirmwareProcessor::FinalizeAction()
{   
    return OTAUtilityFirmwareProcessor::getInstance()->FinalizeAction917();
}

} // namespace chip
