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

#include <platform/nxp/k32w/common/OTAImageProcessorImpl.h>
#include <src/include/platform/CHIPDeviceLayer.h>

#include <src/app/clusters/ota-requestor/OTARequestorInterface.h>

#include <platform/nxp/k32w/k32w1/OTAFirmwareProcessor.h>

#include "OtaSupport.h"

#ifndef CONFIG_CHIP_K32W1_MAX_ENTRIES_TEST
#define CONFIG_CHIP_K32W1_MAX_ENTRIES_TEST 0
#endif

#ifndef CONFIG_CHIP_K32W1_OTA_ABORT_HOOK
#define CONFIG_CHIP_K32W1_OTA_ABORT_HOOK 0
#endif

#define APPLICATION_PROCESSOR_TAG 1

extern "C" void HAL_ResetMCU(void);

#define ResetMCU HAL_ResetMCU

#if USE_SMU2_STATIC
// The attribute specifier should not be changed.
static chip::OTAFirmwareProcessor gApplicationProcessor __attribute__((section(".smu2")));
#else
static chip::OTAFirmwareProcessor gApplicationProcessor;
#endif

CHIP_ERROR ProcessDescriptor(void * descriptor)
{
    auto desc = static_cast<chip::OTAFirmwareProcessor::Descriptor *>(descriptor);
    ChipLogDetail(SoftwareUpdate, "Descriptor: %ld, %s, %s", desc->version, desc->versionString, desc->buildDate);

    return CHIP_NO_ERROR;
}

extern "C" WEAK CHIP_ERROR OtaHookInit()
{
#if CONFIG_CHIP_K32W1_MAX_ENTRIES_TEST
    static chip::OTAFirmwareProcessor processors[8];
#endif

    gApplicationProcessor.RegisterDescriptorCallback(ProcessDescriptor);

    auto & imageProcessor = chip::OTAImageProcessorImpl::GetDefaultInstance();
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(APPLICATION_PROCESSOR_TAG, &gApplicationProcessor));

#if CONFIG_CHIP_K32W1_MAX_ENTRIES_TEST
    for (auto i = 0; i < 8; i++)
    {
        processors[i].RegisterDescriptorCallback(ProcessDescriptor);
        ReturnErrorOnFailure(imageProcessor.RegisterProcessor(i + 4, &processors[i]));
    }
#endif // CONFIG_CHIP_K32W1_MAX_ENTRIES_TEST

    return CHIP_NO_ERROR;
}

extern "C" WEAK void OtaHookReset()
{
    // Process all idle saves
    NvShutdown();
    // Set the bootloader flags
    OTA_SetNewImageFlag();
    ResetMCU();
}

extern "C" WEAK void OtaHookAbort()
{
    /*
     Disclaimer: This is not default behavior and it was not checked against
     Matter specification compliance. You should use this at your own discretion.

     Use CONFIG_CHIP_K32W1_OTA_ABORT_HOOK to enable/disable this feature (disabled by default).
     This hook is called inside OTAImageProcessorImpl::HandleAbort to schedule a retry (when enabled).
    */
#if CONFIG_CHIP_K32W1_OTA_ABORT_HOOK
    auto & imageProcessor   = chip::OTAImageProcessorImpl::GetDefaultInstance();
    auto & providerLocation = imageProcessor.GetBackupProvider();

    if (providerLocation.HasValue())
    {
        auto * requestor = chip::GetRequestorInstance();
        requestor->SetCurrentProviderLocation(providerLocation.Value());
        if (requestor->GetCurrentUpdateState() == chip::OTARequestorInterface::OTAUpdateStateEnum::kIdle)
        {
            chip::DeviceLayer::SystemLayer().ScheduleLambda([requestor] { requestor->TriggerImmediateQueryInternal(); });
        }
        else
        {
            ChipLogError(SoftwareUpdate, "OTA requestor not in kIdle");
        }
    }
    else
    {
        ChipLogError(SoftwareUpdate, "Backup provider info not available");
    }
#endif
}
