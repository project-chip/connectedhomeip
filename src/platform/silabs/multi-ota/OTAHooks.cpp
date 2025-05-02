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

#include <include/platform/CHIPDeviceLayer.h>
#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include <platform/silabs/multi-ota/OTAFactoryDataProcessor.h>
#ifndef SLI_SI91X_MCU_INTERFACE
#include <platform/silabs/multi-ota/OTAFirmwareProcessor.h>
#endif

#if SL_WIFI
#include <platform/silabs/multi-ota/SiWx917/OTAWiFiFirmwareProcessor.h>
#endif

#if OTA_TEST_CUSTOM_TLVS
#include <platform/silabs/multi-ota/OTACustomProcessor.h>
#endif

CHIP_ERROR chip::OTAMultiImageProcessorImpl::ProcessDescriptor(void * descriptor)
{
#ifndef SLI_SI91X_MCU_INTERFACE
    [[maybe_unused]] auto desc = static_cast<chip::OTAFirmwareProcessor::Descriptor *>(descriptor);
    ChipLogDetail(SoftwareUpdate, "Descriptor: %ld, %s, %s", desc->version, desc->versionString, desc->buildDate);
#endif

#if SL_WIFI
    auto descWiFi = static_cast<chip::OTAWiFiFirmwareProcessor::Descriptor *>(descriptor);
    ChipLogDetail(SoftwareUpdate, "Descriptor: %ld, %s, %s", descWiFi->version, descWiFi->versionString, descWiFi->buildDate);
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::OTAMultiImageProcessorImpl::OtaHookInit()
{
    // static chip::OTAFactoryDataProcessor sFactoryDataProcessor;

    auto & imageProcessor = chip::OTAMultiImageProcessorImpl::GetDefaultInstance();
//    sFactoryDataProcessor.RegisterDescriptorCallback(ProcessDescriptor);
#ifndef SLI_SI91X_MCU_INTERFACE
    static chip::OTAFirmwareProcessor sApplicationProcessor;
    sApplicationProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    ReturnErrorOnFailure(
        imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kApplicationProcessor), &sApplicationProcessor));
#endif
    //   ReturnErrorOnFailure(
    //     imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kFactoryDataProcessor), &sFactoryDataProcessor));

#if OTA_TEST_CUSTOM_TLVS
    static chip::OTACustomProcessor customProcessor1;
    static chip::OTACustomProcessor customProcessor2;
    static chip::OTACustomProcessor customProcessor3;

    customProcessor1.RegisterDescriptorCallback(ProcessDescriptor);
    customProcessor2.RegisterDescriptorCallback(ProcessDescriptor);
    customProcessor3.RegisterDescriptorCallback(ProcessDescriptor);

    ReturnErrorOnFailure(
        imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kCustomProcessor1), &customProcessor1));
    ReturnErrorOnFailure(
        imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kCustomProcessor2), &customProcessor2));
    ReturnErrorOnFailure(
        imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kCustomProcessor3), &customProcessor3));
#endif

#ifdef SL_WIFI
    static chip::OTAWiFiFirmwareProcessor sWifiFirmwareProcessor;
    sWifiFirmwareProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    ReturnErrorOnFailure(
        imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kApplicationProcessor), &sWifiFirmwareProcessor));
#ifndef SLI_SI91X_MCU_INTERFACE
    static chip::OTAWiFiFirmwareProcessor sWifiFirmwareProcessor;
    sWifiFirmwareProcessor.RegisterDescriptorCallback(ProcessDescriptor);
    ReturnErrorOnFailure(
        imageProcessor.RegisterProcessor(static_cast<uint32_t>(OTAProcessorTag::kWiFiProcessor), &sWifiFirmwareProcessor));
#endif // SLI_SI91X_MCU_INTERFACE
#endif // SL_WIFI

    return CHIP_NO_ERROR;
}
