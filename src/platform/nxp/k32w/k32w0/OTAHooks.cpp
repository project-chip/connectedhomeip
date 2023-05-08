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

#include <platform/nxp/k32w/k32w0/CHIPDevicePlatformConfig.h>
#include <platform/nxp/k32w/k32w0/OTAApplicationProcessor.h>
#include <platform/nxp/k32w/k32w0/OTABootloaderProcessor.h>
#if CONFIG_CHIP_K32W0_REAL_FACTORY_DATA
#include <platform/nxp/k32w/k32w0/OTAFactoryDataProcessor.h>
#endif // CONFIG_CHIP_K32W0_REAL_FACTORY_DATA

#include "OtaSupport.h"

extern "C" void ResetMCU(void);

static chip::OTAApplicationProcessor sApplicationProcessor;
static chip::OTABootloaderProcessor sBootloaderProcessor;
#if CONFIG_CHIP_K32W0_REAL_FACTORY_DATA
static chip::OTAFactoryDataProcessor sFactoryDataProcessor;
#endif // CONFIG_CHIP_K32W0_REAL_FACTORY_DATA

extern "C" WEAK CHIP_ERROR OtaHookInit()
{
    auto & imageProcessor = chip::OTAImageProcessorImpl::GetDefaultInstance();
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(1, &sApplicationProcessor));
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(2, &sBootloaderProcessor));
#if CONFIG_CHIP_K32W0_REAL_FACTORY_DATA
    ReturnErrorOnFailure(imageProcessor.RegisterProcessor(3, &sFactoryDataProcessor));
#endif // CONFIG_CHIP_K32W0_REAL_FACTORY_DATA

    return CHIP_NO_ERROR;
}

extern "C" WEAK void OtaHookReset()
{
    OTA_SetNewImageFlag();
    ResetMCU();
}
