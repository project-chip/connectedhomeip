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

#include <platform/silabs/multi-ota/MultiOTAImageProcessorImpl.h>
#include <include/platform/CHIPDeviceLayer.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include <platform/silabs/multi-ota/efr32/OTAFirmwareProcessor.h>

// TODO: Find home for CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR
/** 
 * @def CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR
 *
 * Enables default OTA TLV factory data processor.
 * Disabled by default.
 */
#ifndef CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR
#define CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR 0
#endif // CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR


#if CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR
#include <platform/silabs/multi-ota/efr32/FactoryDataProcessor.h>
#endif // CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR


extern "C" void ResetMCU(void);

CHIP_ERROR ProcessDescriptor(void * descriptor)
{
    auto desc = static_cast<chip::OTAFirmwareProcessor::Descriptor *>(descriptor);
    ChipLogDetail(SoftwareUpdate, "Descriptor: %ld, %s, %s", desc->version, desc->versionString, desc->buildDate);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaHookInit()
{
    static chip::OTAFirmwareProcessor sApplicationProcessor;
#if CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR
    static chip::OTAFactoryDataProcessor sFactoryDataProcessor;
#endif // CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR

    sApplicationProcessor.RegisterDescriptorCallback(ProcessDescriptor);

    auto & imageProcessor = chip::OTAImageProcessorImpl::GetDefaultInstance();
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(1, &sApplicationProcessor));
// TODO: add this back once the FactoryDataProcessor Implementation is complete
//#if CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR
//    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(3, &sFactoryDataProcessor));
//#endif // CONFIG_CHIP_SILABS_OTA_FACTORY_DATA_PROCESSOR

    return CHIP_NO_ERROR;
}
