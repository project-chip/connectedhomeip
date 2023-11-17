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

#include <platform/nxp/k32w/k32w0/CHIPDevicePlatformConfig.h>
#include <platform/nxp/k32w/k32w0/OTAFirmwareProcessor.h>
#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
#include <platform/nxp/k32w/k32w0/OTAFactoryDataProcessor.h>
#endif // CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR

#include "OtaSupport.h"

#ifndef CONFIG_CHIP_K32W0_MAX_ENTRIES_TEST
#define CONFIG_CHIP_K32W0_MAX_ENTRIES_TEST 0
#endif

#ifndef CONFIG_CHIP_K32W0_OTA_ABORT_HOOK
#define CONFIG_CHIP_K32W0_OTA_ABORT_HOOK 0
#endif

extern "C" void ResetMCU(void);

CHIP_ERROR ProcessDescriptor(void * descriptor)
{
    auto desc = static_cast<chip::OTAFirmwareProcessor::Descriptor *>(descriptor);
    ChipLogDetail(SoftwareUpdate, "Descriptor: %ld, %s, %s", desc->version, desc->versionString, desc->buildDate);

    return CHIP_NO_ERROR;
}

extern "C" WEAK CHIP_ERROR OtaHookInit()
{
    static chip::OTAFirmwareProcessor sApplicationProcessor;
    static chip::OTAFirmwareProcessor sBootloaderProcessor;
#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
    static chip::OTAFactoryDataProcessor sFactoryDataProcessor;
#endif // CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
#if CONFIG_CHIP_K32W0_MAX_ENTRIES_TEST
    static chip::OTAFirmwareProcessor processors[8];
#endif

    sApplicationProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    sBootloaderProcessor.RegisterDescriptorCallback(ProcessDescriptor);

    auto & imageProcessor = chip::OTAImageProcessorImpl::GetDefaultInstance();
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(1, &sApplicationProcessor));
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(2, &sBootloaderProcessor));
#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(3, &sFactoryDataProcessor));
#endif // CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
#if CONFIG_CHIP_K32W0_MAX_ENTRIES_TEST
    for (auto i = 0; i < 8; i++)
    {
        processors[i].RegisterDescriptorCallback(ProcessDescriptor);
        ReturnErrorOnFailure(imageProcessor.RegisterProcessor(i + 4, &processors[i]));
    }
#endif // CONFIG_CHIP_K32W0_MAX_ENTRIES_TEST

    return CHIP_NO_ERROR;
}

extern "C" WEAK void OtaHookReset()
{
    OTA_CommitCustomEntries();
    ResetMCU();
}

extern "C" WEAK void OtaHookAbort()
{
    /*
     Disclaimer: This is not default behavior and it was not checked against
     Matter specification compliance. You should use this at your own discretion.

     Use CONFIG_CHIP_K32W0_OTA_ABORT_HOOK to enable/disable this feature (disabled by default).
     This hook is called inside OTAImageProcessorImpl::HandleAbort to schedule a retry (when enabled).
    */
#if CONFIG_CHIP_K32W0_OTA_ABORT_HOOK
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
