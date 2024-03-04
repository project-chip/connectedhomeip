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
#include <include/platform/CHIPDeviceLayer.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include <platform/silabs/multi-ota/efr32/OTAFirmwareProcessor.h>
#include <platform/silabs/multi-ota/efr32/OTACustomProcessor.h>

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

    auto & imageProcessor = chip::OTAMultiImageProcessorImpl::GetDefaultInstance();
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(1, &sApplicationProcessor));

    return CHIP_NO_ERROR;
}
