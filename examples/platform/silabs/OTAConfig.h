/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>

#if CHIP_DEVICE_CONFIG_ENABLE_MULTI_OTA_REQUESTOR
#include <platform/silabs/multi-ota/OTAMultiImageProcessorImpl.h>
#else
#include <platform/silabs/OTAImageProcessorImpl.h>
#endif

#if (SL_MATTER_GN_BUILD == 0) && defined(SILABS_OTA_ENABLED)
#include "sl_matter_ota_config.h"
#endif

class OTAConfig
{
public:
    OTAConfig(){};

    static void Init();
    static constexpr uint32_t kInitOTARequestorDelaySec = 3;
};
