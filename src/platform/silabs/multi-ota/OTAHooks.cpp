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

#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include <platform/silabs/multi-ota/OTAFactoryDataProcessor.h>

#ifndef SLI_SI91X_MCU_INTERFACE
#include <platform/silabs/multi-ota/OTAFirmwareProcessor.h>
#endif

#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#include <platform/silabs/multi-ota/SiWx917/OTAWiFiFirmwareProcessor.h>
#endif

#if OTA_TEST_CUSTOM_TLVS
#include <platform/silabs/multi-ota/OTACustomProcessor.h>
#endif

CHIP_ERROR chip::OTAMultiImageProcessorImpl::ProcessDescriptor(void * descriptor)
{
    [[maybe_unused]] auto desc = reinterpret_cast<chip::OTATlvProcessor::Descriptor *>(descriptor);
    ChipLogDetail(SoftwareUpdate, "Descriptor: %ld, %s, %s", desc->version, desc->versionString, desc->buildDate);

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::OTAMultiImageProcessorImpl::OtaHookInit()
{
    auto & imageProcessor = chip::OTAMultiImageProcessorImpl::GetDefaultInstance();

    static chip::OTAFactoryDataProcessor sFactoryDataProcessor;
    sFactoryDataProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(OTAProcessorTag::kFactoryDataProcessor, &sFactoryDataProcessor));

#ifdef SLI_SI91X_MCU_INTERFACE
    // 917SOC: TA is the OTA processor for application image
    static chip::OTAWiFiFirmwareProcessor sApplicationProcessor;
#else  // EFR32
    static chip::OTAFirmwareProcessor sApplicationProcessor;
#endif // SLI_SI91X_MCU_INTERFACE
    sApplicationProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(OTAProcessorTag::kApplicationProcessor, &sApplicationProcessor));

#if defined(SL_WIFI) && defined(EXP_BOARD)
    // 917NCP: register OTA processor
    static chip::OTAWiFiFirmwareProcessor sWiFiProcessor;
    sWiFiProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(OTAProcessorTag::kWiFiTAProcessor, &sWiFiProcessor));
#endif // defined(SL_WIFI) && defined(EXP_BOARD)

#if OTA_TEST_CUSTOM_TLVS
    static chip::OTACustomProcessor customProcessor1;
    static chip::OTACustomProcessor customProcessor2;
    static chip::OTACustomProcessor customProcessor3;

    customProcessor1.RegisterDescriptorCallback(ProcessDescriptor);
    customProcessor2.RegisterDescriptorCallback(ProcessDescriptor);
    customProcessor3.RegisterDescriptorCallback(ProcessDescriptor);

    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(OTAProcessorTag::kCustomProcessor1, &customProcessor1));
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(OTAProcessorTag::kCustomProcessor2, &customProcessor2));
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(OTAProcessorTag::kCustomProcessor3, &customProcessor3));
#endif

    return CHIP_NO_ERROR;
}
