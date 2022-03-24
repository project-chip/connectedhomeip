/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/nrfconnect/OTAImageProcessorImpl.h>

#ifdef CONFIG_CHIP_OTA_REQUESTOR
namespace OTAImageProcessorNrf {
// compile-time factory method
inline chip::DeviceLayer::OTAImageProcessorImpl & Get()
{
#if CONFIG_PM_DEVICE && CONFIG_NORDIC_QSPI_NOR
    static chip::DeviceLayer::ExtFlashHandler sQSPIHandler;
    static chip::DeviceLayer::OTAImageProcessorImplPMDevice sOTAImageProcessor{ sQSPIHandler };
#else
    static chip::DeviceLayer::OTAImageProcessorImpl sOTAImageProcessor;
#endif
    return sOTAImageProcessor;
}
} // namespace OTAImageProcessorNrf

#endif
