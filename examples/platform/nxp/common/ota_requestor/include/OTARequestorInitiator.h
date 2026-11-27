/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
 *    Copyright 2023-2025 NXP
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

#include "app/clusters/ota-requestor/BDXDownloader.h"
#if !CONFIG_CHIP_DATA_MODEL_CODEDRIVEN
#include "app/clusters/ota-requestor/CodegenIntegration.h"
#endif
#include "app/clusters/ota-requestor/DefaultOTARequestor.h"
#include "app/clusters/ota-requestor/DefaultOTARequestorDriver.h"
#include "app/clusters/ota-requestor/DefaultOTARequestorStorage.h"

#ifndef CONFIG_APP_FREERTOS_OS
#include <platform/Zephyr/OTAImageProcessorImpl.h>
#else
#include "platform/nxp/common/ota/OTAImageProcessorImpl.h"
#endif /* CONFIG_APP_FREERTOS_OS */

#include <stdint.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace chip {
namespace NXP {
namespace App {
class OTARequestorInitiator
{
public:
    static OTARequestorInitiator & Instance(void)
    {
        static OTARequestorInitiator gOTARequestorInitiator;
        return gOTARequestorInitiator;
    }
    /* Initialize OTA components */
    static void InitOTA(intptr_t context);
    /* Handle update under test */
    static void HandleSelfTest();

#if CONFIG_CHIP_DATA_MODEL_CODEDRIVEN
    /** Binds application-owned objects before InitOTA; both must outlive the requestor. */
    void SetCodeDrivenContext(OTARequestorAttributes & attributes, DefaultOTARequestorEventGenerator & eventGenerator)
    {
        mCodeDrivenAttributes     = &attributes;
        mCodeDrivenEventGenerator = &eventGenerator;
    }
#endif

    /* OTA components */
    DefaultOTARequestor gRequestorCore;
    DefaultOTARequestorStorage gRequestorStorage;
    DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
    BDXDownloader gDownloader;

private:
#if CONFIG_CHIP_DATA_MODEL_CODEDRIVEN
    OTARequestorAttributes * mCodeDrivenAttributes                = nullptr;
    DefaultOTARequestorEventGenerator * mCodeDrivenEventGenerator = nullptr;
#endif
};
} // namespace App
} // namespace NXP
} // namespace chip
